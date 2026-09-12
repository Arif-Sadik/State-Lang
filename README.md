# StateLang

StateLang is a small state-based, rule-driven programming language and compiler. It is designed to make a complete compiler pipeline easy to inspect: source code is tokenized, parsed, checked, lowered to three-address code, translated to C, and compiled into a Windows executable.

## Compiler Pipeline

`.state` source -> Flex lexer -> tokens -> Bison parser -> AST -> semantic analysis + symbol table -> three-address code -> generated C -> GCC -> Windows `.exe`

## Target Platform

StateLang currently targets Windows and uses PowerShell scripts for the build/demo/test workflow.

## Requirements

Install or make available on `PATH`:

- Flex or `win_flex`
- Bison or `win_bison`
- GCC
- PowerShell

The tested Windows toolchain used Flex 2.6.4, GNU Bison 3.8.2 through `win_bison`, and GCC 15.1.0.

## Build

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

This creates:

- `bin\statelangc.exe`
- generated Flex/Bison C files under `build\generated\`

## Run the Demo

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_demo.ps1
```

Expected runtime output from the generated executable:

```text
Executing action: cool
```

The demo generates:

- `build\ir\demo_temperature.tac`
- `build\c\demo_temperature.c`
- `build\executables\demo_temperature.exe`
- `build\demo\demo_transcript.txt`

## Run Tests

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_tests.ps1
```

Current verified result:

```text
Total: 12
Passed: 12
Failed: 0
```

Test outputs are written under `build\tests\`.

## Example StateLang Program

```state
state temperature = 45;

if (temperature > 40) {
    action = "cool";
}
else {
    action = "continue";
}

execute action;
```

## Language Notes

- Supported inferred types: `int`, `float`, `string`, `bool`
- `state x = expression;` declares a state
- `action` is a predefined string variable, not a keyword
- `execute action;` is symbolic and prints the selected action; it does not run operating-system commands
- Control flow supports `if`, `else if`, `else`, and `while`
- I/O supports `print` and `input`

## Limitations

StateLang version 1 intentionally keeps the language small:

- one program-level namespace
- no functions
- no arrays
- no classes or structs
- no pointers exposed to StateLang
- no optimizer
- no LLVM backend
- no networking or concurrency
