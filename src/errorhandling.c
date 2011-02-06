#include "errorhandling.h"
#include <stdio.h>

void _printErrorString(error_code ecode, char *code_msg, char *msg)
{
    if(strlen(msg) == 0)
    {
	fprintf(stderr, "ERROR (%d): ", ecode);
	fprintf(stderr, "%s", code_msg);
	fprintf(stderr, ".\n\n");
    }
    else
    {
	fprintf(stderr, "ERROR (%d): ", ecode);
	fprintf(stderr, "%s", code_msg);
	fprintf(stderr, ":\n   ");
	fprintf(stderr, "%s", msg);
	fprintf(stderr, "\n\n");
    }

    fflush(stderr);
}


void printError(error_code ecode, char *msg)
{
    switch(ecode)
    {

    case PGE_EOF:
	_printErrorString(ecode, "Unexpected End of File encountered", msg);
	return;
    case PGE_PARSING_ERROR:
	_printErrorString(ecode, "Parsing Error", msg);
	return;
    default:
	_printErrorString(ecode, "Unknown Error", msg);
    }
}

void MemCheckViolation(const char* varname, const char* file, 
		       const char* func, int line)
{
    fprintf(stderr, "\nERROR: OUT OF MEMORY.");				
    fprintf(stderr, "\nAttempt to allocate memory for %s failed", varname);
    fprintf(stderr, "\nGenerated in %s, function %s, line %d.",	file, func, line);				
    fprintf(stderr, "\nPointer set to NULL; Program may become unstable.\n"); 
    fflush(stderr);							
}
