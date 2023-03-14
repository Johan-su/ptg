#include "../src/ptg.hpp"
#include <assert.h>


enum Token
{
    TOKEN_a,
    TOKEN_b,
    TOKEN_c,
    TOKEN_d,
    TOKEN_e,
    TOKEN_f,
    TOKEN_g,
    TOKEN_h,
    TOKEN_i,
    TOKEN_j,
    TOKEN_k,
    TOKEN_l,
    TOKEN_m,
    TOKEN_n,
    TOKEN_o,
    TOKEN_p,
    TOKEN_q,
    TOKEN_r,
    TOKEN_s,
    TOKEN_t,
    TOKEN_u,
    TOKEN_v,
    TOKEN_w,
    TOKEN_x,
    TOKEN_y,
    TOKEN_z,
    TOKEN_plus,
    TOKEN_minus,
    TOKEN_times,
    TOKEN_divide,
    TOKEN_equal,
    TOKEN_open,
    TOKEN_close,
    TOKEN_0,
    TOKEN_1,
    TOKEN_2,
    TOKEN_3,
    TOKEN_4,
    TOKEN_5,
    TOKEN_6,
    TOKEN_7,
    TOKEN_8,
    TOKEN_9,
    TOKEN_End,
};


static const char *bnf_source =
    "TOKENS"
    "a;"
    "b;"
    "c;"
    "d;"
    "e;"
    "f;"
    "g;"
    "h;"
    "i;"
    "j;"
    "k;"
    "l;"
    "m;"
    "n;"
    "o;"
    "p;"
    "q;"
    "r;"
    "s;"
    "t;"
    "u;"
    "v;"
    "w;"
    "x;"
    "y;"
    "z;"
    "+;"
    "-;"
    "*;"
    "/;"
    "=;"
    "(;"
    ");"
    "0;"
    "1;"
    "2;"
    "3;"
    "4;"
    "5;"
    "6;"
    "7;"
    "8;"
    "9;"
    "End;" 
    ":"
    "BNF"
    "<S> := <S'>;"
    "<S'> := <FuncDecl>;"
    "<S'> := <VarDecl>;"
    "<S'> := <E>;"
    "<S'> := ;"
    //
    "<FuncDecl> := <Id>'('<E>')' '=' <E>;"
    "<VarDecl> := <Id> '=' <E>;"
    "<Id> := 'a';"
    "<Id> := 'b';"
    "<Id> := 'c';"
    "<Id> := 'd';"
    "<Id> := 'e';"
    "<Id> := 'f';"
    "<Id> := 'g';"
    "<Id> := 'h';"
    "<Id> := 'i';"
    "<Id> := 'j';"
    "<Id> := 'k';"
    "<Id> := 'l';"
    "<Id> := 'm';"
    "<Id> := 'n';"
    "<Id> := 'o';"
    "<Id> := 'p';"
    "<Id> := 'q';"
    "<Id> := 'r';"
    "<Id> := 's';"
    "<Id> := 't';"
    "<Id> := 'u';"
    "<Id> := 'v';"
    "<Id> := 'w';"
    "<Id> := 'x';"
    "<Id> := 'y';"
    "<Id> := 'z';"
    //
    "<E> := '('<E>')';"
    "<E> := <Number>;"
    "<E> := <Var>;"
    "<E> := <FuncCall>;"
    //
    "<E> := '-'<E>;"
    "<E> := '+'<E>;"
    "<E> := <E> '/' <E>;"
    "<E> := <E> '*' <E>;"
    "<E> := <E> '-' <E>;"
    "<E> := <E> '+' <E>;"
    //
    "<FuncCall> := <Id>'('<E>')';"
    "<Var> := <Id>;"
    "<Number> := <Number><Digit>;"
    "<Number> := <Digit>;"
    //
    "<Digit> := '0';"
    "<Digit> := '1';"
    "<Digit> := '2';"
    "<Digit> := '3';"
    "<Digit> := '4';"
    "<Digit> := '5';"
    "<Digit> := '6';"
    "<Digit> := '7';"
    "<Digit> := '8';"
    "<Digit> := '9';"
    ":";









