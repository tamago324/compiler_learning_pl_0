#include "getSource.h"
#include <stdio.h>
#include <string.h>

// ソースファイル
static FILE *fpi;
// 出力ファイル
static FILE *fptex;

/*
    ファイルを開く
*/
int openSource(char fileName[]) {
    char fileNameO[30];

    if (fopen_s(&fpi, fileName, "r") != 0) {
        printf("can't open %s\n", fileName);
        return 0;
    }

    // 出力ファイルの作成
    strcpy_s(fileNameO, sizeof(fileNameO), fileName);
    strcat_s(fileNameO, sizeof(fileNameO) + 4, ".tex");

    if (fopen_s(&fptex, fileNameO, "w") != 0) {
        printf("can't open %s\n", fileNameO);
        return 0;
    }
    return 1;
}

/*
    ファイルを閉じる
*/
void closeSource() {
    fclose(fpi);
    fclose(fptex);
}
