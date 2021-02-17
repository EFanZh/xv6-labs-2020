#include "kernel/types.h"

// Include group separator.

#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef _Bool bool;

static const int STDERR = 2;
static const bool false = 0;
static const bool true = 1;

typedef struct {
    const char *data;
    uint length;
} StringSlice;

static StringSlice make_string_slice(const char *s) {
    const StringSlice result = {s, strlen(s)};

    return result;
}

static StringSlice get_base_name(StringSlice path) {
    uint i = path.length;

    for (; i != 0; --i) {
        if (path.data[i - 1] == '/') {
            break;
        }
    }

    const StringSlice result = {path.data + i, path.length - i};

    return result;
}

static bool string_slice_equal(StringSlice lhs, StringSlice rhs) {
    return lhs.length == rhs.length && memcmp(lhs.data, rhs.data, lhs.length) == 0;
}

static bool join_path(StringSlice lhs, StringSlice rhs, char *buffer, uint buffer_size, StringSlice *result) {
    if (lhs.length == 0 || rhs.data[0] == '/') {
        *result = rhs;

        return true;
    }

    uint result_length;

    if (lhs.data[lhs.length - 1] == '/') {
        result_length = lhs.length + rhs.length;

        if (result_length >= buffer_size) {
            return false;
        }

        memcpy(buffer, lhs.data, lhs.length);
        memcpy(buffer + lhs.length, rhs.data, rhs.length + 1);
    } else {
        result_length = lhs.length + 1 + rhs.length;

        if (result_length >= buffer_size) {
            return false;
        }

        memcpy(buffer, lhs.data, lhs.length);
        buffer[lhs.length] = '/';
        memcpy(buffer + lhs.length + 1, rhs.data, rhs.length + 1);
    }

    result->data = buffer;
    result->length = result_length;

    return true;
}

static int find(StringSlice path, StringSlice name) {
    const int fd = open(path.data, O_RDONLY);

    if (fd == -1) {
        fprintf(STDERR, "Unable to open %s.", path.data);

        return 1;
    }

    struct stat file_stat;

    if (fstat(fd, &file_stat) == -1) {
        fprintf(STDERR, "Unable to get stat for %s.", path.data);

        close(fd);

        return 1;
    }

    if (file_stat.type == T_DIR) {
        struct dirent dir_entry;

        for (;;) {
            const int read_size = read(fd, &dir_entry, sizeof(dir_entry));

            if (read_size == -1) {
                fprintf(STDERR, "Unable to read directory %s.", path.data);

                close(fd);

                return 1;
            }

            if (read_size == 0) {
                break;
            }

            if (dir_entry.inum != 0 &&
                !(dir_entry.name[0] == '.' &&
                  (dir_entry.name[1] == 0 || (dir_entry.name[1] == '.' && dir_entry.name[2] == 0)))) {
                const StringSlice file_name = make_string_slice(dir_entry.name);
                char file_path_buffer[MAXPATH];
                StringSlice file_path_slice;

                if (join_path(path, file_name, file_path_buffer, MAXPATH, &file_path_slice)) {
                    find(file_path_slice, name);
                } else {
                    fprintf(STDERR, "Path is too long.");

                    close(fd);

                    return 1;
                }
            }
        }
    } else if (string_slice_equal(get_base_name(path), name)) {
        printf("%s\n", path.data);
    }

    close(fd);

    return 0;
}

int main(int argc, char **argv) {
    if (argc == 3) {
        const StringSlice path = make_string_slice(argv[1]);
        const StringSlice name = make_string_slice(argv[2]);

        exit(find(path, name));
    } else {
        printf("Usage: %s <PATH> <NAME>\n", argv[0]);

        exit(1);
    }
}
