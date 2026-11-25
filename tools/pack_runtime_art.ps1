# Optional art-authoring step. Requires only Windows PowerShell / WPF.
# Never write the originals; build.bat embeds the ready-to-use outputs only.
param([string[]]$Only=@())
$ErrorActionPreference='Stop'
Add-Type -AssemblyName PresentationCore
$assets=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\assets'))
$output=[IO.Directory]::CreateDirectory((Join-Path $assets 'runtime-art'))
foreach($name in ('units','motion-0','motion-1','motion-2','motion-3','motion-4','ultimate',
                  'portraits-silk','portraits-iron','title-landscape','title-swordsman','domains',
                  'domain-props','result-crests','roster-busts')){
    if($Only.Count -and $name -notin $Only){continue}
    $src=[IO.File]::OpenRead((Join-Path $assets "$name.png"))
    try{
        $im=[Windows.Media.Imaging.BitmapDecoder]::Create($src,'PreservePixelFormat','OnLoad').Frames[0]
        $pix=[Windows.Media.Imaging.FormatConvertedBitmap]::new($im,[Windows.Media.PixelFormats]::Bgra32,$null,0)
        $enc=[Windows.Media.Imaging.WmpBitmapEncoder]::new()
        $enc.ImageQualityLevel=0.93
        $enc.InterleavedAlpha=$false
        $enc.AlphaQualityLevel=1
        $enc.Frames.Add([Windows.Media.Imaging.BitmapFrame]::Create($pix))
        $path=Join-Path $output.FullName "$name.jxr"
        $out=[IO.File]::Create($path)
        try{$enc.Save($out)}finally{$out.Dispose()}
        Write-Output ("{0,-20} {1,10} -> {2,10}" -f $name,$src.Length,(Get-Item -LiteralPath $path).Length)
    }finally{$src.Dispose()}
}
