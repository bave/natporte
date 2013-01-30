#ifndef ALIAS_H
#define ALIAS_H 1

#include <vector>

#include "structure.h"
#include "utils.hpp"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#include "hostid.h"

extern bool opt_hostid;
extern bool opt_nagete_df;
extern size_t wan_if_mtu;
extern size_t lan_if_mtu;

#define ALIAS_THROUGH      5
#define ALIAS_RESET_TABLE  4
#define ALIAS_DEL_TABLE    3
#define ALIAS_FROM_ME      2
#define ALIAS_TO_ME        1
#define ALIAS_DOALIAS      0
#define ALIAS_NO_TABLE    -1
#define ALIAS_TOOSHORT    -2
#define ALIAS_DODROP      -3
#define ALIAS_NOT_SUPPORT -4
#define ALIAS_TOO_BIG     -5

#define ALIAS_MIN_SIZE 20

#define ALIAS_FROM_LAN2WAN 0
#define ALIAS_FROM_WAN2LAN 1
#define ALIAS_FROM_UNKNOWN 2

#define ALIAS_IS_CLASS_A(i)  (((uint32_t)(i) & 0x80000000) == 0)
#define ALIAS_IS_CLASS_B(i)  (((uint32_t)(i) & 0xc0000000) == 0x80000000)
#define ALIAS_IS_CLASS_C(i)  (((uint32_t)(i) & 0xe0000000) == 0xc0000000)
#define ALIAS_IS_CLASS_D(i)  (((uint32_t)(i) & 0xf0000000) == 0xe0000000)
#define ALIAS_IS_EXP(i)      (((uint32_t)(i) & 0xf0000000) == 0xf0000000)
#define ALIAS_IS_LOOPBACK(i) (((uint32_t)(i) & 0xff000000) == 0x7f000000)
#define ALIAS_IS_LOCAL(i)    (((uint32_t)(i) & 0xffff0000) == 0xa9fe0000)

// 10.0.0.0/8
// 172.16.0.0/12
// 192.168.0.0/16
// 100.64.0.0/10
#define ALIAS_IS_PRIVATE(i) ((((uint32_t)(i) & 0xff000000) == 0x0a000000)|| \
                             (((uint32_t)(i) & 0xfff00000) == 0xac100000)|| \
                             (((uint32_t)(i) & 0xffff0000) == 0xc0a80000)|| \
                             (((uint32_t)(i) & 0xffc00000) == 0x64400000))


std::vector<uint32_t> lan_addr;
std::vector<uint32_t> wan_addr;
std::vector<uint32_t> pool_addr;

#ifdef __MACH__
struct icmphdr {
    uint8_t  icmp_type;
    uint8_t  icmp_code;
    uint16_t icmp_cksum;
};
#endif

extern uint32_t icmp_wan_addr;

/*
static inline const char* table_tcp_status(int s_tf_flag)
{
    switch (s_tf_flag)
    {
        case S_TF_ESTABLISHED:
        {
            return "S_TF_ESTABLISHED";
        }

        case 0x01:
        {
            return "S_TF_SYN_SENT";
        }

        case 0x02:
        {
            return "S_TF_SYNACK_RECIVED";
        }

        case 0x03:
        {
            return "S_TF_FIN_WAIT_L2W";
        }

        case 0x04:
        {
            return "S_TF_FIN_WAIT_W2L";
        }

        case 0x05:
        {
            return "S_TF_FINACK_WAIT_L2W";
        }

        case 0x06:
        {
            return "S_TF_FINACK_WAIT_W2L";
        }

        case 0x07:
        {
            return "S_TF_CLOSING_L2W";
        }

        case 0x08:
        {
            return "S_TF_CLOSING_W2L";
        }

        case 0x09:
        {
            return "S_TF_CLOSED";
        }

        case 0x0A:
        {
            return "S_TF_RESET";
        }

        case 0x0F:
        {
            return "S_TF_UNKNOWN";
        }
    }
    return NULL;
}
*/


