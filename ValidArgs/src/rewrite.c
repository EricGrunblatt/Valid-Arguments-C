#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"
#include "write.h"

static int limitCounter = 0;
/**
 * @brief Traces out the process in which the term is divided
 *
 * @param term The term to be traced
 * @return int 0 if successful, -1 if not
 */
int reverki_trace(REVERKI_TERM *term, int dotIndex) {
    for(int i = 0; i < dotIndex; i++) {
        fprintf(stderr, ".");
    }
    reverki_unparse_term(term, stderr);
    fprintf(stderr, "\n");
    if(term->type == REVERKI_PAIR_TYPE) {
        reverki_trace(term->value.pair.fst, dotIndex + 1);
        reverki_trace(term->value.pair.snd, dotIndex + 1);
    }
    return 0;
}

int reverki_statistics() {
    fprintf(stderr, "Atoms used: %d, free: %d\n", *pAtomCounter, REVERKI_NUM_ATOMS - *pAtomCounter);
    fprintf(stderr, "Terms used: %d, free: %d\n", *pTermCounter, REVERKI_NUM_TERMS - *pTermCounter);
    fprintf(stderr, "Rules used: %d, free: %d\n", *pRuleCounter, REVERKI_NUM_RULES - *pRuleCounter);
    return 0;
}

REVERKI_TERM *reverki_rewrite_helper(REVERKI_TERM *pat, REVERKI_TERM *tgt, REVERKI_RULE *rule_list, REVERKI_SUBST *subst, int index) {
    if((tgt->type == REVERKI_CONSTANT_TYPE || tgt->type == REVERKI_VARIABLE_TYPE)) {
        if(reverki_match(pat, tgt, subst)) {
            REVERKI_TERM *varcon = reverki_apply(*subst, rule_list->rhs);

            if((global_options & TRACE_OPTION) == TRACE_OPTION) {
                REVERKI_SUBST tempSub = *subst;
                // Trace used
                for(int i = 0; i < index; i++) {
                    fprintf(stderr, ".");
                }
                reverki_unparse_term(tgt, stderr);
                fprintf(stderr, "\n");

                // Rules/Substitution
                fprintf(stderr, "==> rule: ");
                reverki_unparse_rule(rule_list, stderr);
                fprintf(stderr, ", subst: ");
                while(tempSub != NULL) {
                    reverki_unparse_rule(tempSub, stderr);
                    fprintf(stderr, " ");
                    tempSub = tempSub->next;
                }
                fprintf(stderr, ".\n");

                // Term created
                reverki_trace(varcon, index);
            }

            return varcon;
        } else {
            return tgt;
        }
    }
    if(reverki_match(pat, tgt, subst)) {
        REVERKI_TERM *newTerm = reverki_apply(*subst, rule_list->rhs);

        if((global_options & TRACE_OPTION) == TRACE_OPTION) {
            REVERKI_SUBST tempSub = *subst;
            // Trace used
            for(int i = 0; i < index; i++) {
                fprintf(stderr, ".");
            }
            reverki_unparse_term(tgt, stderr);
            fprintf(stderr, "\n");

            // Rules/Substitution
            fprintf(stderr, "==> rule: ");
            reverki_unparse_rule(rule_list, stderr);
            fprintf(stderr, ", subst: ");
            while(tempSub != NULL) {
                reverki_unparse_rule(tempSub, stderr);
                fprintf(stderr, " ");
                tempSub = tempSub->next;
            }
            fprintf(stderr, ".\n");

            // Term created
            reverki_trace(newTerm, index);
        }

        return newTerm;
    }
    REVERKI_TERM *lhs = reverki_rewrite_helper(pat, tgt->value.pair.fst, rule_list, subst, index+1);
    REVERKI_TERM *rhs = reverki_rewrite_helper(pat, tgt->value.pair.snd, rule_list, subst, index+1);
    if(reverki_compare_term(tgt->value.pair.fst, lhs)) {
        if(reverki_compare_term(tgt->value.pair.snd, rhs)) {
            return reverki_make_pair(lhs, rhs);
        }
        return reverki_make_pair(lhs, tgt->value.pair.snd);
    } else if(reverki_compare_term(tgt->value.pair.snd, rhs)) {
        return reverki_make_pair(tgt->value.pair.fst, rhs);
    }
    return tgt;
}


/**
 * @brief  This function rewrites a term, using a specified list of rules.
 * @details  The specified term is rewritten, using the specified list of
 * rules, until no more rewriting is possible (i.e. the result is a
 * "normal form" with respect to the rules).
 * Each rewriting step involves choosing one of the rules, matching the
 * left-hand side of the rule against a subterm of the term being rewritten,
 * and, if the match is successful, returning a new term obtained by
 * replacing the matched subterm by the term obtained by applying the
 * matching substitution to the right-hand side of the rule.
 *
 * The result of rewriting will in general depend on the order in which
 * the rules are tried, as well as the order in which subterms are selected
 * for matching against the rules.  These orders can affect whether the
 * rewriting procedure terminates, as well as what final term results from
 * the rewriting.  We will use a so-called "leftmost-innermost" strategy
 * for choosing the order in which rewriting occurs.  In this strategy,
 * subterms are recursively rewritten, in left-to-right order, before their
 * parent term is rewritten.  Once no further rewriting is possible for
 * subterms, rewriting of the parent term is attempted.  Since rewriting
 * of the parent term affects what subterms there are, each time the parent
 * term is changed rewriting is restarted, beginning again with recursive
 * rewriting of the subterms.  Rules are always tried in the order in which
 * they occur in the rule list: a rule occurring later in the list is
 * never used to rewrite a term unless none of the rules occurring earlier
 * can be applied.
 *
 * @param rule_list  The list of rules to be used for rewriting.
 * @param term  The term to be rewritten.
 * @return  The rewritten term.  This term should have the property that
 * no further rewriting will be possible on it or any of its subterms,
 * using rules in the specified list.
 */
REVERKI_TERM *reverki_rewrite(REVERKI_RULE *rule_list, REVERKI_TERM *term) {
    // Pointer to substitution list
    REVERKI_TERM *newTerm = term;
    REVERKI_RULE *tempList = rule_list;
    while(tempList != NULL) {

        if((global_options & LIMIT_OPTION) == LIMIT_OPTION) {
            int limit = global_options >> 32;
            if(limit < limitCounter) {
                fprintf(stderr, "Rewrite limit exceeded\n");
                abort();
            }
        }

        REVERKI_RULE *substRule = NULL;
        REVERKI_SUBST *gSubst = &substRule;

        // Loop until compared terms are equal
        reverki_rewrite_helper(tempList->lhs, newTerm, tempList, gSubst, 0);

        if(reverki_compare_term(newTerm, (reverki_term_storage + *pTermCounter-1))) {
            newTerm = (reverki_term_storage + *pTermCounter-1);
            limitCounter++;
            return reverki_rewrite(rule_list, (reverki_term_storage + *pTermCounter-1));
        } else {
            tempList =  tempList->next;
        }
    }

    return term;
}
