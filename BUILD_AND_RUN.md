# Build and Run

Open PowerShell in the repository root.

## Build the Compiler

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

Output:

```text
bin\statelangc.exe
```

## Run the Demo

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_demo.ps1
```

Expected executable output:

```text
Executing action: cool
```

## Run Tests

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_tests.ps1
```

Expected summary:

```text
Tests complete: total=12 passed=12 failed=0
```

## Direct Compiler Use

```powershell
.\bin\statelangc.exe .\samples\demo_temperature.state --emit-tac .\build\ir\demo_temperature.tac --emit-c .\build\c\demo_temperature.c --exe .\build\executables\demo_temperature.exe --dump-ast --dump-symbols
.\build\executables\demo_temperature.exe
```
