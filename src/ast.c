#include "ast.h"

#include <stdlib.h>
#include <string.h>

static char *copy_text(const char *text) {
    if (!text) {
        return NULL;
    }
    size_t len = strlen(text);
    char *copy = (char *)malloc(len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, text, len + 1);
    return copy;
}

static Expr *new_expr(ExprKind kind, int line) {
    Expr *expr = (Expr *)calloc(1, sizeof(Expr));
    if (!expr) {
        return NULL;
    }
    expr->kind = kind;
    expr->type = TYPE_UNKNOWN;
    expr->line = line;
    return expr;
}

static Stmt *new_stmt(StmtKind kind, int line) {
    Stmt *stmt = (Stmt *)calloc(1, sizeof(Stmt));
    if (!stmt) {
        return NULL;
    }
    stmt->kind = kind;
    stmt->line = line;
    return stmt;
}

Expr *ast_expr_int(int value, int line) {
    Expr *expr = new_expr(EXPR_INT, line);
    expr->as.int_value = value;
    return expr;
}

Expr *ast_expr_float(double value, int line) {
    Expr *expr = new_expr(EXPR_FLOAT, line);
    expr->as.float_value = value;
    return expr;
}

Expr *ast_expr_string(const char *value, int line) {
    Expr *expr = new_expr(EXPR_STRING, line);
    expr->as.string_value = copy_text(value);
    return expr;
}

Expr *ast_expr_bool(int value, int line) {
    Expr *expr = new_expr(EXPR_BOOL, line);
    expr->as.bool_value = value ? 1 : 0;
    return expr;
}

Expr *ast_expr_identifier(const char *name, int line) {
    Expr *expr = new_expr(EXPR_IDENTIFIER, line);
    expr->as.identifier = copy_text(name);
    return expr;
}

Expr *ast_expr_unary(OperatorKind op, Expr *inner, int line) {
    Expr *expr = new_expr(EXPR_UNARY, line);
    expr->as.unary.op = op;
    expr->as.unary.expr = inner;
    return expr;
}

Expr *ast_expr_binary(OperatorKind op, Expr *left, Expr *right, int line) {
    Expr *expr = new_expr(EXPR_BINARY, line);
    expr->as.binary.op = op;
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}

Stmt *ast_stmt_state_decl(const char *name, Expr *initializer, int line) {
    Stmt *stmt = new_stmt(STMT_STATE_DECL, line);
    stmt->as.state_decl.name = copy_text(name);
    stmt->as.state_decl.initializer = initializer;
    return stmt;
}

Stmt *ast_stmt_assignment(const char *name, Expr *value, int line) {
    Stmt *stmt = new_stmt(STMT_ASSIGN, line);
    stmt->as.assignment.name = copy_text(name);
    stmt->as.assignment.value = value;
    return stmt;
}

Stmt *ast_stmt_if(Expr *condition, StmtList *then_branch, StmtList *else_branch, int line) {
    Stmt *stmt = new_stmt(STMT_IF, line);
    stmt->as.if_stmt.condition = condition;
    stmt->as.if_stmt.then_branch = then_branch;
    stmt->as.if_stmt.else_branch = else_branch;
    return stmt;
}

Stmt *ast_stmt_while(Expr *condition, StmtList *body, int line) {
    Stmt *stmt = new_stmt(STMT_WHILE, line);
    stmt->as.while_stmt.condition = condition;
    stmt->as.while_stmt.body = body;
    return stmt;
}

Stmt *ast_stmt_print(Expr *value, int line) {
    Stmt *stmt = new_stmt(STMT_PRINT, line);
    stmt->as.print_stmt.value = value;
    return stmt;
}

Stmt *ast_stmt_input(const char *name, int line) {
    Stmt *stmt = new_stmt(STMT_INPUT, line);
    stmt->as.input_stmt.name = copy_text(name);
    return stmt;
}

Stmt *ast_stmt_execute(const char *name, int line) {
    Stmt *stmt = new_stmt(STMT_EXECUTE, line);
    stmt->as.execute_stmt.name = copy_text(name);
    return stmt;
}

StmtList *ast_stmt_list_append(StmtList *list, Stmt *stmt) {
    StmtList *node = (StmtList *)calloc(1, sizeof(StmtList));
    node->stmt = stmt;
    if (!list) {
        return node;
    }
    StmtList *tail = list;
    while (tail->next) {
        tail = tail->next;
    }
    tail->next = node;
    return list;
}

void ast_free_expr(Expr *expr) {
    if (!expr) {
        return;
    }
    switch (expr->kind) {
        case EXPR_STRING:
            free(expr->as.string_value);
            break;
        case EXPR_IDENTIFIER:
            free(expr->as.identifier);
            break;
        case EXPR_UNARY:
            ast_free_expr(expr->as.unary.expr);
            break;
        case EXPR_BINARY:
            ast_free_expr(expr->as.binary.left);
            ast_free_expr(expr->as.binary.right);
            break;
        default:
            break;
    }
    free(expr);
}

