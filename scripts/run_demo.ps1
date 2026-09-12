Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$Compiler = Join-Path $Root "bin\statelangc.exe"
$Source = Join-Path $Root "samples\demo_temperature.state"
$Tac = Join-Path $Root "build\ir\demo_temperature.tac"
$C = Join-Path $Root "build\c\demo_temperature.c"
$Exe = Join-Path $Root "build\executables\demo_temperature.exe"
$DemoOut = Join-Path $Root "build\demo"
New-Item -ItemType Directory -Force -Path $DemoOut, (Split-Path $Tac), (Split-Path $C), (Split-Path $Exe) | Out-Null

$Transcript = Join-Path $DemoOut "demo_transcript.txt"
"StateLang PR-3 Demo Transcript" | Set-Content -Encoding utf8 $Transcript
"Timestamp: $(Get-Date -Format o)" | Add-Content -Encoding utf8 $Transcript
$CompileCommand = "`"$Compiler`" `"$Source`" --emit-tac `"$Tac`" --emit-c `"$C`" --exe `"$Exe`" --dump-ast --dump-symbols"
"Command: $CompileCommand" | Add-Content -Encoding utf8 $Transcript
cmd /c $CompileCommand | Out-File -FilePath $Transcript -Append -Encoding utf8
if ($LASTEXITCODE -ne 0) {
    throw "Demo compiler command failed with exit code $LASTEXITCODE"
}
"Command: `"$Exe`"" | Add-Content -Encoding utf8 $Transcript
cmd /c "`"$Exe`"" | Out-File -FilePath $Transcript -Append -Encoding utf8
if ($LASTEXITCODE -ne 0) {
    throw "Demo executable failed with exit code $LASTEXITCODE"
}


Write-Host "Demo complete."



