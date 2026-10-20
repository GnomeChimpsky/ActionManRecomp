# Completes overlay_toolchain\ beside ActionManRecomp.exe with the two pinned
# third-party pieces the release zip could not carry: the embeddable CPython
# 3.13.1 and TinyCC 0.9.27 (win64). Both are verified against the SHA-256
# pins in psxrecomp/tools/release_stage.py before anything is extracted.
#
# With them in place the runtime turns level code it captures while you play
# into native code (cache\...), so overlays stop running on the interpreter.
# Without them the game still runs correctly, only that code stays interpreted.
#
# Usage (from the folder that holds ActionManRecomp.exe):
#   powershell -ExecutionPolicy Bypass -File get_overlay_toolchain.ps1
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$tk   = Join-Path $root 'overlay_toolchain'
if (-not (Test-Path (Join-Path $tk 'psxrecomp-game.exe'))) {
    throw "overlay_toolchain\psxrecomp-game.exe not found next to this script; run it from the release folder."
}
$pins = @(
    @{ Name = 'python'; Url = 'https://www.python.org/ftp/python/3.13.1/python-3.13.1-embed-amd64.zip';
       Sha = '7b7923ff0183a8b8fca90f6047184b419b108cb437f75fc1c002f9d2f8bcec16' },
    @{ Name = 'tcc';    Url = 'https://download.savannah.gnu.org/releases/tinycc/tcc-0.9.27-win64-bin.zip';
       Sha = '34a721949a2583fdff725312da092fa0f5f1f284b702e6f811c6954714faabb2' }
)
$tmp = Join-Path $env:TEMP 'action-man-recomp-toolchain'
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
foreach ($p in $pins) {
    $zip = Join-Path $tmp ([IO.Path]::GetFileName($p.Url))
    if (-not (Test-Path $zip) -or ((Get-FileHash $zip -Algorithm SHA256).Hash.ToLower() -ne $p.Sha)) {
        Write-Host "Downloading $($p.Url)"
        Invoke-WebRequest -Uri $p.Url -OutFile $zip -UseBasicParsing
    }
    $have = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
    if ($have -ne $p.Sha) { Remove-Item $zip; throw "SHA-256 mismatch for $($p.Url): $have" }
    $dest = Join-Path $tk $p.Name
    if (Test-Path $dest) { Remove-Item -Recurse -Force $dest }
    if ($p.Name -eq 'tcc') {
        $x = Join-Path $tmp 'tcc_extract'
        if (Test-Path $x) { Remove-Item -Recurse -Force $x }
        Expand-Archive -Path $zip -DestinationPath $x
        $inner = Join-Path $x 'tcc'
        if (-not (Test-Path $inner)) { $inner = $x }
        Copy-Item -Recurse $inner $dest
        $notice = Join-Path $root 'licenses\TinyCC-LICENSE.txt'
        if (Test-Path $notice) { Copy-Item $notice (Join-Path $dest 'COPYING') }
    } else {
        Expand-Archive -Path $zip -DestinationPath $dest
    }
    Write-Host "Installed $($p.Name) -> $dest"
}
if (-not (Test-Path (Join-Path $tk 'python\python.exe'))) { throw 'python\python.exe missing after extraction' }
Write-Host 'Overlay toolchain complete. Start ActionManRecomp.exe as usual.'
