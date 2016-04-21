#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <iostream>
#include <vector>
#include <sstream>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define BSWAP64(IN, OUT)           \
asm volatile ("movq %1, %%rax\n\t" \
              "bswapq %%rax  \n\t" \
              "movq %%rax, %0\n\t" \
              : "=r" (OUT)         \
              : "r" (IN)           \
              : "rax"              \
              );

#define BSWAP32(IN, OUT)           \
asm volatile ("movl %1, %%eax\n\t" \
              "bswapl %%eax  \n\t" \
              "movl %%eax, %0\n\t" \
              : "=r" (OUT)         \
              : "r" (IN)           \
              : "eax"              \
              );
#define BSWAP16(IN, OUT) OUT=(uint16_t)((IN)<<8|(IN)>>8)

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

static inline void native_lock(volatile int *lock)
{
    while (__sync_bool_compare_and_swap(lock, 0, 1) == 0) {
        sched_yield();
    }
    return;
}

static inline void native_unlock(volatile int *lock)
{
    *lock = 0;
    return;
}

uint64_t check_wan_if(char* wan_if)
{
    //printf("%s\n", wan_if);
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    strcpy(buf, wan_if);
    return *(uint64_t*)&buf;
}

uint64_t check_lan_if(char* lan_if)
{
    //printf("%s\n", lan_if);
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    strcpy(buf, lan_if);
    return *(uint64_t*)&buf;
}

void memdump(void* buffer, int length)
{
    uint32_t* addr32 = (uint32_t*)buffer;
    int i;
    int j;
    int k;
    int lines = length/16 + (length%16?1:0);
    for (i=0; i<lines; i++) {
        printf("%p : %08x %08x %08x %08x\n",
                addr32,
                htonl(*(addr32)),
                htonl(*(addr32+1)),
                htonl(*(addr32+2)),
                htonl(*(addr32+3))
        );
        addr32 += 4;
    }

    j = length%16;
    if (j == 0) return;
    k = 0;
    uint8_t*  addr8 = (uint8_t*)addr32;
    printf("%p : ", addr8);
    for (i=0; i<16; i++) {
        if (k%4 == 0 && i != 0) printf(" ");
        if (j > i) {
            printf("%02x", *addr8);
            addr8++;
        } else {
            printf("XX");
        }
        k++;
    }
    printf("\n");
    return;
}

std::string tcp_flag_info(uint8_t flags)
{
    std::string s;
    s.clear();
    if (flags & TH_FIN)  s.append("F");
    if (flags & TH_SYN)  s.append("S");
    if (flags & TH_RST)  s.append("R");
    if (flags & TH_PUSH) s.append("P");
    if (flags & TH_ACK)  s.append("A");
    if (flags & TH_URG)  s.append("U");
    if (flags & TH_ECE)  s.append("E");
    if (flags & TH_CWR)  s.append("C");
    return s;
}

