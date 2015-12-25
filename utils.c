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
