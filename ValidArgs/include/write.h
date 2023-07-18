// Trace function for rewrite
extern int reverki_trace(REVERKI_TERM *term, int dotIndex);

// Statistics function
extern int reverki_statistics();

// Counter for all of the rules
extern int *pRuleCounter;

// Counter for all of the terms
extern int *pTermCounter;

// Counter for all of the atoms
extern int *pAtomCounter;

// Checks if two character strings are equal
extern int equalStrings(char *a, char *b);