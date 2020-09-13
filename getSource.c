#include "getSource.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1行あたりの最大文字数
#define MAXLINE 120
// エラーの最大数
#define MAXERROR 30

// ソースファイル
static FILE *fpi;
// 出力ファイル
static FILE *fptex;

// 行のデータ
static char line[MAXLINE];
// 行の位置
static int lineIndex;
// 最後に読んだ文字
static char ch;

// エラーの数
static int errorCnt;

static char nextChar();

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

/*
    次のトークンを読み出し、返す
*/
char nextChar() {
    char c;

    if (lineIndex == -1) {
        if (fgets(line, MAXLINE, fpi) != NULL) {
            // 位置を初期化
            lineIndex = 0;
        } else {
            // コンパイル終了
            errorF("end of file\n");
        }
    }

    if ((c = line[lineIndex++]) == '\n') {
        // 次の行を読む準備
        lineIndex = -1;
        return '\n';
    }

    return c;
}

/*
    いろいろ初期化
*/
void initSource() {
    lineIndex = -1;
    ch = '\n';
}

/*
    エラーの数をチェックし、多かったら終わり
*/
void errorCntCheck() {
    if (errorCnt++ > MAXERROR) {
        fprintf_s(fptex, "too many errors\nend{document}\n");
        printf("abort compilation");
        exit(1);
    }
}

/*
    エラーメッセージをtexファイルに出力
*/
void errorMessage(char *m) {
    fprintf_s(fptex, "$^ {%s}$", m);
    errorCntCheck();
}

/*
    エラーメッセージを出力し、コンパイル終了
*/
void errorF(char *m) {
    errorMessage(m);
    fprintf_s(fptex, "fatal errors\n\\end{document}\n");
    if (errorCnt) {
        printf("total %d errors\n", errorCnt);
    }
    printf("abort compilation\n");
    exit(1);
}
