#include "../src/ptg_internal.hpp"
#include <string.h>



enum Token
{
    TOKEN_plus,
    TOKEN_minus,
    TOKEN_times,
    TOKEN_divide,
    TOKEN_caret,
    TOKEN_equal,
    TOKEN_open,
    TOKEN_close,
    TOKEN_Number,
    TOKEN_Id,
    TOKEN_Sep,
    TOKEN_End,
};


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
        if (is_alpha(str[index])) 
        {
            const char *start = &str[index];
            U32 count = 1;
            while (is_alpha(str[index + count]) || is_number(str[index + count]))
            {
                count += 1;
            }
            index += count - 1;

            token_list[token_count++] = {TOKEN_Id, start, count, 1};
        }
        else if (is_number(str[index]))
        {
            const char *start = &str[index];
            U32 count = 1;
            while (is_number(str[index + count]))
            {
                count += 1;
            }
            index += count - 1;

            token_list[token_count++] = {TOKEN_Number, start, count, 1};
        }
        else if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1, 1};
        else if (str[index] == '-') token_list[token_count++] = {TOKEN_minus, &str[index], 1, 1};
        else if (str[index] == '*') token_list[token_count++] = {TOKEN_times, &str[index], 1, 1};
        else if (str[index] == '/') token_list[token_count++] = {TOKEN_divide, &str[index], 1, 1};
        else if (str[index] == '^') token_list[token_count++] = {TOKEN_caret, &str[index], 1, 1};
        else if (str[index] == '=') token_list[token_count++] = {TOKEN_equal, &str[index], 1, 1};
        else if (str[index] == '(') token_list[token_count++] = {TOKEN_open, &str[index], 1, 1};
        else if (str[index] == ')') token_list[token_count++] = {TOKEN_close, &str[index], 1, 1};
        else if (is_whitespace(str[index])) {}
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};

    return true;
}







#include <stdlib.h>

#include "common_test.cpp"

int main(void)
{
    char *bnf_source = file_to_str("./tests/bnf10.txt");
    assert_always(bnf_source != nullptr);

    ParseTable *table = create_and_print_table(bnf_source);
    free(bnf_source);



    test_str(table, true, "", "REDUCE, 6\nREDUCE, 2\nACCEPT\n");
    test_str(table, false, "()", "SHIFT, 7\nINVALID, 0\nlookahead_lr_index: 7, state: 7\nUnexpected ) token");
    test_str(table, true, "1+1", "SHIFT, 8\nREDUCE, 10\nSHIFT, 17\nSHIFT, 8\nREDUCE, 10\nREDUCE, 19\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "1*1", "SHIFT, 8\nREDUCE, 10\nSHIFT, 15\nSHIFT, 8\nREDUCE, 10\nREDUCE, 17\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "1/1", "SHIFT, 8\nREDUCE, 10\nSHIFT, 14\nSHIFT, 8\nREDUCE, 10\nREDUCE, 16\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "1-1", "SHIFT, 8\nREDUCE, 10\nSHIFT, 16\nSHIFT, 8\nREDUCE, 10\nREDUCE, 18\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "-1+1", "SHIFT, 10\nSHIFT, 8\nREDUCE, 10\nREDUCE, 13\nSHIFT, 17\nSHIFT, 8\nREDUCE, 10\nREDUCE, 19\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "--1*1", "SHIFT, 10\nSHIFT, 10\nSHIFT, 8\nREDUCE, 10\nREDUCE, 13\nREDUCE, 13\nSHIFT, 15\nSHIFT, 8\nREDUCE, 10\nREDUCE, 17\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "--1^1^5", "SHIFT, 10\nSHIFT, 10\nSHIFT, 8\nREDUCE, 10\nREDUCE, 13\nREDUCE, 13\nSHIFT, 13\nSHIFT, 8\nREDUCE, 10\nREDUCE, 15\nSHIFT, 13\nSHIFT, 8\nREDUCE, 10\nREDUCE, 15\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "a=f(g)*44358340834683406*555543431265345348505+53492358+0/6-86546546546+h(c)", "SHIFT, 6\nSHIFT, 19\nSHIFT, 21\nSHIFT, 34\nSHIFT, 21\nREDUCE, 11\nSHIFT, 36\nREDUCE, 20\nREDUCE, 12\nSHIFT, 15\nSHIFT, 8\nREDUCE, 10\nREDUCE, 17\nSHIFT, 15\nSHIFT, 8\nREDUCE, 10\nREDUCE, 17\nSHIFT, 17\nSHIFT, 8\nREDUCE, 10\nREDUCE, 19\nSHIFT, 17\nSHIFT, 8\nREDUCE, 10\nSHIFT, 14\nSHIFT, 8\nREDUCE, 10\nREDUCE, 16\nSHIFT, 16\nSHIFT, 8\nREDUCE, 10\nREDUCE, 18\nREDUCE, 19\nSHIFT, 17\nSHIFT, 21\nSHIFT, 34\nSHIFT, 21\nREDUCE, 11\nSHIFT, 36\nREDUCE, 20\nREDUCE, 12\nREDUCE, 19\nREDUCE, 8\nREDUCE, 4\nREDUCE, 2\nACCEPT\n");
    test_str(table, true, "++++1", "SHIFT, 11\nSHIFT, 11\nSHIFT, 11\nSHIFT, 11\nSHIFT, 8\nREDUCE, 10\nREDUCE, 14\nREDUCE, 14\nREDUCE, 14\nREDUCE, 14\nREDUCE, 5\nREDUCE, 2\nACCEPT\n");

    fprintf(stderr, "Finished %s\n", __FILE__);
}