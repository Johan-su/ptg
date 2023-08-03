#include "../src/ptg_internal.hpp"
#include <string.h>

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
    "<S'> := <FuncDecl> | <VarDecl> | <E> | ;"
    //
    "<FuncDecl> := <Id>'('<E>')' '=' <E>;"
    "<VarDecl> := <Id> '=' <E>;"
    "<Id> := 'a' | "
            "'b' | "
            "'c' | "
            "'d' | "
            "'e' | "
            "'f' | "
            "'g' | "
            "'h' | "
            "'i' | "
            "'j' | "
            "'k' | "
            "'l' | "
            "'m' | "
            "'n' | "
            "'o' | "
            "'p' | "
            "'q' | "
            "'r' | "
            "'s' | "
            "'t' | "
            "'u' | "
            "'v' | "
            "'w' | "
            "'x' | "
            "'y' | "
            "'z';"
    //
    "<E> := '('<E>')' | "
            "<Number> | "
            "<Var> | "
            "<FuncCall> | "
            "'-'<E> | "
            "'+'<E> | "
            "<E> '/' <E> | "
            "<E> '*' <E> | "
            "<E> '-' <E> | "
            "<E> '+' <E>;"
    //
    "<FuncCall> := <Id>'('<E>')';"
    "<Var> := <Id>;"
    "<Number> := <Number><Digit> | <Digit>;"
    //
    "<Digit> := '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';"
    ":";









static ParseToken token_list[512] = {};
static unsigned int token_count = 0;

static char err_msg[5000] = {'\0'};

static bool parse_str(const char *str, ParseTable *table, Expr **out_tree)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index], 1, 4};
        else if (str[index] == 'b') token_list[token_count++] = {TOKEN_b, &str[index], 1, 4};
        else if (str[index] == 'c') token_list[token_count++] = {TOKEN_c, &str[index], 1, 4};
        else if (str[index] == 'd') token_list[token_count++] = {TOKEN_d, &str[index], 1, 4};
        else if (str[index] == 'e') token_list[token_count++] = {TOKEN_e, &str[index], 1, 4};
        else if (str[index] == 'f') token_list[token_count++] = {TOKEN_f, &str[index], 1, 4};
        else if (str[index] == 'g') token_list[token_count++] = {TOKEN_g, &str[index], 1, 4};
        else if (str[index] == 'h') token_list[token_count++] = {TOKEN_h, &str[index], 1, 4};
        else if (str[index] == 'i') token_list[token_count++] = {TOKEN_i, &str[index], 1, 4};
        else if (str[index] == 'j') token_list[token_count++] = {TOKEN_j, &str[index], 1, 4};
        else if (str[index] == 'k') token_list[token_count++] = {TOKEN_k, &str[index], 1, 4};
        else if (str[index] == 'l') token_list[token_count++] = {TOKEN_l, &str[index], 1, 4};
        else if (str[index] == 'm') token_list[token_count++] = {TOKEN_m, &str[index], 1, 4};
        else if (str[index] == 'n') token_list[token_count++] = {TOKEN_n, &str[index], 1, 4};
        else if (str[index] == 'o') token_list[token_count++] = {TOKEN_o, &str[index], 1, 4};
        else if (str[index] == 'p') token_list[token_count++] = {TOKEN_p, &str[index], 1, 4};
        else if (str[index] == 'q') token_list[token_count++] = {TOKEN_q, &str[index], 1, 4};
        else if (str[index] == 'r') token_list[token_count++] = {TOKEN_r, &str[index], 1, 4};
        else if (str[index] == 's') token_list[token_count++] = {TOKEN_s, &str[index], 1, 4};
        else if (str[index] == 't') token_list[token_count++] = {TOKEN_t, &str[index], 1, 4};
        else if (str[index] == 'u') token_list[token_count++] = {TOKEN_u, &str[index], 1, 4};
        else if (str[index] == 'v') token_list[token_count++] = {TOKEN_v, &str[index], 1, 4};
        else if (str[index] == 'w') token_list[token_count++] = {TOKEN_w, &str[index], 1, 4};
        else if (str[index] == 'x') token_list[token_count++] = {TOKEN_x, &str[index], 1, 4};
        else if (str[index] == 'y') token_list[token_count++] = {TOKEN_y, &str[index], 1, 4};
        else if (str[index] == 'z') token_list[token_count++] = {TOKEN_z, &str[index], 1, 4};
        else if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1, 4};
        else if (str[index] == '-') token_list[token_count++] = {TOKEN_minus, &str[index], 1, 4};
        else if (str[index] == '*') token_list[token_count++] = {TOKEN_times, &str[index], 1, 4};
        else if (str[index] == '/') token_list[token_count++] = {TOKEN_divide, &str[index], 1, 4};
        else if (str[index] == '=') token_list[token_count++] = {TOKEN_equal, &str[index], 1, 4};
        else if (str[index] == '(') token_list[token_count++] = {TOKEN_open, &str[index], 1, 4};
        else if (str[index] == ')') token_list[token_count++] = {TOKEN_close, &str[index], 1, 4};
        else if (str[index] == '0') token_list[token_count++] = {TOKEN_0, &str[index], 1, 4};
        else if (str[index] == '1') token_list[token_count++] = {TOKEN_1, &str[index], 1, 4};
        else if (str[index] == '2') token_list[token_count++] = {TOKEN_2, &str[index], 1, 4};
        else if (str[index] == '3') token_list[token_count++] = {TOKEN_3, &str[index], 1, 4};
        else if (str[index] == '4') token_list[token_count++] = {TOKEN_4, &str[index], 1, 4};
        else if (str[index] == '5') token_list[token_count++] = {TOKEN_5, &str[index], 1, 4};
        else if (str[index] == '6') token_list[token_count++] = {TOKEN_6, &str[index], 1, 4};
        else if (str[index] == '7') token_list[token_count++] = {TOKEN_7, &str[index], 1, 4};
        else if (str[index] == '8') token_list[token_count++] = {TOKEN_8, &str[index], 1, 4};
        else if (str[index] == '9') token_list[token_count++] = {TOKEN_9, &str[index], 1, 4};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};

    memset(err_msg, 0, sizeof(err_msg));
    bool result = parse(token_list, token_count, table, 0, out_tree, err_msg, sizeof(err_msg));
    printf("%s", err_msg);
    return result;
}



static const char *create_wide_string(const char *str)
{
    Usize len = str_len(str);
    char *new_str = (char *)malloc((len + 1) * 4);

    for (Usize i = 0; i < len; ++i)
    {
        new_str[i] = str[i];
        new_str[i + 1] = '\0';
        new_str[i + 2] = '\0';
        new_str[i + 3] = '\0';
    }
    return new_str;
}

int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);


    Expr *tree;

    assert_always(parse_str(create_wide_string(""), table, nullptr));
    assert_always(!parse_str(create_wide_string("5a"), table, nullptr));
    assert_always(parse_str(create_wide_string("1+1"), table, nullptr));
    assert_always(parse_str(create_wide_string("1*1"), table, nullptr));
    assert_always(parse_str(create_wide_string("1/1"), table, nullptr));
    assert_always(parse_str(create_wide_string("1-1"), table, nullptr));
    assert_always(parse_str(create_wide_string("-1+1"), table, nullptr));
    assert_always(parse_str(create_wide_string("--1*1"), table, nullptr));
    assert_always(parse_str(create_wide_string("a=f(g)*44358340834683406*555543431265345348505+53492358+0/6-86546546546+h(c)"), table, &tree));
    graphviz_from_syntax_tree("./input.dot", tree);
    printf("Finished %s\n", __FILE__);
}