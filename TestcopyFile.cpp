#include <cstdio>
#include <cstring>
#include <cerrno>

#include "covstdlib.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <target>\n", argv[0]);
        return 1;
    }

    if (cov::copyFile(argv[1], argv[2]) != 0) {
        printf("%s\n", strerror(errno));
        return 1;
    } else {
        printf("Success\n");
    }

    return 0;
}
