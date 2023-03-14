#include "../src/ptg.hpp"
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



static long long token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table, Lexer *lex)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == 'a') token_list[token_count++] = TOKEN_a;
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
    assert(parse_str("a", table, lexer));
    assert(!parse_str("aa", table, lexer));
    assert(!parse_str("aaa", table, lexer));
    assert(!parse_str("aaaa", table, lexer));
    assert(!parse_str("aaaa", table, lexer));
    assert(!parse_str("aaaa", table, lexer));
    assert(!parse_str("312321", table, lexer));
    assert(!parse_str("23123", table, lexer));
    assert(!parse_str("asfasd", table, lexer));

}