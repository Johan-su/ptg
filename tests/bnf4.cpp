#include "../src/ptg.hpp"
#include <assert.h>

enum Token
{
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
    "<S> := <Number>;"
    "<Number> := <Number><Digit>;"
    "<Number> := <Digit>;"
    "<Digit> := '0';"
    "<Digit> := '1';"
    "<Digit> := '2';"
    "<Digit> := '3';"
    "<Digit> := '4';"
    "<Digit> := '5';"
    "<Digit> := '6';"
    "<Digit> := '7';"
    "<Digit> := '8';"
    "<Digit> := '9';"
    ":";


static long long token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table, Lexer *lex)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '0') token_list[token_count++] = TOKEN_0;
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

    // FILE *f = fopen("input.dot", "w");
    // write_states_as_graph(f, state_list, state_count);
    // fclose(f);
    // print_table(table, lexer, state_count);

    assert(!parse_str("", table, lexer));
    assert(!parse_str("abcd", table, lexer));
    assert(parse_str("0", table, lexer));
    assert(parse_str("00", table, lexer));
    assert(parse_str("1", table, lexer));
    assert(parse_str("11", table, lexer));
    assert(parse_str("2492328501235823580", table, lexer));
    assert(parse_str("323123", table, lexer));
    assert(parse_str("6364859", table, lexer));
    assert(parse_str("123456", table, lexer));

}