static inline uint8_t tcp_state_check(uint8_t tflag, uint8_t pflag, int from)
{
    if (from == ALIAS_FROM_WAN2LAN) {
        switch(tflag)
        {
            case S_TF_ESTABLISHED:
            {
                //printf("est\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_FIN) {
                    return S_TF_FIN_WAIT_L2W;
                } else {
                    return S_TF_ESTABLISHED;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:W2L:S_TF_ESTABLISHED:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_SYN_SENT:
            {
                //printf("w2l_syn_sent\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_SYN) {
                    return S_TF_SYNACK_RECIVED;
                } 
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:W2L:S_TF_SYN_SENT:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_SYNACK_RECIVED:
            {
                //printf("syn_recived\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if ((pflag&(TH_SYN|TH_ACK)) == (TH_SYN|TH_ACK)) {
                    return S_TF_ESTABLISHED;
                } else if (pflag&TH_ACK) {
                    return S_TF_SYNACK_RECIVED;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:W2L:S_TF_SYNACK_RECIVED:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_FIN_WAIT_W2L:
            {
                //printf("fin_wait_w2l\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_ACK && ((pflag&TH_FIN) != TH_FIN)) {
                    return S_TF_FINACK_WAIT_W2L;
                } else if (pflag&(TH_ACK|TH_FIN)) {
                    return S_TF_CLOSING_L2W;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:S_TF_FIN_WAIT_W2L:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_FINACK_WAIT_W2L:
            {
                //printf("finack_wait_w2l\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&(TH_FIN)) {
                    return S_TF_CLOSING_L2W;
                } else if (pflag&TH_ACK) {
                    return S_TF_FINACK_WAIT_W2L;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:W2L:S_TF_FINACK_WAIT_W2L:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_CLOSING_W2L:
            {
                //printf("closing_w2l\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_ACK) {
                    return S_TF_CLOSED;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:W2L:S_TF_CLOSING_W2L:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_CLOSED:
            {
                break;
            }

            case S_TF_RESET:
            {
                break;
            }

            case S_TF_FINACK_WAIT_L2W:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else {
                    return S_TF_FINACK_WAIT_L2W;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:W2L:S_TF_FINACK_WAIT_L2W:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_CLOSING_L2W:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else {
                    return S_TF_CLOSING_L2W;
                }
                break;
            }

            case S_TF_FIN_WAIT_L2W:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else {
                    return S_TF_FIN_WAIT_L2W;
                }
                break;
            }

            default:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":"
                          << table_tcp_status(tflag)
                          << ":break:W2L:default:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }
        }
    } else if (from == ALIAS_FROM_LAN2WAN) {
        switch(tflag)
        {
            case S_TF_ESTABLISHED:
            {
                //printf("est\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_FIN) {
                    return S_TF_FIN_WAIT_W2L;
                } else {
                    return S_TF_ESTABLISHED;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:L2W:S_TF_ESTABLISHED:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_SYN_SENT:
            {
                //printf("l2w_syn_sent\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_SYN) {
                    return S_TF_SYNACK_RECIVED;
                } 
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:L2W:S_TF_SYN_SENT:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_SYNACK_RECIVED:
            {
                //printf("syn_recived\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if ((pflag&(TH_SYN|TH_ACK)) == (TH_SYN|TH_ACK)) {
                    return S_TF_ESTABLISHED;
                } else if (pflag&TH_SYN) {
                    return S_TF_SYNACK_RECIVED;
                } 
#ifdef DEBUG
                std::cout << __LINE__
                          << "break:L2W:S_TF_SYNACK_RECIVED:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_FIN_WAIT_L2W:
            {
                //printf("fin_wait_l2w\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_ACK && ((pflag&TH_FIN) != TH_FIN)) {
                    return S_TF_FINACK_WAIT_L2W;
                } else if (pflag&(TH_ACK|TH_FIN)) {
                    return S_TF_CLOSING_W2L;
                } 
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:L2W:S_TF_FIN_WAIT_L2W:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }
            case S_TF_FINACK_WAIT_L2W:
            {
                //printf("finack_wait_l2w\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&(TH_FIN)) {
                    return S_TF_CLOSING_W2L;
                } else if (pflag&TH_ACK) {
                    return S_TF_FINACK_WAIT_L2W;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:L2W:S_TF_FINACK_WAIT_L2W:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_CLOSING_L2W:
            {
                //printf("closing_l2w\n");
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else if (pflag&TH_ACK) {
                    return S_TF_CLOSED;
                } 
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:L2W:S_TF_CLOSING_L2W:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_CLOSED:
            {
                break;
            }

            case S_TF_RESET:
            {
                break;
            }

            case S_TF_FINACK_WAIT_W2L:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else {
                    return S_TF_FINACK_WAIT_W2L;
                }
#ifdef DEBUG
                std::cout << __LINE__
                          << ":break:L2W:S_TF_FINACK_WAIT_W2L:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }

            case S_TF_CLOSING_W2L:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else {
                    return S_TF_CLOSING_W2L;
                }
                break;
            }

            case S_TF_FIN_WAIT_W2L:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                } else {
                    return S_TF_FIN_WAIT_W2L;
                }
                break;
            }

            default:
            {
                if (pflag&TH_RST) {
                    return S_TF_RESET;
                }
#ifdef DEBUG
                std::cout << __LINE__ 
                          << ":"
                          << table_tcp_status(tflag)
                          << ":break:L2W:default:"
                          << tcp_flag_info(pflag)
                          << std::endl;
#endif
                break;
            }
        }
    } else {
        ;
    }

#ifdef DEBUG
    std::cout << __LINE__ 
              << ":"
              << table_tcp_status(tflag) 
              << "->S_TF_UNKNOWN:"
              << tcp_flag_info(pflag)
              << std::endl;
#endif
    return S_TF_UNKNOWN;
}

static inline uint16_t checksum(const uint8_t* buf, size_t size, uint32_t adjust)                     
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


/*
 * 0                16               31
 * +----------------+----------------+       ∧
 * |       Source IPv4 Address       |       |
 * +----------------+----------------+       |
 * |     Destination IPv4 Address    | pseudo-header
 * +--------+-------+----------------+       |
 * | dummmy | Proto | TCP/UDP SegLen |       |
 * +--------+-------+----------------+       ∨
 * :                                 :
 */
static inline void checksum_transport(struct ip* iphdr, size_t size)
{
    uint32_t pseudoSum = 0;
    uint8_t protocol;
    uint8_t* l3_buf = (uint8_t*)iphdr;
    uint8_t* l4_buf = (uint8_t*)iphdr+(iphdr->ip_hl<<2);

    // Src Address ipv4
    pseudoSum += (l3_buf[12]<<8) | l3_buf[13];
    pseudoSum += (l3_buf[14]<<8) | l3_buf[15];

    // Dst Address ipv4
    pseudoSum += (l3_buf[16]<<8) | l3_buf[17];
    pseudoSum += (l3_buf[18]<<8) | l3_buf[19];

    // Protocol Number
    pseudoSum += protocol = iphdr->ip_p;

    size_t segment_size = size - (iphdr->ip_hl<<2);
    pseudoSum += segment_size;

    // protocol check !!
    if (protocol == IPPROTO_TCP) { 
        //pseudoSum += (uint8_t)IPPROTO_TCP;
        struct tcphdr* tcphdr = (struct tcphdr*)l4_buf;
        tcphdr->th_sum = 0x0000;
        tcphdr->th_sum = htons(checksum(l4_buf, segment_size, pseudoSum));
    } else if (protocol == IPPROTO_UDP) { 
        //pseudoSum += (uint8_t)IPPROTO_UDP;
        struct udphdr* udphdr = (struct udphdr*)l4_buf;
        udphdr->uh_sum = 0x0000;
        udphdr->uh_sum = htons(checksum(l4_buf, segment_size, pseudoSum));
    } else if (protocol == IPPROTO_ICMP) {
        struct icmphdr* icmphdr = (struct icmphdr*)l4_buf;
        icmphdr->icmp_cksum = 0x0000;
        icmphdr->icmp_cksum = htons(checksum((uint8_t*)icmphdr, segment_size, 0));
    } else {
        ;
    }

    return;
}

static inline void checksum_ip(struct ip* iphdr)
{
    int header_length;
    uint16_t cksum;

    header_length = iphdr->ip_hl<<2;

    iphdr->ip_sum = 0x0000;
    cksum = checksum((uint8_t*)iphdr, header_length, 0);
    iphdr->ip_sum = htons(cksum);

    return;
}


static inline void set_checksum(char* buf, size_t size)
{
    struct ip* iphdr = (struct ip*)buf;
#ifdef IP_CHECKSUM
    checksum_ip(iphdr);
#endif
    checksum_transport(iphdr, size);
    return;
}

uint16_t diff_checksum(uint16_t* base_sum, uint64_t orig, uint64_t alias, int word16_length)
{
    int i;
    int32_t accumulate;
    uint16_t* orig16 = (uint16_t*)&orig;
    uint16_t* alias16 = (uint16_t*)&alias;
             
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

static inline void set_diff_checksum(char* buf,
        uint32_t orig_ip, uint16_t orig_port, 
        uint32_t alias_ip, uint16_t alias_port)
{
    uint64_t orig;
    uint64_t alias;
    orig = orig_ip;
    alias = alias_ip;
    struct ip* iphdr = (struct ip*)buf;
#ifdef IP_CHECKSUM
    iphdr->ip_sum = diff_checksum(&iphdr->ip_sum, orig, alias, 2);
#endif
    orig += orig_port;
    alias += alias_port;
    if (iphdr->ip_p == IPPROTO_TCP) {
        struct tcphdr* tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));
        tcphdr->th_sum = diff_checksum(&tcphdr->th_sum, orig, alias, 2);
    } else if (iphdr->ip_p == IPPROTO_UDP) {
        struct udphdr* udphdr = (struct udphdr*)(buf+(iphdr->ip_hl<<2));
        udphdr->uh_sum = diff_checksum(&udphdr->uh_sum, orig, alias, 2);
    } else {
        ;
    }
    return;
}

static inline int is_too_big(char* buf, size_t size, uint32_t from)
{
    if (htons(((struct ip*)buf)->ip_off)&IP_DF) {
        if (from == ALIAS_FROM_LAN2WAN) {
            if (opt_nagete_df) {
                return 0;
            } else {
                if (opt_hostid) {
                    if (size > wan_if_mtu - 4) {
                        return -1;
                    }
                } else {
                    if (size > wan_if_mtu) {
                        return -1;
                    }
                }
            }
        } else if (from == ALIAS_FROM_WAN2LAN) {
            if (opt_nagete_df) {
                return 0;
            } else {
                if (opt_hostid) {
                    if (size > lan_if_mtu - 4) {
                        return -1;
                    }
                } else {
                    if (size > lan_if_mtu) {
                        return -1;
                    }
                }
            }
        } else {
            return 0;
        }
    } else {
        return 0;
    }
    return 0;
}

static inline int chk_drop(uint32_t src, uint32_t dst, uint32_t from)
{
    uint32_t src_host = ntohl(src);
    uint32_t dst_host = ntohl(dst);

    if (from == ALIAS_FROM_LAN2WAN) {
        if (!ALIAS_IS_PRIVATE(src_host)||
             ALIAS_IS_CLASS_D(dst_host)||
             src_host == 0 ||
             dst_host == 0)
        {
            return -1;
        }
    } else if (from == ALIAS_FROM_WAN2LAN) {
        if ( ALIAS_IS_LOCAL(dst_host)   ||
             ALIAS_IS_CLASS_D(dst_host) ||
             ALIAS_IS_LOOPBACK(dst_host)||
             src_host == 0 ||
             dst_host == 0)
        {
            return -1;
        }
    } else if (from == ALIAS_FROM_UNKNOWN) {
        ;
    }

    return 0;
}

static inline int chk_selfaddr(uint32_t src, uint32_t dst, uint32_t from)
{
    //uint32_t src_host = ntohl(src);
    uint32_t dst_host = ntohl(dst);

    int i;
    if (from == ALIAS_FROM_LAN2WAN) {
        for (i=0; i<(int)lan_addr.size(); i++) {
            if (lan_addr[i] == dst_host) {
                return ALIAS_TO_ME;
            }
        }
    } else if (from == ALIAS_FROM_UNKNOWN) {
        ;
    } else if (from == ALIAS_FROM_WAN2LAN) {
        for (i=0; i<(int)wan_addr.size(); i++) {
            if (wan_addr[i] == dst_host) {
                return ALIAS_TO_ME;
            }
        }
    }

    return 0;
}

static inline void tcp_reset_alias(char* buf, ssize_t size)
{
    struct ip* iphdr = (struct ip*)buf;
    struct tcphdr* tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));

    uint32_t tmp32;
    uint16_t tmp16;

    tmp32 = iphdr->ip_src.s_addr;
    iphdr->ip_src.s_addr = iphdr->ip_dst.s_addr;
    iphdr->ip_dst.s_addr = tmp32;

    tmp16 = tcphdr->th_sport;
    tcphdr->th_sport = tcphdr->th_dport;
    tcphdr->th_dport = tmp16;
    tcphdr->th_flags = TH_RST|TH_ACK;
    tcphdr->th_ack = tcphdr->th_seq + htonl(1);
    tcphdr->th_seq = 0; 
     
    set_checksum((char*)iphdr, size);

    return;
}

