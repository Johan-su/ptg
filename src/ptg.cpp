#include "ptg.hpp"
#include <stdlib.h>
#include "ptg_internal.hpp"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>



// https://cs.stackexchange.com/questions/152523/how-is-the-lookahead-for-an-lr1-automaton-computed
// https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf



// TODO list:
// TODO: add better error handling when creating parsing table
// TODO: when parsing, output the amount of chars written
// TODO: predict required memory to parse list of tokens, if possible

// TODO(Johan): make thread safe
static char g_msg_buffer[2048];
const char *get_last_error()
{
    return g_msg_buffer;
}





bool print_formated(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(g_msg_buffer, sizeof(g_msg_buffer), format, args);
    va_end(args);

    return true;
}


static void fprintf_state_expression(FILE *f, const State_Expression *expr, const Grammar *gram)
{
    const BNFExpression *bexpr = &gram->exprs[expr->grammar_prod_index];
    String non_terminal_str = gram->LR_items_str[bexpr->non_terminal.lr_item];


    assert_always(non_terminal_str.stride == 1);
    fprintf(f, "<%.*s> :=", (int)(non_terminal_str.length), non_terminal_str.data);
    for (Usize i = 0; i < bexpr->prod_count; ++i)
    {

        if (i == expr->dot)
        {
            fprintf(f, " ? ");
        }
        else
        {
            fprintf(f, " ");
        }

        if (bexpr->prod_tokens[i].lr_item >= 0)
        {
            String expr_str = gram->LR_items_str[bexpr->prod_tokens[i].lr_item];
            if (bexpr->prod_tokens[i].type == TokenType::TERMINAL)
            {
                fprintf(f, "\'%.*s\'", (int)expr_str.length, expr_str.data);
            }
            else if (bexpr->prod_tokens[i].type == TokenType::NONTERMINAL)
            {
                fprintf(f, "<%.*s>", (int)expr_str.length, expr_str.data);
            }
            else
            {
                assert_always(false);
            }
        }
    }
    if (expr->dot >= bexpr->prod_count)
    {
        fprintf(f, " ?");
    }

    fprintf(f, " [");

    if (expr->look_ahead_count > 0)
    {
        for (Usize i = 0; i < expr->look_ahead_count - 1; ++i)
        {
            String look_str = gram->LR_items_str[expr->look_aheads[i].lr_item];
            assert_always(look_str.stride == 1);
            fprintf(f, "%.*s,", (int)look_str.length, look_str.data);
        }
        String look_str = gram->LR_items_str[expr->look_aheads[expr->look_ahead_count - 1].lr_item];
        fprintf(f, "%.*s", (int)look_str.length, look_str.data);
    }
    fprintf(f, "]");
}

static void fprint_BNF(FILE *stream, const BNFExpression *expr, const Grammar *gram)
{
    String non_terminal_str = gram->LR_items_str[expr->non_terminal.lr_item];

    assert_always(non_terminal_str.stride == 1);
    fprintf(stream, "<%.*s> :=", (int)(non_terminal_str.length), non_terminal_str.data);
    for (Usize i = 0; i < expr->prod_count; ++i)
    {
        fprintf(stream, " ");

        if (expr->prod_tokens[i].lr_item >= 0)
        {
            String expr_str = gram->LR_items_str[expr->prod_tokens[i].lr_item];
            if (expr->prod_tokens[i].type == TokenType::TERMINAL)
            {
                fprintf(stream, "\'%.*s\'", (int)expr_str.length, expr_str.data);
            }
            else if (expr->prod_tokens[i].type == TokenType::NONTERMINAL)
            {
                fprintf(stream, "<%.*s>", (int)expr_str.length, expr_str.data);
            }
            else
            {
                assert_always(false);
            }
        }
    }
}






static bool is_BNFExpression(const BNFExpression *b0, const BNFExpression *b1)
{
    if (b0->non_terminal.type != b1->non_terminal.type) return false;
    if (b0->non_terminal.lr_item != b1->non_terminal.lr_item) return false;
    if (b0->prod_count != b1->prod_count) return false;

    for (Usize i = 0; i < b0->prod_count; ++i)
    {
        if (b0->prod_tokens[i].type != b1->prod_tokens[i].type) return false;
        if (b0->prod_tokens[i].lr_item != b1->prod_tokens[i].lr_item) return false;
    }

    return true;
}


static bool is_state_expression_ignore_lookahead(const State_Expression *e0, const State_Expression *e1)
{
    if (e0->dot != e1->dot) return false;
    if (e0->grammar_prod_index != e1->grammar_prod_index) return false;

    return true;
}


static bool is_state_expression(const State_Expression *e0, const State_Expression *e1)
{
    if (e0->dot != e1->dot) return false;
    if (e0->grammar_prod_index != e1->grammar_prod_index) return false;
    if (e0->look_ahead_count != e1->look_ahead_count) return false;

    for (Usize i = 0; i < e0->look_ahead_count; ++i)
    {
        if (!is_bnf_token(e0->look_aheads[i], e1->look_aheads[i]))
        {
            return false;
        }
    }
    return true;
}



static bool is_state(const State *s0, const State *s1)
{
    assert_always(s0->expr_count <= 512);
    assert_always(s1->expr_count <= 512);


    if (s0->creation_token.lr_item != s1->creation_token.lr_item) return false;
    if (s0->creation_token.type != s1->creation_token.type) return false;
    if (s0->expr_count != s1->expr_count) return false;


    for (Usize i = 0; i < s0->expr_count; ++i)
    {
        if (!is_state_expression(&s0->exprs[i], &s1->exprs[i]))
        {
            return false;
        }
    }

    return true;
}


