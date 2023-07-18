/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>

/*
 * USAGE macro to be called from main() to print a help message and exit
 * with a specified exit status.
 */
#define USAGE(program_name, retcode) do { \
fprintf(stderr, "USAGE: %s %s\n", program_name, \
"[-h] [-v|-r] [-t|-s] [-l LIMIT]\n" \
"   -h       Help: displays this help menu.\n" \
"If -h is not specified, then exactly one of -v or -r must be used, and this argument must\n" \
"be the first.\n" \
"   -v       Validate: the program parses the input, but performs no rewriting.\n" \
"   -r       Rewrite: the program performs rewriting of terms read from the input.\n" \
"The following may be used either with -v or -r\n" \
"   -s       Statistics: shows summary statistics before the program terminates.\n" \
"The following may only be used with -r\n" \
"   -t       Trace: displays trace information during rewriting (may only be used with -r).\n" \
"   -l       Limit: The associated numeric LIMIT argument specifies the maximum number of\n" \
"            steps of rewriting that the program will perform (range: 1 - 2^32-1).  If the\n" \
"            number of rewriting steps would exceed this limit, then the program aborts.\n" \
"            If this option is not specified, then there is no limit on the number of steps.\n" \
); \
exit(retcode); \
} while(0)

/*
 * Options info, set by validargs.
 *   If -h is specified, then the HELP_OPTION bit is set.
 *   If -v is specified, then the VALIDATE_OPTION bit is set.
 *   If -r is specified, then the REWRITE_OPTION bit is set.
 *   If -t is specified, then the TRACE_OPTION bit is set.
 *   If -s is specified, then the STATISTICS_OPTION bit is set.
 *   If -l is specified, then the LIMIT_OPTION bit is set, and the
 *     most-significant four bytes contain the specified limit on the number
 *     of rewriting steps to be performed.  If -l is not specified,
 *     then LIMIT_OPTION is not set, the most-significant four bytes are 0,
 *     and no limit is imposed.
 */
long global_options;

#define HELP_OPTION (0x00000001)
#define VALIDATE_OPTION (0x00000002)
#define REWRITE_OPTION (0x00000004)
#define TRACE_OPTION (0x00000008)
#define STATISTICS_OPTION (0x00000010)
#define LIMIT_OPTION (0x00000020)

/*
 * Buffer for accumulating the print name of an atom during parsing.
 * You *must* use this, because you are not allowed to declare any arrays.
 */
char reverki_pname_buffer[REVERKI_PNAME_BUFFER_SIZE];

/*
 * Storage for atoms.  Every atom is stored in one of the structures in this
 * array.  An entry in the array is in use if and only if it has a non-zero
 * type field.  At any time, there can be at most one atom with a given pname.
 * We do this so that atoms can be compared using pointer equality, rather
 * than having to compare their pnames.
 */
#define REVERKI_NUM_ATOMS 100
REVERKI_ATOM reverki_atom_storage[REVERKI_NUM_ATOMS];

/*
 * Atom-related functions that you are to implement.
 * See the stubs in atom.c for specifications.
 */
extern REVERKI_ATOM *reverki_parse_atom(FILE *in);
extern int reverki_unparse_atom(REVERKI_ATOM *atom, FILE *out);

/*
 * Storage for terms.  Every term is stored in one of the structures in this
 * array.  An entry in the array is in use if and only if it has a non-zero
 * type field.  It is permitted for two distinct entries in this array to
 * represent equal terms.  This means that to compare terms for equality,
 * we actually have to compare them using a recursive traversal.  Although we
 * could arrange that no two terms in the array are equal (as we do for atoms),
 * this makes things more complicated and we are not going to go there.
 */
#define REVERKI_NUM_TERMS 10000
REVERKI_TERM reverki_term_storage[REVERKI_NUM_TERMS];

/*
 * Term-related functions that you are to implement.
 * See the stubs in term.c for specifications.
 */
extern REVERKI_TERM *reverki_make_variable(REVERKI_ATOM *atom);
extern REVERKI_TERM *reverki_make_constant(REVERKI_ATOM *atom);
extern REVERKI_TERM *reverki_make_pair(REVERKI_TERM *fst, REVERKI_TERM *snd);
extern REVERKI_TERM *reverki_parse_term(FILE *in);
extern int reverki_unparse_term(REVERKI_TERM *term, FILE *out);
extern int reverki_compare_term(REVERKI_TERM *term1, REVERKI_TERM *term2);

/*
 * Storage for rules.  Every rule is stored in one of the structures in this
 * array.  An entry in the array is in use if and only if its left-hand side
 * is non-NULL.
 */
#define REVERKI_NUM_RULES 1000
REVERKI_RULE reverki_rule_storage[REVERKI_NUM_RULES];

/*
 * Rule-related functions that you are to implement.
 * See the stubs in rule.c for specifications.
 */
extern REVERKI_RULE *reverki_make_rule(REVERKI_TERM *lhs, REVERKI_TERM *rhs);
extern REVERKI_RULE *reverki_parse_rule(FILE *in);
extern int reverki_unparse_rule(REVERKI_RULE *term, FILE *out);

/*
 * Substitution-related functions that you are to implement.
 * See the stubs in subst.c for specifications.
 */
extern int reverki_match(REVERKI_TERM *pat, REVERKI_TERM *tgt, REVERKI_SUBST *substp);
extern REVERKI_TERM *reverki_apply(REVERKI_SUBST subst, REVERKI_TERM *term);

/*
 * Function you are to implement that performs rewriting of a specified term,
 * given a list of rules.  See the stub in rewrite.c for specifications.
 */
extern REVERKI_TERM *reverki_rewrite(REVERKI_RULE *rule_list, REVERKI_TERM *term);

/*
 * Function you are to implement that validates and interprets command-line arguments
 * to the program.  See the stub in validargs.c for specifications.
 */
extern int validargs(int argc, char **argv);

#endif