void ast_free_stmt(Stmt *stmt) {
    if (!stmt) {
        return;
    }
    switch (stmt->kind) {
        case STMT_STATE_DECL:
            free(stmt->as.state_decl.name);
            ast_free_expr(stmt->as.state_decl.initializer);
            break;
        case STMT_ASSIGN:
            free(stmt->as.assignment.name);
            ast_free_expr(stmt->as.assignment.value);
            break;
        case STMT_IF:
            ast_free_expr(stmt->as.if_stmt.condition);
            ast_free_stmt_list(stmt->as.if_stmt.then_branch);
            ast_free_stmt_list(stmt->as.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            ast_free_expr(stmt->as.while_stmt.condition);
            ast_free_stmt_list(stmt->as.while_stmt.body);
            break;
        case STMT_PRINT:
            ast_free_expr(stmt->as.print_stmt.value);
            break;
        case STMT_INPUT:
            free(stmt->as.input_stmt.name);
            break;
        case STMT_EXECUTE:
            free(stmt->as.execute_stmt.name);
            break;
    }
    free(stmt);
}

void ast_free_stmt_list(StmtList *list) {
    while (list) {
        StmtList *next = list->next;
        ast_free_stmt(list->stmt);
        free(list);
        list = next;
    }
}

const char *type_name(ValueType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "bool";
        case TYPE_ERROR: return "error";
        default: return "unknown";
    }
}

const char *operator_name(OperatorKind op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LE: return "<=";
        case OP_GE: return ">=";
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        case OP_NOT: return "!";
        case OP_NEG: return "-";
        default: return "?";
    }
}

static void print_indent(FILE *out, int indent) {
    for (int i = 0; i < indent; i++) {
        fputs("  ", out);
    }
}

static void print_expr(FILE *out, const Expr *expr, int indent) {
    if (!expr) {
        print_indent(out, indent);
        fputs("<null expr>\n", out);
        return;
    }
    print_indent(out, indent);
    switch (expr->kind) {
        case EXPR_INT:
            fprintf(out, "Int(%d) : %s\n", expr->as.int_value, type_name(expr->type));
            break;
        case EXPR_FLOAT:
            fprintf(out, "Float(%g) : %s\n", expr->as.float_value, type_name(expr->type));
            break;
        case EXPR_STRING:
            fprintf(out, "String(%s) : %s\n", expr->as.string_value, type_name(expr->type));
            break;
        case EXPR_BOOL:
            fprintf(out, "Bool(%s) : %s\n", expr->as.bool_value ? "true" : "false", type_name(expr->type));
            break;
        case EXPR_IDENTIFIER:
            fprintf(out, "Identifier(%s) : %s\n", expr->as.identifier, type_name(expr->type));
            break;
        case EXPR_UNARY:
            fprintf(out, "Unary(%s) : %s\n", operator_name(expr->as.unary.op), type_name(expr->type));
            print_expr(out, expr->as.unary.expr, indent + 1);
            break;
        case EXPR_BINARY:
            fprintf(out, "Binary(%s) : %s\n", operator_name(expr->as.binary.op), type_name(expr->type));
            print_expr(out, expr->as.binary.left, indent + 1);
            print_expr(out, expr->as.binary.right, indent + 1);
            break;
    }
}

static void print_stmt(FILE *out, const Stmt *stmt, int indent) {
    if (!stmt) {
        return;
    }
    print_indent(out, indent);
    switch (stmt->kind) {
        case STMT_STATE_DECL:
            fprintf(out, "StateDecl(%s)\n", stmt->as.state_decl.name);
            print_expr(out, stmt->as.state_decl.initializer, indent + 1);
            break;
        case STMT_ASSIGN:
            fprintf(out, "Assignment(%s)\n", stmt->as.assignment.name);
            print_expr(out, stmt->as.assignment.value, indent + 1);
            break;
        case STMT_IF:
            fputs("If\n", out);
            print_indent(out, indent + 1);
            fputs("Condition\n", out);
            print_expr(out, stmt->as.if_stmt.condition, indent + 2);
            print_indent(out, indent + 1);
            fputs("Then\n", out);
            for (const StmtList *node = stmt->as.if_stmt.then_branch; node; node = node->next) {
                print_stmt(out, node->stmt, indent + 2);
            }
            if (stmt->as.if_stmt.else_branch) {
                print_indent(out, indent + 1);
                fputs("Else\n", out);
                for (const StmtList *node = stmt->as.if_stmt.else_branch; node; node = node->next) {
                    print_stmt(out, node->stmt, indent + 2);
                }
            }
            break;
        case STMT_WHILE:
            fputs("While\n", out);
            print_expr(out, stmt->as.while_stmt.condition, indent + 1);
            for (const StmtList *node = stmt->as.while_stmt.body; node; node = node->next) {
                print_stmt(out, node->stmt, indent + 1);
            }
            break;
        case STMT_PRINT:
            fputs("Print\n", out);
            print_expr(out, stmt->as.print_stmt.value, indent + 1);
            break;
        case STMT_INPUT:
            fprintf(out, "Input(%s)\n", stmt->as.input_stmt.name);
            break;
        case STMT_EXECUTE:
            fprintf(out, "Execute(%s)\n", stmt->as.execute_stmt.name);
            break;
    }
}

void ast_print_stmt_list(FILE *out, const StmtList *list) {
    fputs("Program\n", out);
    for (const StmtList *node = list; node; node = node->next) {
        print_stmt(out, node->stmt, 1);
    }
}
