"""Read-only audit of the shipped Windows PE, not an estimate from the asset folder.

Optional developer tool: python -m pip install pefile; python tools/audit_exe.py
"""
from collections import Counter
from hashlib import sha256
from pathlib import Path
import argparse
import re
import pefile


def audit(exe):
    root = Path(__file__).resolve().parents[1]
    sources = {int(i): root / f for i, f in re.findall(
        r'^(\d+)\s+RCDATA\s+"([^"]+)"', (root / "src/resources.rc").read_text(), re.M)}
    binary = exe.read_bytes()
    pe = pefile.PE(data=binary)
    counts, sizes, seen = Counter(), Counter(), set()
    for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if t.id not in (3, 10, 14, 24):
            raise ValueError(f"Unexpected resource type: {t.id or t.name}")
        if t.id != 10:  # Windows icons, icon group or application manifest
            continue
        for name in t.directory.entries:
            for lang in name.directory.entries:
                idx = name.id
                d = lang.data.struct
                b = pe.get_data(d.OffsetToData, d.Size)
                if idx in seen or idx not in sources or b != sources[idx].read_bytes():
                    raise ValueError(f"Unrecognized or mismatched embedded resource: {idx}")
                seen.add(idx)
                if b.startswith(b'\xff\xd8\xff') or b.startswith(b'II\xbc\x01') or b.startswith(b'\x89PNG\r\n\x1a\n'):
                    kind = "still images / sprite atlases"
                elif b[:4] == b'RIFF' and b[8:12] == b'WAVE':
                    kind = "WAV audio"
                else:
                    raise ValueError(f"Unexpected payload format: {idx}")
                counts[kind] += 1
                sizes[kind] += len(b)
    if seen != set(sources) or pe.get_overlay():
        raise ValueError("Missing resource or unexpected PE overlay")
    for p in (root / "docs").glob("*.mp4"):
        if p.read_bytes() in binary:
            raise ValueError(f"Documentation clip found in executable: {p.name}")
    for k in counts:
        print(f"{k}: {counts[k]} entries, {sizes[k]:,} bytes")
    print(f"Other PE data: {len(binary)-sum(sizes.values()):,} bytes; appended overlay: 0 bytes")
    print(f"No embedded video. Total: {len(binary):,} bytes; SHA-256 {sha256(binary).hexdigest()}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("exe", nargs="?", type=Path, default=Path(__file__).resolve().parents[1] / "Riftward.exe")
    audit(parser.parse_args().exe)
