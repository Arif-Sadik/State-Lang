#ifndef STATELANG_AST_H
#define STATELANG_AST_H

#include <stdio.h>

typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_ERROR
} ValueType;

typedef enum {
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_IDENTIFIER,
    EXPR_UNARY,
    EXPR_BINARY
} ExprKind;

typedef enum {
    STMT_STATE_DECL,
    STMT_ASSIGN,
    STMT_IF,
    STMT_WHILE,
    STMT_PRINT,
    STMT_INPUT,
    STMT_EXECUTE
} StmtKind;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_NEG
} OperatorKind;

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct StmtList StmtList;

struct Expr {
    ExprKind kind;
    ValueType type;
    int line;
    union {
        int int_value;
        double float_value;
        char *string_value;
        int bool_value;
        char *identifier;
        struct {
            OperatorKind op;
            Expr *expr;
        } unary;
        struct {
            OperatorKind op;
            Expr *left;
            Expr *right;
        } binary;
    } as;
};

struct Stmt {
    StmtKind kind;
    int line;
    union {
        struct {
            char *name;
            Expr *initializer;
        } state_decl;
        struct {
            char *name;
            Expr *value;
        } assignment;
        struct {
            Expr *condition;
            StmtList *then_branch;
            StmtList *else_branch;
        } if_stmt;
        struct {
            Expr *condition;
            StmtList *body;
        } while_stmt;
        struct {
            Expr *value;
        } print_stmt;
        struct {
            char *name;
        } input_stmt;
        struct {
            char *name;
        } execute_stmt;
    } as;
};

struct StmtList {
    Stmt *stmt;
    StmtList *next;
};

Expr *ast_expr_int(int value, int line);
Expr *ast_expr_float(double value, int line);
Expr *ast_expr_string(const char *value, int line);
Expr *ast_expr_bool(int value, int line);
Expr *ast_expr_identifier(const char *name, int line);
Expr *ast_expr_unary(OperatorKind op, Expr *expr, int line);
Expr *ast_expr_binary(OperatorKind op, Expr *left, Expr *right, int line);

Stmt *ast_stmt_state_decl(const char *name, Expr *initializer, int line);
Stmt *ast_stmt_assignment(const char *name, Expr *value, int line);
Stmt *ast_stmt_if(Expr *condition, StmtList *then_branch, StmtList *else_branch, int line);
Stmt *ast_stmt_while(Expr *condition, StmtList *body, int line);
Stmt *ast_stmt_print(Expr *value, int line);
Stmt *ast_stmt_input(const char *name, int line);
Stmt *ast_stmt_execute(const char *name, int line);
StmtList *ast_stmt_list_append(StmtList *list, Stmt *stmt);

void ast_free_expr(Expr *expr);
void ast_free_stmt(Stmt *stmt);
void ast_free_stmt_list(StmtList *list);
void ast_print_stmt_list(FILE *out, const StmtList *list);

const char *type_name(ValueType type);
const char *operator_name(OperatorKind op);

#endif
