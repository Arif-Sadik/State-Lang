#ifndef STATELANG_SYMBOL_TABLE_H
#define STATELANG_SYMBOL_TABLE_H

#include "ast.h"

#include <stdio.h>

typedef enum {
    SYMBOL_STATE,
    SYMBOL_PREDEFINED
} SymbolKind;

typedef struct {
    char *name;
    ValueType type;
    SymbolKind kind;
    int initialized;
} Symbol;

typedef struct {
    Symbol *items;
    int count;
    int capacity;
} SymbolTable;

void symbol_table_init(SymbolTable *table);
void symbol_table_free(SymbolTable *table);
int symbol_table_add(SymbolTable *table, const char *name, ValueType type, SymbolKind kind, int initialized);
Symbol *symbol_table_find(SymbolTable *table, const char *name);
const Symbol *symbol_table_find_const(const SymbolTable *table, const char *name);
void symbol_table_add_predefined(SymbolTable *table);
void symbol_table_print(FILE *out, const SymbolTable *table);
const char *symbol_kind_name(SymbolKind kind);

#endif
