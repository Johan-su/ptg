#include "../src/ptg.hpp"
#include <assert.h>

enum Token
{
    TOKEN_plus,
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
    "\"+\";"
    "\"0\";"
    "\"1\";"
    "\"2\";"
    "\"3\";"
    "\"4\";"
    "\"5\";"
    "\"6\";"
    "\"7\";"
    "\"8\";"
    "\"9\";"
    "\"End\";"
    ":"
    "BNF"
    "<S> := <E>;"
    "<E> := <E>'+'<E>;"
    "<E> := '0';"
    "<E> := '1';"
    "<E> := '2';"
    "<E> := '3';"
    "<E> := '4';"
    "<E> := '5';"
    "<E> := '6';"
    "<E> := '7';"
    "<E> := '8';"
    "<E> := '9';"
    "<E> := ;"
    ":";


static long long token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table, Lexer *lex)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = TOKEN_plus;
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
    assert(parse_str("5+1", table, lexer));
    assert(parse_str("5+4+3+2+1", table, lexer));
    assert(parse_str("+4+3+2+", table, lexer));
    assert(parse_str("+", table, lexer));
    assert(!parse_str("00", table, lexer));
    assert(!parse_str("5120412505721057214901279+4", table, lexer));
}