static inline int other_if_check(char* buf, ssize_t size)
{
    struct ip* iphdr = (struct ip*)buf;
    if (iphdr->ip_p == IPPROTO_ICMP) {
        icmp_bucket.local   = iphdr->ip_src.s_addr; 
        icmp_bucket.foreign = iphdr->ip_dst.s_addr; 
        iphdr->ip_sum = 0x0000;
    }
    return ALIAS_THROUGH;
}

static inline int wan2lan_alias(char* buf, ssize_t size)
{
    if (size <= ALIAS_MIN_SIZE) { 
        return ALIAS_TOOSHORT;
    }

    struct node n;
    struct node* table;

    struct ip* iphdr = (struct ip*)buf;

    if (iphdr->ip_v != 4) {
        return ALIAS_DODROP;
    }

    n.protocol = iphdr->ip_p;
    n.lan.ip   = 0x000000;
    n.wan.ip   = iphdr->ip_dst.s_addr;

    int ret;
    ret = chk_drop(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_WAN2LAN);
    if (ret != 0) {
        return ALIAS_DODROP;
    }

    ret = is_too_big(buf, size, ALIAS_FROM_WAN2LAN);
    if (ret != 0) {
        return ALIAS_TOO_BIG;
    }

    switch (iphdr->ip_p)
    {
        case IPPROTO_TCP:
        {
            // L4 size check ------------
            // return ALIAS_TOO_SHROT;
            // ------------------------
            struct tcphdr* tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));
            n.lan.port = 0x0000;
            n.wan.port = tcphdr->th_dport;

            n.w2l_key  = var_wan(&n);
            table = find_w2l(&n);

            if (table == NULL) {
                ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_WAN2LAN);
                if (ret == ALIAS_TO_ME) {
                    return ALIAS_TO_ME;
                }
                return ALIAS_NO_TABLE;
            } else if (table->flags == S_TF_CLOSED) { 
                //table->atime = time(NULL);
                attach(table);
                iphdr->ip_dst.s_addr = table->lan.ip;
                tcphdr->th_dport = table->lan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->wan.ip, table->wan.port,
                                                table->lan.ip, table->lan.port);
