%{
#include "ast.h"
#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
extern int yylineno;
extern FILE *yyin;
void yyrestart(FILE *input_file);
void yyerror(const char *s);

StmtList *g_program = NULL;

static int current_line(void) {
    return yylineno > 0 ? yylineno : 1;
}
%}

%define parse.error verbose

%code requires {
#include "ast.h"
}

%union {
    int int_value;
    double float_value;
    char *string_value;
    int bool_value;
    Expr *expr;
    Stmt *stmt;
    StmtList *stmt_list;
}

%token STATE IF ELSE WHILE PRINT INPUT EXECUTE
%token <bool_value> BOOL_LITERAL
%token <string_value> IDENTIFIER STRING_LITERAL
%token <int_value> INT_LITERAL
%token <float_value> FLOAT_LITERAL
%token PLUS MINUS MULT DIV GT LT GE LE EQ NE AND OR NOT ASSIGN
%token SEMICOLON LPAREN RPAREN LBRACE RBRACE
%token INVALID_TOKEN

%type <stmt_list> program statement_list block optional_else
%type <stmt> statement state_decl assignment if_stmt while_stmt print_stmt input_stmt execute_stmt
%type <expr> expression logical_or logical_and equality comparison additive multiplicative unary primary

%start program

%%

program
    : statement_list { g_program = $$ = $1; }
    ;

statement_list
    : %empty { $$ = NULL; }
    | statement_list statement { $$ = ast_stmt_list_append($1, $2); }
    ;

statement
    : state_decl { $$ = $1; }
    | assignment { $$ = $1; }
    | if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | print_stmt { $$ = $1; }
    | input_stmt { $$ = $1; }
    | execute_stmt { $$ = $1; }
    ;

state_decl
    : STATE IDENTIFIER ASSIGN expression SEMICOLON {
        $$ = ast_stmt_state_decl($2, $4, current_line());
        free($2);
    }
    ;

assignment
    : IDENTIFIER ASSIGN expression SEMICOLON {
        $$ = ast_stmt_assignment($1, $3, current_line());
        free($1);
    }
    ;

if_stmt
    : IF LPAREN expression RPAREN block optional_else {
        $$ = ast_stmt_if($3, $5, $6, current_line());
    }
    ;

optional_else
    : %empty { $$ = NULL; }
    | ELSE block { $$ = $2; }
    | ELSE if_stmt { $$ = ast_stmt_list_append(NULL, $2); }
    ;

while_stmt
    : WHILE LPAREN expression RPAREN block {
        $$ = ast_stmt_while($3, $5, current_line());
    }
    ;

print_stmt
    : PRINT expression SEMICOLON { $$ = ast_stmt_print($2, current_line()); }
    ;

input_stmt
    : INPUT IDENTIFIER SEMICOLON {
        $$ = ast_stmt_input($2, current_line());
        free($2);
    }
    ;

execute_stmt
    : EXECUTE IDENTIFIER SEMICOLON {
        $$ = ast_stmt_execute($2, current_line());
        free($2);
    }
    ;

block
    : LBRACE statement_list RBRACE { $$ = $2; }
    ;

expression
    : logical_or { $$ = $1; }
    ;

logical_or
    : logical_or OR logical_and { $$ = ast_expr_binary(OP_OR, $1, $3, current_line()); }
    | logical_and { $$ = $1; }
    ;

logical_and
    : logical_and AND equality { $$ = ast_expr_binary(OP_AND, $1, $3, current_line()); }
    | equality { $$ = $1; }
    ;

equality
    : equality EQ comparison { $$ = ast_expr_binary(OP_EQ, $1, $3, current_line()); }
    | equality NE comparison { $$ = ast_expr_binary(OP_NE, $1, $3, current_line()); }
    | comparison { $$ = $1; }
    ;

comparison
    : comparison LT additive { $$ = ast_expr_binary(OP_LT, $1, $3, current_line()); }
    | comparison GT additive { $$ = ast_expr_binary(OP_GT, $1, $3, current_line()); }
    | comparison LE additive { $$ = ast_expr_binary(OP_LE, $1, $3, current_line()); }
    | comparison GE additive { $$ = ast_expr_binary(OP_GE, $1, $3, current_line()); }
    | additive { $$ = $1; }
    ;

additive
    : additive PLUS multiplicative { $$ = ast_expr_binary(OP_ADD, $1, $3, current_line()); }
    | additive MINUS multiplicative { $$ = ast_expr_binary(OP_SUB, $1, $3, current_line()); }
    | multiplicative { $$ = $1; }
    ;

multiplicative
    : multiplicative MULT unary { $$ = ast_expr_binary(OP_MUL, $1, $3, current_line()); }
    | multiplicative DIV unary { $$ = ast_expr_binary(OP_DIV, $1, $3, current_line()); }
    | unary { $$ = $1; }
    ;

unary
    : NOT unary { $$ = ast_expr_unary(OP_NOT, $2, current_line()); }
    | MINUS unary { $$ = ast_expr_unary(OP_NEG, $2, current_line()); }
    | primary { $$ = $1; }
    ;

primary
    : INT_LITERAL { $$ = ast_expr_int($1, current_line()); }
    | FLOAT_LITERAL { $$ = ast_expr_float($1, current_line()); }
    | STRING_LITERAL { $$ = ast_expr_string($1, current_line()); free($1); }
    | BOOL_LITERAL { $$ = ast_expr_bool($1, current_line()); }
    | IDENTIFIER { $$ = ast_expr_identifier($1, current_line()); free($1); }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error [line %d]: %s\n", current_line(), s);
}

int parse_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Compiler Error: could not open source file %s\n", path);
        return 0;
    }
    yyin = file;
    yyrestart(file);
    yylineno = 1;
    g_program = NULL;
    int result = yyparse();
    fclose(file);
    return result == 0 && g_lexical_error_count == 0;
}
