#include <stdio.h>
#include "getSource.h"

int main(int argc, char const* argv[])
{
    // ソースファイルのファイル名
    char fileName[30];
    printf("enter source file name\n");
    scanf_s("%s", fileName);
    if (!openSource(fileName)) {
        return -1;
    }
    closeSource();
    return 0;
}
