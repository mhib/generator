#include <stdio.h>
#include "post.h"
int main() {
    Post p = { .content = sdsnew("Asdf"), .title = sdsnew("Gg")};
    printf("Hello %s %s.", p.title, p.content);
    return 0;
}