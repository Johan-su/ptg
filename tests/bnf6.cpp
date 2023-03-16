#include "../src/ptg_internal.hpp"
#include <assert.h>

enum Token
{
    TOKEN_plus,
    TOKEN_I,
    TOKEN_End,
};


// example in https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf
static const char *bnf_source = 
    "TOKENS"
    "+;"
    "I;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>;"
    "<E> := <T>'+'<E>;"
    "<E> := <T>;"
    "<T> := 'I';"
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
        else if (str[index] == 'I') token_list[token_count++] = {TOKEN_I, &str[index], 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 1};

   return parse(token_list, token_count, table, 0, nullptr);
}


int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    // print_table(table);

    assert(!parse_str("", table));
    assert(parse_str("I+I", table));
    assert(parse_str("I+I+I+I+I+I", table));
    assert(!parse_str("I+I+I+I+I+", table));
}