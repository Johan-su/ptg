#ifndef PTG_LEXER_HPP
#define PTG_LEXER_HPP

#include "ptg_internal.hpp"

struct FirstSet
{
    Usize terminal_count;
    String terminals[128];
};

struct Lexer
{
    BNFExpression exprs[2048];
    U32 expr_count;

    U32 terminals_count;
    U32 LR_items_count;
    String LR_items[128];

    FirstSet *first_sets;
};


I64 get_ir_item_index(const Lexer *lex, String d);
Errcode parse_bnf_src(Lexer *lex, const char *src);


#endif // PTG_LEXER_HPP

#ifdef PTG_LEXER_IMPLMENTATION
#undef PTG_LEXER_IMPLMENTATION 


I64 get_ir_item_index(const Lexer *lex, String d)
{
    for (I64 i = 0; i < (I64)lex->LR_items_count; ++i)
    {
        if (is_str(d, lex->LR_items[i]))
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


static BNFToken parse_non_terminal_id(const char *src, Usize *cursor)
{
    String id = {};
    id.data = src + *cursor;
    id.length = 0;
    id.stride = 1;

    while (src[*cursor] != '>')
    {
        assert_debug(src[*cursor] != '\0');
        *cursor += 1;
        id.length += 1;
    }

    assert_debug(id.length > 0);
    BNFToken token = {};
    token.data = id;
    token.type = TokenType::NONTERMINAL;
    return token;
}


static BNFToken parse_terminal(const char *src, Usize *cursor)
{
    String id = {};
    id.data = src + *cursor;
    id.length = 0;
    id.stride = 1;

    while (src[*cursor] != '\'')
    {
        assert_debug(src[*cursor] != '\0');
        *cursor += 1;
        id.length += 1;
    }

    BNFToken token = {};
    token.data = id;
    token.type = TokenType::TERMINAL;
    return token;
}


static Errcode parse_BNFexpr_and_add_to_lexer(Lexer *lex, const char *src, Usize *cursor)
{
    TRY(expect(src, cursor, "<"));
    BNFToken expr_non_terminal = parse_non_terminal_id(src, cursor);
    if (get_ir_item_index(lex, expr_non_terminal.data) != -1)
    {
        return_with_error(1, "ERROR: <%.*s> already found, use '|' to define multiple productions for a non terminal\n", (int)expr_non_terminal.data.length, expr_non_terminal.data.data);
    }
    if (!is_str(expr_non_terminal.data, make_string("S")))
    {
        lex->LR_items[lex->LR_items_count++] = expr_non_terminal.data;
    }
    TRY(expect(src, cursor, ">"));
    move_past_whitespace(src, cursor);
    TRY(expect(src, cursor, ":="));
    move_past_whitespace(src, cursor);



    while (src[*cursor] != '\0')
    {
        bool should_break = false;
        BNFExpression expr = {};
        expr.non_terminal = expr_non_terminal;

        Usize cursor_start = *cursor;
        Usize prod_count = 0;
        while (src[*cursor] != ';' && src[*cursor] != '\0')
        {
            if (src[*cursor] == '\'')
            {
                *cursor += 1;
                parse_terminal(src, cursor);
                prod_count += 1;
                TRY(expect(src, cursor, "\'"));
                move_past_whitespace(src, cursor);
            }
            else if (src[*cursor] == '<')
            {
                *cursor += 1;
                parse_non_terminal_id(src, cursor);
                prod_count += 1;
                TRY(expect(src, cursor, ">"));
                move_past_whitespace(src, cursor);
            }
            else if (src[*cursor] == '|')
            {
                *cursor += 1;
                move_past_whitespace(src, cursor);
                break;
            }
            else
            {
                return_with_error(1, "ERROR: unexpected %c in bnf\n", src[*cursor]);
            }
        }

        if (prod_count > 0)
        {
            expr.prod_tokens = alloc(BNFToken, prod_count);
        }

        *cursor = cursor_start;
        expr.prod_count = 0;   
        while (src[*cursor] != '\0')
        {
            if (src[*cursor] == '\'')
            {
                *cursor += 1;
                BNFToken terminal = parse_terminal(src, cursor);
                expr.prod_tokens[expr.prod_count++] = terminal;
                TRY(expect(src, cursor, "\'"));
                move_past_whitespace(src, cursor);
            }
            else if (src[*cursor] == ';')
            {
                break;
            }
            else if (src[*cursor] == '<')
            {
                *cursor += 1;
                BNFToken non_terminal = parse_non_terminal_id(src, cursor);
                expr.prod_tokens[expr.prod_count++] = non_terminal;
                TRY(expect(src, cursor, ">"));
                move_past_whitespace(src, cursor);
            }
            else if (src[*cursor] == '|')
            {
                *cursor += 1;
                move_past_whitespace(src, cursor);
                if (src[*cursor] == ';')
                {
                    should_break = false;
                    goto continue_outer;
                }
                break;
            }
            else
            {
                assert_always(false);
            }

        }
        should_break = src[*cursor] == ';';
        continue_outer:;
        lex->exprs[lex->expr_count++] = expr;
        if (should_break) break;

    }
    move_past_whitespace(src, cursor);
    return 0;
}


static Errcode parse_token(const char *src, Usize *cursor, Lexer *lex)
{
    String token_str = {};
    token_str.data = &src[*cursor];
    Usize count = 0;
    while (src[*cursor] != ';')
    {
        if (src[*cursor] == '\0')
        {
            return_with_error(1, "ERROR: failed to parse token\n");
        }


        count += 1;
        *cursor += 1;
    }
    token_str.length = count;
    token_str.stride = 1;

    lex->LR_items[lex->LR_items_count++] = token_str;
    lex->terminals_count += 1;
    return 0; 
}


static void add_element_to_first_set(FirstSet *first_array, I64 lr_index, String terminal)
{
    assert_always(lr_index >= 0);
    FirstSet *set = &first_array[lr_index];


    for (Usize i = 0; i < set->terminal_count; ++i)
    {
        // skip adding if the terminal is already in the set
        if (is_str(set->terminals[i], terminal))
        {
            return;
        }
    }
    set->terminals[set->terminal_count++] = terminal;
}


static void set_first(Lexer *lex)
{
    Usize stack_size = 2048;
    String *non_terminal_stack = alloc(String, stack_size);
    Usize stack_count = stack_size;

    bool *checked_table = alloc(bool, lex->LR_items_count);

    for (Usize i = 0; i < lex->terminals_count; ++i)
    {
        for (Usize j = 0; j < lex->LR_items_count; ++j)
        {
            checked_table[j] = false;
        }
        String terminal = lex->LR_items[i];

        for (Usize j = 0; j < lex->expr_count; ++j)
        {
            BNFExpression *expr = &lex->exprs[j];
            if (expr->prod_count > 0 && is_str(expr->prod_tokens[0].data, terminal))
            {
                assert_debug(stack_count > 0);
                non_terminal_stack[--stack_count] = expr->non_terminal.data;
            }
        }

        while (stack_count < stack_size)
        {
            String non_terminal = non_terminal_stack[stack_count++];
            if (non_terminal.length == 0) goto end;

            // TODO(Johan) maybe remove this check somehow
            if (is_str(make_string("S"), non_terminal)) continue;

            I64 lr_index = get_ir_item_index(lex, non_terminal);
            assert_always(lr_index != -1);

            if (checked_table[lr_index]) continue;

            add_element_to_first_set(lex->first_sets, lr_index, terminal);

            for (Usize j = 0; j < lex->expr_count; ++j)
            {
                BNFExpression *expr = &lex->exprs[j];
                if (expr->prod_count > 0 && is_str(expr->prod_tokens[0].data, non_terminal))
                {
                    assert_debug(stack_count > 0);
                    non_terminal_stack[--stack_count] = expr->non_terminal.data;
                }
            }

            checked_table[lr_index] = true;
        }
    }


    for (I64 i = 0; i < (I64)lex->terminals_count; ++i)
    {
        add_element_to_first_set(lex->first_sets, i, lex->LR_items[i]);
    }

    end:
    free(checked_table);
    free(non_terminal_stack);
}


Errcode parse_bnf_src(Lexer *lex, const char *src)
{
    Usize cursor = 0;
    // parse tokens
    {
        move_past_whitespace(src, &cursor);
        TRY(expect(src, &cursor, "TOKENS"));
        move_past_whitespace(src, &cursor);
        while (src[cursor] != ':')
        {
            move_past_whitespace(src, &cursor);
            TRY(parse_token(src, &cursor, lex));
            TRY(expect(src, &cursor, ";"));
            move_past_whitespace(src, &cursor);
        }
        TRY(expect(src, &cursor, ":"));
    }
    // parse bnf
    {
        move_past_whitespace(src, &cursor);
        TRY(expect(src, &cursor, "BNF"));
        move_past_whitespace(src, &cursor);
        while (src[cursor] != ':')
        {
            assert_always(lex->expr_count < ARRAY_COUNT(lex->exprs));
            move_past_whitespace(src, &cursor);

            TRY(parse_BNFexpr_and_add_to_lexer(lex, src, &cursor));
            TRY(expect(src, &cursor, ";"));
            move_past_whitespace(src, &cursor);
        }
        TRY(expect(src, &cursor, ":"));
    }

    Usize set_size = lex->LR_items_count;
    FirstSet *first = alloc(FirstSet, set_size);

    lex->first_sets = first;
    set_first(lex);
    return 0;
}









#endif // PTG_LEXER_IMPLMENTATION