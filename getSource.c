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

// 最後に読んだトークン
static Token cToken;

/* static KindT idKind; */

// そのトークンの前のスペースの個数
static int spaces;
// その前の CR の個数
static int CR;
// トークンを印字したか？
static int printed;

// エラーの数
static int errorCnt;

static char nextChar();
static void printSpaces();
static void printcToken();

/* キーワード表でのエントリ */
struct keyWd {
    // 原始プログラム内の文字列
    //   (構文解析器にとってはどうでもいい)
    char *lexeme;
    // トークン名 や キーワードの種類
    //   (文法での終端記号を意味する)
    KeyId keyId;
};

/*
    キーワード表
        識別子と予約語 を見分けるために使用する
        原始プログラム上のつづり と トークン名の対応表
*/
static struct keyWd KeyWdTbl[] = {
    {"begin", Begin},
    {"end", End},
    {"if", If},
    {"then", Then},
    {"while", While},
    {"do", Do},
    {"return", Ret},
    {"function", Func},
    {"var", Var},
    {"const", Const},
    {"odd", Odd},
    {"write", Write},
    {"writeln", WriteLn},
    {"$dummy1", end_of_KeyWd},

    {"+", Plus},
    {"-", Minus},
    {"*", Mult},
    {"/", Div},
    {"(", Lparen},
    {")", Rparen},
    {"=", Equal},
    {"<", Lss},
    {">", Gtr},
    {"<>", NotEq},
    {"<=", LssEq},
    {">=", GtrEq},
    {",", Comma},
    {".", Period},
    {";", Semicolon},
    {":=", Assign},
    {"$dummy2", end_of_KeySym},
};

/*
    キーk は予約語か？
*/
int isKeyWd(KeyId k) { return (k < end_of_KeyWd); }

/*
    キーk は記号か？
*/
int isKeySym(KeyId k) { return (end_of_KeyWd < k & k < end_of_KeySym); }

// 文字の種類を表す表
// TODO: なんのチェックに使っているのか
static KeyId charClassTbl[256];

// 文字の種類を表す表の初期化
static void initCharClassTbl() {

    // 初期化
    for (int i = 0; i < 256; i++) {
        charClassTbl[i] = others;
    }
    for (int i = '0'; i < '9'; i++) {
        charClassTbl[i] = digit;
    }
    for (int i = 'A'; i < 'Z'; i++) {
        charClassTbl[i] = letter;
    }
    for (int i = 'a'; i < 'z'; i++) {
        charClassTbl[i] = letter;
    }

    // 記号
    charClassTbl['+'] = Plus;
    charClassTbl['-'] = Minus;
    charClassTbl['*'] = Mult;
    charClassTbl['/'] = Div;
    charClassTbl['('] = Lparen;
    charClassTbl[')'] = Rparen;
    charClassTbl['='] = Equal;
    charClassTbl['<'] = Lss;
    charClassTbl['>'] = Gtr;
    charClassTbl[','] = Comma;
    charClassTbl['.'] = Period;
    charClassTbl[';'] = Semicolon;
    charClassTbl[':'] = colon;

    // 以下の記号については、字句解析中に記号表に入れる
    /* '<>' (NotEq) */
    /* '<=' (LssEq) */
    /* '>=' (GtrEq) */
    /* ':=' (Assign) */
}

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
