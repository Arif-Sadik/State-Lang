#include "codegen.h"

#include <stdio.h>
#include <string.h>

static const char *c_operator(OperatorKind op) {
    return operator_name(op);
}

static void c_expr(FILE *out, const Expr *expr) {
    switch (expr->kind) {
        case EXPR_INT:
            fprintf(out, "%d", expr->as.int_value);
            break;
        case EXPR_FLOAT:
            fprintf(out, "%g", expr->as.float_value);
            break;
        case EXPR_STRING:
            fputc('"', out);
            for (const char *p = expr->as.string_value; *p; p++) {
                if (*p == '"' || *p == '\\') {
                    fputc('\\', out);
                }
                if (*p == '\n') {
                    fputs("\\n", out);
                } else if (*p == '\t') {
                    fputs("\\t", out);
                } else {
                    fputc(*p, out);
                }
            }
            fputc('"', out);
            break;
        case EXPR_BOOL:
            fprintf(out, "%d", expr->as.bool_value ? 1 : 0);
            break;
        case EXPR_IDENTIFIER:
            fprintf(out, "%s", expr->as.identifier);
            break;
        case EXPR_UNARY:
            fprintf(out, "(%s", c_operator(expr->as.unary.op));
            c_expr(out, expr->as.unary.expr);
            fputc(')', out);
            break;
        case EXPR_BINARY:
            fputc('(', out);
            c_expr(out, expr->as.binary.left);
            fprintf(out, " %s ", c_operator(expr->as.binary.op));
            c_expr(out, expr->as.binary.right);
            fputc(')', out);
            break;
    }
}

static void indent(FILE *out, int depth) {
    for (int i = 0; i < depth; i++) {
        fputs("    ", out);
    }
}

static void c_stmt_list(FILE *out, const StmtList *list, const SymbolTable *symbols, int depth);

static const Symbol *lookup(const SymbolTable *symbols, const char *name) {
    return symbol_table_find_const(symbols, name);
}

static void c_print(FILE *out, const Expr *expr, int depth) {
    indent(out, depth);
    if (expr->type == TYPE_STRING) {
        fputs("printf(\"%s\\n\", ", out);
        c_expr(out, expr);
        fputs(");\n", out);
    } else if (expr->type == TYPE_FLOAT) {
        fputs("printf(\"%g\\n\", ", out);
        c_expr(out, expr);
        fputs(");\n", out);
    } else if (expr->type == TYPE_BOOL) {
        fputs("printf(\"%s\\n\", (", out);
        c_expr(out, expr);
        fputs(") ? \"true\" : \"false\");\n", out);
    } else {
        fputs("printf(\"%d\\n\", ", out);
        c_expr(out, expr);
        fputs(");\n", out);
    }
}

static void c_assignment(FILE *out, const char *name, const Expr *value, const SymbolTable *symbols, int depth) {
    const Symbol *symbol = lookup(symbols, name);
    indent(out, depth);
    if (symbol && symbol->type == TYPE_STRING) {
        fprintf(out, "strcpy(%s, ", name);
        c_expr(out, value);
        fputs(");\n", out);
    } else {
        fprintf(out, "%s = ", name);
        c_expr(out, value);
        fputs(";\n", out);
    }
}

static void c_stmt(FILE *out, const Stmt *stmt, const SymbolTable *symbols, int depth) {
    switch (stmt->kind) {
        case STMT_STATE_DECL:
            c_assignment(out, stmt->as.state_decl.name, stmt->as.state_decl.initializer, symbols, depth);
            break;
        case STMT_ASSIGN:
            c_assignment(out, stmt->as.assignment.name, stmt->as.assignment.value, symbols, depth);
            break;
        case STMT_IF:
            indent(out, depth);
            fputs("if (", out);
            c_expr(out, stmt->as.if_stmt.condition);
            fputs(") {\n", out);
            c_stmt_list(out, stmt->as.if_stmt.then_branch, symbols, depth + 1);
            indent(out, depth);
            fputs("}", out);
            if (stmt->as.if_stmt.else_branch) {
                fputs(" else {\n", out);
                c_stmt_list(out, stmt->as.if_stmt.else_branch, symbols, depth + 1);
                indent(out, depth);
                fputs("}", out);
            }
            fputc('\n', out);
            break;
        case STMT_WHILE:
            indent(out, depth);
            fputs("while (", out);
            c_expr(out, stmt->as.while_stmt.condition);
            fputs(") {\n", out);
            c_stmt_list(out, stmt->as.while_stmt.body, symbols, depth + 1);
            indent(out, depth);
            fputs("}\n", out);
            break;
        case STMT_PRINT:
            c_print(out, stmt->as.print_stmt.value, depth);
            break;
        case STMT_INPUT: {
            const Symbol *symbol = lookup(symbols, stmt->as.input_stmt.name);
            indent(out, depth);
            if (symbol && symbol->type == TYPE_STRING) {
                fprintf(out, "scanf(\"%%255s\", %s);\n", stmt->as.input_stmt.name);
            } else if (symbol && symbol->type == TYPE_FLOAT) {
                fprintf(out, "scanf(\"%%lf\", &%s);\n", stmt->as.input_stmt.name);
            } else {
                fprintf(out, "scanf(\"%%d\", &%s);\n", stmt->as.input_stmt.name);
            }
            break;
        }
        case STMT_EXECUTE:
            indent(out, depth);
            fputs("printf(\"Executing action: %s\\n\", action);\n", out);
            break;
    }
}

static void c_stmt_list(FILE *out, const StmtList *list, const SymbolTable *symbols, int depth) {
    for (const StmtList *node = list; node; node = node->next) {
        c_stmt(out, node->stmt, symbols, depth);
    }
}

int codegen_generate_c_file(const char *path, StmtList *program, const SymbolTable *symbols) {
    FILE *out = fopen(path, "w");
    if (!out) {
        fprintf(stderr, "Code Generation Error: could not write C file %s\n", path);
        return 0;
    }
    fputs("#include <stdio.h>\n#include <string.h>\n\nint main(void) {\n", out);
    for (int i = 0; i < symbols->count; i++) {
        const Symbol *symbol = &symbols->items[i];
        indent(out, 1);
        if (symbol->type == TYPE_STRING) {
            fprintf(out, "char %s[256] = \"", symbol->name);
            if (strcmp(symbol->name, "action") == 0) {
                fputs("none", out);
            }
            fputs("\";\n", out);
        } else if (symbol->type == TYPE_FLOAT) {
            fprintf(out, "double %s = 0.0;\n", symbol->name);
        } else {
            fprintf(out, "int %s = 0;\n", symbol->name);
        }
    }
    fputc('\n', out);
    c_stmt_list(out, program, symbols, 1);
    fputs("    return 0;\n}\n", out);
    fclose(out);
    return 1;
}