static long long token_list[512] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table, Lexer *lex)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {


        if (str[index] == 'a') token_list[token_count++] = TOKEN_a;
        else if (str[index] == 'b') token_list[token_count++] = TOKEN_b;
        else if (str[index] == 'c') token_list[token_count++] = TOKEN_c;
        else if (str[index] == 'd') token_list[token_count++] = TOKEN_d;
        else if (str[index] == 'e') token_list[token_count++] = TOKEN_e;
        else if (str[index] == 'f') token_list[token_count++] = TOKEN_f;
        else if (str[index] == 'g') token_list[token_count++] = TOKEN_g;
        else if (str[index] == 'h') token_list[token_count++] = TOKEN_h;
        else if (str[index] == 'i') token_list[token_count++] = TOKEN_i;
        else if (str[index] == 'j') token_list[token_count++] = TOKEN_j;
        else if (str[index] == 'k') token_list[token_count++] = TOKEN_k;
        else if (str[index] == 'l') token_list[token_count++] = TOKEN_l;
        else if (str[index] == 'm') token_list[token_count++] = TOKEN_m;
        else if (str[index] == 'n') token_list[token_count++] = TOKEN_n;
        else if (str[index] == 'o') token_list[token_count++] = TOKEN_o;
        else if (str[index] == 'p') token_list[token_count++] = TOKEN_p;
        else if (str[index] == 'q') token_list[token_count++] = TOKEN_q;
        else if (str[index] == 'r') token_list[token_count++] = TOKEN_r;
        else if (str[index] == 's') token_list[token_count++] = TOKEN_s;
        else if (str[index] == 't') token_list[token_count++] = TOKEN_t;
        else if (str[index] == 'u') token_list[token_count++] = TOKEN_u;
        else if (str[index] == 'v') token_list[token_count++] = TOKEN_v;
        else if (str[index] == 'w') token_list[token_count++] = TOKEN_w;
        else if (str[index] == 'x') token_list[token_count++] = TOKEN_x;
        else if (str[index] == 'y') token_list[token_count++] = TOKEN_y;
        else if (str[index] == 'z') token_list[token_count++] = TOKEN_z;
        else if (str[index] == '+') token_list[token_count++] = TOKEN_plus;
        else if (str[index] == '-') token_list[token_count++] = TOKEN_minus;
        else if (str[index] == '*') token_list[token_count++] = TOKEN_times;
        else if (str[index] == '/') token_list[token_count++] = TOKEN_divide;
        else if (str[index] == '=') token_list[token_count++] = TOKEN_equal;
        else if (str[index] == '(') token_list[token_count++] = TOKEN_open;
        else if (str[index] == ')') token_list[token_count++] = TOKEN_close;
        else if (str[index] == '0') token_list[token_count++] = TOKEN_0;
        else if (str[index] == '1') token_list[token_count++] = TOKEN_1;
        else if (str[index] == '2') token_list[token_count++] = TOKEN_2;
        else if (str[index] == '3') token_list[token_count++] = TOKEN_3;
        else if (str[index] == '4') token_list[token_count++] = TOKEN_4;
        else if (str[index] == '5') token_list[token_count++] = TOKEN_5;
        else if (str[index] == '6') token_list[token_count++] = TOKEN_6;
        else if (str[index] == '7') token_list[token_count++] = TOKEN_7;
        else if (str[index] == '8') token_list[token_count++] = TOKEN_8;
        else if (str[index] == '9') token_list[token_count++] = TOKEN_9;
        else return false;
    }
    token_list[token_count++] = TOKEN_End;

   return parse(token_list, token_count, table, lex);
}




int main(void)
{
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    ParseTable *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);


    assert(parse_str("", table, lexer));
    assert(parse_str("1+1", table, lexer));
    assert(parse_str("1*1", table, lexer));
    assert(parse_str("1/1", table, lexer));
    assert(parse_str("1-1", table, lexer));
    assert(parse_str("-1+1", table, lexer));
    assert(parse_str("--1*1", table, lexer));
    assert(parse_str("a=f(g)*44358340834683406*555543431265345348505+53492358+0/6-86546546546+h(c)", table, lexer));
}