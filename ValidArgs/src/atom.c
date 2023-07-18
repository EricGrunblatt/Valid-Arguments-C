#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"
#include "write.h"

int atomCounter = 0;
int *pAtomCounter = &atomCounter;

/**
 * @brief returns true if the ascii value pertains to whitespace
 * 
 * @param val The ascii value of the current character 
 * @return int 1 if the value is whitespace, 0 if not
 */
int isWhiteSpace(int val) {
    if((val > 8 && val < 14) || val == 32 || val == 40 || val == 41 || val == 44 || val == 91 || val == 93) {
        return 1;
    }
    return 0;
}

/**
 * @brief returns true if the ascii value pertains to '(', ')', '[',  ']', or ','
 * 
 * @param val The ascii value of the current character
 * @return int 1 if the value is one of those characters mentioned in @brief, 0 if not
 */
int validFirstChar(int val) {
    if(val == 40 || val == 41 || val == 44 || val == 91 || val == 93) {
        return 0;
    }
    return 1;
}

/*
 * @brief  Parse an atom  from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as an atom.  An atom may start with any non-whitespace character that is not
 * a left '(' or right '(' parenthesis, a left '[' or right ']' square bracket,
 * or a comma ','.  If the first character read is one of '(', ')', '[', ']', then it
 * is not an error; instead, the character read is pushed back into the input stream
 * and NULL is returned.  Besides the first character, an atom may consist
 * of any number of additional characters (other than whitespace and the punctuation
 * mentioned previously), up to a maximum length of REVERKI_PNAME_BUFFER_SIZE-1.
 * If this maximum length is exceeded, then an error message is printed (on stderr)
 * and NULL is returned.  When a whitespace or punctuation character is encountered
 * that signals the end of the atom, this character is pushed back into the input
 * stream.  If the atom is terminated due to EOF, no character is pushed back.
 * An atom that starts with a lower-case letter has type REVERKI_VARIABLE_TYPE,
 * otherwise it has type REVERKI_CONSTANT_TYPE.  An atom is returned having the
 * sequence of characters read as its pname, and having the type determined by this
 * pname.  There can be at most one atom having a given pname, so if the pname read
 * is already the pname of an existing atom, then a pointer to the existing atom is
 * returned; otherwise a pointer to a new atom is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the atom, if the parse was successful, otherwise NULL if
 * an error occurred.
 */
REVERKI_ATOM *reverki_parse_atom(FILE *in) {
    if(in == NULL) { return NULL; }
    
    int c;
    if((c = fgetc(in)) != EOF) {
        // First character is invalid
        if(!validFirstChar(c)) {
            if(c != EOF) { ungetc(c, in); }
            return NULL;

        // Type Variable
        } else {
            int index = 0;

            // Add to pname buffer
            do {
                *(reverki_pname_buffer + index) = c;
                index++;
            } while(!isWhiteSpace(c = fgetc(in)) && index < REVERKI_PNAME_BUFFER_SIZE-1 && c != EOF);
            *(reverki_pname_buffer + index) = '\0';

            if(index >= REVERKI_PNAME_BUFFER_SIZE-1 && !isWhiteSpace(c)) {
                return NULL;
            }
            if(isWhiteSpace(c)) {
                ungetc(c, in);
            }

            //Check if atom exists
            index = 0;
            while(index <= atomCounter  && index < REVERKI_NUM_ATOMS) {
                if(equalStrings((reverki_atom_storage + index)->pname, reverki_pname_buffer)) {
                    return (reverki_atom_storage + index);
                }
                index++;
            }

            index = atomCounter;

            // Type variable
            if(*(reverki_pname_buffer + 0) > 96 && *(reverki_pname_buffer + 0) < 123) { 
                (reverki_atom_storage + index)->type = REVERKI_VARIABLE_TYPE; 
            }
            // Type constant
            else { (reverki_atom_storage + index)->type = REVERKI_CONSTANT_TYPE; } 

            // Copy pname buffer to newAtom.pname
            int charIndex = 0;
            while(*(reverki_pname_buffer + charIndex) != '\0') {
                *((reverki_atom_storage + index)->pname + charIndex) = *(reverki_pname_buffer + charIndex);
                charIndex++;
            }
            atomCounter++;
            if(atomCounter > REVERKI_NUM_ATOMS) {
                fprintf(stderr, "Atom limit exceeded");
                return NULL;
            }
            return (reverki_atom_storage + index);
        }
    }
    return NULL;

}

/*
 * @brief  Output the pname of a specified atom to the specified output stream.
 * @details  The pname of the specified atom is output to the specified output stream.
 * If the output is successful, then 0 is returned.  If any error occurs then the
 * value EOF is returned.
 * @param atom  The atom whose pname is to be printed.
 * @param out  Stream to which the pname is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_atom(REVERKI_ATOM *atom, FILE *out) {
    int index = 0;
    if(*(atom->pname + index) == '\0') {
        return EOF;
    }
    do {
        fprintf(out, "%c", *(atom->pname + index));
        index++;
    } while(*(atom->pname + index) != '\0' && *(atom->pname + index) != 127);

    return 0;
}

