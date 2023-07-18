#include <stdio.h>
#include <stdlib.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"
#include "write.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);

    int result = validargs(argc, argv);
    if(result || ((global_options & HELP_OPTION) == HELP_OPTION)) {
        USAGE(*argv, EXIT_SUCCESS);
    } else if((global_options & VALIDATE_OPTION) == VALIDATE_OPTION ||
    (global_options & REWRITE_OPTION) == REWRITE_OPTION) {
        // PARSING TERMS AND RULES/REVERKI_MATCH
        int c;
        while((c = fgetc(stdin)) != EOF) {
            // '(' indicates start of term
            if(c == 40) {
                ungetc(c, stdin);
                REVERKI_TERM *newTerm = reverki_parse_term(stdin);
                if(newTerm == NULL) { abort(); }
            // '[' indicates start of rule
            } else if(c == 91) {
                ungetc(c, stdin);
                reverki_parse_rule(stdin);
            } else if(c == 41) {
                fprintf(stderr, "Encountered ), which is invalid");
            } else if(c == 93) {
                fprintf(stderr, "Encountered ], which is invalid");
            }
        }
        if((global_options & VALIDATE_OPTION) == VALIDATE_OPTION) {
            if((global_options & STATISTICS_OPTION) == STATISTICS_OPTION) {
                reverki_statistics();
            }
        } else {
            if((global_options & TRACE_OPTION) == TRACE_OPTION) {
                for(int i = 0; i < *pRuleCounter; i++) {
                    fprintf(stderr, "# ");
                    reverki_unparse_rule((reverki_rule_storage + i), stderr);
                    fprintf(stderr, "\n");
                }
                fprintf(stderr, "# ");
                reverki_unparse_term((reverki_term_storage + *pTermCounter-1), stderr);
                fprintf(stderr, "\n");
                reverki_trace((reverki_term_storage + *pTermCounter-1), 0);
            }

            REVERKI_TERM *newTerm = reverki_rewrite((reverki_rule_storage + *pRuleCounter-1), (reverki_term_storage + *pTermCounter-1));
            if(stdout == NULL) {
                reverki_unparse_term(newTerm, stderr);
                fprintf(stderr, "\n");
            } else {
                reverki_unparse_term(newTerm, stdout);
                fputc('\n', stdout);
            }

            if((global_options & STATISTICS_OPTION) == STATISTICS_OPTION) {
                reverki_statistics();
            }
        }

    }



    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
