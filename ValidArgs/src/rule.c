#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"
#include "write.h"

int ruleCounter = 0;
int *pRuleCounter = &ruleCounter;


/*
 * @brief  Create a rule with a specified left-hand side and right-hand side terms.
 * @details  A rule is created that contains specified terms as its left-hand side
 * and right-hand side.
 * @param lhs  Term to use as the left-hand side of the rule.
 * @param rhs  Term to use as the right-hand side of the rule.
 * @return  A pointer to the newly created rule.
 */
REVERKI_RULE *reverki_make_rule(REVERKI_TERM *lhs, REVERKI_TERM *rhs) {
    // Create new rule and pointer to new rule
    REVERKI_RULE newRule;
    REVERKI_RULE *pNewRule;
    pNewRule = &newRule;

    // Specify the left-hand/right-hand sides
    newRule.lhs = lhs;
    newRule.rhs = rhs;

    return pNewRule;
}

/*
 * @brief  Parse a rule from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as a rule.  A rule starts with a left square bracket '[', followed by a term,
 * then a comma ',' and a second term, and finally terminated by a right square bracket ']'.
 * Arbitrary whitespace may appear before a term or in between the atoms and punctuation
 * that make up the rule.  If, while parsing a rule, the first non-whitespace character
 * seen is not the required initial left square bracket, then the character read is
 * pushed back into the input stream and NULL is returned.  If, while parsing a rule,
 * the required commma ',' or right square bracket ']' is not seen, or parsing one of
 * the two subterms fails, then the unexpected character read is pushed back to the
 * input stream, an error message is issued (to stderr) and NULL is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the newly created rule, if parsing was successful,
 * otherwise NULL.
 */
REVERKI_RULE *reverki_parse_rule(FILE *in) {
    // TO BE IMPLEMENTED.
    REVERKI_TERM *lhs, *rhs;
    int commaEncountered = 0;
    int c;
    while((c = fgetc(in)) != EOF) {
        // Start rule
        while(c < 33) {
            c = fgetc(in);
        }

        if(c == 91) {
            continue;

        // End rule
        } else if(c == 93) {
            if(ruleCounter <= REVERKI_NUM_RULES) {
                int index = ruleCounter;
                ruleCounter++;
                (reverki_rule_storage + index)->lhs = lhs;
                (reverki_rule_storage + index)->rhs = rhs;
                if(index > 0) {
                    (reverki_rule_storage + index)->next = (reverki_rule_storage + (index-1));
                } else {
                    (reverki_rule_storage + index)->next = NULL;
                }
                return (reverki_rule_storage + index);
            } else {
                fprintf(stderr, "Rule limit exceeded");
                return NULL;
            }
        // Parse term
        } else if(c == 40 || (c > 32 && c != 44 && c != 91 && c != 93 && c != 127)) {
            // Left side term
            if(!commaEncountered) {
                ungetc(c, in);
                lhs = reverki_parse_term(in);
                if(lhs == NULL) { return NULL; }

            // Right side term
            } else {
                ungetc(c, in);
                rhs = reverki_parse_term(in);
                if(rhs == NULL) { return NULL; }
            }

        // Encountered comma
        } else if(c == 44) {
            commaEncountered = 1;
        }
    }

    return NULL;
}

/*
 * @brief  Output a textual representation of a specified rule to a specified output stream.
 * @details  A textual representation of the specified rule is output to the specified
 * output stream.  The textual representation is of a form from which the original rule
 * can be reconstructed using reverki_parse_rule.  If the output is successful, then 0
 * is returned.  If any error occurs then the value EOF is returned.
 * @param rule  The rule to be printed.
 * @param out  Stream to which the rule is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_rule(REVERKI_RULE *rule, FILE *out) {
    fprintf(out, "[");
    reverki_unparse_term(rule->lhs, out);
    fprintf(out, ", ");
    reverki_unparse_term(rule->rhs, out);
    fprintf(out, "]");
    return 0;
}

