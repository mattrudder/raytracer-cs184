
#include <ctype.h>
#include <stdio.h>

#include "utils.h"

char* trim(char* line)
{
    while (isspace(*line))
        line++;

    return line;
}

void con(char* line)
{
    printf("%s\n", line);
}

bool isEmpty(char* line)
{
    return line == NULL || *line == '\0';
}