#else
                set_checksum((char*)iphdr, size);
#endif
            } else if (table->flags == S_TF_RESET) { 
                //table->atime = time(NULL);
                attach(table);
                iphdr->ip_dst.s_addr = table->lan.ip;
                tcphdr->th_dport = table->lan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->wan.ip, table->wan.port,
                                                table->lan.ip, table->lan.port);
#else
                set_checksum((char*)iphdr, size);
#endif
            } else {
                table->flags = tcp_state_check(table->flags, tcphdr->th_flags, ALIAS_FROM_WAN2LAN);
                if (table->flags==S_TF_CLOSED) {
#ifdef WAIT_TIME_DELETE
                    attach(table);
                    iphdr->ip_dst.s_addr = table->lan.ip;
                    tcphdr->th_dport = table->lan.port;
#ifdef DIFF_CKSUM
                    set_diff_checksum((char*)iphdr, table->wan.ip, table->wan.port,
                                                    table->lan.ip, table->lan.port);
#else
                    set_checksum((char*)iphdr, size);
#endif
#endif
                    return ALIAS_DEL_TABLE;
                } else if (table->flags == S_TF_RESET) {
#ifdef WAIT_TIME_DELETE
                    //attach(table);
                    //iphdr->ip_dst.s_addr = table->lan.ip;
                    //tcphdr->th_dport = table->lan.port;
#ifdef DIFF_CKSUM
                    //set_diff_checksum((char*)iphdr, table->wan.ip, table->wan.port,
                    //                                table->lan.ip, table->lan.port);
#else
                    //set_checksum((char*)iphdr, size);
#endif
#endif
                    return ALIAS_RESET_TABLE;
                } else if (table->flags == S_TF_UNKNOWN) {
                    // XXX debug
                    printf("%s:%ddo_drop of unknow state tcp\n", __FILE__, __LINE__);
                    return ALIAS_DODROP;
                } else {
                    attach(table);
                    iphdr->ip_dst.s_addr = table->lan.ip;
                    tcphdr->th_dport = table->lan.port;
#ifdef DIFF_CKSUM
                    set_diff_checksum((char*)iphdr, table->wan.ip, table->wan.port,
                                                    table->lan.ip, table->lan.port);
#else
                    set_checksum((char*)iphdr, size);
#endif
                }
            }

            break;
        }

        case IPPROTO_UDP:
        {
            // L4 size check ------------
            // return ALIAS_TOO_SHROT;
            // ------------------------
            struct udphdr* udphdr = (struct udphdr*)(buf+(iphdr->ip_hl<<2));
            n.lan.port = 0x0000;
            n.wan.port = udphdr->uh_dport;

            //printf("w2l_n\n");
            //print_node(&n);
            n.w2l_key = var_wan(&n);
            table = find_w2l(&n);
            //printf("w2l_table\n");
            //print_node(table);

            if (table == NULL) {
                ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_WAN2LAN);
                if (ret == ALIAS_TO_ME) {
                    return ALIAS_TO_ME;
                }
                return ALIAS_NO_TABLE;
            } else {
                //table->atime = time(NULL);
                attach(table);
                iphdr->ip_dst.s_addr = table->lan.ip;
                udphdr->uh_dport = table->lan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->wan.ip, table->wan.port,
                                                table->lan.ip, table->lan.port);
