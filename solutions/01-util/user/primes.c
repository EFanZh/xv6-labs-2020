#include "kernel/types.h"
#include "user/user.h"

static const int STDIN = 0;

static void inner() {
    for (;;) {
        int filter;

        read(STDIN, &filter, sizeof(filter));

        printf("prime %d\n", filter);

        // Find the next prime.

        int buffer;

        do {
            if (read(STDIN, &buffer, sizeof(buffer)) == 0) {
                return;
            }
        } while (buffer % filter == 0);

        // Create child process.

        int pipe_fds[2];

        pipe(pipe_fds);

        if (fork() == 0) {
            close(pipe_fds[1]);

            // Redirect read pipe to standard input.

            close(STDIN);
            dup(pipe_fds[0]);

            close(pipe_fds[0]);

            // Go to start.
        } else {
            close(pipe_fds[0]);

            write(pipe_fds[1], &buffer, sizeof(buffer));

            while (read(STDIN, &buffer, sizeof(buffer)) != 0) {
                if (buffer % filter != 0) {
                    write(pipe_fds[1], &buffer, sizeof(buffer));
                }
            }

            close(pipe_fds[1]);

            int status;

            wait(&status);

            break;
        }
    }
}

int main(int argc, char **argv) {
    int pipe_fds[2];

    close(STDIN);
    pipe(pipe_fds);

    if (fork() == 0) {
        close(pipe_fds[1]);

        inner();

        close(pipe_fds[0]);
    } else {
        close(pipe_fds[0]);

        for (int i = 2; i != 36; ++i) {
            write(pipe_fds[1], &i, sizeof(i));
        }

        close(pipe_fds[1]);

        int status;

        wait(&status);
    }

    exit(0);
}
