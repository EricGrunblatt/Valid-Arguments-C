#include <stdlib.h>
#include <stdio.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"
#include "write.h"

int termCounter = 0;
int *pTermCounter = &termCounter;

/**
 * @brief returns true if the ascii value pertains to an invalid character for a term
 * 
 * @param val The ascii value of the current character 
 * @return int 1 if the value is invalid, 0 if not
 */
int invalidCharForTerm(int val) {
    if((val > 8 && val < 14) || val == 32 || val == 44 || val == 91 || val == 93) {
        return 1;
    }
    return 0;
}

/*
 * @brief  Create a variable term from a specified atom.
 * @details  A term of type REVERKI_VARIABLE is created that contains the
 * specified atom.  The atom must also have type REVERKI_VARIABLE, otherwise
 * an error message is printed and the program aborts.
 * @param atom  The atom from which the term is to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_variable(REVERKI_ATOM *atom) {
    if(atom->type == REVERKI_VARIABLE_TYPE) {
        // Set variable type and union type
        int index = termCounter;
        (reverki_term_storage + termCounter)->type = REVERKI_VARIABLE_TYPE;
        (reverki_term_storage + termCounter)->value.atom = atom;
        termCounter++;
        if(termCounter > REVERKI_NUM_TERMS) {
            fprintf(stderr, "Term limit exceeded");
            return NULL;
        }
        return (reverki_term_storage + index);
    }

    fprintf(stderr, "Atom given is not of type variable\n");
    return NULL;
}

/*
 * @brief  Create a constant term from a specified atom.
 * @details  A term of type REVERKI_CONSTANT is created that contains the
 * specified atom.  The atom must also have type REVERKI_CONSTANT, otherwise
 * an error message is printed and the program aborts.
 * @param atom  The atom from which the constant is to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_constant(REVERKI_ATOM *atom) {
    if(atom->type == REVERKI_CONSTANT_TYPE) {
        // Set constant type and union type
        int index = termCounter;
        (reverki_term_storage + termCounter)->type = REVERKI_CONSTANT_TYPE;
        (reverki_term_storage + termCounter)->value.atom = atom;
        termCounter++;
        if(termCounter > REVERKI_NUM_TERMS) {
            fprintf(stderr, "Term limit exceeded");
            return NULL;
        }
        return (reverki_term_storage + index);
    }

    fprintf(stderr, "Atom given is not of type constant\n");
    return NULL;
}

/*
 * @brief  Create a pair term from specified subterms.
 * @details  A term of type REVERKI_PAIR is created that contains specified
 * terms as its first and second subterms.
 * @param fst  The first (or "left-hand") subterm of the pair to be constructed.
 * @param snd  The second (or "right-hand") subterm of the pair to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_pair(REVERKI_TERM *fst, REVERKI_TERM *snd) {
    // Set pair type and union type
    int index = termCounter;
    (reverki_term_storage + termCounter)->type = REVERKI_PAIR_TYPE;
    (reverki_term_storage + termCounter)->value.pair.fst = fst;
    (reverki_term_storage + termCounter)->value.pair.snd = snd;
    termCounter++;
    if(termCounter > REVERKI_NUM_TERMS) {
        fprintf(stderr, "Term limit exceeded");
        return NULL;
    }
    return (reverki_term_storage + index);
}

/*
 * @brief  Compare two specified terms for equality.
 * @details  The two specified terms are compared for equality.  Equality of terms
 * means that they have the same type and that corresponding atoms or subterms they
 * contain are recursively equal.
 * @param term1  The first of the two terms to be compared.
 * @param term2  The second of the two terms to be compared.
 * @return  Zero if the specified terms are equal, otherwise nonzero.
 */
int reverki_compare_term(REVERKI_TERM *term1, REVERKI_TERM *term2) {
    if(term1->type != term2->type) {
        return -1;
    } else if(term1->type == REVERKI_VARIABLE_TYPE || term1->type == REVERKI_CONSTANT_TYPE) {
        if(term1->value.atom->pname != term2->value.atom->pname) {
            return -1;
        }
        return 0;
    } else if(term1->type == REVERKI_PAIR_TYPE) {
        if(term1->value.pair.fst->type == term2->value.pair.fst->type &&
        term1->value.pair.snd->type == term2->value.pair.snd->type) {
            return reverki_compare_term(term1->value.pair.fst, term2->value.pair.fst) || reverki_compare_term(term1->value.pair.snd, term2->value.pair.snd);
        }
        return -1;
    }
    return 0;
}

/*
 * @brief  Parse a term from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as a term.  A term may either have the form of a single atom, or of a
 * sequence of terms enclosed by left parenthesis '(' and right parenthesis ')'.
 * Arbitrary whitespace may appear before a term or in between the atoms and punctuation
 * that make up the term.  Any such whitespace is not part of the term, and is ignored.
 * When a whitespace or punctuation character is encountered that signals the end of
 * the term, this character is pushed back into the input stream.  If the term is
 * terminated due to EOF, no character is pushed back.  A term of the form
 * ( a b c d ... ) is regarded as an abbreviation for ( ... ( ( a b ) c ) d ) ...);
 * that is, parentheses in a term may be omitted under the convention that the subterms
 * associate to the left.  If, while reading a term, a syntactically incorrect atom
 * or improperly matched parentheses are encountered, an error message is issued
 * (to stderr) and NULL is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the newly created term, if parsing was successful,
 * otherwise NULL.
 */
