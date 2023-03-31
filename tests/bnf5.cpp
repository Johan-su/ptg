#include "../src/ptg_internal.hpp"

#include <string.h>

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
    "<E> := 'a' | ;"
    ":";



static ParseToken token_list[128] = {};
static unsigned int token_count = 0;

static char msg[2048] = {};

static bool parse_str(const char *str, ParseTable *table)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index], 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0};


    memset(msg, 0, sizeof(msg));
    return parse(token_list, token_count, table, 0, nullptr, msg, sizeof(msg));
}




int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    // print_table(table);

    assert(parse_str("", table));
    assert(parse_str("a", table));
    assert(!parse_str("aa", table));
    printf("%s\n", msg);
    assert(!parse_str("aaa", table));
    assert(!parse_str("aaaa", table));
    assert(!parse_str("aaaa", table));
    assert(!parse_str("aaaa", table));
    assert(!parse_str("312321", table));
    assert(!parse_str("23123", table));
    assert(!parse_str("asfasd", table));
    printf("Finished %s\n", __FILE__);}