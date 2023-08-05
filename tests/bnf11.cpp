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





    test_str(table, true, "0++", "SHIFT, 2\nREDUCE, 2\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "+", "REDUCE, 3\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "", "REDUCE, 3\nACCEPT\n");
    test_str(table, false, "00", "SHIFT, 2\nINVALID, 0\nlookahead_lr_index: 1, state: 2\nUnexpected 0 token");

    fprintf(stderr, "Finished %s\n", __FILE__);
}