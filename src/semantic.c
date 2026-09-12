#include "semantic.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static void semantic_error(SemanticContext *ctx, int line, const char *message, const char *name) {
    if (name) {
        fprintf(stderr, "Semantic Error [line %d]: %s '%s'\n", line, message, name);
    } else {
        fprintf(stderr, "Semantic Error [line %d]: %s\n", line, message);
    }
    ctx->error_count++;
}

void semantic_init(SemanticContext *ctx) {
    symbol_table_init(&ctx->symbols);
    symbol_table_add_predefined(&ctx->symbols);
    ctx->error_count = 0;
}

void semantic_free(SemanticContext *ctx) {
    symbol_table_free(&ctx->symbols);
}

static int is_numeric(ValueType type) {
    return type == TYPE_INT || type == TYPE_FLOAT;
}

static int types_compatible(ValueType target, ValueType value) {
    if (target == value) {
        return 1;
    }
    if (target == TYPE_FLOAT && value == TYPE_INT) {
        return 1;
    }
    return 0;
}

static int literal_zero(const Expr *expr) {
    if (!expr) {
        return 0;
    }
    if (expr->kind == EXPR_INT) {
        return expr->as.int_value == 0;
    }
    if (expr->kind == EXPR_FLOAT) {
        return fabs(expr->as.float_value) < 0.0000001;
    }
    return 0;
}

