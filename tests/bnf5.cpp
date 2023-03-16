#include "../src/ptg_internal.hpp"
#include <assert.h>


enum Token
{
    TOKEN_a,
    TOKEN_End,
};


static const char *bnf_source =
    "TOKENS"
    "a;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>;"
    "<E> := ;"
    "<E> := 'a';"
    ":";



static ParseToken token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index], 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 1};

   return parse(token_list, token_count, table, 0, nullptr);
}




int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    assert(parse_str("", table));
    assert(parse_str("a", table));
    assert(!parse_str("aa", table));
    assert(!parse_str("aaa", table));
    assert(!parse_str("aaaa", table));
    assert(!parse_str("aaaa", table));
    assert(!parse_str("aaaa", table));
    assert(!parse_str("312321", table));
    assert(!parse_str("23123", table));
    assert(!parse_str("asfasd", table));

}