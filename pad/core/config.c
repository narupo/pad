#include <pad/core/config.h>

void
PadConfig_Del(PadConfig *self) {
    if (self) {
        free(self);
    }
}

PadConfig *
PadConfig_New(void) {
    PadConfig *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    return self;
}

char *
Pad_PopTailSlash(char *path);

PadConfig *
PadConfig_Init(PadConfig *self) {
    strcpy(self->line_encoding, "lf");

    // standard libraries
    if (!PadFile_Solve(self->std_lib_dir_path, sizeof self->std_lib_dir_path, "~/.pad/stdlib")) {
        PadErr_Err("failed to solve path for standard libraries directory");
        return NULL;
    }

    return self;
}
