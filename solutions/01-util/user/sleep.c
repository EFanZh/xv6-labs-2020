#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char **argv) {
    if (argc == 2) {
        const int duration = atoi(argv[1]);

        sleep(duration);

        exit(0);
    } else {
        printf("Usage: %s <DURATION>", argv[0]);

        exit(1);
    }
}
