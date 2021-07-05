#include <pad/core/args.h>

PadDistriArgs *
PadDistriArgs_Distribute(PadDistriArgs *dargs, int argc, char **argv) {
    PadCStrAry *app_args = PadCStrAry_New();
    PadCStrAry *cmd_args = PadCStrAry_New();

    int m = 0;
    for (int i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            PadCStrAry_Push(app_args, arg);
            m = 10;
            break;
        case 10:
            if (arg[0] == '-') {
                PadCStrAry_Push(app_args, arg);
            } else {
                PadCStrAry_Push(cmd_args, arg);
                m = 20;
            }
            break;
        case 20:
            PadCStrAry_Push(cmd_args, arg);
            break;
        }
    }

    dargs->argc = PadCStrAry_Len(app_args);
    dargs->argv = PadCStrAry_EscDel(app_args);
    dargs->cmd_argc = PadCStrAry_Len(cmd_args);
    dargs->cmd_argv = PadCStrAry_EscDel(cmd_args);

    return dargs;
}
