Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$Src = Join-Path $Root "src"
$Generated = Join-Path $Root "build\generated"
$Bin = Join-Path $Root "bin"
$LogDir = Join-Path $Root "build\logs"


New-Item -ItemType Directory -Force -Path $Generated, $Bin, $LogDir | Out-Null

$Bison = (Get-Command bison -ErrorAction SilentlyContinue)
if (-not $Bison) {
    $Bison = (Get-Command win_bison -ErrorAction SilentlyContinue)
}
if (-not $Bison) {
    throw "Bison/win_bison was not found."
}

$Flex = (Get-Command flex -ErrorAction SilentlyContinue)
if (-not $Flex) {
    $Flex = (Get-Command win_flex -ErrorAction SilentlyContinue)
}
if (-not $Flex) {
    throw "Flex/win_flex was not found."
}

$Gcc = (Get-Command gcc -ErrorAction SilentlyContinue)
if (-not $Gcc) {
    throw "gcc was not found."
}

$VersionFile = Join-Path $LogDir "toolchain_versions.txt"
"StateLang PR-3 Toolchain Versions" | Set-Content $VersionFile
"Timestamp: $(Get-Date -Format o)" | Add-Content $VersionFile
"Windows:" | Add-Content $VersionFile
(Get-ComputerInfo | Select-Object -Property WindowsProductName, WindowsVersion, OsArchitecture | Format-List | Out-String) | Add-Content $VersionFile
"flex path: $($Flex.Source)" | Add-Content $VersionFile
(& $Flex.Source --version 2>&1 | Out-String) | Add-Content $VersionFile
"bison path: $($Bison.Source)" | Add-Content $VersionFile
(& $Bison.Source --version 2>&1 | Out-String) | Add-Content $VersionFile
"gcc path: $($Gcc.Source)" | Add-Content $VersionFile
(& $Gcc.Source --version 2>&1 | Out-String) | Add-Content $VersionFile

$BisonLog = Join-Path $LogDir "bison_output.txt"
$BuildLog = Join-Path $LogDir "compiler_build.txt"
$ParserC = Join-Path $Generated "statelang.tab.c"
$LexerC = Join-Path $Generated "lex.yy.c"
$Exe = Join-Path $Bin "statelangc.exe"

Push-Location $Generated
try {
    & $Bison.Source -Wall -Wcounterexamples -d -v -o $ParserC (Join-Path $Src "statelang.y") *> $BisonLog
    & $Flex.Source -o $LexerC (Join-Path $Src "statelang.l") *>> $BisonLog
} finally {
    Pop-Location
}

$Sources = @(
    $ParserC,
    $LexerC,
    (Join-Path $Src "main.c"),
    (Join-Path $Src "ast.c"),
    (Join-Path $Src "symbol_table.c"),
    (Join-Path $Src "semantic.c"),
    (Join-Path $Src "ir.c"),
    (Join-Path $Src "codegen.c")
)

& $Gcc.Source -std=c11 -Wall -Wextra -Wno-unused-function -I $Src -I $Generated @Sources -o $Exe *> $BuildLog

if (-not (Test-Path $Exe)) {
    throw "Build did not produce $Exe"
}

Write-Host "Build succeeded: $Exe"