void fprint_state(FILE *stream, const State *state, const Grammar *gram)
{
    for (Usize j = 0; j < state->expr_count; ++j)
    {
        const State_Expression *expr = &state->exprs[j];
        State *expr_ptr = state->edges[j];


        fprintf_state_expression(stream, expr, gram);

        if (expr_ptr != nullptr)
        {
            fprintf(stream, " -> %d", expr_ptr->state_id);
        }
        else
        {
            fprintf(stream, " -> ");
        }
        fprintf(stream, "\n");
    }
}


static bool is_expr_already_expanded(const State *state, const State_Expression *expr)
{
    for (Usize i = 0; i < state->expr_count; ++i)
    {
        if (is_state_expression(&state->exprs[i], expr))
        {
            return true;
        }
    }
    return false;
}



static I32 get_expr_index_ignore_lookahead(const State *state, const State_Expression *expr)
{
    for (I32 i = 0; i < (I32)state->expr_count; ++i)
    {
        if (is_state_expression_ignore_lookahead(&state->exprs[i], expr))
        {
            return i;
        }
    }
    return -1;
}

static bool has_lookahead(const State_Expression *expr, BNFToken terminal)
{
    for (Usize i = 0; i < expr->look_ahead_count; ++i)
    {
        if (is_bnf_token(expr->look_aheads[i], terminal))
        {
            return true;
        }
    }
    return false;
}

static void push_all_expressions_from_non_terminal_production(State *state, const Grammar *gram)
{
    for (Usize k = 0; k < state->expr_count; ++k)
    {
        assert_always(state->expr_count < ARRAY_COUNT(state->exprs));
        State_Expression *rule_expand_Sexpr = &state->exprs[k];
        const BNFExpression *rule_expand_Bexpr = &gram->exprs[rule_expand_Sexpr->grammar_prod_index];

        if (rule_expand_Sexpr->dot >= rule_expand_Bexpr->prod_count) continue;

        BNFToken rule_to_expand = rule_expand_Bexpr->prod_tokens[rule_expand_Sexpr->dot];
        if (rule_to_expand.type != TokenType::NONTERMINAL) continue;



        // if (rule_expand_expr->prod.expressions[rule_expand_expr->dot + 1].type != TokenType::INVALID)
        if (rule_expand_Sexpr->dot + 1 < rule_expand_Bexpr->prod_count)
        {
            I32 lr_index = rule_expand_Bexpr->prod_tokens[rule_expand_Sexpr->dot + 1].lr_item;

            for (Usize i = 0; i < gram->expr_count; ++i)
            {
                if (gram->exprs[i].non_terminal.lr_item != rule_to_expand.lr_item) continue;


                bool produces_empty_set = false;
                for (Usize j = 0; j < gram->first_sets[lr_index].terminal_count; ++j)
                {
                    BNFToken terminal = gram->first_sets[lr_index].terminals[j];
                    if (terminal.type == TokenType::EMPTY)
                    {
                        produces_empty_set = true;
                        break;
                    }
                }

                for (Usize j = 0; j < gram->first_sets[lr_index].terminal_count; ++j)
                {

                    State_Expression expr = {};
                    expr.dot = 0;
                    expr.grammar_prod_index = i;
                    expr.look_ahead_count = 0;

                    BNFToken terminal = {};
                    terminal.lr_item = gram->first_sets[lr_index].terminals[j].lr_item;
                    terminal.type = TokenType::TERMINAL;



                    I32 state_expr_index = get_expr_index_ignore_lookahead(state, &expr);
                    if (state_expr_index >= 0)
                    {
                        State_Expression *Sexpr = &state->exprs[state_expr_index];
                        if (!has_lookahead(Sexpr, terminal))
                        {
                            Sexpr->look_aheads[Sexpr->look_ahead_count++] = terminal;
                        }
                    }
                    else
                    {
                        expr.look_aheads[expr.look_ahead_count++] = terminal;
                        state->exprs[state->expr_count++] = expr;
                    }
                }


                if (produces_empty_set)
                {
                    State_Expression expr = {};
                    expr.dot = 0;
                    expr.grammar_prod_index = i;
                    expr.look_ahead_count = 0;

                    I32 state_expr_index = get_expr_index_ignore_lookahead(state, &expr);
                    if (state_expr_index >= 0)
                    {
                        State_Expression *Sexpr = &state->exprs[state_expr_index];

                        for (Usize l = 0; l < expr.look_ahead_count; ++l)
                        {
                            BNFToken terminal = expr.look_aheads[l];

                            if (!has_lookahead(Sexpr, terminal))
                            {
                                Sexpr->look_aheads[Sexpr->look_ahead_count++] = terminal;
                            }
                        }

                    }
                    else
                    {
                        for (Usize l = 0; l < rule_expand_Sexpr->look_ahead_count; ++l)
                        {
                            expr.look_aheads[expr.look_ahead_count++] = rule_expand_Sexpr->look_aheads[l];
                        }
                        state->exprs[state->expr_count++] = expr;
                    }
                }
            }

        }
        else
        {
            for (Usize i = 0; i < gram->expr_count; ++i)
            {
                if (gram->exprs[i].non_terminal.lr_item != rule_to_expand.lr_item) continue;

                State_Expression expr = {};
                expr.dot = 0;
                expr.grammar_prod_index = i;
                expr.look_ahead_count = 0;


                I32 state_expr_index = get_expr_index_ignore_lookahead(state, &expr);
                if (state_expr_index >= 0)
                {
                    State_Expression *Sexpr = &state->exprs[state_expr_index];

                    for (Usize l = 0; l < rule_expand_Sexpr->look_ahead_count; ++l)
                    {
                        BNFToken terminal = rule_expand_Sexpr->look_aheads[l];

                        if (!has_lookahead(Sexpr, terminal))
                        {
                            Sexpr->look_aheads[Sexpr->look_ahead_count++] = terminal;
                        }
                    }

                }
                else
                {
                    for (Usize l = 0; l < rule_expand_Sexpr->look_ahead_count; ++l)
                    {
                        expr.look_aheads[expr.look_ahead_count++] = rule_expand_Sexpr->look_aheads[l];
                    }
                    state->exprs[state->expr_count++] = expr;
                }
            }
        }
    }
}


