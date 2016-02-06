#include "post.h"
#include "sundown/html.h"

typedef struct Conf {
    sds title;
    sds introduction;
    sds layout;
    struct tm date;
} Conf;

double post_diff(Post * l, Post * p) {
    return difftime(mktime(&(l -> published_at)), mktime(&(p -> published_at)));
}

void post_sort(Post **T, int Lo, int Hi)
{
    int i,j;
    Post * x;
    x = T[rand() % (Hi + 1 - Lo) + Lo];
    i = Lo;
    j = Hi;
    do
    {
        while (post_diff(T[i], x) > 0) i += 1;
        while (post_diff(T[j], x) < 0) j -= 1;
        if (i<=j)
        {
            Post *tmp = T[i];
            T[i] = T[j];
            T[j] = tmp;
            i += 1; j -= 1;
        }
    } while(i < j);
    if (Lo < j) post_sort(T, Lo, j);
    if (Hi > i) post_sort(T, i, Hi);
}

sds render_markdown(sds input) {

    struct buf *ib, *ob;
    struct sd_callbacks callbacks;
    struct html_renderopt options;
    struct sd_markdown *markdown;


    /* reading everything */
    ib = bufnew(READ_UNIT);
    bufput(ib, input, sdslen(input));
    ob = bufnew(OUTPUT_UNIT);

    sdhtml_renderer(&callbacks, &options, 0);
    markdown = sd_markdown_new(0, 16, &callbacks, &options);

    sd_markdown_render(ob, ib->data, ib->size, markdown);
    sd_markdown_free(markdown);
    sds out = sdsnew(bufcstr(ob));
    bufrelease(ib);
    bufrelease(ob);
    return out;
}

Conf* read_conf(sds input) {
    Conf* conf = (Conf*)malloc(sizeof(Conf));
    conf -> title = NULL;
    conf -> introduction  = NULL;
    conf -> layout  = NULL;
    conf -> date.tm_year = -1;
    sds value = sdsnewlen("", 1000);
    sds token = sdsnewlen("", 1000);
    int tid = 0;
    int vid = 0;
    bool in_token = true;
    for(int x = 0; x <= sdslen(input); x += 1) {
        if(in_token) {
            if(input[x] == ':') {
                in_token = false;
                token[tid] = 0;
            } else {
                token[tid ++] = input[x];
            }
        } else {
            if(input[x] == '\n' || input[x] == '\0') {
                value[vid] = 0;
//                printf("%s %s\n", token, value);
                if(strcmp(token, "date") == 0) {
                    conf -> date = parse_time(value);
                } else if(strcmp(token, "title") == 0) {
                    conf -> title = sdsdup(value);
                    sdstrim(conf -> title, " ");
                } else if(strcmp(token, "layout") == 0) {
                    conf -> layout = sdsdup(value);
                    sdstrim(conf -> layout, " ");
                } else if(strcmp(token, "introduction") == 0) {
                    conf -> introduction = sdsdup(value);
                    sdstrim(conf -> introduction, " ");
                } else {
                    printf("Unknown key: %s.\n", token);
                }
                tid = 0;
                vid = 0;
                value[0] = 0;
                token[0] = 0;
                in_token = true;
            } else {
                value[vid ++] = input[x];
            }
        }
    }
    sdsfree(value);
    sdsfree(token);
    return conf;
}

void get_introduction(Conf * cfg, sds content) {
    sds out = sdsempty();
    char temp[2] = {0, 0};
    for(int x = 0; x < sdslen(content); x += 1) {
        temp[0] = content[x];
        out = sdscatlen(out, temp, 1);
        if(content[x] == '\n' && content[x + 1] == '\n') {
            break;
        }
    }
    cfg -> introduction = sdsdup(out);
    sdsfree(out);
}

