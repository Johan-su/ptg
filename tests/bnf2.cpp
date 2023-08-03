#include "../src/ptg_internal.hpp"


enum Token
{
    TOKEN_plus,
    TOKEN_a,
    TOKEN_b,
    TOKEN_End,
};


static const char *bnf_source = 
    "TOKENS"
    "+;"
    "a;"
    "b;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>'End';"
    "<E> := <T>'+'"
        " | <T>;"
    "<T> := <ID>;"
    "<ID> := 'a'|'b';"
    ":";



static ParseToken token_list[4096] = {};
static unsigned int token_count = 0;
static char msg[2048];


static bool parse_str_to_token_list(const char *str)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1, 1};
        else if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index], 1, 1};
        else if (str[index] == 'b') token_list[token_count++] = {TOKEN_b, &str[index], 1, 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};
    return true;
}



#include "common_test.cpp"

int main(void)
{
    ParseTable *table = create_and_print_table(bnf_source);
    
    test_str(table, true, "a+", "SHIFT, 4\nREDUCE, 4\nREDUCE, 3\nSHIFT, 6\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "b+", "SHIFT, 5\nREDUCE, 5\nREDUCE, 3\nSHIFT, 6\nREDUCE, 1\nACCEPT\n");
    test_str(table, false, "", "INVALID, 0\nlookahead_lr_index: 3, state: 0\nUnexpected  token");
    test_str(table, true, "a", "SHIFT, 4\nREDUCE, 4\nREDUCE, 3\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "b", "SHIFT, 5\nREDUCE, 5\nREDUCE, 3\nREDUCE, 2\nACCEPT\n");
    printf("Finished %s\n", __FILE__);
}