static State g_active_substate = {};

static void create_substates_from_state(State *state, State *state_list, U32 *state_list_count, const Grammar *gram)
{
    assert_always(state->expr_count < ARRAY_COUNT(state->exprs));
    bool *check_list = alloc(bool, state->expr_count);

    for (Usize i = 0; i < state->expr_count; ++i)
    {

        State_Expression active_Sexpr = state->exprs[i];
        const BNFExpression *active_Bexpr = &gram->exprs[active_Sexpr.grammar_prod_index];

        if (active_Bexpr->non_terminal.type == TokenType::EMPTY) continue;



        State *active_substate = &g_active_substate;
        *active_substate = {};
        for (Usize j = i; j < state->expr_count; ++j)
        {
            if (check_list[j])
            {
                continue;
            }
            State_Expression *Sexpr = &state->exprs[j];
            const BNFExpression *Bexpr = &gram->exprs[Sexpr->grammar_prod_index];

            // if (expr->prod_tokens[expr->dot].type == TokenType::INVALID) continue;
            if (Sexpr->dot >= Bexpr->prod_count ||
                active_Sexpr.dot >= active_Bexpr->prod_count ||
                active_Bexpr->prod_tokens[active_Sexpr.dot].lr_item != Bexpr->prod_tokens[Sexpr->dot].lr_item)
                {
                    continue;
                }
            // skip shifting End token as it is the end token and means the parsing completed successfully
            if (Bexpr->prod_tokens[Sexpr->dot].lr_item == (I32)(gram->terminals_count - 1)) continue;

            active_substate->creation_token = active_Bexpr->prod_tokens[active_Sexpr.dot];

            active_substate->exprs[active_substate->expr_count] = *Sexpr;
            active_substate->exprs[active_substate->expr_count].dot += 1;

            State_Expression *last_Sexpr = &active_substate->exprs[active_substate->expr_count];
            const BNFExpression *last_Bexpr = &gram->exprs[last_Sexpr->grammar_prod_index];

            active_substate->expr_count += 1;


            if (last_Sexpr->dot < last_Bexpr->prod_count &&
                last_Bexpr->prod_tokens[last_Sexpr->dot].type == TokenType::NONTERMINAL)
            {
                push_all_expressions_from_non_terminal_production(active_substate, gram);
            }
            check_list[j] = true;


        }
        bool state_was_created = active_substate->expr_count > 0;
        if (state_was_created)
        {
            State *found_state = nullptr;
            for (Usize k = 0; k < *state_list_count; ++k)
            {
                if (is_state(active_substate, &state_list[k]))
                {
                    found_state = &state_list[k];
                    break;
                }
            }

            State *state_to_point_to = nullptr;
            if (found_state == nullptr)
            {
                active_substate->state_id = *state_list_count;
                state_list[*state_list_count] = *active_substate;
                // fprint_state(stderr, &state_list[*state_list_count], gram);
                state_to_point_to = &state_list[*state_list_count];
                *state_list_count += 1;
            }
            else
            {
                state_to_point_to = found_state;
            }

            for (Usize k = 0; k < state->expr_count; ++k)
            {
                State_Expression *k_Sexpr = &state->exprs[k];
                const BNFExpression *k_Bexpr = &gram->exprs[k_Sexpr->grammar_prod_index];


                if (k_Sexpr->dot < k_Bexpr->prod_count &&
                    active_substate->creation_token.lr_item == k_Bexpr->prod_tokens[k_Sexpr->dot].lr_item)
                {
                    state->edges[k] = state_to_point_to;
                }
            }
        }
    }
    free(check_list);
}


void graph_from_state_list(FILE *f, const State *state_list, Usize state_count, const Grammar *gram)
{
    fprintf(f, "digraph G {\n");

    for (Usize i = 0; i < state_count; ++i)
    {
        const State *state = &state_list[i];
        fprintf(f, "n%u [label=\"", state->state_id);
        fprintf(f, "State %u\n", state->state_id);
        fprint_state(f, state, gram);
        fprintf(f, "\"];\n");
        for (Usize j = 0; j < state->expr_count; ++j)
        {
            State *edge = state->edges[j];
            if (edge == nullptr)
            {
                continue;
            }

            for (I64 k = (I64)j - 1; k >= 0; --k)
            {
                if (edge == state->edges[k])
                {
                    goto continue_outer;
                }
            }

            fprintf(f, "n%u -> n%u", state->state_id, edge->state_id);
            {
                String creation_token_str = gram->LR_items_str[edge->creation_token.lr_item];
                if (edge->creation_token.type == TokenType::NONTERMINAL)
                {

                    assert_always(creation_token_str.stride == 1);
                    fprintf(f, " [label=\"%.*s\"];\n",
                        (int)creation_token_str.length, creation_token_str.data);
                }
                else if (edge->creation_token.type == TokenType::TERMINAL)
                {
                    fprintf(f, " [label=\"\'%.*s\'\"];\n",
                        (int)creation_token_str.length, creation_token_str.data);
                }
                else
                {
                    assert_always(false);
                }
            }

            continue_outer:;
        }
    }
    fprintf(f, "}\n");
}









