#ifndef STATELANG_CODEGEN_H
#define STATELANG_CODEGEN_H

#include "ast.h"
#include "symbol_table.h"

int codegen_generate_c_file(const char *path, StmtList *program, const SymbolTable *symbols);

#endif
