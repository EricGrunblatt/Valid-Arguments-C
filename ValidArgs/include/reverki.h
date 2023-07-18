/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef REVERKI_H
#define REVERKI_H

/*
 * Data structure definitions for "Reverki".
 */

/*
 * Type codes for Reverki atoms and terms.
 */
typedef enum {
    REVERKI_NO_TYPE = 0,
    REVERKI_VARIABLE_TYPE = 1,
    REVERKI_CONSTANT_TYPE = 2,
    REVERKI_PAIR_TYPE = 3
} REVERKI_TYPE;

/*
 * An atom is an identifier that represents either a variable or a constant.
 * Each atom has a "pname" (print name), which is how the atom is represented
 * in input or output.  The type of the atom is related to the form of the
 * pname: if the pname starts with a lower-case letter, then the atom is a
 * variable, otherwise it is a constant.
 */
#define REVERKI_PNAME_BUFFER_SIZE 64
typedef struct reverki_atom {
    REVERKI_TYPE type;		            // Type (variable or constant).
    char pname[REVERKI_PNAME_BUFFER_SIZE];  // Print name (null-terminated string).
    struct reverki_atom *next;              // For linking atoms into a list.
} REVERKI_ATOM;

/*
 * A term is either:
 *   A variable, consisting of an atom;
 *   A constant, consisting an atom;
 *   A pair consisting of two subterms.
 */
typedef struct reverki_term {
    REVERKI_TYPE type;			// Type (variable, constant, or pair).
    union {
	REVERKI_ATOM *atom;		// Atom (if variable or constant).
	struct {
	    struct reverki_term *fst;	// First subterm (if pair).
	    struct reverki_term *snd;	// Second subterm (if pair).
	} pair;
    } value;
} REVERKI_TERM;

/*
 * A rule is a pair consisting of two terms: a left-hand-side and a right-hand-side.
 */
typedef struct reverki_rule {
    REVERKI_TERM *lhs;             // The left-hand side.
    REVERKI_TERM *rhs;	           // The right-hand side.
    struct reverki_rule *next;     // For linking rules into a list.
} REVERKI_RULE;

/*
 * A substitution is a mapping that takes variables to terms.
 * Although by doing so we will violate information hiding principles,
 * to avoid introducing yet another class of objects with an associated storage
 * area, we will represent a substitution as a singly linked list of rules,
 * where the LHS of each rule in the list is a term that consists of a single
 * variable and the RHS of each rule is an arbitrary term.  It is required that
 * a given variable can occur as the LHS of at most one rule.
 */
typedef REVERKI_RULE *REVERKI_SUBST;

#endif