void print_table(ParseTable *table)
{
    U8 *table_bin = (U8 *)table;

    // print non_terminals
    {
        StringHeader *string_data = (StringHeader *)(table_bin + table->string_header_start);
        for (Usize i = 0; i < table->string_header_count; ++i)
        {
            StringHeader *header = &string_data[i];
            printf("<%.*s>\n", (int)header->count, table_bin + header->str_start);
        }
    }

    // print exprs
    {
        ParseExpr *expr_data = (ParseExpr *)(table_bin + table->expr_header_start);
        for (Usize i = 0; i < table->expr_count; ++i)
        {
            ParseExpr *header = &expr_data[i];
            printf("<%lld> ->", header->non_terminal);
            I64 *prod_start = (I64 *)(table_bin + header->prod_start);
            for (Usize j = 0; j < header->production_count; ++j)
            {
                printf(" %lld", prod_start[j]);
            }
            printf("\n");
        }
    }

    // print table
    {
        Usize table_height = table->state_count;
        Usize table_width = table->LR_items_count;
        TableOperation *table_data = (TableOperation *)(table_bin + table->table_start);
        for (Usize y = 0; y < table_height; ++y)
        {
            TableOperation *row = table_data + y * table_width;
            for (Usize x = 0; x < table_width; ++x)
            {
                TableOperation *op = row + x;

                printf("[%-7s, %u] ", op_to_str(op->type), op->arg);
            }
            printf("\n");
        }
    }
}



static void table_set(const Grammar *gram, TableOperation *table, State_Expression **meta_expr_table, Usize table_size,
    State_Expression *expr, I64 look_ahead_index, Usize state_id, TableOperation op)
{
    assert_always(look_ahead_index != -1);

    Usize index = (Usize)look_ahead_index + state_id * gram->LR_items_count;
    assert_always(index < table_size);(void)table_size;
    switch (table[index].type)
    {
        case TableOperationType::INVALID:
        {
            table[index] = op;
            meta_expr_table[index] = expr;
        } break;
        case TableOperationType::SHIFT:
        case TableOperationType::REDUCE:
        {
            if (table[index].type == op.type)
            {
                //TODO(Johan) improve using better error handling
                printf("ERROR: table %s - %s conflict\n", op_to_str(table[index].type), op_to_str(op.type));
                printf("--Change ambiguous grammar\n");
                fprintf_state_expression(stdout, meta_expr_table[index], gram);
                printf("\n");
                fprintf_state_expression(stdout, expr, gram);
                printf("\n");
                assert_always(false);
                exit(1);
            }
            else
            {
                #if 1
                printf("WARNING: table %s - %s conflict\n", op_to_str(table[index].type), op_to_str(op.type));

                fprintf_state_expression(stdout, meta_expr_table[index], gram);
                printf("\n");
                fprintf_state_expression(stdout, expr, gram);
                printf("\n");
                #endif
                assert_always(false);
                // Usize table_expr_precedence = 0;
                // Usize new_expr_precedence = 0;
                // {

                //     BNFExpression table_expr = *meta_expr_table[index];
                //     table_expr.dot = 0;
                //     table_expr.look_ahead = {};
                //     BNFExpression new_expr = *expr;
                //     new_expr.dot = 0;
                //     new_expr.look_ahead = {};
                //     for (Usize i = 0; i < gram->expr_count; ++i)
                //     {
                //         if (is_BNFExpression(&table_expr, &gram->exprs[i]))
                //         {
                //             table_expr_precedence = i;
                //             break;
                //         }
                //     }
                //     for (Usize i = 0; i < gram->expr_count; ++i)
                //     {
                //         if (is_BNFExpression(&new_expr, &gram->exprs[i]))
                //         {
                //             new_expr_precedence = i;
                //             break;
                //         }
                //     }
                // }

                // if (table_expr_precedence < new_expr_precedence)
                // {
                //     #if 0
                //     printf("Choosing table %s resolution\n", op_to_str(table[index].type));
                //     #endif
                // }
                // else
                // {
                //     #if 0
                //     printf("Choosing %s resolution\n", op_to_str(op.type));
                //     #endif
                //     table[index] = op;
                // }
            }

        } break;
        case TableOperationType::GOTO:
        {
            assert_always(false);
        } break;
        case TableOperationType::ACCEPT:
        {
            assert_always(false);
        } break;
        default: assert_always(false && "Unreachable");
    }
}

#define PADDING_FOR_ALIGNMENT(ptr, type) ((alignof(type) - ((ptr) % alignof(type))) % alignof(type))

