#include "getSource.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif

// 1行あたりの最大文字数
#define MAXLINE 120
// エラーの最大数
#define MAXERROR 30
// 数値の最大桁数
#define MAXNUM 14
// タブのスペース数
#define TAB 5
// 挿入文字の色
#define INSERT_C "#82b1ff"
// 削除文字の色
#define DELETE_C "#BE5046"
// タイプエラーの色
#define TYPE_C "#C3E88D"

#define COLOR_FG "#bfc7d5"
#define COLOR_BG "#292D3E"

// ソースファイル
static FILE *fpi;
// 出力ファイル
static FILE *fptex;

// 行のデータ
static char line[MAXLINE];
// 行の位置
static int lineIndex;
// 最後に読んだ文字 (常に1文字先読みしている)
static int ch;

// 最後に識別したトークン
// 何に使うの？？
//   -> トークンの表示に使う
static Token cToken;

// 現在のトークンの種類
static KindT idKind;

// そのトークンの前のスペースの個数
static int spaces;
// その前の CR の個数
static int CR;
// トークンを印字したか？
static int printed;

// エラーの数
static int errorCnt;

/* static char nextChar(); */
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
    {"else", Else},
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
    for (int i = '0'; i <= '9'; i++) {
        charClassTbl[i] = digit;
    }
    for (int i = 'A'; i <= 'Z'; i++) {
        charClassTbl[i] = letter;
    }
    for (int i = 'a'; i <= 'z'; i++) {
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
    /* strcat_s(fileNameO, sizeof(fileNameO) + 4, ".tex"); */
    strcat_s(fileNameO, sizeof(fileNameO) + 5, ".html");

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
            printSpaces();
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
    // TODO: なぜ、ここで printed を 1 にするの？
    printed = 1;
    initCharClassTbl();

    // LaTex コマンド
    fprintf(fptex, "<!doctype html>\n");
    fprintf(fptex, "<head>");
    fprintf(fptex, "<meta charset=\"utf-8\">\n");
    fprintf(fptex, "<style type=\"text/css\">\n");
    fprintf(fptex, "body {background: %s; color: %s}\n", COLOR_BG, COLOR_FG);
    fprintf(fptex, ".insert {color: %s}\n", INSERT_C);
    fprintf(fptex, ".delete {color: %s}\n", DELETE_C);
    fprintf(fptex, ".type {color: %s}\n", TYPE_C);
    fprintf(fptex, "</style>");
    fprintf(fptex, "<title>compiled source program</title>\n");
    fprintf(fptex, "</head>");
    fprintf(fptex, "<body>\n");
    fprintf(fptex, "<pre>\n");
}

void finalSource() {
    if (cToken.kind == Period) {
        printcToken();
    } else {
        errorInsert(Period);
    }

    fprintf(fptex, "\n</pre>");
    fprintf(fptex, "\n</body>");
    fprintf(fptex, "\n</html>\n");
}

void errorInsert(KeyId k) {
    fprintf(fptex, "<span class=\"insert\"><b>%s</b></span>",
            KeyWdTbl[k].lexeme);
}

/*
    エラーの数をチェックし、多かったら終わり
*/
void errorCntCheck() {
    if (errorCnt++ > MAXERROR) {
        fprintf_s(fptex, "too many errors\n</pre>\n</body>\n<html>\n");
        printf("abort compilation");
        exit(1);
    }
}

/*
    型エラー を印字
*/
void errorType(char *m) {
    printSpaces();
    fprintf(fptex, "<span class=\"type\">%s</span>", m);
    printcToken();
    errorCntCheck();
}

/*
    識別子が間違えていますよー！！
*/
void errorMissingId() {
    fprintf(fptex, "<span class=\"insert\">Id</span>");
    errorCntCheck();
}

