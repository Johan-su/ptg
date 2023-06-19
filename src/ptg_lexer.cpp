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


static bool is_bnf_token(BNFToken b0, BNFToken b1)
{
    if (b0.lr_item != b1.lr_item) return false;
    if (b0.type != b1.type) return false;

    return true;
}


static void add_element_to_first_set(FirstSet *first_array, I32 non_terminal_index, BNFToken terminal)
{
    assert_always(terminal.type == TokenType::TERMINAL);
    FirstSet *set = &first_array[non_terminal_index];


    for (Usize i = 0; i < set->terminal_count; ++i)
    {
        // skip adding if the terminal is already in the set
        if (is_bnf_token(set->terminals[i], terminal))
        {
            return;
        }
    }
    set->terminals[set->terminal_count++] = terminal;
}



static void set_first(Grammar *gram)
{
    Usize stack_size = 2048;
    BNFToken *non_terminal_stack = alloc(BNFToken, stack_size);
    Usize stack_count = stack_size;

    bool *checked_table = alloc(bool, gram->LR_items_count);

    for (I32 i = 0; i < (I32)gram->terminals_count; ++i)
    {
        for (Usize j = 0; j < gram->LR_items_count; ++j)
        {
            checked_table[j] = false;
        }

        for (Usize j = 0; j < gram->expr_count; ++j)
        {
            BNFExpression *expr = &gram->exprs[j];
            if (expr->prod_count > 0 && expr->prod_tokens[0].lr_item == i)
            {
                assert_debug(stack_count > 0);
                non_terminal_stack[--stack_count] = expr->non_terminal;
            }
        }

        while (stack_count < stack_size)
        {
            BNFToken non_terminal = non_terminal_stack[stack_count++];
            // TODO(Johan) probably not correct if condition
            if (non_terminal.type == TokenType::EMPTY) goto end;

            // TODO(Johan) maybe remove this check somehow
            // if (is_str(make_string("S"), non_terminal)) continue;
            // assert_always(false);

            I32 lr_index = non_terminal.lr_item;
            assert_always(lr_index != -1);

            if (checked_table[lr_index]) continue;

            BNFToken terminal = gram->LR_items[i];
            add_element_to_first_set(gram->first_sets, lr_index, terminal);

            for (Usize j = 0; j < gram->expr_count; ++j)
            {
                BNFExpression *expr = &gram->exprs[j];
                if (expr->prod_count > 0 && expr->prod_tokens[0].lr_item == non_terminal.lr_item)
                {
                    assert_debug(stack_count > 0);
                    non_terminal_stack[--stack_count] = expr->non_terminal;
                }
            }

            checked_table[lr_index] = true;
        }
    }


    for (I32 i = 0; i < (I32)gram->terminals_count; ++i)
    {
        add_element_to_first_set(gram->first_sets, i, gram->LR_items[i]);
    }

    end:
    free(checked_table);
    free(non_terminal_stack);
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
            gram->LR_items[gram->LR_items_count] = BNFToken {
                .type = TokenType::TERMINAL,
                .lr_item = (I32)gram->LR_items_count,
            };
            gram->LR_items_str[gram->LR_items_count] = token.str;
            gram->LR_items_count += 1;
            gram->terminals_count += 1;
        }
        else if (token.kind == TOKEN_KIND::NON_TERMINAL_ID)
        {
            gram->LR_items[gram->LR_items_count] = BNFToken {
                .type = TokenType::NONTERMINAL,
                .lr_item = (I32)gram->LR_items_count,
            };
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
        expr->non_terminal = BNFToken {.type = TokenType::NONTERMINAL, .lr_item = non_terminal_index};

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

        if (prod_count == 0)
        {
            expr->prod_tokens = alloc(BNFToken, 1);
            expr->prod_tokens[expr->prod_count].type = TokenType::EMPTY;
            expr->prod_tokens[expr->prod_count].lr_item = -10000;
            expr->prod_count += 1;

        }
        else
        {
            expr->prod_tokens = alloc(BNFToken, prod_count);
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
                    expr->prod_tokens[expr->prod_count++] = BNFToken {.type = TokenType::TERMINAL, .lr_item = get_ir_item_index(gram, token.str)};
                } break;
                case TOKEN_KIND::NON_TERMINAL_ID:
                {
                    assert_always(j + 1 < lex->count);
                    assert_always(lex->tokens[j + 1].kind != TOKEN_KIND::ASSIGNMENT);
                    expr->prod_tokens[expr->prod_count++] = 
                        BNFToken {.type = TokenType::NONTERMINAL, .lr_item = get_ir_item_index(gram, token.str)};
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
        fprint_BNF(stdout, expr, gram);
        if (return_at_end)
        {
            goto end;
        }
        // i += 1;
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
