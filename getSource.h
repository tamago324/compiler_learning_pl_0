#include <stdio.h>

/* 名前の最大桁数 (null文字が入るため、実際には30文字) */
#define MAXNAME 31

/* トークン名 (それぞれを区別できればいいため、Enum でOK) */
typedef enum {
    /* 予約語 */
    Begin,
    End,
    If,
    Then,
    While,
    Do,
    Ret,
    Func,
    Var,
    Const,
    Odd,
    Write,
    WriteLn,
    end_of_KeyWd,

    /* 演算子と区切り記号 */
    Plus,      // +
    Minus,     // -
    Mult,      // *
    Div,       // /
    Lparen,    // (
    Rparen,    // )
    Equal,     // =
    Lss,       // <
    Gtr,       // >
    NotEq,     // <>
    LssEq,     // <=
    GtrEq,     // >=
    Comma,     // ,
    Period,    // .
    Semicolon, // ;
    Assign,    // :=
    end_of_KeySym,

    /* トークン */
    Id,
    Num,
    nul,
    end_of_Token,

    letter,
    digit,
    colon,
    others

} KeyId;

typedef struct {
    /* トークンの種類
        構文解析器では終端記号として使われる
    */
    KeyId kind;
    // XXX: 属性 (?)
    union {
        // 識別子の場合、名前が入る
        char id[MAXNAME];
        // 数字の場合、値が入る
        int value;
    } u;
} Token;

int openSource(char fileName[]); /* ファイルを開く */
void closeSource();              /* ファイルを閉じる */
void initSource();               /* 初期化 */

char nextChar();   /* 次のトークンを読み出し、返す */
Token nextToken(); /* 次のトークンを読み出し、返す */

void errorNoCheck(); /* エラーのお数をカウント、多すぎたら終わり */
void errorF(char *m); /* エラーメッセージを出力し、コンパイル終了 */
void errorMessage(char *m); /* エラーメッセージを出力 */
