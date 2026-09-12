#ifndef STATELANG_SEMANTIC_H
#define STATELANG_SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

typedef struct {
    SymbolTable symbols;
    int error_count;
} SemanticContext;

void semantic_init(SemanticContext *ctx);
void semantic_free(SemanticContext *ctx);
int semantic_analyze(SemanticContext *ctx, StmtList *program);

#endif
