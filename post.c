#include "post.h"

Post* new_post(const char *title, const char *content) {
    Post *out = (Post*)malloc(sizeof(Post));
    out -> title = sdsnew(title);
    out -> content = sdsnew(content);
    return out;
}

void free_post(Post * p) {
    sdsfree(p -> title);
    sdsfree(p -> content);
    free(p);
}
