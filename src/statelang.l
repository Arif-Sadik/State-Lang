%option noyywrap yylineno
%{
#include "statelang.tab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_lexical_error_count = 0;

static char *copy_text(const char *text) {
    size_t len = strlen(text);
    char *copy = (char *)malloc(len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, text, len + 1);
    return copy;
}

static char *unescape_string(const char *text) {
    size_t len = strlen(text);
    char *copy = (char *)malloc(len + 1);
    size_t j = 0;
    for (size_t i = 1; i + 1 < len; i++) {
        if (text[i] == '\\' && i + 1 < len - 1) {
            i++;
            if (text[i] == 'n') copy[j++] = '\n';
            else if (text[i] == 't') copy[j++] = '\t';
            else copy[j++] = text[i];
        } else {
            copy[j++] = text[i];
        }
    }
    copy[j] = '\0';
    return copy;
}
%}

%%
[ \t\r\n]+                 ;
"//"[^\n]*                 ;
"state"                    return STATE;
"if"                       return IF;
"else"                     return ELSE;
"while"                    return WHILE;
"print"                    return PRINT;
"input"                    return INPUT;
"execute"                  return EXECUTE;
"true"                     { yylval.bool_value = 1; return BOOL_LITERAL; }
"false"                    { yylval.bool_value = 0; return BOOL_LITERAL; }
[0-9]+\.[0-9]+             { yylval.float_value = atof(yytext); return FLOAT_LITERAL; }
[0-9]+                     { yylval.int_value = atoi(yytext); return INT_LITERAL; }
\"([^\\\n]|\\.)*\"         { yylval.string_value = unescape_string(yytext); return STRING_LITERAL; }
[A-Za-z_][A-Za-z0-9_]*     { yylval.string_value = copy_text(yytext); return IDENTIFIER; }
"&&"                       return AND;
"||"                       return OR;
"<="                       return LE;
">="                       return GE;
"=="                       return EQ;
"!="                       return NE;
"+"                        return PLUS;
"-"                        return MINUS;
"*"                        return MULT;
"/"                        return DIV;
">"                        return GT;
"<"                        return LT;
"!"                        return NOT;
"="                        return ASSIGN;
";"                        return SEMICOLON;
"("                        return LPAREN;
")"                        return RPAREN;
"{"                        return LBRACE;
"}"                        return RBRACE;
.                          { fprintf(stderr, "Lexical Error [line %d]: unexpected character '%s'\n", yylineno, yytext); g_lexical_error_count++; return INVALID_TOKEN; }
%%
