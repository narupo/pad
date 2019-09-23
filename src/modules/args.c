#include "modules/args.h"

distribute_args_t *
distribute_args(distribute_args_t *dargs, int argc, char **argv) {
    cstring_array_t *app_args = cstrarr_new();
    cstring_array_t *cmd_args = cstrarr_new();

    int m = 0;
    for (int i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            cstrarr_push(app_args, arg);
            m = 10;
            break;
        case 10:
            if (arg[0] == '-') {
                cstrarr_push(app_args, arg);
            } else {
                cstrarr_push(cmd_args, arg);
                m = 20;
            }
            break;
        case 20:
            cstrarr_push(cmd_args, arg);
            break;
        }
    }

    dargs->argc = cstrarr_len(app_args);
    dargs->argv = cstrarr_escdel(app_args);
    dargs->cmd_argc = cstrarr_len(cmd_args);
    dargs->cmd_argv = cstrarr_escdel(cmd_args);

    return dargs;
}
