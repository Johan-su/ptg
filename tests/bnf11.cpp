#include "../src/ptg_internal.hpp"
#include <string.h>



enum Token
{
    TOKEN_plus,
    TOKEN_zero,
    TOKEN_End,
};



static const char *bnf_source =
    "TOKENS"
    "+;"
    "0;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>'End';"
    "<E> := <E>'+'<E> | "
         "'0' | ;"
    ":";


static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_number(char c)
{
    return c >= '0' && c <= '9';
}


static bool is_whitespace(char c)
{
    switch (c)
    {
        case ' ': return true;
        case '\r': return true;
        case '\n': return true;
        case '\t': return true;
    
        default: return false;
    }
}


static ParseToken token_list[4096] = {};
static unsigned int token_count = 0;

static char msg[2048];

static bool parse_str_to_token_list(const char *str)
{
    token_count = 0;
    for (U32 index = 0; str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1, 1};
        else if (str[index] == '0') token_list[token_count++] = {TOKEN_zero, &str[index], 1, 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};

    return true;
}







#include <stdlib.h>

#include "common_test.cpp"

int main(void)
{
    ParseTable *table = create_and_print_table(bnf_source);





    test_str("0++", true, "", table);

    fprintf(stderr, "%.*s\n", (int)sizeof(msg), msg);
    fprintf(stderr, "Finished %s\n", __FILE__);
}