void errorDelete() {
    int i = (int)cToken.kind;

    printSpaces();
    printed = 1;

    if (i < end_of_KeyWd) {
        /* 予約語 */
        fprintf(fptex, "<span class=\"delete\"><b>%s</b></span>",
                KeyWdTbl[i].lexeme);
    } else if (i < end_of_KeySym) {
        /* 演算子 or 区切り文字 */
        fprintf(fptex, "<span class=\"delete\">%s</span>", KeyWdTbl[i].lexeme);
    } else if (i < (int)Id) {
        /* 識別子 */
        fprintf(fptex, "<span class=\"delete\">%s</span>", cToken.u.id);
    } else if (i < (int)Num) {
        /* 数値 */
        fprintf(fptex, "<span class=\"delete\">%d</span>", cToken.u.value);
    }
}

/*
    エラーメッセージをtexファイルに出力
*/
void errorMessage(char *m) {
    fprintf_s(fptex, "<span class=\"type\">%s</span>", m);
    errorCntCheck();
}

/*
    エラーメッセージを出力し、コンパイル終了
*/
void errorF(char *m) {
    errorMessage(m);
    fprintf_s(fptex, "fatal errors\n</pre>\n</body>\n</html>\n");
    if (errorCnt) {
        printf("total %d errors\n", errorCnt);
    }
    printf("abort compilation\n");
    exit(1);
}

/* 次のトークンを読み出し、返す */
Token nextToken() {
    // 識別子のためのインデックス
    int i = 0;
    int num;
    KeyId cc;
    Token tok;
    char ident[MAXNAME];

    // 表示
    printcToken();
    spaces = 0;
    CR = 0;

    // 空白文字を読み飛ばしつつ、カウントする
    while (1) {
        if (ch == ' ') {
            spaces++;
        } else if (ch == '\t') {
            // タブの場合、スペース数で加算
            spaces += TAB;
        } else if (ch == '\n') {
            //  aa   <CR> ってなってたら、aa の後ろのスペースは要らないため
            spaces = 0;
            CR++;
        } else {
            // 空白文字以外のため、終わり
            break;
        }
        ch = nextChar();
    }

    switch (cc = charClassTbl[ch]) {

    case letter: /* 識別子 */
        do {
            // 名前の最大文字数になるは、名前として認識する
            if (i < MAXNAME) {
                ident[i] = ch;
            }
            i++;
            ch = nextChar();
        } while (charClassTbl[ch] == letter || charClassTbl[ch] == digit);

        if (i >= MAXNAME) {
            errorMessage("too long");
            // NULL文字を入れたいため、インデックスを1つ戻す
            i = MAXNAME - 1;
        }

        ident[i] = '\0';

        /* 予約語か？ */
        for (int i = 0; i < end_of_KeyWd; i++) {
            if (strcmp(ident, KeyWdTbl[i].lexeme) == 0) {
                // 予約語
                tok.kind = KeyWdTbl[i].keyId;
                // 最後に識別したトークンとして覚えておく
                cToken = tok;
                printed = 0;
                return tok;
            }
        }

        /* 予約語ではない識別子 */
        tok.kind = Id;
        strcpy_s(tok.u.id, sizeof(tok.u.id), ident);

        // XXX: 疑問: ここでは、cToken は覚えておかないの？？？
        //      -> return する直前に cToken = tok ってするからここでは代入しない

        break;

    case digit: /* 数値 */
        num = 0;
        do {
            // ASCII だから、このようにして、値を求められる！
            //  ch - '0'
            num = 10 * num + (ch - '0');
            i++;
            ch = nextChar();
        } while (charClassTbl[ch] == digit);

        if (i > MAXNUM) {
            // 最大桁数をオーバーしてたら、エラーメッセージを出す
            errorMessage("too large");
        }
        tok.kind = Num;
        tok.u.value = num;
        break;

    case colon:
        if ((ch = nextChar()) == '=') {
            // ":="
            ch = nextChar();
            tok.kind = Assign;
        } else {
            // TODO: ":" なはずだけど、なぜ、nul？
            tok.kind = nul;
        }
        break;

    case Lss:
        if ((ch = nextChar()) == '=') {
            // "<="
            ch = nextChar();
            tok.kind = LssEq;
        } else if (ch == '>') {
            // "<>"
            ch = nextChar();
            tok.kind = NotEq;
        } else {
            // "<"
            tok.kind = Lss;
        }
        break;

    case Gtr:
        if ((ch = nextChar()) == '=') {
            // "<="
            ch = nextChar();
            tok.kind = GtrEq;
        } else {
            tok.kind = Gtr;
        }
        break;

    default:
        tok.kind = cc;
        // 常に1文字先を読んでいることになっているため
        ch = nextChar();
        break;
    }

    // 最後に識別したトークンとして覚えておく
    cToken = tok;

    printed = 0;

    return tok;
}

