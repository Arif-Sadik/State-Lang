Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$Compiler = Join-Path $Root "bin\statelangc.exe"
$LogDir = Join-Path $Root "build\tests\logs"
$OutRoot = Join-Path $Root "build\tests"
New-Item -ItemType Directory -Force -Path $LogDir, $OutRoot | Out-Null

$Results = @()
$Cases = @(
    @{Category="valid"; Expected=0; Path=(Join-Path $Root "tests\valid")},
    @{Category="lexical_errors"; Expected=2; Path=(Join-Path $Root "tests\lexical_errors")},
    @{Category="syntax_errors"; Expected=2; Path=(Join-Path $Root "tests\syntax_errors")},
    @{Category="semantic_errors"; Expected=3; Path=(Join-Path $Root "tests\semantic_errors")}
)

foreach ($Group in $Cases) {
    Get-ChildItem -Path $Group.Path -Filter "*.state" | Sort-Object Name | ForEach-Object {
        $Name = [IO.Path]::GetFileNameWithoutExtension($_.Name)
        $Dir = Join-Path $OutRoot $Group.Category
        New-Item -ItemType Directory -Force -Path $Dir | Out-Null
        $Tac = Join-Path $Dir "$Name.tac"
        $C = Join-Path $Dir "$Name.c"
        $Exe = Join-Path $Dir "$Name.exe"
        $Log = Join-Path $LogDir "$($Group.Category)_$Name.log"
        $Command = "`"$Compiler`" `"$($_.FullName)`" --emit-tac `"$Tac`" --emit-c `"$C`" --exe `"$Exe`""
        cmd /s /c "$Command > `"$Log`" 2>&1"
        $Exit = $LASTEXITCODE
        $Passed = $false
        if ($Group.Category -eq "valid") {
            if ($Exit -eq 0 -and (Test-Path $Exe)) {
                cmd /s /c "`"$Exe`" >> `"$Log`" 2>&1"
                $Passed = ($LASTEXITCODE -eq 0)
            }
        } else {
            $Passed = ($Exit -eq $Group.Expected)
        }
        $Results += [PSCustomObject]@{
            category=$Group.Category
            file=$_.Name
            expected_exit=$Group.Expected
            actual_exit=$Exit
            passed=$Passed
            log=$Log
        }
    }
}

$Csv = Join-Path $OutRoot "test_results.csv"
$Results | Export-Csv -NoTypeInformation -Path $Csv
$Total = $Results.Count
$PassedCount = @($Results | Where-Object passed).Count
$FailedCount = $Total - $PassedCount
$Summary = Join-Path $OutRoot "test_summary.txt"
"Total: $Total" | Set-Content $Summary
"Passed: $PassedCount" | Add-Content $Summary
"Failed: $FailedCount" | Add-Content $Summary
($Results | Group-Object category | ForEach-Object { "$($_.Name): total=$($_.Count), passed=$(@($_.Group | Where-Object passed).Count)" }) | Add-Content $Summary

Write-Host "Tests complete: total=$Total passed=$PassedCount failed=$FailedCount"


