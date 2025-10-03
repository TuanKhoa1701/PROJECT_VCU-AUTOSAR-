#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int _isatty(int fd) {
    return 1;
}

int _write(int file, char *ptr, int len) {
    return len;
}

int _read(int file, char *ptr, int len) {
    return 0;
}

int _close(int file) {
    return -1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}
// ...existing code...
caddr_t _sbrk(int incr) {
    extern char _end;     // Defined by the linker script
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;
    heap_end += incr;
    return (caddr_t)prev_heap_end;
}
// ...existing code...