#include "draw.h"
#include "ui.h"
#include "audio.h"
#include <windowsx.h>
#include <shellapi.h>
#include <objbase.h>
#include <imm.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static HWND window;
static int ready, closing;
static RECT oldrect;
static DWORD oldstyle;
static void fullscreen(void) {
    if (!app.fullscreen) {
        GetWindowRect(window, &oldrect);
        oldstyle = (DWORD)GetWindowLongPtrW(window, GWL_STYLE);
        MONITORINFO mi = {0};
        mi.cbSize = sizeof mi;
        GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &mi);
        SetWindowLongPtrW(window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
        app.fullscreen = 1;
    } else {
        SetWindowLongPtrW(window, GWL_STYLE, oldstyle);
        SetWindowPos(window, NULL, oldrect.left, oldrect.top, oldrect.right - oldrect.left,
                     oldrect.bottom - oldrect.top, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
        app.fullscreen = 0;
    }
}
static void mouse_at(LPARAM l) {
    app.mx = (float)GET_X_LPARAM(l);
    app.my = (float)GET_Y_LPARAM(l);
    to_virtual(&app.mx, &app.my);
}
static LRESULT CALLBACK proc(HWND w, UINT m, WPARAM a, LPARAM l) {
    switch (m) {
    case WM_GETMINMAXINFO: {
        MINMAXINFO *mi = (MINMAXINFO *)l;
        mi->ptMinTrackSize.x = 800;
        mi->ptMinTrackSize.y = 480;
        return 0;
    }
    case WM_SIZE:
        if (ready && a != SIZE_MINIMIZED)
            draw_resize(LOWORD(l), HIWORD(l));
        return 0;
    case WM_DPICHANGED: {
        RECT *r = (RECT *)l;
        SetWindowPos(w, NULL, r->left, r->top, r->right - r->left, r->bottom - r->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
    }
    case WM_MOUSEMOVE:
        if (ready)
            mouse_at(l);
        return 0;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (ready) {
            mouse_at(l);
            ui_click(m == WM_RBUTTONDOWN);
        }
        return 0;
    case WM_MOUSEWHEEL:
        ui_wheel(GET_WHEEL_DELTA_WPARAM(a));
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
        if (ready)
            ui_key((int)a, m == WM_KEYDOWN);
        return 0;
    case WM_ACTIVATEAPP:
        if (!a && ready)
            ui_blur();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(w, &ps);
        EndPaint(w, &ps);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(w);
        return 0;
    case WM_DESTROY:
        closing = 1;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(w, m, a, l);
    }
}
static int capture(const wchar_t *dir, const wchar_t *name) {
    wchar_t p[1024];
    if (swprintf(p, 1024, L"%ls\\%ls.bmp", dir, name) < 0)
        return 0;
    ui_draw();
    return draw_end() && draw_save_bmp(p);
}
/* Run the same UI and real Direct2D drawing path, using a WIC backbuffer, no desktop required. */
static int gallery(const wchar_t *dir) {
    CreateDirectoryW(dir, NULL);
    if (!draw_init(NULL, 1280, 720))
        return 2;
    int ok = capture(dir, L"title");
    ui_key(VK_RETURN, 1);
    ui_key(VK_RETURN, 0);
    ok &= app.screen == DRAFT;
    app.selected = 2;
    ok &= capture(dir, L"draft");
    ui_start(151);
    app.g.autoplay = 1;
    /* Select a representative real simulation frame, rather than staging combat. */
    static Game snapshot;
    int best = -1000;
    for (int i = 0; i < 3000; i++) {
        game_step(&app.g, 1.f / 30, (Input){0});
        app.g.soundN = 0;
        if (app.g.time < 17 || !alive(&app.g.u[0]) || i % 6)
            continue;
        int weight = 0;
        V p = app.g.u[0].pos;
        for (int j = 1; j < MAXU; j++) {
            Unit *u = &app.g.u[j];
            if (alive(u) && dist(u->pos, p) < 425)
                weight += u->kind == HERO ? 3 : u->kind == CORE ? -15 : 1;
        }
        for (int j = 0; j < MAXFX; j++)
            if (app.g.fx[j].live && dist(app.g.fx[j].a, p) < 385)
                weight += app.g.fx[j].kind == FX_ZONE ? 10 : 1;
        if (weight > best) {
            best = weight;
            snapshot = app.g;
        }
    }
    if (best > -1000)
        app.g = snapshot;
    app.g.autoplay = 0;
    app.mx = 760;
    app.my = 350;
    app.cam = app.g.u[0].pos;
    ui_wheel(0);
    ui_tick(1.f / 60);
    ok &= capture(dir, L"battle");
    ui_key('P', 1);
    ui_key('P', 0);
    ok &= app.overlay == SHOP;
    ok &= capture(dir, L"shop");
    float t = app.g.time;
    ui_tick(.1f);
    ok &= t == app.g.time;
    ui_key(VK_ESCAPE, 1);
    ui_key(VK_ESCAPE, 0);
    ui_key(VK_TAB, 1);
    ok &= app.overlay == SCORE;
    ok &= capture(dir, L"score");
    ui_key(VK_TAB, 0);
    ok &= app.overlay == NONE;
    ui_key(VK_ESCAPE, 1);
    ui_key(VK_ESCAPE, 0);
    ok &= app.overlay == PAUSED;
    ui_key(VK_F1, 1);
    ui_key(VK_F1, 0);
    ok &= app.overlay == GUIDE;
    ok &= capture(dir, L"guide");
    app.overlay = NONE;
    app.g.autoplay = 1;
    for (int n = 0; n < 24000 && app.g.winner < 0; n++)
        game_step(&app.g, .05f, (Input){0});
    ok &= app.g.winner >= 0;
    ok &= capture(dir, L"result");
    draw_close();
    return ok ? 0 : 3;
}
int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show) {
    (void)prev;
    (void)cmd;
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    ui_reset();
    int ac = 0;
    wchar_t **av = CommandLineToArgvW(GetCommandLineW(), &ac);
    if (ac == 3 && !wcscmp(av[1], L"--gallery")) {
        int result = gallery(av[2]);
        LocalFree(av);
        CoUninitialize();
        return result;
    }
    int smoke = ac == 2 && !wcscmp(av[1], L"--window-test");
    LocalFree(av);
    WNDCLASSW wc = {0};
    wc.hInstance = inst;
    wc.lpfnWndProc = proc;
    wc.lpszClassName = L"RiftwardArenaWindow";
    wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(101));
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClassW(&wc))
        return 1;
    RECT area;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &area, 0);
    float s = fminf(1, fminf((area.right - area.left - 50) / 1280.f, (area.bottom - area.top - 80) / 720.f));
    int cw = (int)(1280 * s), ch = (int)(720 * s);
    RECT r = {0, 0, cw, ch};
    AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0);
    int ww = r.right - r.left, hh = r.bottom - r.top;
    window = CreateWindowExW(0, wc.lpszClassName, L"岚境 RIFTWARD  |  离线 5v5 峡谷", WS_OVERLAPPEDWINDOW,
                             area.left + (area.right - area.left - ww) / 2,
                             area.top + (area.bottom - area.top - hh) / 2, ww, hh, NULL, NULL, inst, NULL);
    if (!window)
        return 2;
    /* This window has no text fields. Keep WASD/QER usable when a Chinese IME is active. */
    ImmAssociateContext(window, NULL);
    GetClientRect(window, &r);
    if (!draw_init(window, r.right, r.bottom)) {
        if (!smoke)
            MessageBoxW(window, L"无法初始化 Direct2D 或读取内嵌美术资源。请检查 Windows 图形环境。", L"岚境",
                        MB_ICONERROR);
        draw_close();
        DestroyWindow(window);
        CoUninitialize();
        return 3;
    }
    ready = 1;
    if (smoke) {
        SendMessageW(window, WM_KEYDOWN, VK_RETURN, 0);
        SendMessageW(window, WM_KEYUP, VK_RETURN, 0);
        int ok = app.screen == DRAFT;
        SendMessageW(window, WM_KEYDOWN, '3', 0);
        SendMessageW(window, WM_KEYUP, '3', 0);
        SendMessageW(window, WM_KEYDOWN, VK_RETURN, 0);
        SendMessageW(window, WM_KEYUP, VK_RETURN, 0);
        ok &= app.screen == MATCH && app.g.u[0].role == 2;
        SendMessageW(window, WM_RBUTTONDOWN, 0, MAKELPARAM(cw / 2, ch / 2));
        ok &= app.g.u[0].order == 1;
        V initial = app.g.u[0].pos;
        SendMessageW(window, WM_KEYDOWN, 'D', 0);
        for (int i = 0; i < 10; i++)
            ui_tick(1.f / 60);
        SendMessageW(window, WM_KEYUP, 'D', 0);
        ok &= dist(initial, app.g.u[0].pos) > 12;
        SendMessageW(window, WM_MOUSEMOVE, 0, MAKELPARAM(cw * 2 / 3, ch / 2));
        SendMessageW(window, WM_KEYDOWN, 'Q', 0);
        SendMessageW(window, WM_KEYUP, 'Q', 0);
        ok &= app.g.castCount[0] == 1;
        ui_draw();
        ok &= draw_end();
        SendMessageW(window, WM_KEYDOWN, VK_ESCAPE, 0);
        ok &= app.overlay == PAUSED;
        ready = 0;
        draw_close();
        DestroyWindow(window);
        CoUninitialize();
        return ok ? 0 : 4;
    }
    ShowWindow(window, show);
    UpdateWindow(window);
    audio_open();
    LARGE_INTEGER fq, now, last;
    QueryPerformanceFrequency(&fq);
    QueryPerformanceCounter(&last);
    double acc = 1. / 60;
    MSG msg;
    while (!closing) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                closing = 1;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (closing)
            break;
        if (app.requestFull) {
            app.requestFull = 0;
            fullscreen();
        }
        QueryPerformanceCounter(&now);
        double delta = (double)(now.QuadPart - last.QuadPart) / fq.QuadPart;
        last = now;
        acc += fmin(.12, delta);
        audio_pump(app.screen != MATCH && GetForegroundWindow() == window);
        if (IsIconic(window)) {
            acc = 0;
            MsgWaitForMultipleObjectsEx(0, NULL, 50, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
            continue;
        }
        if (acc >= 1. / 60) {
            while (acc >= 1. / 60) {
                ui_tick(1.f / 60);
                acc -= 1. / 60;
            }
            ui_draw();
            if (!draw_end()) {
                draw_close();
                GetClientRect(window, &r);
                if (!draw_init(window, r.right, r.bottom)) {
                    MessageBoxW(window, L"图形设备不可用，请重新启动游戏。", L"岚境", MB_ICONERROR);
                    break;
                }
            }
        } else {
            DWORD ms = (DWORD)fmax(1, (1. / 60 - acc) * 1000);
            MsgWaitForMultipleObjectsEx(0, NULL, ms, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        }
    }
    ready = 0;
    audio_close();
    draw_close();
    CoUninitialize();
    return 0;
}
