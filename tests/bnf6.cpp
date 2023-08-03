#include "../src/ptg_internal.hpp"

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
    "<S> := <E>'End';"
    "<E> := <T>'+'<E> | <T>;"
    "<T> := 'I';"
    ":";

static ParseToken token_list[128] = {};
static unsigned int token_count = 0;

static char error_msg[2048] = {};

static bool parse_str(const char *str, ParseTable *table)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1, 1};
        else if (str[index] == 'I') token_list[token_count++] = {TOKEN_I, &str[index], 1, 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};

   bool success = parse(token_list, token_count, table, PRINT_EVERY_PARSE_STEP_FLAG, nullptr, error_msg, sizeof(error_msg));
   fprintf(stderr, "%s\n", error_msg);
   return success;
}


int main(void)
{
    ParseTable *table = create_parse_table_from_bnf(bnf_source);

    if (table == nullptr)
    {
        fprintf(stderr, "%s\n", get_last_error());
    }

    assert_always(table != nullptr);

    print_table(table);

    assert_always(!parse_str("", table));
    assert_always(parse_str("I+I", table));
    assert_always(parse_str("I+I+I+I+I+I", table));
    assert_always(!parse_str("I+I+I+I+I+", table));
    printf("Finished %s\n", __FILE__);
}