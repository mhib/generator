#include "utils.h"
bool ends_with(sds string, sds ending) {
    if(sdslen(string) < sdslen(ending)) {
        return false;
    }
    int b = (int) (sdslen(string) - sdslen(ending));
    for(int i = 0; i < sdslen(ending); i += 1) {
        if(string[b + i] != ending[i]) {
            return false;
        }
    }
    return true;
}

char *strdup(const char *str)
{
    size_t n = strlen(str) + 1;
    char *dup = malloc(n);
    if(dup)
    {
        strcpy(dup, str);
    }
    return dup;
}

struct tm parse_time(sds input) {
    struct tm parsed_time;
    int year = -1, month = -1, day = -1, hour = -1, minute = -1, second = -1;
    if(sscanf(input, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) != EOF){
        time_t rawTime;
        time(&rawTime);
        parsed_time = *localtime(&rawTime);

        // tm_year is years since 1900
        parsed_time.tm_year = year - 1900;
        // tm_months is months since january
        parsed_time.tm_mon = month - 1;
        parsed_time.tm_mday = day;
        parsed_time.tm_hour = hour;
        parsed_time.tm_min = minute;
        parsed_time.tm_sec = second;
    } else {
        time_t rawTime;
        time(&rawTime);
        parsed_time = *localtime(&rawTime);
        parsed_time.tm_year = -1;
    }
    return parsed_time;
};

void replace_seperators(sds input, char * separators, char replacement) {
    size_t sep_len = strlen(separators);
    for(size_t i = 0; i < sdslen(input); i += 1) {
        for(size_t x = 0; x < sep_len; x += 1) {
            if(input[i] == separators[x]) {
                input[i] = replacement;
                break;
            }
        }
    }
}
