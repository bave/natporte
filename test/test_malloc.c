#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sysexits.h>
#include <pthread.h>

extern int errno;

#include <sched.h>
inline void native_lock(volatile int *lock)
{
    while (__sync_bool_compare_and_swap(lock, 0, 1) == 0) {
        sched_yield();
    }
    return;
}

inline void native_unlock(volatile int *lock)
{
    *lock = 0;
    return;
}

// time measurement macro
#define TCHK_START(name)           \
    struct timeval name##_prev;    \
    struct timeval name##_current; \
    gettimeofday(&name##_prev, NULL)

#define TCHK_END(name)                                                             \
gettimeofday(&name##_current, NULL);                                               \
time_t name##_sec;                                                                 \
suseconds_t name##_usec;                                                           \
if (name##_current.tv_sec == name##_prev.tv_sec) {                                 \
    name##_sec = name##_current.tv_sec - name##_prev.tv_sec;                       \
    name##_usec = name##_current.tv_usec - name##_prev.tv_usec;                    \
} else if (name ##_current.tv_sec != name##_prev.tv_sec) {                         \
    int name##_carry = 1000000;                                                    \
    name##_sec = name##_current.tv_sec - name##_prev.tv_sec;                       \
    if (name##_prev.tv_usec > name##_current.tv_usec) {                            \
        name##_usec = name##_carry - name##_prev.tv_usec + name##_current.tv_usec; \
        name##_sec--;                                                              \
        if (name##_usec > name##_carry) {                                          \
            name##_usec = name##_usec - name##_carry;                              \
            name##_sec++;                                                          \
        }                                                                          \
    } else {                                                                       \
        name##_usec = name##_current.tv_usec - name##_prev.tv_usec;                \
    }                                                                              \
}                                                                                  \
printf("%s: sec:%lu usec:%06d\n", #name, name##_sec, name##_usec); 


void memdump(void* mem, int i)
{
    if(i >= 2000) {
        printf("allocation memory size over\n");
        return;
    }
    int j;
    int max;
    int *memi;
    int *buf;
    buf = (int*)malloc(2000);
    memset(buf, 0, 2000);
    memcpy(buf, mem, i);
    memi = buf;

    printf("start memory dump %p ***** (16byte alignment)\n", mem);
    max = i / 16 + (i % 16 ? 1 : 0);
    for (j = 0; j < max; j++) {
        printf("%p : %08x %08x %08x %08x\n",
                memi,
                htonl(*(memi)),
                htonl(*(memi+1)),
                htonl(*(memi+2)),
                htonl(*(memi+3))
        );
        memi += 4;
    }
    printf("end memory dump *****\n");
    free(buf);
    return;
}

uint8_t buffer[] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
                    0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

#define MAXLOOP 30000000
int main(void)
{
    int i;
    TCHK_START(normal);
    for (i=0; i<1000000; i++) {
        char* tmp = (char*)malloc(BUFSIZ);
        free(tmp);
    }
    TCHK_END(normal)
    return 0;
}