ParseTable *create_parse_table_from_states(const Grammar *gram, State *state_list, U32 state_count)
{
    Usize table_size = state_count * gram->LR_items_count;
    State_Expression **meta_expr_table = alloc(State_Expression *, table_size);
    if (meta_expr_table == nullptr) return nullptr;


    Usize string_headers_size_bytes = sizeof(StringHeader) * (gram->LR_items_count - gram->terminals_count);
    // get size of strings of the non terminals
    Usize strings_size_bytes = 0;
    {
        for (Usize i = gram->terminals_count; i < gram->LR_items_count; ++i)
        {
            strings_size_bytes += gram->LR_items_str[i].length * sizeof(char);
        }
    }
    Usize expr_headers_size_bytes = sizeof(ParseExpr) * gram->expr_count;
    // get byte size of all productions
    Usize prods_size_bytes = 0;
    for (Usize i = 0; i < gram->expr_count; ++i)
    {
        const BNFExpression *expr = &gram->exprs[i];
        prods_size_bytes += sizeof(I64) * expr->prod_count;
    }
    Usize table_size_bytes = sizeof(TableOperation) * table_size;


    Usize parse_table_size = 0;
    parse_table_size += sizeof(ParseTable);
    Usize table_to_string_header_padding = PADDING_FOR_ALIGNMENT(parse_table_size, StringHeader);
    parse_table_size += table_to_string_header_padding;
    parse_table_size += string_headers_size_bytes;
    Usize string_header_to_expr_header_padding = PADDING_FOR_ALIGNMENT(parse_table_size, ParseExpr);
    parse_table_size += string_header_to_expr_header_padding;
    parse_table_size += expr_headers_size_bytes;
    Usize expr_header_to_table_padding = PADDING_FOR_ALIGNMENT(parse_table_size, TableOperation);
    parse_table_size += expr_header_to_table_padding;
    parse_table_size += table_size_bytes;
    Usize table_to_str_padding = PADDING_FOR_ALIGNMENT(parse_table_size, char);
    parse_table_size += table_to_str_padding;
    parse_table_size += strings_size_bytes;
    Usize str_to_prods_padding = PADDING_FOR_ALIGNMENT(parse_table_size, I64);
    parse_table_size += str_to_prods_padding;
    parse_table_size += prods_size_bytes;

    ParseTable *parse_table = (ParseTable *)calloc(1, parse_table_size);
    if (parse_table == nullptr) return nullptr;



    parse_table->size_in_bytes = (U32)parse_table_size;
    // relative to ParseTable
    parse_table->string_header_start = (U32)(sizeof(*parse_table) + table_to_string_header_padding);
    parse_table->string_header_count = gram->LR_items_count - gram->terminals_count; // should be the same as non terminal count
    parse_table->string_header_size = sizeof(StringHeader);
    // relative to ParseTable
    parse_table->expr_header_start = (U32)(parse_table->string_header_start + sizeof(StringHeader) * parse_table->string_header_count + string_header_to_expr_header_padding);
    parse_table->expr_count = gram->expr_count;
    parse_table->expr_header_size = sizeof(ParseExpr);
    // relative to ParseTable
    parse_table->table_start = (U32)(parse_table->expr_header_start + sizeof(ParseExpr) * parse_table->expr_count + expr_header_to_table_padding);
    parse_table->state_count = state_count;
    parse_table->LR_items_count = gram->LR_items_count;



    U8 *const parse_bin = (U8 *)parse_table;

    assert_always((Usize)(parse_bin + parse_table->string_header_start) % alignof(StringHeader) == 0);
    assert_always((Usize)(parse_bin + parse_table->expr_header_start) % alignof(ParseExpr) == 0);
    assert_always((Usize)(parse_bin + parse_table->table_start) % alignof(TableOperation) == 0);

    // add strings to table
    U8 *string_start = parse_bin + parse_table->table_start + table_size_bytes + table_to_str_padding;
    {
        StringHeader *header_data = (StringHeader *)(parse_bin + parse_table->string_header_start);
        U8 *string_data = string_start;
        // start at end of terminals, (start of non_terminals)
        for (Usize i = gram->terminals_count; i < gram->LR_items_count; ++i)
        {
            StringHeader *header = header_data;
            header->count = (U32)gram->LR_items_str[i].length;
            header->str_start = (U32)(string_data - parse_bin);
            header_data += 1;
            memcpy(string_data, gram->LR_items_str[i].data, sizeof(char) * gram->LR_items_str[i].length);
            string_data += sizeof(char) * gram->LR_items_str[i].length;
        }
        assert_always(string_data == string_start + strings_size_bytes);
    }

    // add expressions to table
    U8 *prod_start = string_start + strings_size_bytes + str_to_prods_padding;
    {
        ParseExpr *header_data = (ParseExpr *)(parse_bin + parse_table->expr_header_start);
        I64 *prod_data = (I64 *)(prod_start);


        for (Usize i = 0; i < gram->expr_count; ++i)
        {
            const BNFExpression *expr = &gram->exprs[i];
            ParseExpr *header = header_data;
            header->non_terminal = expr->non_terminal.type == TokenType::EMPTY ? -1 : expr->non_terminal.lr_item; // -1 in this case represents <S> if no errors
            header->production_count = expr->prod_count;
            header->prod_start = (U32)((U8 *)prod_data - parse_bin);
            header_data += 1;

            for (Usize j = 0; j < expr->prod_count; ++j)
            {
                I64 *prod = prod_data;
                I64 index = expr->prod_tokens[j].lr_item;
                assert_always(index != -1);
                *prod = index;
                prod_data += 1;
            }
        }
        assert_always((U8 *)prod_data == prod_start + prods_size_bytes);

   }



    TableOperation *table_data = (TableOperation *)(parse_bin + parse_table->table_start);

    for (Usize i = 0; i < state_count; ++i)
    {
        State *state = &state_list[i];

        for (Usize j = 0; j < state->expr_count; ++j)
        {
            State_Expression *expr = &state->exprs[j];
            const BNFExpression *bnf_expr = &gram->exprs[expr->grammar_prod_index];
            State *edge = state->edges[j];
            if (edge == nullptr) continue;

            {
                bool already_checked_edge = false;
                for (I64 k = (I64)j - 1; k >= 0; --k)
                {
                    if (state->edges[k] == nullptr) continue;

                    if (is_state(edge, state->edges[k]))
                    {
                        already_checked_edge = true;
                        break;
                    }
                }
                if (already_checked_edge) continue;
            }

            // if (is_str(expr->non_terminal.data, make_string("S"))) continue;


            if (edge->creation_token.type == TokenType::NONTERMINAL)
            {
                TableOperation op = {};
                op.type = TableOperationType::GOTO;
                op.arg = edge->state_id;
                I32 lr_index = edge->creation_token.lr_item;
                table_set(gram, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
            else if (edge->creation_token.type == TokenType::TERMINAL)
            {
                TableOperation op = {};
                op.type = TableOperationType::SHIFT;
                op.arg = edge->state_id;
                I32 lr_index = edge->creation_token.lr_item;
                table_set(gram, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
            else
            {
                assert_always(false);
            }

        }


        for (Usize j = 0; j < state->expr_count; ++j)
        {
            State_Expression *Sexpr = &state->exprs[j];
            const BNFExpression *Bexpr = &gram->exprs[Sexpr->grammar_prod_index];

            if (Sexpr->dot < Bexpr->prod_count &&
                Bexpr->prod_tokens[Sexpr->dot].lr_item == (I32)(gram->terminals_count - 1))
            {
                TableOperation op = {};
                op.type = TableOperationType::ACCEPT;
                op.arg = 0;
                table_set(gram, table_data, meta_expr_table, table_size,
                    Sexpr, (I64)gram->terminals_count - 1, state->state_id, op);
            }
            else if (Sexpr->dot >= Bexpr->prod_count)
            {
                TableOperation op {};
                op.type = TableOperationType::REDUCE;

                I64 index = -1;
                {
                    for (Usize k = 0; k < gram->expr_count; ++k)
                    {
                        if (is_BNFExpression(Bexpr, &gram->exprs[k]))
                        {
                            index = (I64)k;
                            break;
                        }
                    }
                    assert_always(index != -1);
                }

                op.arg = (U32)index;
                assert_always(false);
                // I64 lr_index = get_ir_item_index(gram, expr->look_ahead.data);
                // table_set(gram, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
        }
    }

    return parse_table;
}

static String get_string_from_lr(const ParseTable *table, I64 lr)
{
    //TODO(Johan): check if this is also used for terminals and not only non_terminals
    assert_always(lr >= 0 && lr < table->LR_items_count);


    I64 header_index = lr - (table->LR_items_count - table->string_header_count);

    U8 *parse_bin = (U8 *)table;

    String str = {};
    {
        StringHeader *header_data = (StringHeader *)(parse_bin + table->string_header_start);

        str.data = (char *)(parse_bin + header_data[header_index].str_start);
        str.length = header_data[header_index].count;
        str.stride = 1;
    }
    return str;
}







static bool print_parse_error(char *err_msg_out, Usize *err_msg_size, Usize *err_str_index, const char *format, ...)
{
    if (err_msg_out == nullptr || *err_msg_size <= 0)
    {
        return false;
    }

    va_list args;
    va_start(args, format);
    int char_len = vsnprintf(err_msg_out + *err_str_index, *err_msg_size, format, args);
    va_end(args);
    if (char_len >= 0 && (Usize)char_len < *err_msg_size)
    {
        // successfully wrote string to buffer
        *err_str_index += (Usize)char_len;
        *err_msg_size -= (Usize)char_len;
    }
    else
    {
        // stop print outs after running out of str buffer space
        *err_msg_size = 0;
        return false;
    }
    return true;
}


static bool print_parse_error_string(char *err_msg_out, Usize *err_msg_size, Usize *err_str_index, String str)
{
    if (err_msg_out == nullptr || *err_msg_size <= 0)
    {
        return false;
    }

    if (str.length > *err_msg_size)
    {
        return false;
    }
    for (Usize i = 0; i < str.length; ++i)
    {
        err_msg_out[*err_str_index] = str.data[i * str.stride];
        *err_str_index += 1;
        *err_msg_size -= 1;
    }
    return true;
}



static bool parse_tokens_with_parse_table(const ParseToken *token_list, Usize token_count, const ParseTable *table, U32 flags, Expr **syntax_tree_out, char *err_msg_out, Usize msg_buf_size)
{
    Usize state_stack_size = 1024;
    U32 *state_stack = alloc(U32, state_stack_size);
    Usize state_count = state_stack_size;

    // symbol_stack_size != token_count because a epsilon/nothing string can always be reduced to a non-terminal
    //and as such add arbritrary amount of symbols to the stack
    Usize symbol_stack_size = 1024;
    I64 *symbol_stack = alloc(I64, symbol_stack_size);
    Usize symbol_count = symbol_stack_size;

    // assuming state id 0 is the first state
    state_stack[--state_count] = 0;

    Expr **expr_stack = nullptr;
    Usize expr_stack_count = 0;
    if (syntax_tree_out != nullptr)
    {
        expr_stack = alloc(Expr *, symbol_stack_size);
        expr_stack_count = symbol_stack_size;
    }

    Usize table_width = table->LR_items_count;

    TableOperation *table_data = (TableOperation *)((U8 *)table + table->table_start);

    bool active = true;
    bool succeded_parsing = false;


    Usize err_str_index = 0;

    for (Usize index = 0; active;)
    {
        if (index > token_count)
        {
            active = false;
            succeded_parsing = false;
            break;
        }
        I64 lookahead_lr_index = token_list[index].token_type;
        if (lookahead_lr_index < 0 || lookahead_lr_index >= (I64)table->LR_items_count)
        {
            // provided token not found in grammar
            active = false;
            succeded_parsing = false;
            break;
        }


        TableOperation op = table_data[(Usize)lookahead_lr_index + state_stack[state_count] * table_width];
        switch (op.type)
        {
            case TableOperationType::INVALID:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP_FLAG) == PRINT_EVERY_PARSE_STEP_FLAG)
                {

                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "%s, %u\n", op_to_str(op.type), op.arg);
                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "lookahead_ir_index: %lld, state: %u\n", lookahead_lr_index, state_stack[state_count]);
                }
                print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "Unexpected ");
                print_parse_error_string(err_msg_out, &msg_buf_size, &err_str_index, String {token_list[index].data, token_list[index].length, token_list[index].stride});
                print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, " token");
                // print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "Unexpected %.*s token\n", (int)token_list[index].length, token_list[index].data);
                active = false;
                succeded_parsing = false;
            } break;
            case TableOperationType::SHIFT:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP_FLAG) == PRINT_EVERY_PARSE_STEP_FLAG)
                {
                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "%s, %u\n", op_to_str(op.type), op.arg);
                }
                assert_debug(symbol_count > 0);
                symbol_stack[--symbol_count] = lookahead_lr_index;
                state_stack[--state_count] = op.arg;
                if (syntax_tree_out != nullptr)
                {
                    Expr *expr = alloc(Expr, 1);
                    expr->expr_count = 0;
                    expr->token = token_list[index];
                    assert_debug(expr_stack_count > 0);
                    expr_stack[--expr_stack_count] = expr;
                }
                index += 1;
            } break;
            case TableOperationType::REDUCE:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP_FLAG) == PRINT_EVERY_PARSE_STEP_FLAG)
                {
                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "%s, %u\n", op_to_str(op.type), op.arg);
                }
                assert_debug(op.arg >= 0 && op.arg < table->expr_count);

                // ParseExpr *current_expr =   &lex->exprs[op.arg];

                // shift to correct expr based on op.arg
                ParseExpr *current_expr = op.arg + (ParseExpr *)((U8 *)table + table->expr_header_start);

                I64 left_hand_side_nonterminal = current_expr->non_terminal;
                assert_debug(left_hand_side_nonterminal >= 0);
                Expr *left_hand_expr = nullptr;
                if (syntax_tree_out != nullptr)
                {
                    ParseToken token = {};
                    {
                        String non_terminal_str = get_string_from_lr(table, left_hand_side_nonterminal);
                        token.token_type = left_hand_side_nonterminal;
                        token.data = non_terminal_str.data;
                        token.stride = non_terminal_str.stride;
                        token.length = (U32)non_terminal_str.length;
                    }

                    left_hand_expr = (Expr *)malloc(sizeof(*left_hand_expr) + sizeof(left_hand_expr->exprs[0]) * current_expr->production_count);
                    left_hand_expr->expr_count = 0;
                    left_hand_expr->token = token;
                }

                for (I64 i = (I64)current_expr->production_count - 1; i >= 0; --i)
                {
                    assert_debug(symbol_count <= symbol_stack_size);
                    I64 lr_item = symbol_stack[symbol_count++];
                    I64 prod_lr = ((I64 *)((U8 *)table + current_expr->prod_start))[i];
                    state_count += 1;

                    if (lr_item != prod_lr)
                    {
                        String prod = get_string_from_lr(table, prod_lr);
                        String cur = get_string_from_lr(table, lr_item);

                        // change to better output
                        print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "Expected %.*s but got %.*s\n", (int)prod.length, prod.data, (int)cur.length, cur.data);
                        active = false;
                        succeded_parsing = false;
                        break;
                    }


                    if (syntax_tree_out != nullptr)
                    {
                        assert_debug(expr_stack_count < symbol_stack_size);
                        Expr *e = expr_stack[expr_stack_count++];
                        left_hand_expr->exprs[left_hand_expr->expr_count++] = e;
                    }
                }
                if (syntax_tree_out != nullptr)
                {
                    expr_stack[--expr_stack_count] = left_hand_expr;
                }
                assert_debug(symbol_count > 0);
                symbol_stack[--symbol_count] = left_hand_side_nonterminal;

                Usize top_of_state_stack = state_stack[state_count];

                state_stack[--state_count] = table_data[(Usize)left_hand_side_nonterminal + top_of_state_stack * table_width].arg;

            } break;
            case TableOperationType::GOTO:
            {
                // should never actually happen, as gotos are handled when reducing
                assert_always(false);
                active = false;
                succeded_parsing = false;
            } break;
            case TableOperationType::ACCEPT:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP_FLAG) == PRINT_EVERY_PARSE_STEP_FLAG)
                {
                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "ACCEPT\n");
                }
                active = false;
                succeded_parsing = true;
            } break;

            default:
            {
                assert_always(false && "unreachable");
                active = false;
                succeded_parsing = false;
            }
        }
    }

    if (syntax_tree_out != nullptr && succeded_parsing)
    {
        assert_debug(expr_stack_count < symbol_stack_size);
        *syntax_tree_out = expr_stack[expr_stack_count++];
    }

    free(state_stack);
    free(symbol_stack);
    if (syntax_tree_out != nullptr)
    {
        free(expr_stack);
    }
    return succeded_parsing;
}


