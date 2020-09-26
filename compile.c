#include "codegen.h"
#include "getSource.h"
#include <stdio.h>

#ifndef TBL
#define TBL
#include "table.h"
#endif

/* このエラー数以下なら、OK */
#define MINERROR 3
/* 各ブロックの最初の変数のアドレス
    XXX: なせ、2？
*/
#define FIRSTADDR 2

static void block();      // ブロック
static void constDecl();  // 定数宣言
static void varDecl();    // 変数宣言
static void funcDecl();   // 関数宣言
static void statement();  // 文
static void condition();  // 条件
static void expression(); // 式
static void term();       // 式の項
static void factor();     // 式の因子

// トークンt が statement の先頭のトークンか？
static int isStBeginKey(Token t);

// 次のトークン
Token token;

int compile() {
    int errN;

    printf("start compilation\n");
    initSource();
    // 最初の token 読み出し
    token = nextToken();
    // ブロックの開始 (いろいろ初期化)
    blockBegin(FIRSTADDR);
    block(0);
    finalSource();
    // エラー数の取得
    errN = errorN();
    if (errN != 0) {
        printf("%d errors\n", errN);
    }
    // 目的コードの表示
    listCode();
    // 最大エラー数よりも少ないか？
    return errN < MINERROR;
}

void block(int pIndex) {
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
    /* changeV(pIndex, nextCode()); */
    statement();

    blockEnd();
}

/*
   定数宣言
   constDecl -> const { ident = number // , } ;
*/
void constDecl() {

    /* block の部分で、 const は認識しているため、ここではチェックしない*/

    Token temp;

    while (1) {
        /* { ident = number // , } */

        // ident
        if (token.kind == Id) {
            // 現在のトークンの種類をセット
            setIdKind(constId);
            // 変数名を保持
            temp = token;
            // = のはず
            token = checkGet(nextToken(), Equal);

            if (token.kind == Num) {
                // 定数名と値を登録
                enterTconst(temp.u.id, token.u.value);
            } else {
                // 型エラー
                errorType("number");
            }
            token = nextToken();
        } else {
            // 仮の識別子を insert (phrase level recovery)
            errorMissingId();
        }

        if (token.kind != Comma) {

            if (token.kind == Id) {
                // Id Id となっていたら、, の入れ忘れだと思うから、入れてあげる
                // phrase level recovery
                errorInsert(Comma);
                continue;
            } else {
                break;
            }
        }

        // const a = 10, b = 20;
        //             ^
        // , であるため、次の識別子を読みすすめる
        token = nextToken();
    }

    // ; のはず
    token = checkGet(token, Semicolon);
}

/*
   変数宣言
    varDecl -> var { ident // , } ;
*/
void varDecl() {

    while (1) {
        if (token.kind == Id) {
            setIdKind(varId);
            // 識別子を記号表に登録
            enterTvar(token.u.id);
            token = nextToken();
        } else {
            // 識別子のはずだから、insert (phrase level recovery)
            errorMissingId();
        }

        if (token.kind != Comma) {
            if (token.kind == Id) {
                // , の入れ忘れ  (phrase level recovery)
                errorInsert(Comma);
                continue;
            } else {
                break;
            }
        }

        // var a, b;
        //      ^
        // , であるため、次の識別子を読みすすめる
        token = nextToken();
    }

    // ; のはず
    token = checkGet(token, Semicolon);
}