#else
                set_checksum((char*)iphdr, size);
#endif
            }
            break;

        }

        case IPPROTO_ICMP:
        {
            struct icmphdr* icmphdr = (struct icmphdr*)(buf+(iphdr->ip_hl<<2));
            switch (icmphdr->icmp_type)
            {
                case ICMP_ECHOREPLY:
                case ICMP_TSTAMPREPLY:
                {
                    ret = chk_selfaddr(iphdr->ip_src.s_addr, icmp_bucket.local, ALIAS_FROM_WAN2LAN);
                    if (ret == ALIAS_TO_ME)
                    {
                        return ALIAS_TO_ME;
                    }
                    iphdr->ip_dst.s_addr = icmp_bucket.local;
                    iphdr->ip_sum = 0x0000;
                    break;
                }

                case ICMP_UNREACH:
                case ICMP_SOURCEQUENCH:
                case ICMP_TIMXCEED:
                case ICMP_PARAMPROB:
                {
                    struct ip* ext_iphdr = (struct ip*)(icmphdr+2);
                    struct tcphdr* ext_tcphdr = (struct tcphdr*)(((char*)ext_iphdr)+(ext_iphdr->ip_hl<<2));
                    struct udphdr* ext_udphdr = (struct udphdr*)(((char*)ext_iphdr)+(ext_iphdr->ip_hl<<2));
                    //struct icmphdr* ext_icmphdr = (struct icmphdr*)(((char*)ext_iphdr)+(ext_iphdr->ip_hl<<2));

                    n.protocol = ext_iphdr->ip_p;
                    n.lan.ip   = 0x00000000;
                    n.wan.ip   = ext_iphdr->ip_src.s_addr;

                    if (n.protocol == IPPROTO_ICMP) {
                        ;
                    }else if (n.protocol == IPPROTO_TCP) {
                        n.lan.port = 0x0000;
                        n.wan.port = ext_tcphdr->th_sport;
                    } else if (n.protocol == IPPROTO_UDP) {
                        n.lan.port = 0x0000;
                        n.wan.port = ext_udphdr->uh_sport;
                    } else {
                        return ALIAS_NOT_SUPPORT;
                    }

                    if (n.protocol != IPPROTO_ICMP) {
                        n.w2l_key = var_wan(&n);
                        table = find_w2l(&n);
                        if (table == NULL) {
                            ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_WAN2LAN);
                            if (ret == ALIAS_TO_ME) {
                                return ALIAS_TO_ME;
                            }
                            return ALIAS_NO_TABLE;
                        }
                        if (table->protocol == IPPROTO_TCP) {
                            attach(table);
                            iphdr->ip_dst.s_addr = table->lan.ip;
                            ext_iphdr->ip_src.s_addr = table->lan.ip;
                            ext_tcphdr->th_sport = table->lan.port;
                            checksum_ip(ext_iphdr);
                            icmphdr->icmp_cksum = 0x0000;
                            icmphdr->icmp_cksum = checksum((uint8_t*)icmphdr, size-(iphdr->ip_hl<<2), 0);
                            icmphdr->icmp_cksum = htons(icmphdr->icmp_cksum);
                            break;
                        } else if (table->protocol == IPPROTO_UDP) {
                            attach(table);
                            iphdr->ip_dst.s_addr = table->lan.ip;
                            ext_iphdr->ip_src.s_addr = table->lan.ip;
                            ext_udphdr->uh_sport = table->lan.port;
                            checksum_ip(ext_iphdr);
                            icmphdr->icmp_cksum = 0x0000;
                            icmphdr->icmp_cksum = checksum((uint8_t*)icmphdr, size-(iphdr->ip_hl<<2), 0);
                            icmphdr->icmp_cksum = htons(icmphdr->icmp_cksum);
                            break;
                        } 
                    } else if (n.protocol == IPPROTO_ICMP) {
                        iphdr->ip_dst.s_addr = icmp_bucket.local;
                        ext_iphdr->ip_dst.s_addr = icmp_bucket.local;
                        //iphdr->ip_src.s_addr = icmp_bucket.foreign;
                        ext_iphdr->ip_src.s_addr = icmp_bucket.foreign;
                        checksum_ip(ext_iphdr);
                        icmphdr->icmp_cksum = 0x0000;
                        icmphdr->icmp_cksum = checksum((uint8_t*)icmphdr, size-(iphdr->ip_hl<<2), 0);
                        icmphdr->icmp_cksum = htons(icmphdr->icmp_cksum);
                        break;
                    } else {
                        return ALIAS_NOT_SUPPORT;
                    }
                }

                default:
                {
                    return ALIAS_NOT_SUPPORT;
                }
            }

            break;
        }

        default:
        {
            // L4 size check ------------
            // return ALIAS_TOO_SHROT;
            // ------------------------
            return ALIAS_NOT_SUPPORT;
        }
    }

    return ALIAS_DOALIAS;
}

