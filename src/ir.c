#include "ir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    FILE *out;
    int temp_count;
    int label_count;
} IrContext;

static char *make_name(const char *prefix, int number) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s%d", prefix, number);
    size_t len = strlen(buffer);
    char *copy = (char *)malloc(len + 1);
    memcpy(copy, buffer, len + 1);
    return copy;
}

static char *copy_text(const char *text) {
    size_t len = strlen(text);
    char *copy = (char *)malloc(len + 1);
    memcpy(copy, text, len + 1);
    return copy;
}

static char *quote_text(const char *text) {
    size_t len = strlen(text);
    char *copy = (char *)malloc((len * 2) + 3);
    size_t j = 0;
    copy[j++] = '"';
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '"' || text[i] == '\\') {
            copy[j++] = '\\';
        }
        copy[j++] = text[i];
    }
    copy[j++] = '"';
    copy[j] = '\0';
    return copy;
}

static char *new_temp(IrContext *ctx) {
    ctx->temp_count++;
    return make_name("t", ctx->temp_count);
}

static char *new_label(IrContext *ctx) {
    ctx->label_count++;
    return make_name("L", ctx->label_count);
}

static char *expr_ir(IrContext *ctx, const Expr *expr) {
    char buffer[128];
    switch (expr->kind) {
        case EXPR_INT:
            snprintf(buffer, sizeof(buffer), "%d", expr->as.int_value);
            return copy_text(buffer);
        case EXPR_FLOAT:
            snprintf(buffer, sizeof(buffer), "%g", expr->as.float_value);
            return copy_text(buffer);
        case EXPR_STRING:
            return quote_text(expr->as.string_value);
        case EXPR_BOOL:
            return copy_text(expr->as.bool_value ? "true" : "false");
        case EXPR_IDENTIFIER:
            return copy_text(expr->as.identifier);
        case EXPR_UNARY: {
            char *value = expr_ir(ctx, expr->as.unary.expr);
            char *temp = new_temp(ctx);
            fprintf(ctx->out, "%s = %s%s\n", temp, operator_name(expr->as.unary.op), value);
            free(value);
            return temp;
        }
        case EXPR_BINARY: {
            char *left = expr_ir(ctx, expr->as.binary.left);
            char *right = expr_ir(ctx, expr->as.binary.right);
            char *temp = new_temp(ctx);
            fprintf(ctx->out, "%s = %s %s %s\n", temp, left, operator_name(expr->as.binary.op), right);
            free(left);
            free(right);
            return temp;
        }
    }
    return copy_text("<error>");
}

static void stmt_list_ir(IrContext *ctx, const StmtList *list);

static void stmt_ir(IrContext *ctx, const Stmt *stmt) {
    switch (stmt->kind) {
        case STMT_STATE_DECL: {
            char *value = expr_ir(ctx, stmt->as.state_decl.initializer);
            fprintf(ctx->out, "%s = %s\n", stmt->as.state_decl.name, value);
            free(value);
            break;
        }
        case STMT_ASSIGN: {
            char *value = expr_ir(ctx, stmt->as.assignment.value);
            fprintf(ctx->out, "%s = %s\n", stmt->as.assignment.name, value);
            free(value);
            break;
        }
        case STMT_IF: {
            char *condition = expr_ir(ctx, stmt->as.if_stmt.condition);
            char *else_label = new_label(ctx);
            char *end_label = new_label(ctx);
            fprintf(ctx->out, "JZ %s %s\n", condition, else_label);
            stmt_list_ir(ctx, stmt->as.if_stmt.then_branch);
            fprintf(ctx->out, "JMP %s\n", end_label);
            fprintf(ctx->out, "%s:\n", else_label);
            stmt_list_ir(ctx, stmt->as.if_stmt.else_branch);
            fprintf(ctx->out, "%s:\n", end_label);
            free(condition);
            free(else_label);
            free(end_label);
            break;
        }
        case STMT_WHILE: {
            char *start_label = new_label(ctx);
            char *end_label = new_label(ctx);
            fprintf(ctx->out, "%s:\n", start_label);
            char *condition = expr_ir(ctx, stmt->as.while_stmt.condition);
            fprintf(ctx->out, "JZ %s %s\n", condition, end_label);
            stmt_list_ir(ctx, stmt->as.while_stmt.body);
            fprintf(ctx->out, "JMP %s\n", start_label);
            fprintf(ctx->out, "%s:\n", end_label);
            free(condition);
            free(start_label);
            free(end_label);
            break;
        }
        case STMT_PRINT: {
            char *value = expr_ir(ctx, stmt->as.print_stmt.value);
            fprintf(ctx->out, "PRINT %s\n", value);
            free(value);
            break;
        }
        case STMT_INPUT:
            fprintf(ctx->out, "READ %s\n", stmt->as.input_stmt.name);
            break;
        case STMT_EXECUTE:
            fprintf(ctx->out, "EXEC %s\n", stmt->as.execute_stmt.name);
            break;
    }
}

static void stmt_list_ir(IrContext *ctx, const StmtList *list) {
    for (const StmtList *node = list; node; node = node->next) {
        stmt_ir(ctx, node->stmt);
    }
}

int ir_generate_file(const char *path, StmtList *program) {
    FILE *out = fopen(path, "w");
    if (!out) {
        fprintf(stderr, "Code Generation Error: could not write TAC file %s\n", path);
        return 0;
    }
    IrContext ctx;
    ctx.out = out;
    ctx.temp_count = 0;
    ctx.label_count = 0;
    fputs("# StateLang Three-Address Code\n", out);
    stmt_list_ir(&ctx, program);
    fclose(out);
    return 1;
}
