#include "codegen.h"
#include "compile.h"
#include "getSource.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) {
    // ソースファイルのファイル名
    char fileName[30];

    strcpy_s(fileName, sizeof(fileName), argv[1]);
    /* printf("enter source file name\n"); */
    /* scanf_s("%s", fileName); */
    if (!openSource(fileName)) {
        return -1;
    }
    if (compile(fileName)) {
        // エラー数が3以下なら、実行
        execute();
    }
    closeSource();
    return 0;
}