REVERKI_TERM *reverki_parse_term(FILE *in) {
    if(in == NULL) { return NULL; }
    int c;
    // TODO
    if((c = fgetc(in)) == EOF) { return NULL; }

    // Whitespace
    while((c > 8 && c < 14) || c == 32) {
        c = fgetc(in);
    }
    // Comma, [ or ]
    if(c == 44 || c == 91 || c == 93) {
        ungetc(c, in);
        return NULL;

    // Character is (, so start pair
    } else if(c == 40){
        REVERKI_TERM *lhs, *rhs;
        int lhsBool = 0, rhsBool = 0;

        //Looping until ) is reached
        while((c = fgetc(in)) != 41 && c != EOF) {
            // If open parentheses and lhs is null, parse the left hand side
            if((c == 40 || (c > 32 && c != 44 && c != 91 && c != 93 && c != 127)) && !lhsBool) {
                ungetc(c, in);
                lhs = reverki_parse_term(in);
                if(lhs != NULL) { lhsBool = 1; }

            // If open parentheses and lhs is not null, parse the right hand side
            } else if((c == 40 || (c > 32 && c != 44 && c != 91 && c != 93 && c != 127)) && lhsBool) {
                ungetc(c, in);
                rhs = reverki_parse_term(in);
                if(rhs != NULL) { rhsBool = 1; }


            // If anything valid, parse the atom
            } else if (c > 32 && c != 44 && c != 91 && c != 93 && c != 127) {
                ungetc(c, in);
                REVERKI_ATOM *atom = reverki_parse_atom(in);
                if(atom != NULL && atom->type == REVERKI_VARIABLE_TYPE) { 
                    if(!lhsBool) { lhs = reverki_make_variable(atom); lhsBool = 1; } 
                    else { rhs = reverki_make_variable(atom); rhsBool = 1; }

                }
                else if(atom != NULL && atom->type == REVERKI_CONSTANT_TYPE) { 
                    if(!lhsBool) { lhs = reverki_make_constant(atom); lhsBool = 1; }
                    else { rhs = reverki_make_constant(atom); rhsBool = 1; }
                } else if(atom == NULL) {
                    return NULL;
                }

            // If lhs and rhs are filled, but more atoms are seen
            } else if(lhsBool && rhsBool && c != 41) { 
                int counter = 0;
                REVERKI_TERM *newPair;
                newPair = reverki_make_pair(lhs, rhs);
                while(c != 41 && c != EOF) {
                    if(c > 32 && c != 40 && c != 44 && c != 91 && c != 93 && c != 127) {
                        ungetc(c, in);
                        REVERKI_ATOM *atom = reverki_parse_atom(in);
                        REVERKI_TERM *term;

                        if(atom != NULL && atom->type == REVERKI_VARIABLE_TYPE) {
                            term = reverki_make_variable(atom);
                        } else if(atom != NULL && atom->type == REVERKI_CONSTANT_TYPE) { 
                            term = reverki_make_constant(atom); 
                        } else if(atom == NULL) { 
                            return NULL; 
                        }

                        if(term != 0) {
                            REVERKI_TERM *tempPair;
                            tempPair = reverki_make_pair(newPair, term);
                            if(tempPair == NULL) {
                                return NULL;
                            }
                            newPair = tempPair;
                        }
                    } else if(c == 40) {
                        ungetc(c, in);
                        REVERKI_TERM *temp;
                        temp = reverki_parse_term(in);
                        if(temp == NULL) { return NULL; }
                        REVERKI_TERM *tempPair;
                        tempPair = reverki_make_pair(newPair, temp);
                        if(tempPair == NULL) { return NULL; }
                        newPair = tempPair;
                    }
                    c = fgetc(in);
                }
                return newPair;
            }
        }

        if(lhsBool && rhsBool) {
            REVERKI_TERM *term;
            term = reverki_make_pair(lhs, rhs);
            if(term == NULL) { return NULL; }
            return term;
            //return reverki_make_pair(lhs, rhs);
        }
        return NULL;

    } else if(c > 32 && c != 44 && c != 91 && c != 93 && c != 127) {
        ungetc(c, in);
        REVERKI_ATOM *atom;
        atom = reverki_parse_atom(in);
        if(atom != NULL && atom->type == REVERKI_VARIABLE_TYPE) {
            return reverki_make_variable(atom);
        }
        else if(atom != NULL && atom->type == REVERKI_CONSTANT_TYPE) {
            return reverki_make_constant(atom);
        }
        return NULL;
    }

    return NULL;
}

/*
 * @brief  Output a textual representation of a specified term to a specified output stream.
 * @details  A textual representation of the specified term is output to the specified
 * output stream.  The textual representation is of a form from which the original term
 * can be reconstructed using reverki_parse_term.  If the output is successful, then 0
 * is returned.  If any error occurs then the value EOF is returned.
 * @param term  The term that is to be printed.
 * @param out  Stream to which the term is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_term(REVERKI_TERM *term, FILE *out) {
    if(term->type == REVERKI_PAIR_TYPE) {
        fprintf(out, "(");
        if(term->value.pair.fst->type == REVERKI_PAIR_TYPE) {
            reverki_unparse_term(term->value.pair.fst, out);
            fprintf(out, " ");
        } else {
            fprintf(out, "%s ", term->value.pair.fst->value.atom->pname);
        }

        if(term->value.pair.snd->type == REVERKI_PAIR_TYPE) {
            reverki_unparse_term(term->value.pair.snd, out);
        } else {
            reverki_unparse_atom(term->value.pair.snd->value.atom, out);
        }
        fprintf(out, ")");
    } else if(term->type == REVERKI_CONSTANT_TYPE || term->type == REVERKI_VARIABLE_TYPE) {
        fprintf(out, "%s", term->value.atom->pname);
    } else {
        return EOF;
    }
    return 0;
}