static inline int lan2wan_alias(char* buf, ssize_t size)
{
    if (size <= ALIAS_MIN_SIZE) { 
        return ALIAS_TOOSHORT;
    }

    struct node n;
    struct node* table;

    struct ip* iphdr = (struct ip*)buf;

    if (iphdr->ip_v != 4) {
        return ALIAS_DODROP;
    }

    n.protocol = iphdr->ip_p;
    n.lan.ip   = iphdr->ip_src.s_addr;
    n.wan.ip   = 0x000000;

    int ret;
    ret = chk_drop(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_LAN2WAN);
    if (ret != 0) {
        return ALIAS_DODROP;
    }

    ret = is_too_big(buf ,size, ALIAS_FROM_LAN2WAN);
    if (ret != 0) {
        return ALIAS_TOO_BIG;
    }


    switch (iphdr->ip_p)
    {
        case IPPROTO_TCP:
        {
            // L4 size check ------------
            // return ALIAS_TOO_SHROT;
            // ------------------------
            struct tcphdr* tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));
            n.lan.port = tcphdr->th_sport;
            n.wan.port = 0x0000;
            //n.wan.port = tcphdr->th_dport;

            //printf("compare protocol:%d\n", n.protocol);
            //std::cout << "compare ctime   :" << n.ctime << endl;
            //std::cout << "compare atime   :" << n.atime << endl;
            //std::cout << "compare lan ip  :" << n.lan.ip << endl;
            //std::cout << "compare lan port:" << n.lan.port << endl;
            //std::cout << "compare wan ip  :" << n.wan.ip << endl;
            //std::cout << "compare wan port:" << n.wan.port << endl;

            ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_LAN2WAN);
            if (ret == ALIAS_TO_ME) {
                return ALIAS_TO_ME;
            }

            n.l2w_key = var_lan(&n);
            table = find_l2w(&n);

            if (table == NULL) {
                if (tcphdr->th_flags == TH_SYN) {
                    return ALIAS_NO_TABLE;
                } else {
                    return ALIAS_DODROP;
                }
            } else if (table->flags == S_TF_CLOSED) { 
                //table->atime = time(NULL);
                attach(table);
                iphdr->ip_src.s_addr = table->wan.ip;
                tcphdr->th_sport = table->wan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->lan.ip, table->lan.port,
                                                table->wan.ip, table->wan.port);
#else
                set_checksum((char*)iphdr, size);
#endif
            } else if (table->flags == S_TF_RESET) { 
                //table->atime = time(NULL);
                attach(table);
                iphdr->ip_src.s_addr = table->wan.ip;
                tcphdr->th_sport = table->wan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->lan.ip, table->lan.port,
                                                table->wan.ip, table->wan.port);
#else
                set_checksum((char*)iphdr, size);