/*
   関数宣言
   funcDecl -> function ident ( (ident | e) { ident // , } ) block ;
*/
void funcDecl() {

    int fIndex;
    if (token.kind != Id) {
        // 関数名が来るはずだから、関数名を insert
        errorMissingId();
    } else {
        setIdKind(funcId);

        fIndex = enterTfunc(token.u.id, nextCode());

        // ( が来るはず
        token = checkGet(nextToken(), Lparen);
        // パラメータのレベルは、関数の本体と同じブロックになる
        //  -> そのブロック内で宣言された局所変数と同じってこと！！
        blockBegin(FIRSTADDR);

        while (1) {
            if (token.kind == Id) {
                /* 仮引数の識別子 */
                setIdKind(parId);
                // パラメータ名を名前表に追加
                enterTpar(token.u.id);
                token = nextToken();
            } else {
                /* 仮引数なし */
                break;
            }

            if (token.kind != Comma) {
                if (token.kind == Id) {
                    // phrase level recovery
                    errorInsert(Comma);
                    continue;
                } else {
                    // 複数の引数ではないため、終わり
                    // f1(a, b | ) のように , でもなく、Id でもないなら、終わり
                    break;
                }
            }

            token = nextToken();
        }
        // ) のはず
        token = checkGet(token, Rparen);
        // パラメータの部分が終わったから、ブロックの終わり
        endpar();

        if (token.kind == Semicolon) {
            // 誤り回復
            // XXX: なぜ、ここで ; を判定している？
            //  -> block の前に ;
            //  を書いていたら、消すことで誤りから回復している？
            errorDelete();
            token = nextToken();
        }

        // ブロックのコンパイル
        block(fIndex);
        // ; が来るはず
        token = checkGet(token, Semicolon);
    }
}

/*
    文
*/
void statement() {

    int tIndex;
    KindT k;

    // うまいこと、break; をしないで活用する

    // panic mode recovery を使っている？？
    // Follow(statement) = { . ; end }
    while (1) {

        switch (token.kind) {
        case Id: /* ident := <expression> */
            tIndex = searchT(token.u.id, varId);
            setIdKind(k = kindT(tIndex));

            if (k != varId && k != parId) {
                // 未定義の変数なら、エラー
                // 変数でもなく、パラメータ名でもない
                errorType("var/par");
            }
            // := が来るはず
            token = checkGet(nextToken(), Assign);
            expression();
            // store 命令のコードを生成
            genCodeT(sto, tIndex);
            return;

        case If: /* if <condition> then <statement> */
            token = nextToken();
            condition();
            // then のはず
            token = checkGet(token, Then);
            statement();
            return;

        case Ret: /* return <expression> */
            token = nextToken();
            expression();
            return;

        case Begin: /* begin <statement> { ; <statement> } end */

            // <statement> { ; <statement> }
            token = nextToken();
            while (1) {
                statement();

                // { ; <statement> } の部分
                while (1) {
                    if (token.kind == Semicolon) {
                        token = nextToken();
                        // 次の statement を読み出すため
                        break;
                    }
                    if (token.kind == End) {
                        // ; ではなく、end が来たら、終わり
                        token = nextToken();
                        return;
                    }

                    if (isStBeginKey(token)) {
                        // 次が、statement だから、; の入れ忘れだとわかる
                        // First(statement)  ... statement の First 集合
                        errorInsert(Semicolon);
                        // token が次の statement
                        // 次の statement の最初のトークンだから、
                        // nextToken()は呼び出さない
                        break;
                    }
                    // panic mode recovery
                    errorDelete();
                    token = nextToken();
                }
            }

            // あえて、return せずに、下の case を利用する
            // return;

        case While: /* while <condition> do <statement> */
            token = nextToken();
            condition();
            // Do のはず
            token = checkGet(token, Do);
            statement();
            return;

        case Write: /* write <expression> */
            token = nextToken();
            expression();
            return;

        case WriteLn: /* writeln */
            token = nextToken();
            return;

        case End:
        case Semicolon:
            // TODO: これは何？？
            // 終わりを示している？
            return;

        default:
            // 今読んだトークンを読み捨てる
            // panic mode recovery を使っている
            // Follow(statement) = { . ; end }
            errorDelete();
            token = nextToken();
            // 誤り回復したため、継続できる
            continue;
        }
    }
}