static ValueType analyze_expr(SemanticContext *ctx, Expr *expr) {
    if (!expr) {
        return TYPE_ERROR;
    }
    switch (expr->kind) {
        case EXPR_INT:
            expr->type = TYPE_INT;
            return expr->type;
        case EXPR_FLOAT:
            expr->type = TYPE_FLOAT;
            return expr->type;
        case EXPR_STRING:
            expr->type = TYPE_STRING;
            return expr->type;
        case EXPR_BOOL:
            expr->type = TYPE_BOOL;
            return expr->type;
        case EXPR_IDENTIFIER: {
            Symbol *symbol = symbol_table_find(&ctx->symbols, expr->as.identifier);
            if (!symbol) {
                semantic_error(ctx, expr->line, "Undeclared identifier", expr->as.identifier);
                expr->type = TYPE_ERROR;
                return TYPE_ERROR;
            }
            expr->type = symbol->type;
            return expr->type;
        }
        case EXPR_UNARY: {
            ValueType inner = analyze_expr(ctx, expr->as.unary.expr);
            if (expr->as.unary.op == OP_NOT) {
                if (inner != TYPE_BOOL) {
                    semantic_error(ctx, expr->line, "Logical NOT requires a bool operand", NULL);
                    expr->type = TYPE_ERROR;
                } else {
                    expr->type = TYPE_BOOL;
                }
            } else {
                if (!is_numeric(inner)) {
                    semantic_error(ctx, expr->line, "Unary minus requires a numeric operand", NULL);
                    expr->type = TYPE_ERROR;
                } else {
                    expr->type = inner;
                }
            }
            return expr->type;
        }
        case EXPR_BINARY: {
            ValueType left = analyze_expr(ctx, expr->as.binary.left);
            ValueType right = analyze_expr(ctx, expr->as.binary.right);
            OperatorKind op = expr->as.binary.op;
            if (left == TYPE_ERROR || right == TYPE_ERROR) {
                expr->type = TYPE_ERROR;
                return TYPE_ERROR;
            }
            if (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV) {
                if (!is_numeric(left) || !is_numeric(right)) {
                    semantic_error(ctx, expr->line, "Arithmetic requires numeric operands", NULL);
                    expr->type = TYPE_ERROR;
                } else if (op == OP_DIV && literal_zero(expr->as.binary.right)) {
                    semantic_error(ctx, expr->line, "Division by literal zero", NULL);
                    expr->type = TYPE_ERROR;
                } else {
                    expr->type = (left == TYPE_FLOAT || right == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                }
            } else if (op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE) {
                if (!is_numeric(left) || !is_numeric(right)) {
                    semantic_error(ctx, expr->line, "Relational comparison requires numeric operands", NULL);
                    expr->type = TYPE_ERROR;
                } else {
                    expr->type = TYPE_BOOL;
                }
            } else if (op == OP_EQ || op == OP_NE) {
                if (!types_compatible(left, right) && !types_compatible(right, left)) {
                    semantic_error(ctx, expr->line, "Equality comparison requires compatible operands", NULL);
                    expr->type = TYPE_ERROR;
                } else {
                    expr->type = TYPE_BOOL;
                }
            } else if (op == OP_AND || op == OP_OR) {
                if (left != TYPE_BOOL || right != TYPE_BOOL) {
                    semantic_error(ctx, expr->line, "Logical operation requires bool operands", NULL);
                    expr->type = TYPE_ERROR;
                } else {
                    expr->type = TYPE_BOOL;
                }
            }
            return expr->type;
        }
    }
    return TYPE_ERROR;
}

static void analyze_stmt_list(SemanticContext *ctx, StmtList *list);

static void analyze_stmt(SemanticContext *ctx, Stmt *stmt) {
    switch (stmt->kind) {
        case STMT_STATE_DECL: {
            if (symbol_table_find(&ctx->symbols, stmt->as.state_decl.name)) {
                semantic_error(ctx, stmt->line, "Duplicate state declaration", stmt->as.state_decl.name);
                analyze_expr(ctx, stmt->as.state_decl.initializer);
                return;
            }
            ValueType type = analyze_expr(ctx, stmt->as.state_decl.initializer);
            if (type != TYPE_ERROR) {
                symbol_table_add(&ctx->symbols, stmt->as.state_decl.name, type, SYMBOL_STATE, 1);
            }
            break;
        }
        case STMT_ASSIGN: {
            Symbol *symbol = symbol_table_find(&ctx->symbols, stmt->as.assignment.name);
            if (!symbol) {
                semantic_error(ctx, stmt->line, "Undeclared identifier", stmt->as.assignment.name);
                analyze_expr(ctx, stmt->as.assignment.value);
                return;
            }
            ValueType value_type = analyze_expr(ctx, stmt->as.assignment.value);
            if (value_type != TYPE_ERROR && !types_compatible(symbol->type, value_type)) {
                semantic_error(ctx, stmt->line, "Assignment type mismatch for", stmt->as.assignment.name);
            }
            break;
        }
        case STMT_IF: {
            ValueType condition = analyze_expr(ctx, stmt->as.if_stmt.condition);
            if (condition != TYPE_ERROR && condition != TYPE_BOOL) {
                semantic_error(ctx, stmt->line, "if condition must be bool", NULL);
            }
            analyze_stmt_list(ctx, stmt->as.if_stmt.then_branch);
            analyze_stmt_list(ctx, stmt->as.if_stmt.else_branch);
            break;
        }
        case STMT_WHILE: {
            ValueType condition = analyze_expr(ctx, stmt->as.while_stmt.condition);
            if (condition != TYPE_ERROR && condition != TYPE_BOOL) {
                semantic_error(ctx, stmt->line, "while condition must be bool", NULL);
            }
            analyze_stmt_list(ctx, stmt->as.while_stmt.body);
            break;
        }
        case STMT_PRINT:
            analyze_expr(ctx, stmt->as.print_stmt.value);
            break;
        case STMT_INPUT:
            if (!symbol_table_find(&ctx->symbols, stmt->as.input_stmt.name)) {
                semantic_error(ctx, stmt->line, "input requires declared identifier", stmt->as.input_stmt.name);
            }
            break;
        case STMT_EXECUTE:
            if (strcmp(stmt->as.execute_stmt.name, "action") != 0) {
                semantic_error(ctx, stmt->line, "Only execute action is valid; invalid target", stmt->as.execute_stmt.name);
            }
            break;
    }
}

static void analyze_stmt_list(SemanticContext *ctx, StmtList *list) {
    for (StmtList *node = list; node; node = node->next) {
        analyze_stmt(ctx, node->stmt);
    }
}

int semantic_analyze(SemanticContext *ctx, StmtList *program) {
    analyze_stmt_list(ctx, program);
    return ctx->error_count == 0;
}
