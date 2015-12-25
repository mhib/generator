#ifndef GENERATOR_POST_H
#define GENERATOR_POST_H

#include <stdlib.h>
#include "yaml/yaml.h"
#include "sds/sds.h"

typedef struct Post {
    sds title;
    sds content;
} Post;

void free_post(Post * p);

Post* new_post(const char *title, const char *content);

#endif //GENERATOR_POST_H
