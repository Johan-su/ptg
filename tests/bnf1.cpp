#include "../src/ptg_internal.hpp"


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
    "<S> := <E>'End';"
    "<E> := 'a' | <E>'t';"
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
        if (str[index] == 'a') token_list[token_count++] = {TOKEN_a, &str[index], 1, 1};
        else if (str[index] == 't') token_list[token_count++] = {TOKEN_t, &str[index], 1, 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};
    return true;
}

#include "common_test.cpp"

int main(void)
{
    ParseTable *table = create_and_print_table(bnf_source);


    test_str(table, false, "", "INVALID, 0\nlookahead_lr_index: 2, state: 0\nUnexpected  token");
    test_str(table, true, "a", "SHIFT, 2\nREDUCE, 1\nACCEPT\n");
    test_str(table, false, "t", "INVALID, 0\nlookahead_lr_index: 1, state: 0\nUnexpected t token");
    test_str(table, true, "at", "SHIFT, 2\nREDUCE, 1\nSHIFT, 3\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "att", "SHIFT, 2\nREDUCE, 1\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "atttttttttttttttttttttttttttt", "SHIFT, 2\nREDUCE, 1\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 2\nACCEPT\n");
    test_str(table, false, "aaaaat", "SHIFT, 2\nINVALID, 0\nlookahead_lr_index: 0, state: 2\nUnexpected a token");
    test_str(table, false, "ata", "SHIFT, 2\nREDUCE, 1\nSHIFT, 3\nINVALID, 0\nlookahead_lr_index: 0, state: 3\nUnexpected a token");
    test_str(table, false, "ataatatatatatatatatattttatatataat", "SHIFT, 2\nREDUCE, 1\nSHIFT, 3\nINVALID, 0\nlookahead_lr_index: 0, state: 3\nUnexpected a token");
    printf("Finished %s\n", __FILE__);
}