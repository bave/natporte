#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define CTCHK_START(name)                     \
    static int name##_count = 0;              \
    static time_t name##_sec_total = 0;       \
    static suseconds_t name##_usec_total = 0; \
    struct timeval name##_prev;               \
    struct timeval name##_current;            \
    gettimeofday(&name##_prev, NULL)

#define CTCHK_END(name, count)                                                     \
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
name##_sec_total += name##_sec;                                                    \
name##_usec_total += name##_usec;                                                  \
while (name##_usec_total >= 1000000) {                                             \
    name##_usec_total = name##_usec_total - 1000000;                               \
    name##_sec_total += 1;                                                         \
}                                                                                  \
name##_count++;                                                                    \
if (name##_count >= count) {                                                       \
    printf("%s: sec:%lu usec:%06d\n", #name, name##_sec_total, name##_usec_total); \
    name##_sec_total = 0;                                                          \
    name##_usec_total = 0;                                                         \
    name##_count = 0;                                                              \
}

uint16_t diff_checksum(uint16_t* base_sum, void *orig, void *alias, int word16_length)
{
    int i;
    int32_t accumulate;
    uint16_t *orig16 = orig;
    uint16_t *alias16 = alias;

    accumulate = *base_sum;
    for (i = 0; i<word16_length; i++) {
        accumulate -= *alias16++;
        accumulate += *orig16++;
    }

    if (accumulate < 0) {
        accumulate = -accumulate;
        accumulate = (accumulate >> 16)
                   + (accumulate & 0xffff);
        accumulate += accumulate >> 16;
        return (uint16_t)~accumulate;
    } else {
        accumulate = (accumulate >> 16)
                   + (accumulate & 0xffff);
        accumulate += accumulate >> 16;
        return (uint16_t)accumulate;
    }
}

uint16_t checksum(const uint8_t* buf, size_t size, uint32_t adjust)
{
    uint32_t sum = 0;
    uint16_t element = 0;

    while (size>0) {
        element = (*buf)<<8;
        buf++;
        size--;
        if (size>0) {
            element |= *buf;
            buf++;
            size--;
        }
        sum += element;
    }
    sum += adjust;

    while (sum>0xFFFF) {
        sum = (sum>>16) + (sum&0xFFFF);
    }

    return (~sum) & 0xFFFF;
}

void checksum_transport(struct ip* iphdr, size_t size)
{
    uint32_t pseudoSum = 0;
    uint8_t protocol;
    uint8_t* l3_buf = (uint8_t*)iphdr;
    uint8_t* l4_buf = (uint8_t*)iphdr+(iphdr->ip_hl<<2);

    pseudoSum += (l3_buf[12]<<8) | l3_buf[13];
    pseudoSum += (l3_buf[14]<<8) | l3_buf[15];

    pseudoSum += (l3_buf[16]<<8) | l3_buf[17];
    pseudoSum += (l3_buf[18]<<8) | l3_buf[19];

    pseudoSum += protocol = iphdr->ip_p;

    size_t segment_size = size - (iphdr->ip_hl<<2);
    pseudoSum += segment_size;

    if (protocol == IPPROTO_TCP) { 
        struct tcphdr* tcphdr = (struct tcphdr*)l4_buf;
        tcphdr->th_sum = 0x0000;
        tcphdr->th_sum = htons(checksum(l4_buf, segment_size, pseudoSum));
    } else if (protocol == IPPROTO_UDP) { 
        struct udphdr* udphdr = (struct udphdr*)l4_buf;
        udphdr->uh_sum = 0x0000;
        udphdr->uh_sum = htons(checksum(l4_buf, segment_size, pseudoSum));
    } else {
        ;
    }

    return;
}

void print_sum(uint8_t* ans)
{
    printf("0x%x", *(ans+1));
    printf("%x\n", *(ans));
    return;
}

int main()
{
    uint8_t bin_sample[] = {0x08, 0x00, 0x00, 0x00, 0x56, 0x58, 0x00, 0x01,
                            0x50, 0x8f, 0x92, 0x1b, 0x00, 0x0c, 0x7c, 0x29,
                            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
                            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                            0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
                            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    size_t size = sizeof(bin_sample);
    struct ip* iphdr = (struct ip*)&bin_sample;
    struct tcphdr* tcphdr = (struct tcphdr*)(bin_sample+(iphdr->ip_hl<<2));
    iphdr->ip_p = IPPROTO_TCP;
    iphdr->ip_sum = 0x0000;
    iphdr->ip_sum = htons(checksum((uint8_t*)iphdr, 20, 0));
    checksum_transport(iphdr, size);
    printf("orig_ip_sum:");
    print_sum((uint8_t*)&iphdr->ip_sum);
    printf("orig_tcp_sum:");
    print_sum((uint8_t*)&tcphdr->th_sum);

    uint16_t ret;
    uint32_t orig_ip   = iphdr->ip_src.s_addr;
    uint32_t orig_port = tcphdr->th_sport;
    uint32_t alias_ip = htonl(0x7c010101);
    uint32_t alias_port = htons(0x5000);

    // ip
    uint32_t orig = orig_ip;
    uint32_t alias = alias_ip;
    int i;
    int j = 1000000;
    printf("loop:%d\n", j);
    for (i=0; i<j; i++) {
        CTCHK_START(diff_ip);
        ret = diff_checksum(&iphdr->ip_sum, &orig, &alias, 2);
        CTCHK_END(diff_ip, j-1);
    }
    print_sum((uint8_t*)&ret);

    iphdr->ip_src.s_addr = alias_ip;
    for (i=0; i<j; i++) {
        CTCHK_START(norm_ip);
        iphdr->ip_sum = 0x0000;
        iphdr->ip_sum = htons(checksum((uint8_t*)iphdr, 20, 0));
        CTCHK_END(norm_ip, j-1);
    }
    print_sum((uint8_t*)&iphdr->ip_sum);

    // tcp
    orig = orig_ip + orig_port;
    alias = alias_ip + alias_port;
    for (i=0; i<j; i++) {
        CTCHK_START(diff_tcp);
        ret = diff_checksum(&tcphdr->th_sum, &orig, &alias, 2);
        CTCHK_END(diff_tcp, j-1);
    }
    print_sum((uint8_t*)&ret);

    tcphdr->th_sport = alias_port;
    for (i=0; i<j; i++) {
        CTCHK_START(norm_tcp);
        checksum_transport(iphdr, size);
        CTCHK_END(norm_tcp, j-1);
    }
    print_sum((uint8_t*)&tcphdr->th_sum);

    return 0;
}
