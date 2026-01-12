#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/throw.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <poc_file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(fsize + 1);
    if (!buf) {
        fclose(f);
        return 1;
    }

    if (fread(buf, 1, fsize, f) != (size_t)fsize) {
        free(buf);
        fclose(f);
        return 1;
    }
    buf[fsize] = 0;
    fclose(f);

    mrb_state *mrb = mrb_open();
    if (!mrb) {
        free(buf);
        return 1;
    }

    int arena_idx = mrb_gc_arena_save(mrb);
    mrbc_context *c = mrbc_context_new(mrb);

    mrb_value result = mrb_load_nstring_cxt(mrb, buf, (size_t)fsize, c);

    if (mrb->exc) {
        mrb_print_error(mrb);
        mrb->exc = 0;
    }

    mrbc_context_free(mrb, c);
    mrb_gc_arena_restore(mrb, arena_idx);
    mrb_close(mrb);
    free(buf);

    return 0;
}
