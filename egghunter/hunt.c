#include <stdio.h>
#include <memory.h>
#include <sys/mman.h>

#define INFO "\033[92m[INFO ]\033[39m "
#define ERROR "\033[91m[ERROR]\033[39m "

int main(int argc, char* argv[]) {
    // hide the egg
    int offset = 4096 * 1765 + 63;
    size_t egg = 0xdeadbeef;
    
    char* data = mmap(0, 1024 * 1024 * 16, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
    memcpy(data + offset, &egg, sizeof(egg));
    printf(INFO "Hiding egg @ %p\n", data + offset);
    
    size_t start = (size_t)data - 1024 * 1024 * 8, address = 0;
    printf(INFO "Starting egg hunter @ 0x%zx (%zd MB before egg)\n", start, (size_t)(offset + data - start) / (1024 * 1024));

    // hunt the egg
    asm volatile(
        "1:\n"
        "inc %%rdi\n"
        "xbegin 1b\n"
        "cmpl %%edx, (%%rdi)\n"
        "xend\n"
        "jne 1b\n"
        : "=D"(address) /* found egg location */ : "D"(start) /* start searching */ , "d"(0xdeadbeef) /* egg */);

    // we (hopefully) found the egg
    printf(INFO "Found egg @ 0x%zx\n", address);
    if(address == (size_t)data + offset) {
        printf(INFO "Egg hunt was successful!\n");
    } else {
        printf(ERROR "Egg hunt failed\n");
    }

    return 0;
}

