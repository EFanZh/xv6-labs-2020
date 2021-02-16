#include "kernel/types.h"
#include "user/user.h"

typedef struct {
    int read;
    int write;
} Pipe;

static Pipe create_pipe() {
    int buffer[2];

    pipe(buffer);

    const Pipe pipe = {buffer[0], buffer[1]};

    return pipe;
}

int main(int argc, char **argv) {
    const Pipe ping_pipe = create_pipe();
    const Pipe pong_pipe = create_pipe();
    const int pid = fork();
    const int current_pid = getpid();

    if (pid == 0) {
        int buffer;

        read(ping_pipe.read, &buffer, 1);

        printf("%d: received ping\n", current_pid);

        write(pong_pipe.write, &buffer, 1);
    } else {
        const unsigned char data = 7;

        write(ping_pipe.write, &data, 1);

        int buffer;

        read(pong_pipe.read, &buffer, 1);

        printf("%d: received pong\n", current_pid);

        int status;

        wait(&status);
    }

    exit(0);
}
