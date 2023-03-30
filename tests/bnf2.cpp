#include "../src/ptg_internal.hpp"
#include <assert.h>


enum Token
{
    TOKEN_plus,
    TOKEN_a,
    TOKEN_b,
    TOKEN_End,
};


static const char *bnf_source = 
    "TOKENS"
    "+;"
    "a;"
    "b;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>;"
    "<E> := <T>'+'"
        " | <T>;"
    "<T> := <ID>;"
    "<ID> := 'a'|'b';"
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
        else if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index], 1};
        else if (str[index] == 'b') token_list[token_count++] = {TOKEN_b, &str[index], 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0};

   return parse(token_list, token_count, table, 0, nullptr, nullptr, 0);
}

int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    assert(parse_str("a+", table));
    assert(parse_str("b+", table));
    assert(!parse_str("", table));
    assert(parse_str("a", table));
    assert(parse_str("b", table));
    printf("Finished %s\n", __FILE__);}



