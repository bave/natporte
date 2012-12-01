#ifndef HOSTID_H
#define HOSTID_H

#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>

extern bool opt_hostid;

union hid_storage {
    struct {
        struct in_addr id;
    } ip4_null;
    struct {
        struct in6_addr id;
    } ip6_null;
    struct {
        struct in_addr id;
        struct in_addr tid;
    } ip4_ip4;
    struct {
        struct in6_addr id;
        struct in6_addr tid;
    } ip6_ip6;
    struct {
        struct in_addr id;
        struct in6_addr tid;
    } ip4_ip6;
    struct {
        struct in6_addr id;
        struct in_addr tid;
    } ip6_ip4;
};
#define hid_standard_id hid_storage.ip4_null.id;

struct ip_hid_sub {
    struct {
        uint8_t id:4;
        uint8_t tid:4;      
    } hidsub_idtype;
    uint8_t  hidsub_sequence;
    uint8_t  hidsub_tidlen;
    uint8_t  hidsub_pointer;
};
#define hid_id  hidsub_idtype.id
#define hid_tid hidsub_idtype.tid

#define HID_ID_RSV  0x00
#define HID_ID_IPV4 0x01
#define HID_ID_IPV6 0x02
#define HID_ID_GRE  0x03
#define HID_ID_FLOW 0x04

#define HID_TID_NULL 0x00
#define HID_TID_IPV4 0x01
#define HID_TID_IPV6 0x02
#define HID_TID_MPLS 0x03

struct ip_hid {
    uint8_t  hid_type;
    uint8_t  hid_len;
    uint8_t  hid_num;
    uint8_t  hid_reserved;
};
#define IPOPT_TYPE_COPY    0x80
#define IPOPT_TYPE_DEBUG   0x40
#define IPOPT_TYPE_CONTROL 0x20
#define IPOPT_TYPE_HID 0x1F
#define HID_TYPE (IPOPT_TYPE_COPY|IPOPT_TYPE_DEBUG|IPOPT_TYPE_HID)

static inline size_t get_optsize(struct ip* iphdr)
{
    return (iphdr->ip_hl<<2) - 20;
}

static inline size_t get_size(struct ip* iphdr)
{
    return ntohs(iphdr->ip_len);
}

static inline size_t set_hostid(struct ip* iphdr, size_t size)
{
    if (iphdr->ip_hl == 0x5) {
        //標準的にくわえるだけ
        //preallocat ebufferに+4するのはEnd of Option Listを付けてアライメントを揃えるため
        char hidbuf[sizeof(struct ip_hid)+sizeof(struct ip_hid_sub)+sizeof(struct in_addr)+4];
        memset(hidbuf, 0, sizeof(hidbuf));

        struct ip_hid* hidhdr = (struct ip_hid*)hidbuf;
        hidhdr->hid_type = HID_TYPE;
        hidhdr->hid_len  = sizeof(hidbuf)-4;
        hidhdr->hid_num  = 1;
        hidhdr->hid_reserved = 0;

        struct ip_hid_sub* hidsubhdr = (struct ip_hid_sub*)(hidbuf+sizeof(struct ip_hid));
        hidsubhdr->hid_id  = HID_ID_IPV4;
        hidsubhdr->hid_tid = HID_TID_NULL;
        hidsubhdr->hidsub_sequence = 0;
        hidsubhdr->hidsub_tidlen   = 0;
        hidsubhdr->hidsub_pointer  = sizeof(struct in_addr);

        struct in_addr* id_ipv4 = (struct in_addr*)(hidbuf+sizeof(struct ip_hid)+sizeof(struct ip_hid_sub));
        id_ipv4->s_addr = iphdr->ip_src.s_addr;
        iphdr->ip_len += htons(sizeof(hidbuf));

        char* buf = (char*)iphdr;
        char* mem_src = buf+(iphdr->ip_hl<<2);
        char* mem_dst = buf+(iphdr->ip_hl<<2) + sizeof(hidbuf);
        memmove(mem_dst, mem_src, size-(iphdr->ip_hl<<2));
        memcpy(mem_src, hidbuf, sizeof(hidbuf));

        iphdr->ip_hl += 4;
        size = size + sizeof(hidbuf);

        /*
        printf("%lu\n", size);
        memdump(iphdr, size);
        */

        return size;

    } else if (iphdr->ip_hl >= 0x6 && iphdr->ip_hl <= 0xF) {
        //既存にあるオプションをみる
        //  - HIDがあれば追記
        //  - HIDがなければ新規
        //  - サイズが足りなければなにもしない
        return size;
    } else {
        //なにもできない
        return size;
    }
}

#endif // HOSTID_H
