#include "getSource.h"
#include <stdio.h>

// ブロック
static void block();
// 定数宣言
static void constDecl();
// 変数宣言
static void varDecl();
// 関数宣言
static void funcDecl();
// 文
static void statement();
// 条件
static void condition();
// 式
static void expression();
// 式の項
static void term();
// 式の因子
static void factor();

// 次のトークン
Token token;

int compile() {
    printf("start compilation\n");

    initSource();
    /* token 読み出して、出力もする */
    token = nextToken();
    block();
    finalSource();

    return 1;
}

void block() {
    /* block -> { ( constDecl | varDecl | funcDecl ) } statement */
    while (1) {
        switch (token.kind) {

        case Const:
            /* constDecl */

            // トークンを読み出しておく
            token = nextToken();
            constDecl();
            continue;

        case Var:
            token = nextToken();
            varDecl();
            continue;

        case Func:
            token = nextToken();
            funcDecl();
            continue;

        default:
            break;
        }

        break;
    }
    statement();
}

void constDecl() {
    /* constDecl -> const { ident = number // , } ; */

    /* block の部分で、 const は認識しているため、ここではチェックしない*/

    while (1) {
        /* { ident = number // , } */

        // ident
        if (token.kind == Id) {
            // = のはず
            token = checkGet(nextToken(), Equal);

            if (token.kind == Num) {
                // TODO: 記号表に登録
            } else {
                // 型エラー
                errorType("number");
            }
        } else {
            // 仮の識別子を入れる
            errorMissingId();
        }

        if (token.kind != Comma) {
            break;
        }

        token = nextToken();
    }

    // ;
    token = checkGet(token, Semicolon);
}

void varDecl() {
    while (1) {
        if (token.kind == Id) {
            // TODO: 識別子を記号表に登録
            token = nextToken();
        } else {
            // 識別子のはずだから、エラーとする
            errorMissingId();
        }

        if (token.kind != Comma) {
            break;
        }

        token = nextToken();
    }

    token = checkGet(token, Semicolon);
}

void funcDecl() {
    /* funcDecl -> ident ( (ident | e) { ident // , } ) block ; */

    if (token.kind == Id) {
        // ( が来るはず
        token = checkGet(nextToken(), Lparen);

        while (1) {
            if (token.kind == Id) {
                /* 仮引数の識別子 */
                // TODO: 記号表に登録
                token = nextToken();
            } else {
                /* 仮引数なし */
                break;
            }

            if (token.kind != Comma) {
                // 複数の引数ではないため、終わり
                break;
            }

            token = nextToken();
        }

        // ブロックのコンパイル
        block();
        // ; が来るはず
        token = checkGet(token, Semicolon);
    } else {
        // 関数名が来るはずだから、エラー
        errorMissingId();
    }
}
