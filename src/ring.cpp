#include "ring.h"
#include <stdlib.h>
#include <unistd.h>

struct ring_handler h;
struct ring_buffer th_buf1;
struct ring_buffer th_buf2;
struct ring_buffer th_buf3;

void* push_work(void* args)
{
    pthread_detach(pthread_self());
    for (;;) {
        printf("PUSHED:%d\n", ring_s_push(&h, &th_buf1));
        printf("PUSHED:%d\n", ring_s_push(&h, &th_buf2));
        printf("PUSHED:%d\n", ring_s_push(&h, &th_buf3));
        printf("\n");
        sleep(1);
    }
}

void* pop_work(void* args)
{
    pthread_detach(pthread_self());
    struct ring_buffer tmp_buffer;
    for (;;) {
        printf("POPPED:%d", ring_s_pop_condblock(&h, &tmp_buffer));
        printf("->buffer:%s\n", tmp_buffer.buffer);
    }
}

int main(int argc, char** argv)
{
    int ring_size = 3;
    ring_init(&h, ring_size);
    printf("ring_size is %d\n\n", ring_size);

    // buffer and size arguments, single thread
    char* tmp_buffer;
    size_t tmp_size;
    uint64_t tmp_opaque;
    ring_pop(&h, &tmp_buffer, &tmp_size, NULL);
    printf("%s\n", tmp_buffer);
    printf("%zu\n", tmp_size);
    if (tmp_buffer != NULL) {
        free(tmp_buffer);
    }
    printf("\n");

    char a[] = "test_a";
    printf("push_point:%d\n", h.push_point);
    printf("poph_point:%d\n", h.pop_point);
    printf("PUSHED:%d\n", ring_push(&h, a, sizeof(a), 1));
    printf("\n");

    char b[] = "test_b";
    printf("push_point:%d\n", h.push_point);
    printf("poph_point:%d\n", h.pop_point);
    printf("PUSHED:%d\n", ring_push(&h, b, sizeof(b), 0));
    printf("\n");

    char c[] = "test_c";
    printf("push_point:%d\n", h.push_point);
    printf("poph_point:%d\n", h.pop_point);
    printf("PUSHED:%d\n", ring_push(&h, c, sizeof(c), 1));
    printf("\n");

    ring_pop(&h, &tmp_buffer, &tmp_size, &tmp_opaque);
    printf("%s\n", tmp_buffer);
    printf("%zu\n", tmp_size);
    printf("%llu\n", tmp_opaque);
    if (tmp_buffer != NULL) {
        free(tmp_buffer);
    }
    printf("\n");

    ring_pop(&h, &tmp_buffer, &tmp_size, &tmp_opaque);
    printf("%s\n", tmp_buffer);
    printf("%zu\n", tmp_size);
    printf("%llu\n", tmp_opaque);
    if (tmp_buffer != NULL) {
        free(tmp_buffer);
    }
    printf("\n");

    ring_pop(&h, &tmp_buffer, &tmp_size, &tmp_opaque);
    printf("%s\n", tmp_buffer);
    printf("%zu\n", tmp_size);
    printf("%llu\n", tmp_opaque);
    if (tmp_buffer != NULL) {
        free(tmp_buffer);
    }
    printf("\n");

    printf("push_point:%d\n", h.push_point);
    printf("poph_point:%d\n", h.pop_point);
    printf("PUSHED:%d\n", ring_push(&h, c, sizeof(c), 2));
    printf("\n");

    ring_pop(&h, &tmp_buffer, &tmp_size, &tmp_opaque);
    printf("%s\n", tmp_buffer);
    printf("%zu\n", tmp_size);
    printf("%llu\n", tmp_opaque);
    if (tmp_buffer != NULL) {
        free(tmp_buffer);
    }
    printf("\n");

    // struct ring_buffer argument single thread
    /*
    struct ring_buffer buf1;
    buf1.buffer = "test1";
    buf1.size = strlen(buf1.buffer);
    struct ring_buffer buf2;
    buf2.buffer = "test2";
    buf2.size = strlen(buf2.buffer);
    struct ring_buffer buf3;
    buf3.buffer = "test3";
    buf3.size = strlen(buf3.buffer);

    struct ring_buffer tmp_buffer;
    printf("PUSHED:%d\n", ring_s_push(&h, &buf1));
    printf("PUSHED:%d\n", ring_s_push(&h, &buf2));
    printf("PUSHED:%d\n", ring_s_push(&h, &buf3));
    printf("POPPED:%d(", ring_s_pop(&h, &tmp_buffer)); printf("%s)\n", tmp_buffer.buffer);
    printf("POPPED:%d(", ring_s_pop(&h, &tmp_buffer)); printf("%s)\n", tmp_buffer.buffer);
    printf("PUSHED:%d\n", ring_s_push(&h, &buf1));
    printf("POPPED:%d(", ring_s_pop(&h, &tmp_buffer)); printf("%s)\n", tmp_buffer.buffer);
    printf("PUSHED:%d\n", ring_s_push(&h, &buf2));
    printf("POPPED:%d(", ring_s_pop(&h, &tmp_buffer)); printf("%s)\n", tmp_buffer.buffer);
    exit(0);
    */

    // struct ring_buffer argument, 2 threads
    static char* t1 = "test1";
    th_buf1.buffer = t1;
    th_buf1.size = sizeof(th_buf1.buffer);

    static char* t2 = "test2";
    th_buf2.buffer = t2;
    th_buf1.size = sizeof(th_buf2.buffer);

    static char* t3 = "test3";
    th_buf3.buffer = t3;
    th_buf1.size = sizeof(th_buf3.buffer);

    pthread_t push;
    pthread_t pop;
    int retval;
    retval = pthread_create(&push, NULL, push_work, NULL);
    if (retval != 0) {
        perror("pthread_create:push_work");
        exit(-1);
    }
    retval = pthread_create(&pop, NULL, pop_work, NULL);
    if (retval != 0) {
        perror("pthread_create:pop_work");
        exit(-1);
    }

    for (;;) sleep(1);

    return 0;
}
