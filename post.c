#include "post.h"
#include "sundown/html.h"

typedef struct Conf {
    sds title;
    sds introduction;
    sds layout;
    struct tm date;
} Conf;

int post_diff(Post * l, Post * p) {
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
    sds data = sdsdup(input);
    bufput(ib, data, sdslen(data));
    ob = bufnew(OUTPUT_UNIT);

    sdhtml_renderer(&callbacks, &options, 0);
    markdown = sd_markdown_new(0, 16, &callbacks, &options);

    sd_markdown_render(ob, ib->data, ib->size, markdown);
    sd_markdown_free(markdown);
    sds out = sdsnew(bufcstr(ob));
    bufrelease(ib);
    bufrelease(ob);
    sdsfree(data);
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
                printf("%s %s\n", token, value);
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
        out = sdscat(out, temp);
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
    post -> dir = sdsnew(temp);
    sds out = sdsnew(temp);
    out = sdscatsds(out, post -> title);
    for(int i = 0; i < sdslen(out); i += 1) {
        if(isspace(out[i])) {
            out[i] = '-';
        }
    }
    out = sdscat(out, ".html");
    post -> raw_path = sdsdup(out);
    char * encoded = url_encode(out);
    sdsfree(out);
    out = sdsnew(encoded);
    free(encoded);
    post -> path = out;
}

void create_file(Post * post, char * cwd) {
    sds dir = sdsnew(cwd);
    dir = sdscat(dir, "/");
    dir = sdscatsds(dir, post -> dir);
    _mkdir(dir);
    sds layout_location = sdsnew(cwd);
    layout_location = sdscat(layout_location, "/_");
    layout_location = sdscat(layout_location, post -> layout);
    layout_location = sdscat(layout_location, ".html");
    sds file_contents;
    long input_file_size;
    FILE *input_file = fopen(layout_location, "rb");
    printf("\n\n%s\n", layout_location);
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
    out = fopen(post -> raw_path, "w");
    fputs(rendered, out);
    fclose(out);
    sdsfree(dir);
    sdsfree(rendered);
    sdsfree(layout_location);
    sdsfree(file_contents);
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
                strftime(temp, 100, "%x", &(post -> published_at));
                out = sdscat(out, temp);
                sdsfree(temp);
            }
            i += 1;
            sdsfree(token);
        } else {
            temp[0] = str[i];
            out = sdscat(out, temp);
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
        cfg -> layout = sdsnew("post.html");
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
    printf("Content: %s\n", p -> content);
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

