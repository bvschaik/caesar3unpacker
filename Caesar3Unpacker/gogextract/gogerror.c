#include "gogerror.h"
#include "gogextract.h"

#include <stdarg.h>
#include <stdio.h>

#define ERROR_LENGTH 1000

static char global_error[ERROR_LENGTH];

void gog_clear_error(void)
{
    global_error[0] = 0;
}

void gog_set_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vsnprintf(global_error, ERROR_LENGTH, message, args);
    va_end(args);
}

const char *gogextract_error(void)
{
    return global_error;
}
