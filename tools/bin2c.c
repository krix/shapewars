#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: bin2c <input_file> <variable_name> <output_header>\n");
        return 1;
    }

    const char *input = argv[1];
    const char *varname = argv[2];
    const char *output = argv[3];

    FILE *fi = fopen(input, "rb");
    if (!fi) { fprintf(stderr, "Can't open %s\n", input); return 1; }

    fseek(fi, 0, SEEK_END);
    long size = ftell(fi);
    fseek(fi, 0, SEEK_SET);

    unsigned char *buf = malloc(size);
    fread(buf, 1, size, fi);
    fclose(fi);

    // Build output filename for the .c file
    char cpath[512];
    snprintf(cpath, sizeof(cpath), "%s", output);
    // Remove .h extension, add .c
    char *dot = strrchr(cpath, '.');
    if (dot) *dot = 0;
    char cfile[512];
    snprintf(cfile, sizeof(cfile), "%s.c", cpath);

    FILE *fh = fopen(output, "w");
    if (!fh) { fprintf(stderr, "Can't write %s\n", output); return 1; }
    fprintf(fh, "#ifndef EMBEDDED_%s_H\n", varname);
    fprintf(fh, "#define EMBEDDED_%s_H\n\n", varname);
    fprintf(fh, "extern const unsigned char %s_data[%ld];\n", varname, size);
    fprintf(fh, "#define %s_size %ld\n\n", varname, size);
    fprintf(fh, "#endif\n");
    fclose(fh);

    FILE *fc = fopen(cfile, "w");
    if (!fc) { fprintf(stderr, "Can't write %s\n", cfile); return 1; }
    fprintf(fc, "#include \"%s\"\n\n", strrchr(output, '\\') ? strrchr(output, '\\') + 1 : strrchr(output, '/') ? strrchr(output, '/') + 1 : output);
    fprintf(fc, "const unsigned char %s_data[%ld] = {\n", varname, size);
    for (long i = 0; i < size; i++) {
        if (i % 12 == 0) fprintf(fc, "    ");
        fprintf(fc, "0x%02x,", buf[i]);
        if (i % 12 == 11 || i == size - 1) fprintf(fc, "\n");
        else fprintf(fc, " ");
    }
    fprintf(fc, "};\n");
    fclose(fc);

    printf("Generated %s and %s (%ld bytes)\n", output, cfile, size);
    free(buf);
    return 0;
}
