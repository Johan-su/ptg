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


static void test_str(ParseTable *table, bool expected_parse_bool, const char *str, const char *expected_message)
{
    Expr *tree = nullptr;
    if (!parse_str_to_token_list(str))
    {
        assert_always(expected_parse_bool == false);   
    }

    memset(msg, 0, sizeof(msg));
    bool output = parse(token_list, token_count, table, PRINT_EVERY_PARSE_STEP_FLAG, &tree, msg, sizeof(msg));
    if (output)
    {
        // graphviz_from_syntax_tree("./build/tests/input_bnf10.dot", tree);
    }

    if (expected_message != nullptr)
    {
        String s0 = make_string(msg);
        String s1 = make_string(expected_message);
        if(!is_str(s0, s1))
        {
            fprintf(stderr, "got '%s' but expected '%s'\n", msg, expected_message);
            assert_always(false);
        }
    }
    assert_always(output == expected_parse_bool);

}




static ParseTable *create_and_print_table(const char *src)
{
    ParseTable *table = create_parse_table_from_bnf(src);
    if (table == nullptr)
    {
        fprintf(stderr, "%s", get_last_error());
    }
    assert_always(table != nullptr);
    print_table(table);
    fprintf(stderr, "\n");
    return table;
}