Errcode create_all_substates(State *state_list, U32 *state_count, const Grammar *gram)
{
    *state_count = 0;
    State *state = &state_list[0];

    BNFToken first_non = gram->exprs[0].non_terminal;


    // handle if starting production does not have End terminal
    for (Usize i = 0; i < gram->expr_count; ++i)
    {
        const BNFExpression *expr = &gram->exprs[i];

        if (!is_bnf_token(expr->non_terminal, first_non)) continue;



        if (expr->prod_count <= 0)
        {
            return_with_error(1, "ERROR: starting production cannot be empty");
        }
        if (expr->prod_tokens[expr->prod_count - 1].lr_item != (I32)gram->terminals_count - 1)
        {
            return_with_error(1, "ERROR: starting production has to end with 'End' terminal");
        }
    }

    // create first state by taking the closure of S (starting production)
    {
        for (Usize i = 0; i < gram->expr_count; ++i)
        {
            const BNFExpression *expr = &gram->exprs[i];

            if (!is_bnf_token(expr->non_terminal, first_non)) continue;

            State_Expression Sexpr = {};
            Sexpr.dot = 0;
            Sexpr.grammar_prod_index = i;
            Sexpr.look_ahead_count = 0;

            state->exprs[state->expr_count++] = Sexpr;

        }

        state->state_id = 0;
        state->creation_token = BNFToken {.type = TokenType::TERMINAL, .lr_item = (I32)gram->terminals_count};
        push_all_expressions_from_non_terminal_production(state, gram);
        *state_count += 1;
    }

    for (Usize i = 0; i < *state_count; ++i)
    {
        fprintf(stderr, "State %llu\n", i);
        fprint_state(stderr, &state[i], gram);
        fprintf(stderr, "\n");
        create_substates_from_state(&state[i], state_list, state_count, gram);
    }
    return 0;
}

