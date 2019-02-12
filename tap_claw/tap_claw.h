#ifndef _TAP_CLAW_
#define _TAP_CLAW_

#ifndef _XBEGIN_STARTED
#define _XBEGIN_STARTED		(~0u)
#define _XABORT_EXPLICIT	(1 << 0)
#define _XABORT_RETRY		(1 << 1)
#define _XABORT_CONFLICT	(1 << 2)
#define _XABORT_CAPACITY	(1 << 3)
#define _XABORT_DEBUG		(1 << 4)
#define _XABORT_NESTED		(1 << 5)
#define _XABORT_CODE(x)		(((x) >> 24) & 0xFF)
#endif

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


size_t is_mapped(void* addr) {
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

int is_writable(void* addr) {
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


#endif
