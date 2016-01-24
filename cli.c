#include "gen.h"
#include "cli.h"
const char *help = "Usage:\n"
        "generator: Generates site in current directory\n"
        "generator -d your_directory: Generates site in your_directory\n"
        "generator -nd your_directory Your title: Generates new post with title in your_directory\n"
        "generator -n Your title: Generates new post with title in current directory\n"
        "generator -h and generator --help: Shows this information\n";

int run(int argc, char ** argv) {
    if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        printf("%s", help);
        return 0;
    } else if (argc >= 3 && !strcmp(argv[1], "-n")) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            printf("Invalid directory");
            return 1;
        }
        sds name = sdsnew("");
        for (int i = 2; i < argc; i += 1) {
            name = sdscat(name, argv[i]);
            if (i != argc - 1)
                name = sdscatlen(name, " ", 1);
        }
        new_post_file(cwd, name);
        sdsfree(name);
    } else if (argc >= 4 && !strcmp(argv[1], "-nd")) {
        sds name = sdsnew("");
        for (int i = 3; i < argc; i += 1) {
            name = sdscat(name, argv[i]);
            if (i != argc - 1)
                name = sdscatlen(name, " ", 1);
        }
        new_post_file(argv[2], name);
        sdsfree(name);
    } else if (argc == 3 && !strcmp(argv[1], "-d")) {
        printf("%s\n", generate_site(argv[2]));
    } else {
        printf("%s\n", generate_site(""));
    }
    return 0;
}
