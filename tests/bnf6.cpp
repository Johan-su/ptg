#include "../src/ptg.hpp"
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

static bool parse_str(const char *str, ParseTable *table, Lexer *lex)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index]};
        else if (str[index] == 'I') token_list[token_count++] = {TOKEN_I, &str[index]};
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

    // print_table(table);

    assert(!parse_str("", table, lexer));
    assert(parse_str("I+I", table, lexer));
    assert(parse_str("I+I+I+I+I+I", table, lexer));
    assert(!parse_str("I+I+I+I+I+", table, lexer));
}