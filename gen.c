#include "gen.h"
const char config_identifier[] = "---\n" ;
const char extension[] = ".md";
const int extension_len = 3;
int number_of_posts;
int id;

void set_globals() {
   id = 0;
   number_of_posts = 0;
}

void cp_public(sds dir) {
    sds in_dir = sdsnew(dir);
    in_dir = sdscat(in_dir, "/public ");
    sds out_dir = sdsnew(dir);
    out_dir = sdscat(out_dir, out_folder);
    out_dir = sdscat(out_dir, "public");
    sds command = sdsnew("cp -rf ");
    command = sdscat(command, in_dir);
    command = sdscat(command, out_dir);
    system(command);
    sdsfree(command);
    sdsfree(in_dir);
    sdsfree(out_dir);
}

Post ** load_posts(sds p_dir, sds ext) {
    sds posts_dir = sdsnew(p_dir);
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
    Post** posts = malloc(sizeof(Post*) * number_of_posts);
    if(number_of_posts == 0) return posts;
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
    sdsfree(posts_dir);
    return posts;
}

void generate_posts(Post ** posts, sds cwd) {
    for(int i = 0; i < id; i += 1) {
//        printf("%d: ", i);
        create_file(posts[i], cwd);
//        inspect(posts[i]);
        free_post(posts[i]);
    }
}

char * generate_site(char * p_dir) {
    set_globals();
    srand(time(NULL));
    sds cwd;
    if(strlen(p_dir) == 0) {
        char cwd_t[1024];
        if (getcwd(cwd_t, sizeof(cwd_t)) == NULL) {
            return "Invalid directory";
        }
        cwd = sdsnew(cwd_t);
    } else {
        cwd = sdsnew(p_dir);
    }
    sds ext = sdsnew(extension);
    sds posts_dir = sdsnew(cwd);
    sds out_dir = sdsnew(cwd);
    out_dir = sdscat(out_dir, out_folder);
    sds rm = sdsnew("rm -rf ");
    rm = sdscat(rm, out_dir);
    system(rm);
    sdsfree(rm);
    sdsfree(out_dir);
    FILE *index;
    FILE *out;
    sds in_index_path = sdsnew(cwd);
    in_index_path = sdscatlen(in_index_path, "/", 1);
    in_index_path = sdscat(in_index_path, index_path);
    if(!file_exists(in_index_path)) { return "No _index.html"; }
    
    Post ** posts = load_posts(posts_dir, ext);

    index = fopen(in_index_path, "r");
    sds index_tmp_path = sdsnew(cwd);
    index_tmp_path = sdscat(index_tmp_path, out_folder);
    _mkdir(index_tmp_path);
    index_tmp_path = sdscat(index_tmp_path, out_index_path);
    out = fopen(index_tmp_path, "w");
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
                            break;
                        } else {
                            tmp[0] = stop_iteration[0];
                            post_buff = sdscatlen(post_buff, tmp, 1);
                            post_buff = sdscat(post_buff, buff);
                        }
                    } else {
                        tmp[0] = c;
                        post_buff = sdscat(post_buff, tmp);
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
    cp_public(cwd);
    sdsfree(index_tmp_path);
    sdsfree(start_iteration);
    sdsfree(start_cmp);
    sdsfree(stop_iteration);
    sdsfree(stop_cmp);
    sdsfree(buff);
    fclose(out);
    fclose(index);
    generate_posts(posts, cwd);
    free(posts);
    sdsfree(posts_dir);
    sdsfree(ext);
    sdsfree(cwd);
    return "Generated.";
}
