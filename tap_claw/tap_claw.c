#include <stdio.h>
#include "tap_claw.h"

#define INFO "\033[92m[INFO ]\033[39m "
#define ERROR "\033[91m[ERROR]\033[39m "

void address_info(void* addr, const char* name) {
    int pos = printf(INFO "Address %p", addr);
    if(name) pos += printf(" (%s)", name);
    const char* mapped = "";
    const char* writable = "";
    if(is_mapped(addr)) {
        mapped = "\033[92mmapped\033[39m    ";
        if(is_writable(addr)) {
            writable = "\033[92mwritable\033[39m";
        } else {
            writable = "\033[91mreadonly\033[39m";    
        }
    } else {
        mapped = "\033[91mnot mapped\033[39m";
    }
    pos += printf(": ");
    while(pos < 60) {
        printf(" ");
        pos++;
    }
    printf("%s    %s\n", mapped, writable);
}

int global;

int main() {
    int stack;
    address_info(&stack, "stack");
    address_info(&global, "global");
    address_info(printf, "printf");
    address_info(main, "main");
    address_info((void*)0, "NULL");
    address_info((void*)0xffffffffffffffffull, "kernel");
}
