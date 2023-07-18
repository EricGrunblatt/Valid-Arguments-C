#include <stdlib.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"
#include "write.h"

/**
 * @brief returns 0 if strings are not equal, 1 if they are
 * @details function will go through each string together using the char pointer. If
 * the current char for each string are not equal, 0 is returned. If both reach the end 
 * at the same time, 1 is returned and the strings are equal
 * 
 * @param a The first string being compared
 * @param b The second string being compared
 * @return 0 if strings are not equal, 1 if they are
 */
int equalStrings(char *a, char *b) {
    while(*a != '\0' || *b != '\0') {
        if(*a == *b) {
            a++;
            b++;
        } else if((*a == '\0' && *b != '\0') || (*a != '\0' && *b == '\0') || (*a != *b)) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief parses the string to an integer. If the integer is invalid 0 is returned, 
 * otherwise 1 is returned
 * @details checks if the input is 0 or NULL. If not, it loops through each character to find the 
 * length of the number. It then sets the maximum integer to maxNum and will run a for loop going through
 * each character and parsing it to an integer, then multiplying the currNum by 10 and adding the new integer
 * until the length is reached. It then checks if that number surpassed the maximum integer. If not, it is a valid
 * number. 
 * 
 * @param a The number entered from the command line
 * @return 0 if the digit is not valid, 1 if it is 
 */
int isValidDigit(char *a) {
    if(a == NULL || a == 0) 
        return 0;
    int length = 0;
    char *curr = a;
    while(*curr != '\0') {
        length++;
        curr++;
    }
    int currNum = 0;
    int maxNum = __INT_MAX__;
    for(int i = 0; i < length; i++) {
        if(*a < '0' || *a > '9' || currNum > maxNum || (currNum == maxNum && *a > '7')) 
            return 0;
        currNum = (currNum * 10) + (*a - '0');
        a++;
    }
    if(currNum < 1) {
        return 0;
    }
    return 1;
}

/**
 * @brief Shifts the number created by the limit to match the proper format for global_options
 * @details loops through the number and parsing each integer as it goes through, then adding it to the 
 * number that will later be returned. Once the number is created, it will be multiplied by 2 32 times 
 * to reach the desired format of the limit being returned.
 * 
 * @param a The number entered from the command line
 * @return The number to be added to global_options
 */
long stringToLong(char *a) {
    long currNum = 0;
    while(*a != '\0') {
        currNum = (currNum * 10) + (*a - '0');
        a++;
    }
    currNum = currNum << 32;
    return currNum;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {
    long local_options;
    argv++;
    // Check if arg count is < 1
    if(argc < 1) {
        fprintf(stderr, "Invalid number of args\n");
        return -1;

    // Check if the second argument is "-h"
    } else if(equalStrings(*argv, "-h\0")) {
        global_options = HELP_OPTION;
        return 0;

    // Check if the second argument is "-v"
    } else if(equalStrings(*argv, "-v\0")) {
        argv++;
        int useS = 0;
        local_options = VALIDATE_OPTION;
        for(int i = 2; i < argc; i++) {
            if(!equalStrings(*argv, "-s\0") || equalStrings(*argv, "-v\0") || (equalStrings(*argv, "-s\0") && useS)) {
                local_options = 0;
                fprintf(stderr, "Invalid argument for -v\n");
                return -1;
            } else if(equalStrings(*argv, "<\0")) {
                return 0;
            } else {
                local_options += STATISTICS_OPTION;
                useS = 1;
            }
            argv++;
        }
        global_options = local_options;
        return 0;

    /* Check if second argument is "-r" */
    } else if(equalStrings(*argv, "-r\0")) {
        argv++;
        int useL = 0, useS = 0, useT = 0;
        local_options = REWRITE_OPTION;
        for(int i = 2; i < argc; i++) {
            if(equalStrings(*argv, "-l\0") && !useL) {
                argv++;
                i++;
                if(!isValidDigit(*argv)) {
                    local_options = 0;
                    fprintf(stderr, "Invalid limit\n");
                    return -1;
                } else {
                    local_options += LIMIT_OPTION;
                    local_options += stringToLong(*argv);
                    useL = 1;
                }
            } else if(equalStrings(*argv, "-s\0") && !useS) {
               local_options += STATISTICS_OPTION;
               useS = 1;
            } else if(equalStrings(*argv, "-t\0") && !useT) {
                local_options += TRACE_OPTION;
                useT = 1;
            } else if(equalStrings(*argv, "<\0")) {
                return 0;
            } else {
                local_options = 0;
                fprintf(stderr, "Invalid argument for -r\n");
                return -1;
            }
            argv++;
        }
        global_options = local_options;
        return 0;
    }
    fprintf(stderr, "Invalid\n");
    return -1;
}
