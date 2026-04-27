#include "include/loader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

int main() {
    const char *pathname = "../data/student.csv";
    
    Record *table = load(pathname);
    if (!table) {
        fprintf(
            stderr,
            "Failed to load %s: %s\n",
            pathname,
            strerror(errno)
        );
        exit(EXIT_FAILURE);
    }
    
    show_table(table);
    free_table(table);
}