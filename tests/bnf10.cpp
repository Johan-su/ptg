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







static char *file_to_str(const char *file_path);

#include <stdlib.h>

#include "common_test.cpp"

int main(void)
{
    char *bnf_source = file_to_str("./tests/bnf10.txt");
    assert_always(bnf_source != nullptr);

    ParseTable *table = create_and_print_table(bnf_source);
    assert_always(false && "remove");
    free(bnf_source);





    test_str(table, true, "", "");
    test_str(table, false, "()", "Unexpected ) token");
    test_str(table, true, "1+1", "");
    test_str(table, true, "1*1", "");
    test_str(table, true, "1/1", "");
    test_str(table, true, "1-1", "");
    test_str(table, true, "-1+1", "");
    test_str(table, true, "--1*1", "");
    test_str(table, true, "--1^1^5", "");
    test_str(table, true, "a=f(g)*44358340834683406*555543431265345348505+53492358+0/6-86546546546+h(c)", "");
    test_str(table, true, "++++1", "nullptr");

    fprintf(stderr, "%.*s\n", (int)sizeof(msg), msg);
    fprintf(stderr, "Finished %s\n", __FILE__);
}


































#include <string.h>

static char *file_to_str(const char *file_path)
{
    char *str = nullptr;
    FILE *f = fopen(file_path, "rb");
    if (f == nullptr)
    {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return nullptr;
    }

    long file_size = -1;
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "ERROR: failed to seek file %s\n", file_path);
        goto end_close;
    }
    file_size = ftell(f);
    if (file_size < 0)
    {
        goto end_close;
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "ERROR: failed to seek file %s\n", file_path);
        goto end_close;
    }
    {
        Usize buf_size = (Usize)file_size + 1; 
        str = alloc(char, buf_size);
    }
    if (fread(str, sizeof(*str), (Usize)file_size, f) != (Usize)file_size)
    {
        fprintf(stderr, "ERROR: failed to read data from file %s\n", file_path);
        free(str);
        str = nullptr;
    }

    end_close:
    if (fclose(f) == EOF)
    {
        fprintf(stderr, "ERROR: failed to close file %s\n", file_path);
    }
    return str;
}
