
/* 命令語のコード */
typedef enum codes { lit, opr, lod, sto, ret, ict, jmp, jpc } OpCode;

/* 演算命令の演算部のコード */
typedef enum ops {
    odd,
    add,
    sub,
    mul,
    div,
    wrt,
    wrl,
    eq,   // =
    ls,   // <
    gr,   // >
    neq,  // <>
    lseq, // <=
    greq, // >=
} Operator;

int nextCode();

/* 命令形式1
    op: 機能部
    ti: 名前表で変数が格納されている番地 (table index)
*/
int genCodeT(OpCode op, int tblIdx);
/* 命令形式2
    op: 機能部
    value: 値部 (実際の数値 or ジャンプ先の番地)
*/
int genCodeV(OpCode op, int value);

/* p: 演算命令 */
int genCodeO(Operator p);

/* 戻り命令 */
int genCodeR();

/* i のコードのジャンプ先を現在の cIndex でバックパッチングする */
void backPatch(int i);

/* 目的コードの表示 */
void listCode();
void execute();
