#include <stdlib.h>
#include <stdio.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"
#include "write.h"

int addSubsToList(REVERKI_TERM *pat, REVERKI_TERM *tgt, int beforeRuleCount) {
    // Pattern and target are both pair types, recurse through it again
    if(pat->type == REVERKI_PAIR_TYPE && tgt->type == REVERKI_PAIR_TYPE) {
        return addSubsToList(pat->value.pair.fst, tgt->value.pair.fst, beforeRuleCount) + addSubsToList(pat->value.pair.snd, tgt->value.pair.snd, beforeRuleCount);

    // Pattern and target are both constants
    } else if(pat->type == REVERKI_CONSTANT_TYPE && tgt->type == REVERKI_CONSTANT_TYPE) {
        // Compare terms, if equal return 0, otherwise, return error
        if(!reverki_compare_term(pat, tgt)) {
            return 0;
        }
        return -100;

    //
    } else if(pat->type == REVERKI_VARIABLE_TYPE) {
        // Check if rule exists by looping through each of the rules
        for(int i = beforeRuleCount; i < *pRuleCounter; i++) {

            // If left of rule matches variable
            if(!reverki_compare_term((reverki_rule_storage + i)->lhs, pat)) {

                // If right of rule does not match subterm
                if(reverki_compare_term((reverki_rule_storage + i)->rhs, tgt)) {
                    return -100;
                } else {
                    return 0;
                }
            }
        }

        *(reverki_rule_storage + *pRuleCounter) = *reverki_make_rule(pat, tgt);
        ++*pRuleCounter;
        return 1;
    }

    // Anything else results in an error
    return -100;
}

/**
 * @brief  Match a specified pattern term against a specified target term,
 * returning a substitution in case of a successful match.
 * @details  The pattern is compared with the target, by performing a simultaneous
 * recursive traversal of both.  Constants occurring in the pattern must match
 * exactly corresponding constants in the target.  Variables occurring in the
 * pattern may match any corresponding subterm of the target, except that if there
 * is more than one instance of a particular variable in the pattern, then all
 * instances of that variable must be matched to identical subterms of the target.
 * As the traversal proceeds, additional bindings of variables to subterms are
 * accumulated by prepending them to a substitution, which is accessed and
 * updated via a by-reference parameter.  If the match succeeds, a nonzero value
 * is returned and the final substitution can be obtained by the caller from this
 * by-reference parameter.  If the match should fail at some point, any partially
 * accumulated substitution is recycled, the by-reference parameter is reset to
 * NULL, and 0 is returned.
 * @param pat  The term to be used as the pattern.
 * @param tgt  The term to be used as the target.
 * @param substp  A by-reference parameter that is a pointer to a variable
 * containing a substitution.  The caller should declare such a variable,
 * initialize it to NULL, and pass its address to this function.
 * Upon nonzero return from this function, the variable will contain the
 * accumulated substitution that is the result of the matching procedure.
 * The caller is responsible for recycling this substitution once it is no
 * longer needed.  Upon zero return from this function this variable will
 * have value NULL and the caller should not attempt to use it.
 * @return  Nonzero if the match is successful, 0 otherwise.  If the match is
 * successful, then the by-reference parameter substp will contain a pointer
 * to the accumulated substitution.
 */
int reverki_match(REVERKI_TERM *pat, REVERKI_TERM *tgt, REVERKI_SUBST *substp) {
    int beforeRuleCount = *pRuleCounter;
    // Add to substp if numNewRules > 0
    int numNewRules = addSubsToList(pat, tgt, beforeRuleCount);

    if(numNewRules < 1) {
        int afterRuleCount = *pRuleCounter;
        int beforeAfterDiff = afterRuleCount - beforeRuleCount;
        for(int i = afterRuleCount - 1; i >= afterRuleCount-beforeAfterDiff; i--) {
            (reverki_rule_storage + i)->lhs = NULL;
            (reverki_rule_storage + i)->rhs = NULL;
            (reverki_rule_storage + i)->next = NULL;
            --*pRuleCounter;
        }
        return 0;
    }

    // Add to substp
    REVERKI_SUBST temp = *substp;
    int ruleStart = *pRuleCounter - numNewRules;
    for(int i = *pRuleCounter-1; i >= ruleStart; i--) {
        (reverki_rule_storage + i)->next = (reverki_rule_storage + i-1);
        if(i == ruleStart) {
            (reverki_rule_storage + i)->next = NULL;
        }
    }
    *substp = (reverki_rule_storage + *pRuleCounter-1);

    return 1;
}

REVERKI_TERM *reverki_apply_helper(REVERKI_SUBST subst, REVERKI_TERM *term) {
    REVERKI_SUBST temp = subst;
    while(temp != NULL) {
        if(temp->lhs && !reverki_compare_term(temp->lhs, term)) {
            return temp->rhs;
        }

        temp = temp->next;
    }
    return term;
}

/**
 * @brief  Apply a substitution to a term, producing a term, which in some cases
 * could be the same term as the argument.
 * @details  This function applies a substitution to a term and produces a result
 * term.  A substitution is applied to a term by recursively traversing the term
 * and, for each variable that is encountered, if that variable is one of the
 * key variables mapped by the substitution, replacing that variable by the
 * corresponding value term.  Because terms are immutable, if applying a
 * substitution results in a change to one of the subterms of a pair, then the
 * pair is not modified; rather, a new pair is constructed that contains the new
 * subterm and the other subterm that was not changed.
 * @param subst  The substitution to be applied.
 * @param term  The term to which to apply the substitution.
 * @return  The term constructed by applying the substitution to the term passed
 * as argument.
 */
REVERKI_TERM *reverki_apply(REVERKI_SUBST subst, REVERKI_TERM *term) {
    REVERKI_TERM termForReturn;
    REVERKI_TERM *returnTerm;
    returnTerm = &termForReturn;
    // Recurse through the array
    // If the variable in subst is found, try to replace it
    if(term->type == REVERKI_PAIR_TYPE) {
        REVERKI_TERM *lhs, *rhs;

        if(term->value.pair.fst->type == REVERKI_VARIABLE_TYPE) {
            REVERKI_TERM *newTerm = reverki_apply_helper(subst, term->value.pair.fst);
            lhs = newTerm;

        } else if(term->value.pair.fst->type == REVERKI_PAIR_TYPE) {
            lhs = reverki_apply(subst, reverki_apply(subst, term->value.pair.fst));

        } else {
            lhs = term->value.pair.fst;
        }

        if(term->value.pair.snd->type == REVERKI_VARIABLE_TYPE) {
            REVERKI_TERM *newTerm = reverki_apply_helper(subst, term->value.pair.snd);
            rhs = newTerm;

        } else if(term->value.pair.snd->type == REVERKI_PAIR_TYPE) {
            rhs = reverki_apply(subst, term->value.pair.snd);

        } else {
            rhs = term->value.pair.snd;
        }

        returnTerm = reverki_make_pair(lhs, rhs);
        return returnTerm;
    } else if(term->type == REVERKI_VARIABLE_TYPE) {
        return reverki_apply_helper(subst, term);
    }
    return term;
}