void generate_path(Post * post) {
    struct tm t = post -> published_at;
    char temp[200];
    sprintf(temp, "posts/%d/%d/%d/", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    sds s = sdsnew(temp);
    post -> dir = sdsnew(s);
    sds out = sdsnew(s);
    sdsfree(s);
    out = sdscat(out, post -> title);
    for(int i = 0; i < sdslen(out); i += 1) {
        if(isspace(out[i])) {
            out[i] = '-';
        }
    }
    out = sdscatlen(out, ".html", 5);
    post -> raw_path = sdsnew(out);
    char * encoded = url_encode(out);
    sdsfree(out);
    out = sdsnew(encoded);
    free(encoded);
    post -> path = out;
}

void create_file(Post * post, char * cwd) {
    sds dir = sdsnew(cwd);
    dir = sdscat(dir, out_folder);
    dir = sdscatsds(dir, post -> dir);
    _mkdir(dir);
    sds layout_location = sdsnew(cwd);
    layout_location = sdscatlen(layout_location, "/_layouts/", 10);
    layout_location = sdscat(layout_location, post -> layout);
    layout_location = sdscatlen(layout_location, ".html", 5);
    sds file_contents;
    long input_file_size;
    FILE *input_file = fopen(layout_location, "rb");
    if(input_file == NULL) {
        printf("No layout %s\n", post -> layout);
        return;
    }
    fseek(input_file, 0, SEEK_END);
    input_file_size = ftell(input_file);
    rewind(input_file);
    file_contents = sdsnewlen("", input_file_size);
    fread(file_contents, sizeof(char), input_file_size, input_file);
    fclose(input_file);
    sds rendered = render_post(post, file_contents);
    FILE *out;
    sds path = sdsnew(cwd);
    path = sdscat(path, out_folder);
    path = sdscat(path, post -> raw_path);
    out = fopen(path, "w");
    fputs(rendered, out);
    fclose(out);
    sdsfree(dir);
    sdsfree(rendered);
    sdsfree(layout_location);
    sdsfree(file_contents);
    sdsfree(path);
}

sds render_post(Post * post, sds str) {
    sds out = sdsnew("");
    sds temp = sdsnewlen("", 1);
    for(int i = 0; i < sdslen(str); i += 1) {
        if(str[i] == '{' && str[i + 1] == '{') {
            i += 2;
            sds token = sdsnew("");
            while(str[i] != '}' && i < sdslen(str)) {
                temp[0] = str[i];
                token = sdscatsds(token, temp);
                i += 1;
            }

            if(!strcmp(token, "title")) {
                out = sdscat(out, post -> title);
            } else if(!strcmp(token, "path")) {
                out = sdscat(out, post->path);
            } else if(!strcmp(token, "introduction")) {
                out = sdscat(out, post->introduction);
            } else if(!strcmp(token, "content")) {
                out = sdscat(out, post->content);
            } else if(!strcmp(token, "published_at")) {
                sds temp = sdsnewlen("", 100);
                strftime(temp, 100, "%d/%m/%y %H:%M", &(post -> published_at));
                out = sdscat(out, temp);
                sdsfree(temp);
            } else if(strstr(token, "published_at_format(")) {
                sds format = sdsdup(token);
                sdsrange(format, 20, -2);
                sds temp = sdsnewlen("", 100);
                strftime(temp, 100, format, &(post -> published_at));
                out = sdscat(out, temp);
                sdsfree(temp);
                sdsfree(format);
            }
            i += 1;
            sdsfree(token);
        } else {
            temp[0] = str[i];
            out = sdscatlen(out, temp, 1);
        }
    }
    sdsfree(temp);
    return out;
}

Post* new_post(sds filename, sds config, sds content, int extension_len) {
    Post * out = malloc(sizeof(Post));
    Conf * cfg = read_conf(config);
    out -> content = render_markdown(content);
    if(cfg -> date.tm_year == -1  || cfg -> date.tm_year == 0) {
        sds f_copy = sdsdup(filename);
        sdsrange(f_copy, 0, 9);
        cfg -> date = parse_time(f_copy);
        sdsfree(f_copy);
    }
    if(cfg -> title == NULL || sdslen(cfg -> title) == 0) {
        sdsfree(cfg -> title);
        sds f_copy = sdsdup(filename);
        sdsrange(f_copy, 11, -extension_len - 1);
        replace_seperators(f_copy, "_-", ' ');
        f_copy[0] = toupper(f_copy[0]);
        cfg -> title = f_copy;
    }
    if(cfg -> introduction == NULL || sdslen(cfg -> introduction) == 0) {
        sdsfree(cfg -> introduction);
        get_introduction(cfg, out -> content);
    };
    if(cfg -> layout == NULL || sdslen(cfg -> layout) == 0) {
        sdsfree(cfg -> layout);
        cfg -> layout = sdsnew("post");
    }
    out -> title = cfg -> title;
    out -> published_at = cfg -> date;
    out -> introduction = cfg -> introduction;
    out -> layout = cfg -> layout;
    generate_path(out);
    free(cfg);
    return out;
}

void inspect(Post* p) {
    printf("Title: %s\n", p -> title);
    printf("Path: %s\n", p -> path);
    struct tm t = (p -> published_at);
    printf("Date: %d-%d-%dT%d-%d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min);
    printf("Introduction: %s\n", p -> introduction);
//    printf("Content: %s\n", p -> content);
}

void new_post_file(char *directory, sds name) {
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    char t[256];
    sds out_dir = sdsnew(directory);
    out_dir = sdscat(out_dir, "/_posts/");
    _mkdir(out_dir);
    sds post_file = sdsnew(out_dir);
    strftime(t, 256, "%Y-%m-%d-", timeinfo);
    post_file = sdscat(post_file, t);
    post_file = sdscat(post_file, name);
    replace_seperators(post_file, " ", '-');
    post_file = sdscatlen(post_file, ".md", 3);
    FILE * f = fopen(post_file, "w");
    if(f == NULL) { return; }
    fputs("---\n", f);
    fputs("title: ", f) ;
    fputs(name, f);
    fputs("\n", f);
    strftime(t, 256, "date: %Y-%m-%dT%H:%M:%S\n", timeinfo);
    fputs(t, f);
    fputs("---\n", f);
    fclose(f);
    sdsfree(post_file);
    sdsfree(out_dir);
}

void free_post(Post *p) {
    sdsfree(p->title);
    sdsfree(p->introduction);
    sdsfree(p->content);
    sdsfree(p->layout);
    sdsfree(p->path);
    sdsfree(p->dir);
    sdsfree(p->raw_path);
    free(p);
}

