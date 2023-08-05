#include "../src/ptg_internal.hpp"

enum Token
{
    TOKEN_plus,
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
    "+;"
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
    "<S> := <E>'End';"
    "<E> := <E>'+'<E> | "
         "'0' | '1' | '2' |'3' |'4' | '5' | '6' | '7' | '8' | '9' | ;"
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
        else if (str[index] == '0') token_list[token_count++] = {TOKEN_0, &str[index], 1, 1};
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

    test_str(table, true, "", "REDUCE, 12\nACCEPT\n");
    test_str(table, true, "1+1", "SHIFT, 3\nREDUCE, 3\nSHIFT, 12\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "5+1", "SHIFT, 7\nREDUCE, 7\nSHIFT, 12\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "5+4+3+2+1", "SHIFT, 7\nREDUCE, 7\nSHIFT, 12\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 12\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 12\nSHIFT, 4\nREDUCE, 4\nREDUCE, 1\nSHIFT, 12\nSHIFT, 3\nREDUCE, 3\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "+4+3+2+", "REDUCE, 12\nSHIFT, 12\nSHIFT, 6\nREDUCE, 6\nREDUCE, 1\nSHIFT, 12\nSHIFT, 5\nREDUCE, 5\nREDUCE, 1\nSHIFT, 12\nSHIFT, 4\nREDUCE, 4\nREDUCE, 1\nSHIFT, 12\nREDUCE, 12\nREDUCE, 1\nACCEPT\n");
    test_str(table, true, "+", "REDUCE, 12\nSHIFT, 12\nREDUCE, 12\nREDUCE, 1\nACCEPT\n");
    test_str(table, false, "00", "SHIFT, 2\nINVALID, 0\nlookahead_lr_index: 1, state: 2\nUnexpected 0 token");
    test_str(table, false, "5120412505721057214901279+4", "SHIFT, 7\nINVALID, 0\nlookahead_lr_index: 2, state: 7\nUnexpected 1 token");

    printf("Finished %s\n", __FILE__);
}