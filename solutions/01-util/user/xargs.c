#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

typedef _Bool bool;

static const bool false = (bool)0;
static const bool true = (bool)1;
static const int STDIN = 0;
static const int STDERR = 2;
static void *const NULL = (void *)0;

typedef struct {
    char *data;
    uint capacity;
    uint length;
} String;

static bool string_new(String *target) {
    char *const data = (char *)malloc(1);

    if (data) {
        data[0] = '\0';

        target->data = data;
        target->capacity = 1;
        target->length = 0;

        return true;
    } else {
        return false;
    }
}

static void string_clear(String *target) {
    target->data[0] = '\0';
    target->length = 0;
}

static bool string_push(String *target, char c) {
    if (target->length + 1 == target->capacity) {
        uint new_capacity = target->capacity * 2;
        char *const new_data = (char *)malloc(new_capacity);

        if (new_data) {
            memcpy(new_data, target->data, target->length);
            free(target->data);
            target->data = new_data;
            target->capacity = new_capacity;
        } else {
            return false;
        }
    }

    target->data[target->length] = c;
    target->length += 1;
    target->data[target->length] = '\0';

    return true;
}

typedef enum {
    ReadLineResult_Error,
    ReadLineResult_Done,
    ReadLineResult_Normal,
} ReadLineResult;

static ReadLineResult read_line(String *target) {
    char buffer;
    int read_result = read(STDIN, &buffer, 1);

    if (read_result == -1) {
        return ReadLineResult_Error;
    }

    if (read_result == 0) {
        return ReadLineResult_Done;
    }

    while (buffer != '\n') {
        if (!string_push(target, buffer)) {
            return ReadLineResult_Error;
        }

        read_result = read(STDIN, &buffer, 1);

        if (read_result == -1) {
            return ReadLineResult_Error;
        }

        if (read_result == 0) {
            break;
        }
    }

    return ReadLineResult_Normal;
}

static int inner_main(char **command, uint length) {
    if (length >= MAXARG - 1) {
        fprintf(STDERR, "Too many arguments.\n");

        return 1;
    }

    char *argv[MAXARG];

    memcpy(argv, command, sizeof(char *) * length);
    argv[length + 1] = NULL;

    String command_arg;

    if (!string_new(&command_arg)) {
        fprintf(STDERR, "Not enough memory.\n");

        return 1;
    }

    for (;;) {
        const ReadLineResult read_line_result = read_line(&command_arg);

        if (read_line_result == ReadLineResult_Error) {
            free(command_arg.data);

            return 1;
        }

        if (read_line_result == ReadLineResult_Done) {
            break;
        }

        argv[length] = command_arg.data;

        if (fork() == 0) {
            // Child.

            close(STDIN);

            if (exec(argv[0], argv) == -1) {
                fprintf(STDERR, "Failed to execute");

                for (char *const *arg = argv; *arg != NULL; ++arg) {
                    fprintf(STDERR, " %s", *arg);
                }

                fprintf(STDERR, ".\n");

                exit(1);
            }
        } else {
            // Parent.

            int status;

            wait(&status);

            string_clear(&command_arg);
        }
    }

    free(command_arg.data);

    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Usage: %s COMMAND ARGS ...\n", argv[0]);

        exit(1);
    } else {
        exit(inner_main(argv + 1, argc - 1));
    }
}
