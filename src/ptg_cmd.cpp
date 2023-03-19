#include "ptg_internal.hpp"
#include "ptg.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static void usage(const char *program)
{
    fprintf(stderr, "USAGE: %s\n", program);
    fprintf(stderr, "-i <input path>; Reads bnf from file, by READING FROM STDIN DOES NOT WORK(default reads from stdin).\n");
    fprintf(stderr, "-m <bytes>; allocates bytes for table generation");
    fprintf(stderr, "-target <target>; output target for parsing table:\n    Targets: binary, text, c, rust\n");
    fprintf(stderr, "-o <output path>; outputs to file\n");
}



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
        memset(str, 0, sizeof(*str) * buf_size);
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

enum class OutputTarget
{
    INVALID,
    BINARY,
    TEXT,
    C,
    RUST,
};

// if output_path is null, the function will write to stdout
static int write_output_data_to_target(const void *data, Usize data_size, const char *output_path)
{
    FILE *f = nullptr;
    bool should_close_fd = false;
    const char *file_name = nullptr;
    if (output_path != nullptr)
    {
        f = fopen(output_path, "wb");
        if (f == nullptr)
        {
            fprintf(stderr, "ERROR: failed to open file %s\n", output_path);
            return -1;
        }
        should_close_fd = true;
        file_name = output_path;
    }
    else
    {
        f = stdout;
        should_close_fd = false;
        file_name = "stdout";
    }

    if (fwrite(data, data_size, 1, f) != 1)
    {
        fprintf(stderr, "ERROR: failed to write to file %s\n", file_name);
    }

    if (should_close_fd)
    {
        if (fclose(f) == EOF)
        {
            fprintf(stderr, "ERROR: failed close file %s\n", file_name);
            return -1;
        }
    }
    return 0;
}

static int create_parsing_table_from_cmd(const char *source_path, const char *output_path, OutputTarget output_target)
{
    // TODO(Johan) add support for reading bnf_src form stdin
    char *bnf_src = nullptr;
    if (source_path != nullptr)
    {
        bnf_src = file_to_str(source_path);
    }
    if (bnf_src == nullptr)
    {
        return -1;
    }

    ParseTable *table = create_parse_table_from_bnf(bnf_src);

    
    TableOperation *table_data = (TableOperation *)((table->data + table->data_size) - sizeof(*table_data) * (table->LR_items_count * table->state_count));

    switch(output_target)
    {
        case OutputTarget::INVALID:
        {
            assert(false && "Unreachable");
            exit(-2);
        } break;
        case OutputTarget::BINARY:
        {
            write_output_data_to_target(table, table->data_size, output_path);
        } break;
        case OutputTarget::TEXT:
        {
            const char *format = "[%-7s, %u] ";
            Usize block_size = (Usize)snprintf(nullptr, 0, format, op_to_str(table_data[0].type), table_data[0].arg);
            Usize data_str_size = block_size * table->LR_items_count * table->state_count + table->state_count * 1;
            char *data_str = alloc(char, data_str_size);
            memset(data_str, 0, data_str_size);
            char *temp_block = alloc(char, block_size + 1); // + 1 for null

            for (Usize y = 0; y < table->state_count; ++y)
            {
                for (Usize x = 0; x < table->LR_items_count; ++x)
                {
                    snprintf(temp_block, block_size, format, op_to_str(table_data[x + y * table->LR_items_count].type), table_data[x + y * table->LR_items_count].arg);
                    strcat(data_str, temp_block);
                }
                strcat(data_str, "\n");
            }

            write_output_data_to_target(data_str, sizeof(*data_str) * data_str_size, output_path);

            free(temp_block);
            free(data_str);
        } break;
        case OutputTarget::C:
        {
            const char *pre = "unsigned char table[] = {";
            Usize pre_len = str_len(pre);
            const char *post = "};\n";
            Usize data_str_size = pre_len + 4 * table->data_size + str_len(post);
            char *data_str = alloc(char, data_str_size);
            memset(data_str, 0, sizeof(*data_str) * data_str_size);
            strcat(data_str, pre);
            char temp_buf[5];

            u8 *bin_table = (u8 *)table;
            for (Usize i = 0; i < table->data_size; ++i)
            {
                snprintf(temp_buf, sizeof(temp_buf), "%u,", bin_table[i]);
                strcat(data_str, temp_buf);
            }
            strcat(data_str, post);

            write_output_data_to_target(data_str, sizeof(*data_str) * str_len(data_str), output_path);
            free(data_str);
        } break;
        case OutputTarget::RUST:
        {
            char temp_buffer[64] = {};
            unsigned int pre_len = (unsigned int)snprintf(temp_buffer, sizeof(temp_buffer), "const table : [u8; %u] = [", table->data_size);
            const char *post = "];\n";
            Usize data_str_size = pre_len + 4 * table->data_size + str_len(post);
            char *data_str = alloc(char, data_str_size);
            memset(data_str, 0, sizeof(*data_str) * data_str_size);
            strcat(data_str, temp_buffer);
            char temp_buf[5];

            u8 *bin_table = (u8 *)table;
            for (Usize i = 0; i < table->data_size; ++i)
            {
                snprintf(temp_buf, sizeof(temp_buf), "%u,", bin_table[i]);
                strcat(data_str, temp_buf);
            }
            strcat(data_str, post);

            write_output_data_to_target(data_str, sizeof(*data_str) * str_len(data_str), output_path);
            free(data_str);
        } break;

    }
    free(table);
    return 0;
}


int main(int argc, const char **argv)
{
    const char *program = *argv++;
    if (argc < 2)
    {
        usage(program);
        return -1;
    }
    const char *source_path = nullptr;
    const char *output_path = nullptr;
    OutputTarget output_target = OutputTarget::TEXT;
    // parse commandline
    while (*argv != nullptr)
    {
        if (is_str(make_string(*argv), make_string("-i")))
        {
            argv += 1;
            source_path = *argv++;
        }
        else if (is_str(make_string(*argv), make_string("-m")))
        {
            assert(false && "not implemented");
        }
        else if (is_str(make_string(*argv), make_string("-target")))
        {
            argv += 1;
            if (is_str(make_string(*argv), make_string("binary")))
            {
                argv += 1;
                output_target = OutputTarget::BINARY;
            }
            else if (is_str(make_string(*argv), make_string("text")))
            {
                argv += 1;
                output_target = OutputTarget::TEXT;
            }
            else if (is_str(make_string(*argv), make_string("c")))
            {
                argv += 1;
                output_target = OutputTarget::C;
            }
            else if (is_str(make_string(*argv), make_string("rust")))
            {
                argv += 1;
                output_target = OutputTarget::RUST;
            }
            else
            {
                usage(program);
                return -1;
            }
        }
        else if (is_str(make_string(*argv), make_string("-o")))
        {
            argv += 1;
            output_path = *argv++;
        }
        else
        {
            usage(program);
            return -1;
        }
    }

    return create_parsing_table_from_cmd(source_path, output_path, output_target);
}