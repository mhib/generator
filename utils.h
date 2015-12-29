#ifndef GENERATOR_UTILS_H_H
#define GENERATOR_UTILS_H_H
#include <stdbool.h>
#include "sds/sds.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "sundown/houdini.h"
#include <time.h>
#include <sys/stat.h>



void _mkdir(const char *dir);
char *url_encode(sds str);
char *strdup(const char *str);
bool ends_with(sds string, sds ending);
struct tm parse_time(sds input);
void replace_seperators(sds input, char * separators, char replacement);
#endif //GENERATOR_UTILS_H_H
