#include "ptg_internal.hpp"



I32 get_ir_item_index(const Grammar *lex, String d)
{
    for (I32 i = 0; i < (I32)lex->LR_items_count; ++i)
    {
        if (is_str(d, lex->LR_items_str[i]))
        {
            return i;
        }
    }
    return -1;
}



static Errcode expect(const char *src, Usize *cursor, const char *cmp)
{
    Usize start_cursor = *cursor;
    Usize compare_val = *cursor - start_cursor;
    while (cmp[compare_val] != '\0')
    {
        if (src[*cursor] != cmp[compare_val] || src[*cursor] == '\0')
        {
            return_with_error(1, "ERROR: expected %s but got %s", &cmp[compare_val], &src[*cursor]);
        }
        *cursor += 1;
        compare_val = *cursor - start_cursor;
    }
    return 0;
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


static void move_past_whitespace(const char *src, Usize *cursor)
{
    while (is_whitespace(src[*cursor]))
    {
        *cursor += 1;
    }
}


// bool is_bnf_token(BNFToken b0, BNFToken b1)
// {
//     if (b0.lr_item != b1.lr_item) return false;
//     if (b0.type != b1.type) return false;

//     return true;
// }


//TODO(Johan) simplify if possible
static void set_first(Grammar *gram)
{
    Stack<I32> lr_stack; init_stack(&lr_stack);
    bool *checked_table = alloc(bool, gram->LR_items_count);

    for (I32 i = 0; i < (I32)gram->terminals_count; ++i)
    {
        checked_table[i] = true;
        gram->first_sets[i].terminals[i] = true;
    }

    // push the starting S non terminal
    push(&lr_stack, (I32)gram->terminals_count);
    checked_table[(I32)gram->terminals_count] = true;

    while (true)
    {
        assert_debug(lr_stack.count > 0);
        I32 lr_top = lr_stack.data[lr_stack.count - 1];

        bool pushed_non_terminals = false;

        for (Usize i = 0; i < gram->expr_count; ++i)
        {
            BNFExpression *expr = &gram->exprs[i];

            if (expr->non_terminal != lr_top) continue;


            for (Usize j = 0; j < expr->prod_count ; ++j)
            {
                I32 token = expr->prod_tokens[j];
                if (!is_non_terminal(token, gram)) continue;

                if (!checked_table[token])
                {
                    push(&lr_stack, token);
                    checked_table[token] = true;
                    pushed_non_terminals = true;
                }
            }
        }
        if (!pushed_non_terminals)
        {
            break;
        }
    }

    while (lr_stack.count > 0)
    {
        I32 active_non_terminal = pop(&lr_stack);
        assert_debug(active_non_terminal >= (I32)gram->terminals_count);
        
        FirstSet *set = &gram->first_sets[active_non_terminal];

        // check if non terminal produces empty set
        for (Usize i = 0; i < gram->expr_count; ++i)
        {
            BNFExpression *expr = &gram->exprs[i];

            if (active_non_terminal != expr->non_terminal) continue;

            if (expr->prod_count == 0)
            {
                set->produces_empty_set = true;
                goto break_outer;
            }
        } break_outer:;


        // add first possible terminals to first set
        for (Usize i = 0; i < gram->expr_count; ++i)
        {
            BNFExpression *expr = &gram->exprs[i];

            if (active_non_terminal != expr->non_terminal) continue;

            if (expr->prod_count == 0)
            {

            }
            else if (is_terminal(expr->prod_tokens[0], gram))
            {
                set->terminals[expr->prod_tokens[0]] = true;
            }
            else
            {
                for (Usize j = 0; j < expr->prod_count; ++j)
                {
                    I32 prod_token = expr->prod_tokens[j];
                    if (is_non_terminal(prod_token, gram))
                    {
                        for (Usize k = 0; k < gram->terminals_count; ++k)
                        {
                            if (gram->first_sets[prod_token].terminals[k])
                            {
                                set->terminals[k] = true;
                            }
                        }
                        if (!gram->first_sets[prod_token].produces_empty_set)
                        {
                            break;
                        }
                    }
                    else if (is_terminal(prod_token, gram))
                    {
                        set->terminals[prod_token] = true;
                        break;
                    }
                    else
                    {
                        assert_always(false && "unreachable");
                    }
                }

            }
        }
    }

    free(checked_table);
    free_stack(&lr_stack);
}


static void add_to_gram_if_unique(Grammar *gram, Lex_Token token)
{
    bool already_exists = false;
    for (Usize i = 0; i < gram->LR_items_count; ++i)
    {
        if (is_str(gram->LR_items_str[i], token.str))
        {
            already_exists = true;
            break;
        }
    }
    if (!already_exists)
    {
        if (token.kind == TOKEN_KIND::TERMINAL_ID)
        {
            gram->LR_items_str[gram->LR_items_count] = token.str;
            gram->LR_items_count += 1;
            gram->terminals_count += 1;
        }
        else if (token.kind == TOKEN_KIND::NON_TERMINAL_ID)
        {
            gram->LR_items_str[gram->LR_items_count] = token.str;
            gram->LR_items_count += 1;
        }
        else
        {
            assert_always(false);
        }
    }
}
static const char *TOKEN_KIND_to_str(TOKEN_KIND kind)
{
    switch (kind)
    {
        case TOKEN_KIND::INVALID: return "INVALID";
        case TOKEN_KIND::TERMINAL_ID: return "TERMINAL_ID";
        case TOKEN_KIND::NON_TERMINAL_ID: return "NON_TERMINAL_ID";
        case TOKEN_KIND::ASSIGNMENT: return "ASSIGNMENT";
        case TOKEN_KIND::OR: return "OR";
        case TOKEN_KIND::END: return "END";
    }
    assert_always(false);
    return nullptr;
}

Errcode grammar_from_lexer(Grammar *gram, const Lexer *lex)
{
    memset(gram, 0, sizeof(*gram));

    Usize i = 0;
    // terminals
    do
    {
        add_to_gram_if_unique(gram, lex->tokens[i]);
        i += 1;
    } while (lex->tokens[i].kind == TOKEN_KIND::TERMINAL_ID);


    Lex_Token non_terminal = lex->tokens[i];
    assert_always(non_terminal.kind == TOKEN_KIND::NON_TERMINAL_ID);
    i += 1;
    assert_always(lex->tokens[i].kind == TOKEN_KIND::ASSIGNMENT);
    i += 1;
    add_to_gram_if_unique(gram, non_terminal);
    I32 non_terminal_index = get_ir_item_index(gram, non_terminal.str);

    bool return_at_end = false;

    while (i < lex->count)
    {
        Usize start_non_terminal = i;
        U32 prod_count = 0;
        BNFExpression *expr = &gram->exprs[gram->expr_count++];
        expr->non_terminal = non_terminal_index;

        while (i < lex->count)
        {
            switch (lex->tokens[i].kind)
            {
                case TOKEN_KIND::INVALID:
                {
                    assert_always(false && "Invalid TOKEN_KIND");
                } break;
                case TOKEN_KIND::TERMINAL_ID:
                {
                    prod_count += 1;
                    i += 1;
                } break;
                case TOKEN_KIND::NON_TERMINAL_ID:
                {
                    assert_always(i + 1 < lex->count);
                    if (lex->tokens[i + 1].kind == TOKEN_KIND::ASSIGNMENT)
                    {
                        non_terminal = lex->tokens[i];
                        add_to_gram_if_unique(gram, non_terminal);
                        non_terminal_index = get_ir_item_index(gram, non_terminal.str);
                        i += 2;
                        goto break_loop1;
                    }
                    else
                    {
                        add_to_gram_if_unique(gram, lex->tokens[i]);
                        prod_count += 1;
                        i += 1;
                    }
                } break;
                case TOKEN_KIND::ASSIGNMENT:
                {
                    assert_always(false);
                } break;
                case TOKEN_KIND::OR:
                {
                    i += 1;
                    goto break_loop1;
                } break;
                case TOKEN_KIND::END:
                {
                    return_at_end = true;
                    goto break_loop1;
                } break;
            }
        }
        break_loop1:;

        {
            expr->prod_tokens = alloc(I32, prod_count);
        }

        for (Usize j = start_non_terminal; j < start_non_terminal + prod_count; ++j)
        {
            Lex_Token token = lex->tokens[j];
            switch (token.kind)
            {
                case TOKEN_KIND::INVALID:
                {
                    assert_always(false && "Invalid TOKEN_KIND");
                } break;
                case TOKEN_KIND::TERMINAL_ID:
                {
                    I32 lr_item = get_ir_item_index(gram, token.str);
                    if (lr_item == -1)
                    {
                        assert_always(token.str.stride == 1);
                        return_with_error(1, "Terminal '%.*s' not declared under TOKENS\n", (int)token.str.length, token.str.data);
                    }
                    expr->prod_tokens[expr->prod_count++] = lr_item;
                } break;
                case TOKEN_KIND::NON_TERMINAL_ID:
                {
                    assert_always(j + 1 < lex->count);
                    assert_always(lex->tokens[j + 1].kind != TOKEN_KIND::ASSIGNMENT);
                    expr->prod_tokens[expr->prod_count++] = get_ir_item_index(gram, token.str);
                } break;
                case TOKEN_KIND::ASSIGNMENT:
                {
                    assert_always(false);
                } break;
                case TOKEN_KIND::OR:
                {
                    assert_always(false);
                } break;
                case TOKEN_KIND::END:
                {
                    assert_always(false);
                } break;
            }
        }
        // fprint_BNF(stdout, expr, gram);
        // printf("\n");
        if (return_at_end)
        {
            goto end;
        }
    }
    end:;

    gram->first_sets = alloc(FirstSet, gram->LR_items_count);
    set_first(gram);
    return 0;
}


static Errcode parse_non_terminal_add_to_lexer(Lexer *lex, const char *src, Usize *cursor)
{
    Lex_Token non_terminal = {
        .kind = TOKEN_KIND::NON_TERMINAL_ID,
        .str = {.data = &src[*cursor], .length = 0, .stride = 1},
    };

    if (src[*cursor] == '>')
    {
        return_with_error(1, "Can not have empty non terminal identifier\n");
    }

    while (src[*cursor] != '>')
    {
        *cursor += 1;
        non_terminal.str.length += 1;
    }
    lex->tokens[lex->count++] = non_terminal;
    return 0;
}


Errcode parse_bnf_src(Lexer *lex, const char *src)
{
    Usize cursor = 0;
    // Lex_Token parsing
    {
        move_past_whitespace(src, &cursor);
        TRY(expect(src, &cursor, "TOKENS"));
        move_past_whitespace(src, &cursor);
        while (src[cursor] != ':')
        {
            if (src[cursor] == ';')
            {
                // handle if Lex_Token does not have a name
                return_with_error(1, "Can not have empty Token identifier\n");
            }


            Lex_Token Lex_Token = {
                .kind = TOKEN_KIND::TERMINAL_ID,
                .str = {.data = &src[cursor], .length = 0, .stride = 1},
            };
            Usize i = 0;
            for (; src[cursor] != ';'; ++i)
            {
                if (src[cursor] == '\0') return_with_error(1, "Unexpected end of string");

                cursor += 1;
            }
            Lex_Token.str.length = i;
            lex->tokens[lex->count++] = Lex_Token;
            cursor += 1;
            move_past_whitespace(src, &cursor);
        }
        cursor += 1;
        move_past_whitespace(src, &cursor);
    }


    // Expression parsing
    {
        TRY(expect(src, &cursor, "BNF"));
        move_past_whitespace(src, &cursor);

        while (src[cursor] != ':')
        {
            TRY(expect(src, &cursor, "<"));

            TRY(parse_non_terminal_add_to_lexer(lex, src, &cursor));
            // expect '>'
            cursor += 1;
            move_past_whitespace(src, &cursor);

            {
                Lex_Token assign = {
                    .kind = TOKEN_KIND::ASSIGNMENT,
                    .str = {.data = &src[cursor], .length = 2, .stride = 1}
                };
                if (src[cursor] != ':')
                {
                    return_with_error(1, "Expected :\n");
                }
                cursor += 1;
                if (src[cursor] != '=')
                {
                    return_with_error(1, "Expected =\n");
                }
                lex->tokens[lex->count++] = assign;
            }
            cursor += 1;
            move_past_whitespace(src, &cursor);

            while (src[cursor] != ';')
            {
                switch (src[cursor])
                {
                    case '<':
                    {
                        cursor += 1;
                        TRY(parse_non_terminal_add_to_lexer(lex, src, &cursor));
                        // expect '>'
                        cursor += 1;
                        move_past_whitespace(src, &cursor);
                    } break;
                    case '\'':
                    {
                        cursor += 1;
                        Lex_Token terminal = {
                            .kind = TOKEN_KIND::TERMINAL_ID,
                            .str = {.data = &src[cursor], .length = 0, .stride = 1},
                        };

                        if (src[cursor] == '\'')
                        {
                            return_with_error(1, "Can not have empty terminal identifier\n");
                        }

                        while (src[cursor] != '\'')
                        {
                            cursor += 1;
                            terminal.str.length += 1;
                        }
                        lex->tokens[lex->count++] = terminal;
                        cursor += 1;
                        move_past_whitespace(src, &cursor);
                    } break;
                    case '|':
                    {
                        lex->tokens[lex->count++] = Lex_Token {
                            .kind = TOKEN_KIND::OR,
                            .str = {.data = &src[cursor], .length = 1, .stride = 1},
                        };
                        cursor += 1;
                        move_past_whitespace(src, &cursor);
                    } break;
                    default:
                    {
                        return_with_error(1, "Unexpected %c\n", src[cursor]);
                    } break;
                }
            }
            cursor += 1;
            move_past_whitespace(src, &cursor);
        }
        lex->tokens[lex->count++] = {
            .kind = TOKEN_KIND::END,
            .str = {.data = nullptr, .length = 0, .stride = 0},
        };
    }
    return 0;
}
