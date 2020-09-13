#include <stdio.h>
#include "getSource.h"
#include "compile.h"

int main(int argc, char const* argv[])
{
    // ソースファイルのファイル名
    char fileName[30];
    printf("enter source file name\n");
    scanf_s("%s", fileName);
    if (!openSource(fileName)) {
        return -1;
    }
    if (compile(fileName)) {
        /* execute(); */
    }
    closeSource();
    return 0;
}