#endif
            } else {
                table->flags = tcp_state_check(table->flags, tcphdr->th_flags, ALIAS_FROM_LAN2WAN);
                if (table->flags==S_TF_CLOSED) {
#ifdef WAIT_TIME_DELETE
                    attach(table);
                    iphdr->ip_src.s_addr = table->wan.ip;
                    tcphdr->th_sport = table->wan.port;
#ifdef DIFF_CKSUM
                    set_diff_checksum((char*)iphdr, table->lan.ip, table->lan.port,
                                                    table->wan.ip, table->wan.port);
#else
                    set_checksum((char*)iphdr, size);
#endif
#endif
                    return ALIAS_DEL_TABLE;
                } else if (table->flags == S_TF_RESET) {
#ifdef WAIT_TIME_DELETE
                    //attach(table);
                    //iphdr->ip_src.s_addr = table->wan.ip;
                    //tcphdr->th_sport = table->wan.port;
#ifdef DIFF_CKSUM
                    //set_diff_checksum((char*)iphdr, table->lan.ip, table->lan.port,
                    //                                table->wan.ip, table->wan.port);
#else
                    //set_checksum((char*)iphdr, size);
#endif
#endif
                    return ALIAS_RESET_TABLE;
                } else if (table->flags == S_TF_UNKNOWN) {
                    // XXX debug
                    printf("%s:%ddo_drop of unknow state tcp\n", __FILE__, __LINE__);
                    return ALIAS_RESET_TABLE;
                    //or
                    //return ALIAS_DODROP;
                } else {
                    //table->atime = time(NULL);
                    attach(table);
#ifdef OPT_HOSTID
                    if (opt_hostid) {
                        size = set_hostid(iphdr, size);
                        tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));
                    }
#endif
                    iphdr->ip_src.s_addr = table->wan.ip;
                    tcphdr->th_sport = table->wan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->lan.ip, table->lan.port,
                                                table->wan.ip, table->wan.port);
#else
                    set_checksum((char*)iphdr, size);
#endif
                }
            }

            //std::cout << "ip sum :0x" << std::hex << htons(iphdr->ip_sum) << endl;
            //std::cout << "tcp sum:0x" << std::hex << htons(tcphdr->th_sum) << endl;
            //tcphdr->th_sum = 0x0000;

            break;
        }

        case IPPROTO_UDP:
        {
            // L4 size check ------------
            // return ALIAS_TOO_SHROT;
            // ------------------------
            struct udphdr* udphdr = (struct udphdr*)(buf+(iphdr->ip_hl<<2));
            n.lan.port = udphdr->uh_sport;
            n.wan.port = 0x0000;
            //n.wan.port = udphdr->uh_dport;

            ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_LAN2WAN);
            if (ret == ALIAS_TO_ME) {
                // for hairpin routing XXX
                ret = wan2lan_alias(buf, size);
                if (ret != ALIAS_DOALIAS) {
                    return ALIAS_TO_ME;
                }
            }

            //printf("l2w_n\n");
            //print_node(&n);
            n.l2w_key = var_lan(&n);
            table = find_l2w(&n);
            //printf("l2w_table\n");
            //print_node(table);

            if (table == NULL) {
                return ALIAS_NO_TABLE;
            } else {
                //table->atime = time(NULL);
                attach(table);
                // do alias
#ifdef OPT_HOSTID
                if (opt_hostid) {
                    size = set_hostid(iphdr, size);
                    udphdr = (struct udphdr*)(buf+(iphdr->ip_hl<<2));
                }
#endif
                iphdr->ip_src.s_addr = table->wan.ip;
                udphdr->uh_sport = table->wan.port;
#ifdef DIFF_CKSUM
                set_diff_checksum((char*)iphdr, table->lan.ip, table->lan.port,
                                                table->wan.ip, table->wan.port);
#else
                set_checksum((char*)iphdr, size);
#endif
                //std::cout << "ip sum :0x" << std::hex << htons(iphdr->ip_sum) << endl;
                //std::cout << "udp sum:0x" << std::hex << htons(udphdr->uh_sum) << endl;
                //udphdr->uh_sum = 0x0000;
            }
            break;

        }

        case IPPROTO_ICMP:
        {
            struct icmphdr* icmphdr = (struct icmphdr*)(buf+(iphdr->ip_hl<<2));
            switch (icmphdr->icmp_type)
            {
                case ICMP_ECHO:
                case ICMP_TSTAMPREPLY:
                {
                    ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr, ALIAS_FROM_LAN2WAN);
                    if (ret == ALIAS_TO_ME) {
                        return ALIAS_TO_ME;
                    }
                    icmp_bucket.local   = iphdr->ip_src.s_addr; 
                    icmp_bucket.foreign = iphdr->ip_dst.s_addr; 
                    iphdr->ip_src.s_addr = icmp_wan_addr;
                    iphdr->ip_sum = 0x0000;
                    break;
                }

                default:
                {
                    return ALIAS_NOT_SUPPORT;
                }
            }
            break;
        }

        default:
        {
            // L4 size check ------------
            // return ALIAS_TOO_SHROT;
            // ------------------------
            return ALIAS_NOT_SUPPORT;
        }
    }
    return ALIAS_DOALIAS;
}

