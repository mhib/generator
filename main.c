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
char index_path[] = "_index.html";
char out_index_path[] = "index.html";


int main() {
    srand(time(NULL));
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
                sdsfree(config);
                sdsfree(content);
            }
            sdsfree(name);
            sdsfree(file_name);
        }
        sdsfree(line);
        closedir(d);
    }
    post_sort(posts, 0, id - 1);
    FILE *index;
    FILE *out;
    index = fopen(index_path, "r");
    out = fopen(out_index_path, "w");
    if(index == NULL) return 1;
    sds start_iteration = sdsnew("{{each_post}}");
    sds start_cmp = sdsdup(start_iteration);
    sdsrange(start_cmp, 1, -1);
    sds stop_iteration = sdsnew("{{end}}");
    sds stop_cmp = sdsdup(stop_iteration);
    sdsrange(stop_cmp, 1, -1);
    char c;
    sds buff = sdsnewlen("", 100);
    while((c = fgetc(index)) != EOF) {
       if(c == start_iteration[0]) {
           fgets(buff, sdslen(start_cmp) + 1, index);
           if(!strcmp(start_cmp, buff)) {
               sds post_buff = sdsnew("");
               sds tmp = sdsnewlen("", 1);
               while((c = fgetc(index)) != EOF) {
                   if(c == stop_iteration[0]) {
                       fgets(buff, sdslen(stop_cmp) + 1, index);
                       if(!strcmp(stop_cmp, buff)) {
                           for(int i = 0; i < id; i += 1) {
                               sds rendered = render_post(posts[i], post_buff);
                               fputs(rendered, out);
                               sdsfree(rendered);
                           }
                           // fputs(post_buff, out);
                          // out.write(cos(post_buff))
                           break;
                       } else {
                           tmp[0] = stop_iteration[0];
                           post_buff = sdscat(post_buff, tmp);
                           post_buff = sdscat(post_buff, buff);
                       }
                   } else {
                       tmp[0] = c;
                       post_buff = sdscatsds(post_buff, tmp);
                   }
               }
               sdsfree(tmp);
               sdsfree(post_buff);
           } else {
               fputc(start_iteration[0], out);
               fputs(buff, out);
           }
       } else {
           fputc(c, out);
       }
    }
    sdsfree(start_iteration);
    sdsfree(start_cmp);
    sdsfree(stop_iteration);
    sdsfree(stop_cmp);
    sdsfree(buff);
    fclose(out);
    fclose(index);
    for(int i = 0; i < id; i += 1) {
        printf("%d: ", i);
        create_file(posts[i], cwd);
        inspect(posts[i]);
        free_post(posts[i]);
    }
    free(posts);
    sdsfree(posts_dir);
    sdsfree(ext);
    return 0;
}