bool graphviz_from_syntax_tree(const char *file_path, Expr *tree_list)
{
    FILE *f = fopen(file_path, "wb");
    if (f == nullptr)
    {
        return_with_error(false, "ERROR: failed to open file %s\n", file_path);
    }

    fprintf(f, "graph G {\n");

    Usize stack_size = 1024;
    Expr **expr_stack = alloc(Expr *, stack_size);
    Usize stack_count = stack_size;

    expr_stack[--stack_count] = tree_list;

    while (stack_count < stack_size)
    {
        assert_debug(stack_count < stack_size);
        Expr *active_expr = expr_stack[stack_count++];


        {
            String ir_str = {active_expr->token.data, active_expr->token.length, active_expr->token.stride};
            fprintf(f, "n%llu [label=\"", (Usize)active_expr);
            for (Usize i = 0; i < ir_str.length; ++i)
            {
                fprintf(f, "%c", ir_str.data[i * ir_str.stride]);
            }
            fprintf(f, "\"];\n");
        }

        for (I64 i = (I64)active_expr->expr_count - 1; i >= 0; --i)
        {
            fprintf(f, "n%llu -- n%llu\n", (Usize)active_expr, (Usize)active_expr->exprs[i]);
            assert_debug(stack_count > 0);
            expr_stack[--stack_count] = active_expr->exprs[i];
        }
    }

    fprintf(f, "}\n");
    free(expr_stack);
    fclose(f);
    return true;
}

