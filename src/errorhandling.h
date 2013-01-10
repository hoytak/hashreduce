/***************************************************
 *
 * Defines errors that can occur in the program.
 *
 **************************************************/

#ifndef PGERRORS_H
#define PGERRORS_H

#include <stdio.h>
#include <string.h>

typedef int error_code;


/*****************************************
 *
 *  Different error codes.
 *
 ****************************************/

#define NO_ERROR 0

#define PGE_MEMORY_ERROR 1


/* File IO  */

#define PGE_EOF  32
#define PGE_PARSING_ERROR 33


void _printErrorString(error_code ecode, char *code_msg, char *msg);
void printError(error_code ecode, char *msg);

/********************************************************************************
 *
 *  Macros for errors and warnings.
 *
 ********************************************************************************/

/* Use a warning when an expected condition is not met, but the
 * program will likely not become unstable.
 */

#define WARNING(condition, m)						\
    do{									\
	if(unlikely((condition)) )					\
	{								\
	    fprintf(stderr, "\nWARNING %s", m);				\
	    fprintf(stderr, "\nTriggered in %s, function %s, line %d, %s\n", \
		    __FILE__, __func__, __LINE__, #condition);		\
	    fflush(stderr);						\
	}								\
    }while(0)

/* Use an error when an expected condition is not met and the program
 * may become unstable or not behave as expected as a result.
 */

#define ERROR(condition, m)						\
    do{									\
	if(unlikely((condition)) )					\
	{								\
	    fprintf(stderr, "\nERROR: %s", m);				\
	    fprintf(stderr, "\nTriggered in %s, function %s, line %d, condition %s", \
		    __FILE__, __func__, __LINE__, #condition);		\
	    fprintf(stderr, "\nProgram may become unstable or not behave as expected.\n"); \
	    fflush(stderr);						\
	}								\
    }while(0)

/* A general macro to check memory variables.  Instead of putting a
 * lot of error towards handling null pointers, we simply write an
 * error message to stderr in anticipation of a segfault.
 */

void MemCheckViolation(const char* varname, const char* file, const char* func, int line);

#define MEMCHECK(var)							\
    do{									\
	if(unlikely(var == NULL) )					\
	    MemCheckViolation(#var, __FILE__, __func__, __LINE__);	\
    }while(0)
    
#endif
