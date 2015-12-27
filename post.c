#include "post.h"
#include "sundown/html.h"

typedef struct Conf {
    sds title;
    sds introduction;
    sds layout;
    struct tm date;
} Conf;

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

void clean_cfg(Conf * cfg) {
    sdsfree(cfg -> title);
    sdsfree(cfg -> introduction);
    sdsfree(cfg -> layout);
    free(cfg);
}

Post* new_post(sds filename, sds config, sds content, int extension_len) {
    Post * out = malloc(sizeof(Post));
    Conf * cfg = read_conf(config);
    sdsfree(config);
    out -> content = render_markdown(content);
    if(cfg -> date.tm_year == -1  || cfg -> date.tm_year == 0) {
        sds f_copy = sdsdup(filename);
        sdsrange(f_copy, 0, 9);
        cfg -> date = parse_time(f_copy);
        sdsfree(f_copy);
    }
    if(cfg -> title == NULL || sdslen(cfg -> title) == 0) {
        sds f_copy = sdsdup(filename);
        sdsrange(f_copy, 11, -extension_len - 1);
        replace_seperators(f_copy, "_-", ' ');
        f_copy[0] = toupper(f_copy[0]);
        cfg -> title = f_copy;
    }
    if(cfg -> introduction == NULL || sdslen(cfg -> introduction) == 0) {
        get_introduction(cfg, out -> content);
    };
    out -> title = sdsdup(cfg -> title);
    out -> published_at = cfg -> date;
    out -> introduction = sdsdup(cfg -> introduction);
    out -> layout = sdsdup(cfg -> layout);
    clean_cfg(cfg);
    sdsfree(content);
    return out;
}

void inspect(Post* p) {
    printf("Title: %s\n", p -> title);
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
    free(p);
}

