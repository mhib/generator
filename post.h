#ifndef GENERATOR_POST_H
#define GENERATOR_POST_H

#include "utils.h"
#include <stdlib.h>
#include "sds/sds.h"
#include "sundown/markdown.h"
#define READ_UNIT 1024
#define OUTPUT_UNIT 64

typedef struct Post {
    sds title;
    sds introduction;
    sds content;
    sds layout;
    struct tm published_at;
} Post;

void free_post(Post *p);

Post* new_post(sds filename, sds config, sds content, int extenssion_len);
void inspect(Post* p);

#endif //GENERATOR_POST_H
