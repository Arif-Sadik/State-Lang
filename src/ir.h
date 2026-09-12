#ifndef STATELANG_IR_H
#define STATELANG_IR_H

#include "ast.h"

int ir_generate_file(const char *path, StmtList *program);

#endif