/* t.kind == k のチェック */
Token checkGet(Token t, KeyId k) {
    /*
        t.kind == k の場合、
            次のトークンを読んで返す
        t.kind != k の場合、
            エラーメッセージを出力し、
            t と k の両方が記号 or 両方が予約語の場合、
                t を捨て、次のトークンを読んで返す
                (t を k で置き換えたことにする (誤り訂正？))
        それ以外の場合、
            k を挿入したことにして、t を返す
    */

    if (t.kind == k) {
        // 合っているため、次を読み出す
        return nextToken();
    }

    if ((isKeyWd(k) && isKeyWd(t.kind)) || (isKeySym(k) && isKeySym(t.kind))) {
        // 間違えたのかな？ってことで置き換える (phrase level recovery)
        errorDelete();
        errorInsert(k);
        return nextToken();
    }

    // ぜんぜん違うから、正しいと思われるものを insert して、そのまま返す
    // (phrase level recovery)
    errorInsert(k);
    return t;
}

/*
    空白や改行の印字
*/
static void printSpaces() {
    // この書き方面白い
    while (CR-- > 0) {
        fprintf_s(fptex, "\n");
    }
    while (spaces-- > 0) {
        fprintf_s(fptex, " ");
    }
    // 初期化
    CR = 0;
    spaces = 0;
}

/*
    現在のトークンの種類によって、書体を変えて印字 (cToken の表示)
*/
void printcToken() {
    int i = (int)cToken.kind;

    if (printed) {
        // すでに印字済みなら、終わり
        printed = 0;
        return;
    }

    printed = 1;
    printSpaces();

    if (i < end_of_KeyWd) {
        /* 予約語 */
        // (bold 太字)
        fprintf_s(fptex, "<b>%s</b>", KeyWdTbl[i].lexeme);
        printf("<\"%s\">\n", KeyWdTbl[i].lexeme);
    } else if (i < end_of_KeySym) {
        /* 記号か演算子 */
        fprintf_s(fptex, "%s", KeyWdTbl[i].lexeme);
        printf("<\"%s\">\n", KeyWdTbl[i].lexeme);
    } else if (i == (int)Id) {
        /* 識別子 */
        printf("<Id, \"%s\">\n", cToken.u.id);

        switch (idKind) {
        case varId:
            // 変数 (なし)
            fprintf_s(fptex, "%s", cToken.u.id);
            return;

        case parId:
            // パラメータ (斜体)
            fprintf_s(fptex, "<i>%s</i>", cToken.u.id);
            return;

        case funcId:
            // 関数 (italic 強調)
            fprintf_s(fptex, "<i>%s</i>", cToken.u.id);
            return;

        case constId:
            // 定数 (Sans-serif)
            fprintf_s(fptex, "<tt>%s</tt>", cToken.u.id);
            return;
        }
    } else if (i == (int)Num) {
        /* 数値 */
        fprintf_s(fptex, "%d", cToken.u.value);
        printf("<Num, %d>\n", cToken.u.value);
    }
}

/*
    エラーの個数を返す
*/
int errorN() { return errorCnt; }

/* 現在のトークンの種類をセット */
void setIdKind(KindT k) { idKind = k; }
