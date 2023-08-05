#include "ptg.hpp"
#include <stdlib.h>
#include "ptg_internal.hpp"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


static bool is_non_terminal(I32 lr, const Grammar *gram)
{
    return lr >= (I32)gram->terminals_count && lr < (I32)gram->LR_items_count;
}
static bool is_terminal(I32 lr, const Grammar *gram)
{
    return lr < (I32)gram->terminals_count && lr >= 0;
}



// https://cs.stackexchange.com/questions/152523/how-is-the-lookahead-for-an-lr1-automaton-computed
// https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf
// https://jsmachines.sourceforge.net/machines/lalr1.html


// TODO list:
// TODO: add better error handling when creating parsing table
// TODO: when parsing, output the amount of chars written
// TODO: predict required memory to parse list of tokens, if possible

// TODO(Johan): make thread safe
static char g_msg_buffer[1024];
const char *get_last_error()
{
    return g_msg_buffer;
}

Usize get_error_size()
{
    Usize c = 0;

    while (g_msg_buffer[c] != '\0') c += 1;

    return c;
}


__attribute__((format(printf, 1, 2)))
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
    assert_debug(bexpr->non_terminal >= (I32)gram->terminals_count);
    String non_terminal_str = gram->LR_items_str[bexpr->non_terminal];


    assert_debug(non_terminal_str.stride == 1);
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

        if (bexpr->prod_tokens[i] >= 0)
        {
            String expr_str = gram->LR_items_str[bexpr->prod_tokens[i]];
            if (is_terminal(bexpr->prod_tokens[i], gram))
            {
                fprintf(f, "\'%.*s\'", (int)expr_str.length, expr_str.data);
            }
            else if (is_non_terminal(bexpr->prod_tokens[i], gram))
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
    {
        bool first_found = false;
        for (Usize i = 0; i < gram->LR_items_count; ++i)
        {
            if (expr->look_aheads[i])
            {
                String look_str = gram->LR_items_str[i];
                assert_always(look_str.stride == 1);
                if (!first_found)
                {
                    fprintf(f, "%.*s", (int)look_str.length, look_str.data);
                    first_found = true;
                }
                else
                {
                    fprintf(f, ", %.*s", (int)look_str.length, look_str.data);
                }
            }
        }
    }
    fprintf(f, "]");
}

