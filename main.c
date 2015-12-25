#include "utils.h"
#include "post.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
sds extension;
int number_of_posts = 0;

void set_globals() {
   extension = sdsnew(".md") ;
}

void clean_globals() {
   sdsfree(extension);
}

int main() {
    set_globals();
    // sds heh = sdsnew("ąść≠≠a");
    printf("%d\n", ends_with(sdsnew("asdasdasdas.md"), extension));
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
    sds posts_dir = sdsnew(cwd);
    posts_dir = sdscat(posts_dir, "/_posts");
    printf("Post directory: %s.\n", posts_dir);
    DIR           *d;
    struct dirent *dir;
    d = opendir(posts_dir);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            sds name = sdsnew(dir -> d_name);
            if(ends_with(name, extension))
                number_of_posts += 1;
            sdsfree(name);
        }
        closedir(d);
    }
    printf("Number of posts: %d.\n", number_of_posts);
    // Post * posts = (Post*)malloc(sizeof(Post) * number_of_posts);
    clean_globals();
    return 0;
}