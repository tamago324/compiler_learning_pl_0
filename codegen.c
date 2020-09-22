#include "codegen.h"
#include "getSource.h"

#ifndef TBL
#define TBL
#include "table.h"
#endif

/* 最後に生成した命令後のインデックス */
static int cIndex = -1;

int nextCode() { return cIndex + 1; }
