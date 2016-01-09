#ifndef GENERATOR_POST_H
#define GENERATOR_POST_H

#include "utils.h"
#include <stdlib.h>
#include "sds/sds.h"
#include "sundown/markdown.h"
#include "constants.h"
#define READ_UNIT 1024
#define OUTPUT_UNIT 64

typedef struct Post {
    sds title;
    sds introduction;
    sds content;
    sds layout;
    sds path;
    sds dir;
    sds raw_path;
    struct tm published_at;
} Post;

void free_post(Post *p);
void create_file(Post *p, char * cwd);
void post_sort(Post **T, int Lo, int Hi);
sds render_post(Post * post, sds str);
Post* new_post(sds filename, sds config, sds content, int extenssion_len);
void inspect(Post* p);

#endif //GENERATOR_POST_H
