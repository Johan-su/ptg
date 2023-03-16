#include "../src/ptg_internal.hpp"
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
    "+;"
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


static ParseToken token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1};
        else if (str[index] == '0') token_list[token_count++] = {TOKEN_0, &str[index], 1};
        else if (str[index] == '1') token_list[token_count++] = {TOKEN_1, &str[index], 1};
        else if (str[index] == '2') token_list[token_count++] = {TOKEN_2, &str[index], 1};
        else if (str[index] == '3') token_list[token_count++] = {TOKEN_3, &str[index], 1};
        else if (str[index] == '4') token_list[token_count++] = {TOKEN_4, &str[index], 1};
        else if (str[index] == '5') token_list[token_count++] = {TOKEN_5, &str[index], 1};
        else if (str[index] == '6') token_list[token_count++] = {TOKEN_6, &str[index], 1};
        else if (str[index] == '7') token_list[token_count++] = {TOKEN_7, &str[index], 1};
        else if (str[index] == '8') token_list[token_count++] = {TOKEN_8, &str[index], 1};
        else if (str[index] == '9') token_list[token_count++] = {TOKEN_9, &str[index], 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 1};

   return parse(token_list, token_count, table, 0, nullptr);
}



int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    assert(parse_str("", table));
    assert(parse_str("1+1", table));
    assert(parse_str("5+1", table));
    assert(parse_str("5+4+3+2+1", table));
    assert(parse_str("+4+3+2+", table));
    assert(parse_str("+", table));
    assert(!parse_str("00", table));
    assert(!parse_str("5120412505721057214901279+4", table));
}