char* packet_info(struct ip* ip)
{
    static char buffer[256];
    char* buf = buffer;
    struct ip6_hdr* ip6;
    struct tcphdr* tcphdr;
    struct udphdr* udphdr;
    struct icmp*   icmphdr;
    char src[sizeof("255.255.255.255")];
    char dst[sizeof("255.255.255.255")];
    int ret;

    std::string f;

    buf[0] = 0;
    src[0] = 0;
    dst[0] = 0;

    if (ip->ip_v == 4) {

        strcpy(src, inet_ntoa(ip->ip_src));
        strcpy(dst, inet_ntoa(ip->ip_dst));

        if (htons(ip->ip_off)&IP_DF) {
            ret = sprintf(buf, "[DF]");
            buf += ret;
        }
        if (htons(ip->ip_off)&IP_MF) {
            ret = sprintf(buf, "[MF]");
            buf += ret;
        }

        switch (ip->ip_p)
        {
            case (2):
            {
                ret = sprintf(buf, "[IPv4][IGMP] %s -> %s", src, dst);
                buf += ret;
                break;
            }

            case (47):
            {
                ret = sprintf(buf, "[IPv4][GRE] %s -> %s", src, dst);
                buf += ret;
                break;
            }

            case IPPROTO_TCP:
            {
                tcphdr = (struct tcphdr*)((char*)ip+(ip->ip_hl<<2));
                f = tcp_flag_info(tcphdr->th_flags);
                ret = sprintf(buf, "[IPv4][TCP][%s] %s:%d -> %s:%d",
                              f.c_str(),
                              src,
                              ntohs(tcphdr->th_sport),
                              dst,
                              ntohs(tcphdr->th_dport));
                buf += ret;
                break;
            }

            case IPPROTO_UDP:
            {
                udphdr = (struct udphdr*)((char*)ip+(ip->ip_hl<<2));
                ret = sprintf(buf, "[IPv4][UDP] %s:%d -> %s:%d",
                              src,
                              ntohs(udphdr->uh_sport),
                              dst,
                              ntohs(udphdr->uh_dport));
                buf += ret;
                break;
            }

            case IPPROTO_ICMP:
            {
                icmphdr = (struct icmp*)((char*)ip+(ip->ip_hl<<2));
                ret = sprintf(buf, "[IPv4][ICMP] %s -> %s %u(%u)",
                              src,
                              dst,
                              icmphdr->icmp_type,
                              icmphdr->icmp_code);
                buf += ret;
                break;
            }

            default:
            {
                ret = sprintf(buf, "[IPv4][%d] %s -> %s ", ip->ip_p, src, dst);
                buf += ret;
                break;
            }
        }
        return buffer;

    } else if(ip->ip_v == 6) {

        ip6 = (struct ip6_hdr*)ip;

        switch (ip6->ip6_nxt)
        {
            case (47):
            {
                sprintf(buf, "[IPv6][GRE]");
                break;
            }

            case IPPROTO_ICMPV6:
            {
                sprintf(buf, "[IPv6][ICMP6]");
                break;
            }

            case IPPROTO_TCP:
            {
                sprintf(buf, "[IPv6][TCP]");
                break;
            }

            case IPPROTO_UDP:
            {
                sprintf(buf, "[IPv6][UDP]");
                break;
            }

            default:
            {
                sprintf(buf, "[IPv6][%d]", ip6->ip6_nxt);
                break;
            }
        }
        return buffer;

    } else {

        sprintf(buf, "[%d]", ip->ip_v);

    }

    return buffer;
}

void fill_scopeid(struct sockaddr_in6 *sin6)
{
    if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
        sin6->sin6_scope_id = ntohl(*(u_int32_t*)&sin6->sin6_addr.s6_addr[2]);
        sin6->sin6_addr.s6_addr[2] = 0;
        sin6->sin6_addr.s6_addr[3] = 0;
    }
    return;
}

uint32_t string2addr_calc(std::string& s1, std::string& s2)
{
        //if(!(isv4addr_str(s1)&&isv4addr_str(s2))) return 0;

        uint32_t addr;
        uint32_t mask;
        const char* temp1 = s1.c_str();
        const char* temp2 = s2.c_str();
        inet_aton(temp1, (struct in_addr*)&addr);
        inet_aton(temp2, (struct in_addr*)&mask);

        return addr & mask;
}

uint32_t string2addr(std::string& s1)
{
        std::string s2("255.255.255.255");
        return string2addr_calc(s1, s2);
}

std::string addr2string_calc(uint32_t addr, uint32_t mask)
{
        //if(!(isv4addr_num(addr)&&isv4addr_num(mask))) return NULL;

        uint32_t net = addr & mask;
        char* temp = inet_ntoa(*(struct in_addr*)&net);
        return std::string(temp);
}

std::string addr2string(uint32_t addr)
{
        return addr2string_calc(addr, 0xffffffff);
}

void split(const std::string& s_src, const char *c, std::vector<std::string>& s_dst)
{
    // s_src : src string
    // c     : separator
    // s_dst : dst string vecotor

    using namespace std;
    std::string::size_type i = 0;
    std::string::size_type j = s_src.find(c);
    std::string tmp = s_src.substr(i,j-i);

    s_dst.clear();

    if (tmp.size() == 0) return;
    s_dst.push_back(tmp);

    while(j != std::string::npos){
        i = j++;
        j = s_src.find(c,j);
        if (j == std::string::npos){
            s_dst.push_back(s_src.substr(i+1, s_src.size()));
            break;
        }
        tmp = s_src.substr(i,j-i);
        s_dst.push_back(tmp.substr(1,tmp.size()));
    }
    return;
}

size_t get_mtu(const char* ifname)
{
    int fd;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, ifname, strlen(ifname));
    if (ioctl(fd, SIOCGIFMTU, &ifr) != 0) {
        perror("ioctl");
        return -1;
    }
    close(fd);
    return ifr.ifr_mtu;
}

#endif // UTILS_H


