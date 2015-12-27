#include "utils.h"
#include "post.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <strings.h>
int number_of_posts = 0;
int id = 0;
const char config_identifier[] = "---\n" ;
const char extension[] = ".md";
const int extension_len = 3;
char posts_dir_name[] = "/_posts/";


int main() {
    sds ext = sdsnew(extension);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
    sds posts_dir = sdsnew(cwd);
    posts_dir = sdscat(posts_dir, posts_dir_name);
    printf("Post directory: %s.\n", posts_dir);
    DIR           *d;
    struct dirent *dir;
    d = opendir(posts_dir);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if(dir -> d_name[0] == '.') { continue; }
            sds name = sdsnew(dir -> d_name);
            if(ends_with(name, ext))
                number_of_posts += 1;
            sdsfree(name);
        }
        closedir(d);
    }
    printf("Number of posts: %d.\n", number_of_posts);
    if(number_of_posts == 0) return 0;
    Post** posts = malloc(sizeof(Post*) * number_of_posts);
    d = opendir(posts_dir);
    if (d)
    {
        sds line = sdsnewlen("", 1000);
        while ((dir = readdir(d)) != NULL)
        {
            if(dir -> d_name[0] == '.') { continue; }
            sds name = sdsnew(dir -> d_name);
            sds file_name = sdsdup(posts_dir);
            file_name = sdscatsds(file_name, name);
            if(ends_with(file_name, ext)) {
                printf("Processing %s\n", name);
                FILE *f = fopen(file_name, "rt");
                if(f == NULL) { continue; }
                bool config_to_load = true;
                bool in_config = false;
                sds config = sdsempty();
                sds content = sdsempty();
                while(fgets(line, 1000, f) != NULL) {
                    if(config_to_load) {
                        sdstrim(line, " ");
                        if(strcmp(line, config_identifier) == 0) {
                            in_config = true;
                            config_to_load = false;
                        } else {
                            config_to_load = false;
                            content = sdscat(content, line);
                        }

                    } else if(in_config) {
                        sdstrim(line, " ");
                        if(strcmp(line, config_identifier) == 0) {
                            in_config = false;
                        } else {
                            config = sdscat(config, line);
                        }
                    } else {
                        content = sdscat(content, line);
                    }
                }
                fclose(f);
                posts[id ++]  = new_post(name, config, content, extension_len);
            }
            sdsfree(name);
            sdsfree(file_name);
        }
        sdsfree(line);
        closedir(d);
    }
    for(int i = 0; i < id; i += 1) {
        inspect(posts[i]);
        free_post(posts[i]);
    }
    free(posts);
    sdsfree(posts_dir);
    sdsfree(ext);
    return 0;
}