#include "edit.h"

enum {
    NCOMMAND = 256,
};

static void _Noreturn
edit_usage(void) {
    term_eprintf(
        "cap edit\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap edit file-name [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help\tdisplay usage\n"
        "\n"
    );
    exit(EXIT_FAILURE);
}

int
edit_main(int argc, char* argv[]) {
    char const* editpath = "/usr/bin/vi";
    char const* fname = NULL;

    //! Get edit file base name
    if (argc < 2) {
        edit_usage();
    }
    fname = argv[1];
    
    //! Load config
    Config* config = config_new();
    if (!config) {
        WARN("Failed to construct config");
        goto fail_config;
    }

    //! Make file path from edit file base name
    char* spath = config_make_path_from_base(config, fname);
    if (!spath) {
        WARN("Failed to make path from \"%s\"", fname);
        goto fail_make_path;
    }

    //! Execute command with fork, exec family, wait
    pid_t pid = fork();

    if (pid < 0) {
        WARN("Failed to fork");
        free(spath);
        goto fail_fork;
    } else if (pid == 0) {
        //! Are child process
        execl(editpath, editpath, spath, NULL); //< Good bye!
        WARN("Failed to execl");
        free(spath);
        goto fail_exec;
    }

    //! Are parent process
    free(spath);
    config_delete(config);

    if (wait(NULL) == -1) {
        WARN("Failed to wait");
    }
    return 0;

fail_exec:
    config_delete(config);
    return 4;

fail_fork:
    config_delete(config);
    return 3;

fail_make_path:
    config_delete(config);
    return 2;

fail_config:
    return 1;
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

    return 0;
}
#endif