size_t mk_fn_packet(char* buf, size_t size)
{
    // icmp-type:3(error)-code:4(fragmentation needed)
    /* RFC 1191
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Type = 3    |   Code = 4    |           Checksum            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |           unused = 0          |         Next-Hop MTU          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |      Internet Header + 64 bits of Original Datagram Data      |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
    static uint16_t id = 0;
    id++;

    char tmp[BUFSIZ];
    memset(tmp, 0, sizeof(tmp));

    struct subhdr {
        uint16_t unused;
        uint16_t next_mtu;
    };

    struct exthdr {
        struct ip iphdr;
        char content[8];
    };

    struct ip* iphdr            = (struct ip*)buf;
    struct ip* tmp_iphdr        = (struct ip*)tmp;
    struct icmphdr* tmp_icmphdr = (struct icmphdr*)(tmp+sizeof(struct ip));
    struct subhdr* tmp_subhdr   = (struct subhdr*)(tmp+sizeof(struct ip)+sizeof(struct icmphdr));
    struct exthdr* tmp_exthdr   = (struct exthdr*)(tmp+sizeof(struct ip)+sizeof(struct icmphdr)+sizeof(struct subhdr));


    /*
    0x45, 0x00, 0x00, 0x38, iphdr
    0x00, 0x27, 0x40, 0x00,
    0x40, 0x01, 0xb7, 0x4a,
    0xc0, 0xa8, 0x01, 0x01,
    0xc0, 0xa8, 0x01, 0x02,

    0x03, 0x04, 0x7c, 0xbd, icmphdr
    0x00, 0x00, 0x03, 0x20, subhdr

    0x45, 0x00, 0x05, 0x94, iphdr
    0x95, 0xe9, 0x40, 0x00,
    0x40, 0x01, 0x1b, 0x2b,
    0xc0, 0xa8, 0x01, 0x02,
    0xc0, 0xa8, 0x02, 0x02,

    0x08, 0x00, 0x3b, 0x0f, L4_8byte
    0x3a, 0x0d, 0x00, 0x02
    */

    char sample_ip[]   = { 0x45, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00,
                           0xFF, 0x01, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00 };
    /*
    0x45 0x00 0x00 0x00 // var+hl, tos, length
    0x00 0x00 0x00 0x00 // id, offset
    0xFF 0x01 0x11 0x11 // ttl, proto, checksum
    0x00 0x00 0x00 0x00 // src_ip
    0x00 0x00 0x00 0x00 // dst_ip
    */

    char sample_icmp[]    = { 0x03, 0x04, 0x00, 0x00 };
    char sample_subicmp[] = { 0x00, 0x00, 0x05, 0xDC };
    /*
    0x03 0x04 0x00 0x00 // type, code, checksum,
    0x00 0x00 0x05 0xDC // unused, mtu
    */

    uint32_t local_addr = lan_addr.at(lan_addr.size()-1);
    size_t tmp_size = sizeof(struct ip)
                    + sizeof(struct icmphdr)
                    + sizeof(struct subhdr)
                    + sizeof(struct exthdr);

    memcpy(tmp_iphdr, sample_ip, sizeof(sample_ip));
    tmp_iphdr->ip_len = htons(tmp_size);
    tmp_iphdr->ip_id  = htons(id);
    tmp_iphdr->ip_dst = iphdr->ip_src;
    tmp_iphdr->ip_src.s_addr = htonl(local_addr);

    memcpy(tmp_icmphdr, sample_icmp, sizeof(sample_icmp));
    memcpy(tmp_subhdr, sample_subicmp, sizeof(sample_subicmp));
    tmp_subhdr->next_mtu = htons(wan_if_mtu-50);

    memcpy(&tmp_exthdr->iphdr, iphdr, sizeof(struct ip));
    tmp_exthdr->iphdr.ip_hl = 0x5;
    tmp_exthdr->iphdr.ip_len -= (iphdr->ip_hl - 0x5);

    if (iphdr->ip_p == IPPROTO_TCP) {

        struct tcphdr* tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));
        memcpy(tmp_exthdr->content, tcphdr, 8);
        checksum_ip(&tmp_exthdr->iphdr);
        checksum_transport(&tmp_exthdr->iphdr, sizeof(exthdr));

    } else if (iphdr->ip_p == IPPROTO_UDP) {

        struct udphdr* udphdr = (struct udphdr*)(buf+(iphdr->ip_hl<<2));
        memcpy(tmp_exthdr->content, udphdr, sizeof(tmp_exthdr->content));
        checksum_ip(&tmp_exthdr->iphdr);
        checksum_transport(&tmp_exthdr->iphdr, sizeof(exthdr));
        printf("udp_dport:%d\n", udphdr->uh_dport);
        printf("udp_sport:%d\n", udphdr->uh_sport);

    } else if (iphdr->ip_p == IPPROTO_ICMP) {

        struct icmphdr* icmphdr = (struct icmphdr*)(buf+(iphdr->ip_hl<<2));
        memcpy(tmp_exthdr->content, icmphdr, 8);
        checksum_ip(&tmp_exthdr->iphdr);
        checksum_transport(&tmp_exthdr->iphdr, sizeof(exthdr));

    } else {

        return 0;

    }

    checksum_ip(tmp_iphdr);
    checksum_transport(tmp_iphdr, tmp_size);

    printf("\n");
    memdump(buf, size);
    printf("size:%lu\n", size);

    memset(buf, 0, size);
    memcpy(buf, tmp, tmp_size);

    printf("\n");
    memdump(buf, tmp_size);
    printf("size:%lu\n", tmp_size);

    return tmp_size;
}


#endif
