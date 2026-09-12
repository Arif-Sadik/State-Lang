#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>

static char *copy_text(const char *text) {
    size_t len = strlen(text);
    char *copy = (char *)malloc(len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, text, len + 1);
    return copy;
}

void symbol_table_init(SymbolTable *table) {
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

void symbol_table_free(SymbolTable *table) {
    for (int i = 0; i < table->count; i++) {
        free(table->items[i].name);
    }
    free(table->items);
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

static int ensure_capacity(SymbolTable *table) {
    if (table->count < table->capacity) {
        return 1;
    }
    int next_capacity = table->capacity == 0 ? 8 : table->capacity * 2;
    Symbol *next = (Symbol *)realloc(table->items, sizeof(Symbol) * next_capacity);
    if (!next) {
        return 0;
    }
    table->items = next;
    table->capacity = next_capacity;
    return 1;
}

Symbol *symbol_table_find(SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            return &table->items[i];
        }
    }
    return NULL;
}

const Symbol *symbol_table_find_const(const SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            return &table->items[i];
        }
    }
    return NULL;
}

int symbol_table_add(SymbolTable *table, const char *name, ValueType type, SymbolKind kind, int initialized) {
    if (symbol_table_find(table, name)) {
        return 0;
    }
    if (!ensure_capacity(table)) {
        return 0;
    }
    Symbol *symbol = &table->items[table->count++];
    symbol->name = copy_text(name);
    symbol->type = type;
    symbol->kind = kind;
    symbol->initialized = initialized;
    return 1;
}

void symbol_table_add_predefined(SymbolTable *table) {
    symbol_table_add(table, "action", TYPE_STRING, SYMBOL_PREDEFINED, 1);
}

const char *symbol_kind_name(SymbolKind kind) {
    return kind == SYMBOL_PREDEFINED ? "predefined" : "state";
}

void symbol_table_print(FILE *out, const SymbolTable *table) {
    fprintf(out, "%-20s %-10s %-12s %-12s\n", "Name", "Type", "Kind", "Initialized");
    fprintf(out, "%-20s %-10s %-12s %-12s\n", "----", "----", "----", "-----------");
    for (int i = 0; i < table->count; i++) {
        const Symbol *symbol = &table->items[i];
        fprintf(out, "%-20s %-10s %-12s %-12s\n",
                symbol->name,
                type_name(symbol->type),
                symbol_kind_name(symbol->kind),
                symbol->initialized ? "yes" : "no");
    }
}
