Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$Generated = Join-Path $Root "build\generated"
$Ir = Join-Path $Root "build\ir"
$C = Join-Path $Root "build\c"
$Executables = Join-Path $Root "build\executables"
$Bin = Join-Path $Root "bin"

foreach ($Path in @($Generated, $Ir, $C, $Executables, $Bin)) {
    if (Test-Path $Path) {
        Get-ChildItem -LiteralPath $Path -Force | Remove-Item -Recurse -Force
    } else {
        New-Item -ItemType Directory -Force -Path $Path | Out-Null
    }
}

Write-Host "Clean complete."
