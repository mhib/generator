#include "post.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>

int number_of_posts = 0;

int main() {
    sds heh = sdsnew("ąść≠≠");
    printf("%s %d\n", heh, (int)sdslen(heh));
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
            if (dir->d_type == DT_REG)
            {
                number_of_posts += 1;
            }
        }
        closedir(d);
    }
    printf("Number of posts: %d.\n", number_of_posts);
    // Post * posts = (Post*)malloc(sizeof(Post) * number_of_posts);
    return 0;
}