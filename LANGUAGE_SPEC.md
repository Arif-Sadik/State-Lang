# StateLang Language Specification

StateLang is a small state-based, rule-driven language.

## Files

StateLang source files use the `.state` extension.

## Types

Supported value types are inferred from initializers:

- `int`
- `float`
- `string`
- `bool`

Examples:

```state
state count = 3;
state ratio = 20.5;
state status = "ready";
state active = true;
```

## Identifiers and Keywords

Identifier pattern:

```text
[A-Za-z_][A-Za-z0-9_]*
```

Keywords:

```text
state if else while print input execute true false
```

`action` is a predefined string identifier initialized to `"none"`. It is not a keyword.

## Comments

Single-line comments begin with `//`.

## Statements

```state
state x = 10;
x = x + 1;

if (x > 5) {
    action = "high";
}
else {
    action = "low";
}

while (x > 0) {
    x = x - 1;
}

print x;
input x;
execute action;
```

## Operators

Arithmetic:

```text
+ - * /
```

Comparison:

```text
< > <= >= == !=
```

Logical:

```text
&& || !
```

Assignment:

```text
=
```

## Precedence

Highest to lowest:

1. parentheses
2. unary `!` and unary `-`
3. `*` and `/`
4. `+` and `-`
5. `<`, `>`, `<=`, `>=`
6. `==`, `!=`
7. `&&`
8. `||`

## Semantic Rules

- Identifiers must be declared before use, except predefined `action`.
- Duplicate state declarations are invalid.
- Assignment types must be compatible.
- Arithmetic requires numeric operands.
- `if` and `while` conditions must be boolean.
- `input` requires an already declared identifier.
- Only `execute action;` is valid.
- `execute action;` prints a symbolic message and does not execute operating-system commands.

## Current Limitations

StateLang currently has one program-level namespace and does not support functions, arrays, classes, structs, pointers, networking, concurrency, optimization, or an LLVM backend.