/*
    panic mode recovery のためのもの
    First(statement) - { e } を表している
*/
int isStBeginKey(Token t) {
    // First(<statement>) - { e } なら、; を忘れたことになる
    switch (t.kind) {
    case Begin:
    case If:
    case While:
    case Ret:
    case Write:
    case WriteLn:
        return 1;
    default:
        return 0;
    }
}

/*
   条件
    condition -> odd <expression>
               | <expression> ( = | <> | > | < | <= | >= ) <expression>
*/
void condition() {
    if (token.kind == Odd) {
        token = nextToken();
        expression();
    } else {

        expression();

        // ( = | <> | > | < | <= | >= ) の部分
        switch (token.kind) {
        /* 演算子 */
        case Equal: // =
        case NotEq: // <>
        case Gtr:   // <
        case Lss:   // >
        case GtrEq: // <=
        case LssEq: // >=
            break;
        default:
            // 誤り回復？？
            errorType("rel-op");
            break;
        }

        token = nextToken();
        expression();
    }
}

/*
    式
    expression -> ( + | - | e ) <term> { ( + | - ) <term> }
*/
void expression() {
    // ( + | - | e ) <term>
    if (token.kind == Plus || token.kind == Minus) {
        token = nextToken();
        term();
    } else {
        term();
    }

    // { ( + | - ) <term> }
    while (token.kind == Plus || token.kind == Minus) {
        // まずは読みすすめる
        token = nextToken();
        term();
    }
}

/*
    term -> <factor> { ( * | / ) <factor> }
*/
void term() {
    factor();

    while (token.kind == Mult || token.kind == Div) {
        token = nextToken();
        factor();
    }
}

/*
   因子
    factor -> ident
            | number
            | ident '(' ( e | <expression> { , <expression> } ) ')'
            | '(' <expression> ')'

    '(' と ')' は原子プログラムにそのまま現れる
    ident は、プログラムの意味で見分ける
*/
void factor() {

    int tIndex, i;
    KindT k;

    if (token.kind == Num) {
        /* number */
        token = nextToken();
    } else if (token.kind == Lparen) {
        /* '(' <expression> ')' */
        token = nextToken();
        expression();
        // ) のはず
        token = checkGet(token, Rparen);
    } else if (token.kind == Id) {
        // ident か ident '(' ( e | <expression> { , <expression> } ) ')'
        tIndex = searchT(token.u.id, varId);
        k = kindT(tIndex);
        switch (k) {

        /* 変数名 or パラメータ名 */
        case varId:
        case parId:
            token = nextToken();
            break;

        /* 定数 */
        case constId:
            token = nextToken();
            break;

        /* 関数呼び出し */
        case funcId:
            token = nextToken();

            if (token.kind == Lparen) {
                // 実引数の個数
                i = 0;
                token = nextToken();
                if (token.kind != Rparen) {
                    // 引数ありの場合
                    for (;;) {
                        i++;
                        expression();
                        // もし、',' なら、実引数が続く
                        if (token.kind == Comma) {
                            token = nextToken();
                            continue;
                        }
                        // ')' のはず
                        // チェック & 誤り回復
                        token = checkGet(token, Rparen);
                        break;
                    }
                } else {
                    // 引数なしの関数呼び出しってこと
                    token = nextToken();
                }

                // もし、関数定義のパラメータと合わなかった場合、エラー
                if (pars(tIndex) != i) {
                    printf("%d", pars(tIndex));
                    errorMessage("#par");
                }
            } else {
                // おかしい...
                // 誤り回復
                // '()' を insert
                errorInsert(Lparen);
                errorInsert(Rparen);
            }
            break;
        }
    }

    // TODO: まだ、因子が続くなら、エラー
    /* switch (token.kind) { */
    /* case Id: */
    /* case Num: */
    /* case Lparen: */
    /*     errorMissingOp(); */
    /*     factor(); */
    /* default: */
    /*     return; */
    /* } */
}
