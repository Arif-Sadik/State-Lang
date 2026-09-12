#ifndef STATELANG_COMPILER_H
#define STATELANG_COMPILER_H

#include "ast.h"

extern StmtList *g_program;
extern int g_lexical_error_count;

int parse_file(const char *path);

#endif
