#include "../src/ptg.hpp"
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



static bool parse_str(const char *str, ParseTable *table, Lexer *lex)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index]};
        else if (str[index] == 't') token_list[token_count++] = {TOKEN_t, &str[index]};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr};

   return parse(token_list, token_count, table, lex);
}


int main(void)
{
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    ParseTable *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);




    assert(!parse_str("", table, lexer));
    assert(parse_str("a", table, lexer));
    assert(!parse_str("t", table, lexer));
    assert(parse_str("at", table, lexer));
    assert(parse_str("att", table, lexer));
    assert(parse_str("atttttttttttttttttttttttttttt", table, lexer));
    assert(!parse_str("aaaaat", table, lexer));
    assert(!parse_str("ata", table, lexer));
    assert(!parse_str("ataatatatatatatatatattttatatataat", table, lexer));
}