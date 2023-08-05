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
    "<S> := <Number>'End';"
    "<Number> := <Number><Digit> | <Digit>;"
    "<Digit> := '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';"
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
        if (str[index] == '0') token_list[token_count++] = {TOKEN_0, &str[index], 1, 1};
        else if (str[index] == '1') token_list[token_count++] = {TOKEN_1, &str[index], 1, 1};
        else if (str[index] == '2') token_list[token_count++] = {TOKEN_2, &str[index], 1, 1};
        else if (str[index] == '3') token_list[token_count++] = {TOKEN_3, &str[index], 1, 1};
        else if (str[index] == '4') token_list[token_count++] = {TOKEN_4, &str[index], 1, 1};
        else if (str[index] == '5') token_list[token_count++] = {TOKEN_5, &str[index], 1, 1};
        else if (str[index] == '6') token_list[token_count++] = {TOKEN_6, &str[index], 1, 1};
        else if (str[index] == '7') token_list[token_count++] = {TOKEN_7, &str[index], 1, 1};
        else if (str[index] == '8') token_list[token_count++] = {TOKEN_8, &str[index], 1, 1};
        else if (str[index] == '9') token_list[token_count++] = {TOKEN_9, &str[index], 1, 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};

    return true;
}


#include "common_test.cpp"

int main(void)
{
    ParseTable *table = create_and_print_table(bnf_source);


    test_str(table, false, "", "INVALID, 0\nlookahead_lr_index: 10, state: 0\nUnexpected  token");
    test_str(table, false, "abcd", "INVALID, 0\nlookahead_lr_index: 10, state: 0\nUnexpected  token");
    test_str(table, true, "0", "SHIFT, 3\nREDUCE, 3\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "00", "SHIFT, 3\nREDUCE, 3\nREDUCE, 2\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "1", "SHIFT, 4\nREDUCE, 4\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "11", "SHIFT, 4\nREDUCE, 4\nREDUCE, 2\nSHIFT, 4\nREDUCE, 4\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "2492328501235823580", "SHIFT, 5\nREDUCE, 5\nREDUCE, 2\nSHIFT, 7\nREDUCE, 7\nREDUCE, 1\nSHIFT, 12\nREDUCE, 12\nREDUCE, 1\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 11\nREDUCE, 11\nREDUCE, 1\nSHIFT, 8\nREDUCE, 8\nREDUCE, 1\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nSHIFT, 4\nREDUCE, 4\nREDUCE, 1\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 8\nREDUCE, 8\nREDUCE, 1\nSHIFT, 11\nREDUCE, 11\nREDUCE, 1\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 8\nREDUCE, 8\nREDUCE, 1\nSHIFT, 11\nREDUCE, 11\nREDUCE, 1\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "323123", "SHIFT, 6\nREDUCE, 6\nREDUCE, 2\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 4\nREDUCE, 4\nREDUCE, 1\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "6364859", "SHIFT, 9\nREDUCE, 9\nREDUCE, 2\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 9\nREDUCE, 9\nREDUCE, 1\nSHIFT, 7\nREDUCE, 7\nREDUCE, 1\nSHIFT, 11\nREDUCE, 11\nREDUCE, 1\nSHIFT, 8\nREDUCE, 8\nREDUCE, 1\nSHIFT, 12\nREDUCE, 12\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "123456", "SHIFT, 4\nREDUCE, 4\nREDUCE, 2\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 7\nREDUCE, 7\nREDUCE, 1\nSHIFT, 8\nREDUCE, 8\nREDUCE, 1\nSHIFT, 9\nREDUCE, 9\nREDUCE, 1\nACCEPT\n");

    printf("Finished %s\n", __FILE__);
}