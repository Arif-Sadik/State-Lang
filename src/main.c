#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "ir.h"
#include "semantic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *input_path;
    const char *tac_path;
    const char *c_path;
    const char *exe_path;
    int dump_ast;
    int dump_symbols;
    int compile_exe;
} Options;

static void usage(void) {
    puts("StateLang compiler");
    puts("Usage: statelangc <source.state> --emit-tac <file> --emit-c <file> [--exe <file>] [--dump-ast] [--dump-symbols]");
}

static int parse_args(int argc, char **argv, Options *options) {
    memset(options, 0, sizeof(*options));
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else if (strcmp(argv[i], "--dump-ast") == 0) {
            options->dump_ast = 1;
        } else if (strcmp(argv[i], "--dump-symbols") == 0) {
            options->dump_symbols = 1;
        } else if (strcmp(argv[i], "--emit-tac") == 0 && i + 1 < argc) {
            options->tac_path = argv[++i];
        } else if (strcmp(argv[i], "--emit-c") == 0 && i + 1 < argc) {
            options->c_path = argv[++i];
        } else if (strcmp(argv[i], "--exe") == 0 && i + 1 < argc) {
            options->exe_path = argv[++i];
            options->compile_exe = 1;
        } else if (!options->input_path) {
            options->input_path = argv[i];
        } else {
            fprintf(stderr, "Compiler Error: unknown argument %s\n", argv[i]);
            return -1;
        }
    }
    if (!options->input_path) {
        usage();
        return -1;
    }
    return 1;
}

static int run_gcc(const char *c_path, const char *exe_path) {
    char command[2048];
    snprintf(command, sizeof(command), "gcc \"%s\" -o \"%s\"", c_path, exe_path);
    printf("GCC command: %s\n", command);
    int code = system(command);
    if (code != 0) {
        fprintf(stderr, "Code Generation Error: GCC failed with exit code %d\n", code);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    Options options;
    int arg_result = parse_args(argc, argv, &options);
    if (arg_result <= 0) {
        return arg_result == 0 ? 0 : 1;
    }

    g_lexical_error_count = 0;
    if (!parse_file(options.input_path)) {
        ast_free_stmt_list(g_program);
        return 2;
    }

    SemanticContext semantic;
    semantic_init(&semantic);
    int ok = semantic_analyze(&semantic, g_program);
    if (!ok) {
        semantic_free(&semantic);
        ast_free_stmt_list(g_program);
        return 3;
    }

    if (options.dump_ast) {
        ast_print_stmt_list(stdout, g_program);
    }
    if (options.dump_symbols) {
        symbol_table_print(stdout, &semantic.symbols);
    }
    if (options.tac_path && !ir_generate_file(options.tac_path, g_program)) {
        semantic_free(&semantic);
        ast_free_stmt_list(g_program);
        return 4;
    }
    if (options.c_path && !codegen_generate_c_file(options.c_path, g_program, &semantic.symbols)) {
        semantic_free(&semantic);
        ast_free_stmt_list(g_program);
        return 5;
    }
    if (options.compile_exe) {
        if (!options.c_path) {
            fprintf(stderr, "Compiler Error: --exe requires --emit-c\n");
            semantic_free(&semantic);
            ast_free_stmt_list(g_program);
            return 6;
        }
        if (!run_gcc(options.c_path, options.exe_path)) {
            semantic_free(&semantic);
            ast_free_stmt_list(g_program);
            return 7;
        }
    }

    printf("StateLang compilation completed.\n");
    semantic_free(&semantic);
    ast_free_stmt_list(g_program);
    return 0;
}
