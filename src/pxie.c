#define PARSEC_IMPLEMENTATION
#include <parsec.h>

#include "pxie.h"

int main(int argc, char *argv[]) {
    parsec_init("pxie", "Pixel art software");

    int *pixels = parsec_int_ref(
        "-p", "--pixels", 20,
        "Total of pixels per row and column"
    );

    parsec_parse(argc, argv);

    draw_loop(*pixels);

    return 0;
}