static void fprint_BNF(FILE *stream, const BNFExpression *expr, const Grammar *gram)
{
    String non_terminal_str = gram->LR_items_str[expr->non_terminal];

    assert_always(non_terminal_str.stride == 1);
    fprintf(stream, "<%.*s> :=", (int)(non_terminal_str.length), non_terminal_str.data);
    for (Usize i = 0; i < expr->prod_count; ++i)
    {
        fprintf(stream, " ");

        if (expr->prod_tokens[i] >= 0)
        {
            String expr_str = gram->LR_items_str[expr->prod_tokens[i]];
            if (is_terminal(expr->prod_tokens[i], gram))
            {
                fprintf(stream, "\'%.*s\'", (int)expr_str.length, expr_str.data);
            }
            else if (is_non_terminal(expr->prod_tokens[i], gram))
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
    if (b0->non_terminal != b1->non_terminal) return false;
    if (b0->prod_count != b1->prod_count) return false;

    for (Usize i = 0; i < b0->prod_count; ++i)
    {
        if (b0->prod_tokens[i] != b1->prod_tokens[i]) return false;
    }

    return true;
}


static bool is_state_expression_ignore_lookahead(const State_Expression *e0, const State_Expression *e1)
{
    if (e0->dot != e1->dot) return false;
    if (e0->grammar_prod_index != e1->grammar_prod_index) return false;

    return true;
}

static bool is_state_ignore_lookahead(const State *s0, const State *s1)
{
    assert_always(s0->expr_count <= 512);
    assert_always(s1->expr_count <= 512);


    if (s0->creation_token != s1->creation_token) return false;
    if (s0->expr_count != s1->expr_count) return false;


    for (Usize i = 0; i < s0->expr_count; ++i)
    {
        if (!is_state_expression_ignore_lookahead(&s0->exprs[i], &s1->exprs[i]))
        {
            return false;
        }
    }

    return true;
}



static bool is_state_expression(const Grammar *gram, const State_Expression *e0, const State_Expression *e1)
{
    if (e0->dot != e1->dot) return false;
    if (e0->grammar_prod_index != e1->grammar_prod_index) return false;

    for (Usize i = 0; i < gram->LR_items_count; ++i)
    {
        if (e0->look_aheads[i] != e1->look_aheads[i])
        {
            return false;
        }
    }
    return true;
}



static bool is_state(const Grammar *gram, const State *s0, const State *s1)
{
    assert_always(s0->expr_count <= 512);
    assert_always(s1->expr_count <= 512);


    if (s0->creation_token != s1->creation_token) return false;
    if (s0->expr_count != s1->expr_count) return false;


    for (Usize i = 0; i < s0->expr_count; ++i)
    {
        if (!is_state_expression(gram, &s0->exprs[i], &s1->exprs[i]))
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


static bool is_expr_already_expanded(const Grammar *gram, const State *state, const State_Expression *expr)
{
    for (Usize i = 0; i < state->expr_count; ++i)
    {
        if (is_state_expression(gram, &state->exprs[i], expr))
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

static bool has_lookahead(const State_Expression *expr, I32 terminal)
{
    return expr->look_aheads[terminal];
}




/*
    For an item like A → α.B with a lookahead of {L}, add new rules
    like B → .γ with a lookahead of {L}.
    • For an item like A → α.Bβ, with a lookahead of {L}, add new rules
    like B → .γ with a lookahead as follows:
    – If β cannot produce epsilon, the lookahead is FIRST(β).
    – If β can produce epsilon, the lookahead is FIRST(β) ∪ {L}
*/
//TODO(Johan) fix
static void push_all_expressions_from_non_terminal_production(State *state, const Grammar *gram)
{
    assert_always(state->expr_count < ARRAY_COUNT(state->exprs));

    for (Usize i = 0; i < state->expr_count; ++i)
    {
        State_Expression *rule_expand_Sexpr = &state->exprs[i];
        const BNFExpression *rule_expand_Bexpr = &gram->exprs[rule_expand_Sexpr->grammar_prod_index];        

        if (rule_expand_Sexpr->dot >= rule_expand_Bexpr->prod_count) continue;


        I32 non_terminal_to_expand = rule_expand_Bexpr->prod_tokens[rule_expand_Sexpr->dot];
        if (!is_non_terminal(non_terminal_to_expand, gram)) continue;



        bool dot_on_last = rule_expand_Sexpr->dot + 1 == rule_expand_Bexpr->prod_count;
        if (dot_on_last)
        {
            for (U32 k = 0; k < gram->expr_count; ++k)
            {
                const BNFExpression *Bexpr = &gram->exprs[k];
                if (Bexpr->non_terminal != non_terminal_to_expand) continue;

                State_Expression expr = {};
                expr.dot = 0;
                expr.grammar_prod_index = k;

                I32 index = get_expr_index_ignore_lookahead(state, &expr);

                if (index >= 0)
                {
                    for (Usize l = 0; l < gram->terminals_count; ++l)
                    {
                        if (rule_expand_Sexpr->look_aheads[l])
                        {
                            state->exprs[index].look_aheads[l] = true;
                        }
                    }
                }
                else
                {
                    for (Usize l = 0; l < gram->terminals_count; ++l)
                    {
                        if (rule_expand_Sexpr->look_aheads[l])
                        {
                            expr.look_aheads[l] = true;
                        }
                    }
                    state->exprs[state->expr_count++] = expr;
                }
            }
        }
        else
        {
            for (U32 k = 0; k < gram->expr_count; ++k)
            {
                const BNFExpression *Bexpr = &gram->exprs[k];
                if (Bexpr->non_terminal != non_terminal_to_expand) continue;


                State_Expression expr = {};
                expr.dot = 0;
                expr.grammar_prod_index = k;

                U32 prod_token_index = rule_expand_Sexpr->dot + 1; 
                assert_debug(prod_token_index < rule_expand_Bexpr->prod_count);

                I32 index = get_expr_index_ignore_lookahead(state, &expr);
                if (index >= 0)
                {
                    while (prod_token_index != rule_expand_Bexpr->prod_count)
                    {
                        I32 active_lr = rule_expand_Bexpr->prod_tokens[prod_token_index];
                        for (Usize l = 0; l < gram->terminals_count; ++l)
                        {
                            if (gram->first_sets[active_lr].terminals[l])
                            {
                                state->exprs[index].look_aheads[l] = true;
                            }
                        }

                        if (gram->first_sets[active_lr].produces_empty_set)
                        {
                            prod_token_index += 1;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (prod_token_index == rule_expand_Bexpr->prod_count)
                    {
                        for (Usize l = 0; l < gram->terminals_count; ++l)
                        {
                            if (rule_expand_Sexpr->look_aheads[l])
                            {
                                state->exprs[index].look_aheads[l] = true;
                            }
                        } 
                    }
                }
                else
                {
                    while (prod_token_index != rule_expand_Bexpr->prod_count)
                    {
                        I32 active_lr = rule_expand_Bexpr->prod_tokens[prod_token_index];
                        for (Usize l = 0; l < gram->terminals_count; ++l)
                        {
                            if (gram->first_sets[active_lr].terminals[l])
                            {
                                expr.look_aheads[l] = true;
                            }
                        }

                        if (gram->first_sets[active_lr].produces_empty_set)
                        {
                            prod_token_index += 1;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (prod_token_index == rule_expand_Bexpr->prod_count)
                    {
                        for (Usize l = 0; l < gram->terminals_count; ++l)
                        {
                            if (rule_expand_Sexpr->look_aheads[l])
                            {
                                expr.look_aheads[l] = true;
                            }
                        } 
                    }
                    state->exprs[state->expr_count++] = expr;
                }
            }
        }
    }
}


static void create_substates_from_state(State *state, State *state_list, U32 *state_list_count, const Grammar *gram)
{
    assert_always(state->expr_count < ARRAY_COUNT(state->exprs));
    bool *check_list = alloc(bool, state->expr_count);
    State *active_substate = alloc(State, 1);

    for (Usize i = 0; i < state->expr_count; ++i)
    {
        State_Expression active_Sexpr = state->exprs[i];
        const BNFExpression *active_Bexpr = &gram->exprs[active_Sexpr.grammar_prod_index];


        if (active_Sexpr.dot >= active_Bexpr->prod_count) continue;



        // State *active_substate = &g_active_substate;
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
                active_Bexpr->prod_tokens[active_Sexpr.dot] != Bexpr->prod_tokens[Sexpr->dot])
                {
                    continue;
                }
            // skip shifting End token as it is the end token and means the parsing completed successfully
            if (Bexpr->prod_tokens[Sexpr->dot] == (I32)(gram->terminals_count - 1)) continue;

            active_substate->creation_token = active_Bexpr->prod_tokens[active_Sexpr.dot];

            active_substate->exprs[active_substate->expr_count] = *Sexpr;
            active_substate->exprs[active_substate->expr_count].dot += 1;

            State_Expression *last_Sexpr = &active_substate->exprs[active_substate->expr_count];
            const BNFExpression *last_Bexpr = &gram->exprs[last_Sexpr->grammar_prod_index];

            active_substate->expr_count += 1;


            if (last_Sexpr->dot < last_Bexpr->prod_count && 
                is_non_terminal(last_Bexpr->prod_tokens[last_Sexpr->dot], gram))
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
                if (is_state_ignore_lookahead(active_substate, &state_list[k]))
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
                for (Usize k = 0; k < active_substate->expr_count; ++k)
                {
                    State_Expression *state_to_point_to_expr = &state_to_point_to->exprs[k];
                    State_Expression *expr = &active_substate->exprs[k];
                    for (Usize l = 0; l < gram->LR_items_count; ++l)
                    {
                        if (expr->look_aheads[l])
                        {
                            state_to_point_to_expr->look_aheads[l] = true;
                        }
                    }
                }
            }


            for (Usize k = 0; k < state->expr_count; ++k)
            {
                State_Expression *k_Sexpr = &state->exprs[k];
                const BNFExpression *k_Bexpr = &gram->exprs[k_Sexpr->grammar_prod_index];


                if (k_Sexpr->dot < k_Bexpr->prod_count &&
                    active_substate->creation_token == k_Bexpr->prod_tokens[k_Sexpr->dot])
                {
                    state->edges[k] = state_to_point_to;
                }
            }
        }
    }
    

    free(active_substate);
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
                String creation_token_str = gram->LR_items_str[edge->creation_token];
                if (is_non_terminal(edge->creation_token, gram))
                {

                    assert_always(creation_token_str.stride == 1);
                    fprintf(f, " [label=\"%.*s\"];\n",
                        (int)creation_token_str.length, creation_token_str.data);
                }
                else if (is_terminal(edge->creation_token, gram))
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






void print_first_sets(const Grammar *gram)
{
    for (Usize i = 0; i < gram->LR_items_count; ++i)
    {
        String lr_first_set_str = gram->LR_items_str[i];
        assert_always(lr_first_set_str.stride == 1);
        printf("%.*s [", (int)lr_first_set_str.length, lr_first_set_str.data);
        FirstSet *set = &gram->first_sets[i];
        bool found_first = false;
        for (Usize j = 0; j < gram->terminals_count; ++j)
        {
            if (set->terminals[j])
            {
                if (!found_first)
                {
                    String first_terminal = gram->LR_items_str[j];
                    assert_always(first_terminal.stride == 1);
                    printf("%.*s", (int)first_terminal.length, first_terminal.data);
                    found_first = true;
                    continue;
                }

                {
                    String terminal_in_first_set = gram->LR_items_str[j];
                    assert_always(terminal_in_first_set.stride == 1);
                    printf(", %.*s", (int)terminal_in_first_set.length, terminal_in_first_set.data);
                }
            }
        }
        if (set->produces_empty_set)
        {
            printf(", empt]\n");
        }
        else
        {
            printf("]\n");
        }
    }
}


void print_table(const ParseTable *table)
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
            printf("<%d> ->", header->non_terminal);
            I32 *prod_start = (I32 *)(table_bin + header->prod_start);
            for (Usize j = 0; j < header->production_count; ++j)
            {
                printf(" %d", prod_start[j]);
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
                printf("[%s %2u] ", op_to_str_short_color(op->type), op->arg);
            }
            printf("\n");
            // flush is needed here because if stdout is a file then \n does not flush (that is the theory atleast).
            // Which causes the internal printf buffer to get full.
            fflush(stdout);
        }
    }
}



static void table_set(const Grammar *gram, TableOperation *table, State_Expression **meta_expr_table, Usize table_size,
    State_Expression *expr, I32 look_ahead_index, Usize state_id, TableOperation op)
{
    assert_always(look_ahead_index >= 0);

    Usize index = (Usize)look_ahead_index + state_id * gram->LR_items_count;
    assert_always(index < table_size);(void)table_size;
    switch (table[index].type)
    {
        case TableOperationType::INVALID:
        {
            table[index] = op;
            meta_expr_table[index] = expr;
        } break;
        case TableOperationType::ACCEPT:
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
                assert_always(false && "Fix");
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

                State_Expression *table_sexpr = meta_expr_table[index];


                if (table_sexpr->grammar_prod_index < expr->grammar_prod_index)
                {
                    #if 1
                    printf("Choosing table %s resolution\n", op_to_str(table[index].type));
                    #endif
                }
                else
                {
                    #if 1
                    printf("Choosing %s resolution\n", op_to_str(op.type));
                    #endif
                    table[index] = op;
                    meta_expr_table[index] = expr;
                }
            }

        } break;
        case TableOperationType::GOTO:
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
        prods_size_bytes += sizeof(I32) * expr->prod_count;
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
    Usize str_to_prods_padding = PADDING_FOR_ALIGNMENT(parse_table_size, I32);
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
        I32 *prod_data = (I32 *)(prod_start);


        for (Usize i = 0; i < gram->expr_count; ++i)
        {
            const BNFExpression *expr = &gram->exprs[i];
            ParseExpr *header = header_data;
            header->non_terminal = expr->non_terminal;
            header->production_count = expr->prod_count;
            header->prod_start = (U32)((U8 *)prod_data - parse_bin);
            header_data += 1;

            for (Usize j = 0; j < expr->prod_count; ++j)
            {
                I32 *prod = prod_data;
                I32 index = expr->prod_tokens[j];
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
            // const BNFExpression *bnf_expr = &gram->exprs[expr->grammar_prod_index];
            State *edge = state->edges[j];
            if (edge == nullptr) continue;

            {
                bool already_checked_edge = false;
                for (I32 k = (I32)j - 1; k >= 0; --k)
                {
                    if (state->edges[k] == nullptr) continue;

                    if (is_state(gram, edge, state->edges[k]))
                    {
                        already_checked_edge = true;
                        break;
                    }
                }
                if (already_checked_edge) continue;
            }

            // if (is_str(expr->non_terminal.data, make_string("S"))) continue;


            if (is_non_terminal(edge->creation_token, gram))
            {
                TableOperation op = {};
                op.type = TableOperationType::GOTO;
                op.arg = edge->state_id;
                I32 lr_index = edge->creation_token;
                table_set(gram, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
            else if (is_terminal(edge->creation_token, gram))
            {
                TableOperation op = {};
                op.type = TableOperationType::SHIFT;
                op.arg = edge->state_id;
                I32 lr_index = edge->creation_token;
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
                Bexpr->prod_tokens[Sexpr->dot] == (I32)(gram->terminals_count - 1))
            {
                TableOperation op = {};
                op.type = TableOperationType::ACCEPT;
                op.arg = 0;
                table_set(gram, table_data, meta_expr_table, table_size,
                    Sexpr, (I32)gram->terminals_count - 1, state->state_id, op);
            }
            else if (Sexpr->dot >= Bexpr->prod_count)
            {
                TableOperation op {};
                op.type = TableOperationType::REDUCE;


                op.arg = Sexpr->grammar_prod_index;

                for (I32 k = 0; k < (I32)gram->LR_items_count; ++k)
                {
                    if (Sexpr->look_aheads[k])
                    {
                        table_set(gram, table_data, meta_expr_table, table_size, Sexpr, k, state->state_id, op);
                    }
                }
            }
        }
    }

    return parse_table;
}

static String get_string_from_lr(const ParseTable *table, I32 lr)
{
    //TODO(Johan): check if this is also used for terminals and not only non_terminals
    assert_always(lr >= 0 && lr < (I32)table->LR_items_count);


    I32 header_index = lr - (I32)(table->LR_items_count - table->string_header_count);

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






__attribute__((format(printf, 4, 5)))
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
    Stack<U32> state_stack; init_stack(&state_stack);
    Stack<I32> symbol_stack; init_stack(&symbol_stack);

    push(&state_stack, (U32)0);


    Stack<Expr *> expr_stack;
    if (syntax_tree_out != nullptr)
    {
        init_stack(&expr_stack);
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
        I32 lookahead_lr_index = token_list[index].token_type;
        if (lookahead_lr_index < 0 || lookahead_lr_index >= (I32)table_width)
        {
            // provided token not found in grammar
            active = false;
            succeded_parsing = false;
            break;
        }


        TableOperation op = table_data[(Usize)lookahead_lr_index + state_stack.data[state_stack.count - 1] * table_width];
        switch (op.type)
        {
            case TableOperationType::INVALID:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP_FLAG) == PRINT_EVERY_PARSE_STEP_FLAG)
                {

                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "%s, %u\n", op_to_str(op.type), op.arg);
                    print_parse_error(err_msg_out, &msg_buf_size, &err_str_index, "lookahead_lr_index: %d, state: %u\n", lookahead_lr_index, state_stack.data[state_stack.count - 1]);
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
                push(&symbol_stack, lookahead_lr_index);
                push(&state_stack, op.arg);
                if (syntax_tree_out != nullptr)
                {
                    Expr *expr = alloc(Expr, 1);
                    expr->expr_count = 0;
                    expr->token = token_list[index];
                    push(&expr_stack, expr);
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

                // shift to correct expr based on op.arg
                ParseExpr *current_expr = op.arg + (ParseExpr *)((U8 *)table + table->expr_header_start);

                I32 left_hand_side_nonterminal = current_expr->non_terminal;
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
                    I32 prod_lr = ((I32 *)((U8 *)table + current_expr->prod_start))[i];
                    I32 lr_item = pop(&symbol_stack);
                    pop(&state_stack);

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
                        Expr *e = pop(&expr_stack);
                        left_hand_expr->exprs[left_hand_expr->expr_count++] = e;
                    }
                }
                if (syntax_tree_out != nullptr)
                {
                    push(&expr_stack, left_hand_expr);
                }
                push(&symbol_stack, left_hand_side_nonterminal);

                Usize top_of_state_stack = state_stack.data[state_stack.count - 1];
                push(&state_stack, table_data[(Usize)left_hand_side_nonterminal + top_of_state_stack * table_width].arg);

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
        *syntax_tree_out = pop(&expr_stack);
    }

    free_stack(&state_stack);
    free_stack(&symbol_stack);

    if (syntax_tree_out != nullptr)
    {
        free_stack(&expr_stack);
    }
    return succeded_parsing;
}


Errcode create_all_substates(State *state_list, U32 *state_count, const Grammar *gram)
{
    *state_count = 0;
    State *state = &state_list[0];

    I32 first_non = gram->exprs[0].non_terminal;


    // handle if starting production does not have End terminal
    {
        const BNFExpression *expr = &gram->exprs[0];
        if (expr->prod_count <= 0)
        {
            return_with_error(1, "ERROR: starting production cannot be empty\n");
        }
        if (expr->prod_tokens[expr->prod_count - 1] != (I32)gram->terminals_count - 1)
        {
            String str = gram->LR_items_str[gram->terminals_count - 1];
            assert_debug(str.stride == 1);
            return_with_error(1, "ERROR: starting production has to end with '%.*s' (the last terminal in the tokens) terminal\n", (int)str.length, str.data);
        }
        for (Usize i = 1; i < gram->expr_count; ++i)
        {
            if (gram->exprs[i].non_terminal == (I32)gram->terminals_count)
            {
                return_with_error(1, "ERROR: cannot have multiple starting productions\n");
            }
        }
    }

    // create first state by taking the closure of the (starting production)
    {
        for (U32 i = 0; i < gram->expr_count; ++i)
        {
            const BNFExpression *expr = &gram->exprs[i];

            if (expr->non_terminal != first_non) continue;

            State_Expression Sexpr = {};
            Sexpr.dot = 0;
            Sexpr.grammar_prod_index = i;

            state->exprs[state->expr_count++] = Sexpr;

        }

        state->state_id = 0;
        // the starting non terminal is the last lr item.
        state->creation_token = (I32)gram->terminals_count;
        push_all_expressions_from_non_terminal_production(state, gram);
        *state_count += 1;
    }

    for (Usize i = 0; i < *state_count; ++i)
    {
        create_substates_from_state(&state[i], state_list, state_count, gram);
    }
    //TODO(Johan) fix so duplication is not needed.
    // if a earlier substate gets modified the change is not propagated correctly
    // as such a repeat run through is done to fix it.
    for (Usize i = 0; i < *state_count; ++i)
    {
        create_substates_from_state(&state[i], state_list, state_count, gram);
    }

    for (Usize i = 0; i < *state_count; ++i)
    {
        fprintf(stderr, "State %llu\n", i);
        fprint_state(stderr, &state[i], gram);
        fprintf(stderr, "\n");
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


    Stack<Expr *> expr_stack; init_stack(&expr_stack);



    push(&expr_stack, tree_list);

    while (expr_stack.count != 0)
    {
        Expr *active_expr = pop(&expr_stack);

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
            push(&expr_stack, active_expr->exprs[i]);
        }
    }

    fprintf(f, "}\n");
    free_stack(&expr_stack);
    fclose(f);
    return true;
}

bool write_parse_table_from_bnf(void *buffer, U32 *buffer_size, const char *src)
{
    assert_debug(buffer != nullptr);
    assert_debug(buffer_size != nullptr);
    assert_debug(src != nullptr);
    ParseTable *table = create_parse_table_from_bnf(src);
    if (table == nullptr)
    {
        return false;
    }

    bool return_val = true;
    U32 table_size = table->size_in_bytes;
    if (table_size > *buffer_size)
    {
        *buffer_size = table_size - *buffer_size;
        print_formated("Buffer smaller than table");
        return_val = false;   
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
    // print_first_sets(gram);

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

    // FILE *f = fopen("./state_input.dot", "wb");
    // graph_from_state_list(f, state_list, state_count, gram);
    // fclose(f);
    // fprintf(stderr, "DEBUG: created graph of states\n");

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

U32 get_table_size(const ParseTable *table)
{
    return table->size_in_bytes;
}