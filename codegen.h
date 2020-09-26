
/* 命令語のコード */
typedef enum codes {
    opr,
    lod,
    sto,
    jmp
} OpCode;

int nextCode();

/* 命令語の生成
    op: 機能部
    ti: 名前表で変数が格納されている番地 (table index)
*/
int genCodeT(OpCode op, int tblIdx);

/* 目的コードの表示 */
void listCode();
