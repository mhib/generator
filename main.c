#include "gen.h"

const char *help = "Usage:\n"
"generator: Generates site in current directory\n"
"generator -d your_directory: Generates site in your_directory\n"
"generator -n your_directory Your title: Generates new post with title in your_directory\n"
"generator -h and generator --help: Shows this information\n";

int main(int argc, char ** argv) {
    if(argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        printf("%s", help);
        return 0;
    } else if(argc >= 4 && !strcmp(argv[1], "-n")) {
        sds name = sdsnew("");
        for(int i = 3; i < argc; i += 1) {
            name = sdscat(name, argv[i]);
            if(i != argc - 1)
                name = sdscatlen(name, " ", 1);
        }
        new_post_file(argv[2], name);
        sdsfree(name);
    } else if(argc == 3 && !strcmp(argv[1], "-d")) {
        generate_site(argv[2]);
    } else {
        generate_site("");
    }
    return 0;
}