U32 write_parse_table_from_bnf(void *buffer, U32 buffer_size, const char *src)
{
    assert_debug(buffer != nullptr);
    ParseTable *table = create_parse_table_from_bnf(src);
    U32 table_size = table->size_in_bytes;
    U32 return_val = 0;
    if (table_size > buffer_size)
    {
        return_val = table_size - buffer_size;
    }
    else
    {
        memcpy(buffer, table, table_size);
    }
    free(table);
    return return_val;
}

ParseTable *create_parse_table_from_bnf(const char *src)
{
    Lexer *lex = alloc(Lexer, 1);
    if (lex == nullptr) return nullptr;

    if (parse_bnf_src(lex, src))
    {
        free(lex);
        return nullptr;
    }


    Grammar *gram = alloc(Grammar, 1);
    if (gram == nullptr)
    {
        free(lex);
        return nullptr;
    }

    if (grammar_from_lexer(gram, lex))
    {
        free(gram);
        free(lex);
        return nullptr;
    }


    State *state_list = alloc(State, 1024);
    if (state_list == nullptr)
    {
        free(gram);
        free(lex);
        return nullptr;
    }

    U32 state_count;
    if (create_all_substates(state_list, &state_count, gram))
    {
        free(state_list);
        free(gram);
        free(lex);
        return nullptr;
    }

    assert_always(false);
    ParseTable *table = create_parse_table_from_states(gram, state_list, state_count);

    free(state_list);
    free(gram);
    free(lex);
    return table;
}


#include "ptg_lexer.cpp"


bool parse(const ParseToken *token_list, U32 token_count, const ParseTable *table, U32 flags, Expr **opt_tree_out, char *opt_error_msg_out, Usize msg_buf_size)
{
    return parse_tokens_with_parse_table(token_list, token_count, table, flags, opt_tree_out, opt_error_msg_out, msg_buf_size);
}

bool parse_bin(const ParseToken *token_list, U32 token_count, const U8 *table, U32 flags, Expr **opt_tree_out, char *opt_error_msg_out, Usize msg_buf_size)
{
    return parse_tokens_with_parse_table(token_list, token_count, (ParseTable *)table, flags, opt_tree_out, opt_error_msg_out, msg_buf_size);
}

U32 get_table_size(ParseTable *table)
{
    return table->size_in_bytes;
}