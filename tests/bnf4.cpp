#include "../src/ptg_internal.hpp"

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
    "<Number> := <Number><Digit> | <Digit>;"
    "<Digit> := '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';"
    ":";


static ParseToken token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '0') token_list[token_count++] = {TOKEN_0, &str[index], 1};
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
    token_list[token_count++] = {TOKEN_End, nullptr, 0};

   return parse(token_list, token_count, table, 0, nullptr, nullptr, 0);
}


int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    // FILE *f = fopen("input.dot", "w");
    // write_states_as_graph(f, state_list, state_count);
    // fclose(f);
    // print_table(table, lexer, state_count);

    assert_always(!parse_str("", table));
    assert_always(!parse_str("abcd", table));
    assert_always(parse_str("0", table));
    assert_always(parse_str("00", table));
    assert_always(parse_str("1", table));
    assert_always(parse_str("11", table));
    assert_always(parse_str("2492328501235823580", table));
    assert_always(parse_str("323123", table));
    assert_always(parse_str("6364859", table));
    assert_always(parse_str("123456", table));
    printf("Finished %s\n", __FILE__);
}