#include "../src/ptg_internal.hpp"
#include <assert.h>


enum Token
{
    TOKEN_a,
    TOKEN_t,
    TOKEN_End,
};

const char *bnf_source = 
    "TOKENS"
    "a;"
    "t;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>;"
    "<E> := 'a';"
    "<E> := <E>'t';"
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
        else if (str[index] == 't') token_list[token_count++] = {TOKEN_t, &str[index], 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0};

   return parse(token_list, token_count, table, 0, nullptr);
}


int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    // print_table(table);



    assert(!parse_str("", table));
    assert(parse_str("a", table));
    assert(!parse_str("t", table));
    assert(parse_str("at", table));
    assert(parse_str("att", table));
    assert(parse_str("atttttttttttttttttttttttttttt", table));
    assert(!parse_str("aaaaat", table));
    assert(!parse_str("ata", table));
    assert(!parse_str("ataatatatatatatatatattttatatataat", table));
    printf("Finished %s\n", __FILE__);
}