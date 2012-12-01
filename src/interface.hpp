#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "utils.hpp"
#include "log.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include <string.h>
#include <netdb.h>

#include <iostream>
#include <vector>

class interface
{
    public:
        interface();
        ~interface();
        void getList(void);

        int listsize();
        int list4size();
        int list6size();

        std::string name(int i);
        std::string name4(int i);
        std::string name6(int i);

        std::string addr(int i);
        std::string addr4(int i);
        std::string addr6(int i);

        std::string mask(int i);
        std::string mask4(int i);
        std::string mask6(int i);

        std::string family(int i);
        std::string family4(int i);
        std::string family6(int i);

        std::string broad(int i);
        std::string broad4(int i);
        std::string broad6(int i);

        struct _interface {
            std::string name;
            std::string family;
            std::string addr;
            std::string mask;
            std::string broad;
        };
        std::vector<struct _interface> list;
        std::vector<struct _interface> list4;
        std::vector<struct _interface> list6;

    private:
        const char* fmtaddr(struct in_addr addr);
        void sa2addr(struct sockaddr* sa, std::string &s);
};

interface::interface()
{
    list.clear();
    list4.clear();
    list6.clear();
    getList();
};

interface::~interface()
{
};

int interface::listsize()
{
    return list.size();
}

int interface::list4size()
{
    return list4.size();
}

int interface::list6size()
{
    return list6.size();
}

std::string interface::broad(int i)
{
    return list[i].broad;
}
std::string interface::broad4(int i)
{
    return list4[i].broad;
}

std::string interface::broad6(int i)
{
    return list6[i].broad;
}


std::string interface::name(int i)
{
    return list[i].name;
}
std::string interface::name4(int i)
{
    return list4[i].name;
}

std::string interface::name6(int i)
{
    return list6[i].name;
}

std::string interface::addr(int i)
{
    return list[i].addr;
}

std::string interface::addr4(int i)
{
    return list4[i].addr;
}

std::string interface::addr6(int i)
{
    return list6[i].addr;
}

std::string interface::mask(int i)
{
    return list[i].mask;
}

std::string interface::mask4(int i)
{
    return list4[i].mask;
}

std::string interface::mask6(int i)
{
    return list6[i].mask;
}

std::string interface::family(int i)
{
    return list[i].family;
}

std::string interface::family4(int i)
{
    return list4[i].family;
}

std::string interface::family6(int i)
{
    return list6[i].family;
}

void interface::getList(void)
{
    struct ifaddrs *ifs;
    struct ifaddrs *ifp;
    struct _interface buffer;

    if (getifaddrs(&ifs) != 0) {
        PERROR("getifaddrs");
        freeifaddrs(ifs);
        exit(-1);
    }

    for (ifp = ifs; ifp; ifp = ifp->ifa_next) {
        int ifaAF = ifp->ifa_addr->sa_family;
        if (ifp->ifa_addr == NULL) {
            continue;
        }
        else if (ifaAF != AF_INET && ifaAF != AF_INET6) {
            continue;
        }
        else if (ifaAF == AF_INET) {
            buffer.family = "AF_INET";
            ifp->ifa_netmask->sa_family = AF_INET;
        }
        else if (ifaAF == AF_INET6) {
            buffer.family = "AF_INET6";
            ifp->ifa_netmask->sa_family = AF_INET6;
        }

        // ifname
        buffer.name = ifp->ifa_name;

        // address
        sa2addr(ifp->ifa_addr, buffer.addr);

        // network mask
        sa2addr(ifp->ifa_netmask, buffer.mask);

        // boradcast
        struct sockaddr_storage ss;
        memset(&ss, 0, sizeof(struct sockaddr_storage));
        if (buffer.family.compare("AF_INET") == 0) {
            struct sockaddr_in* sain = (struct sockaddr_in*)&ss;
            sain->sin_family = AF_INET;
            sain->sin_addr.s_addr = ((struct sockaddr_in*)(ifp->ifa_addr))->sin_addr.s_addr |
                                    ~(((struct sockaddr_in*)ifp->ifa_netmask)->sin_addr.s_addr);
            sa2addr((struct sockaddr*)&ss, buffer.broad);
        } else if (buffer.family.compare("AF_INET6") == 0) {
            struct sockaddr_in6* sa6 = (struct sockaddr_in6*)&ss;
            sa6->sin6_family = AF_INET6;
            sa6->sin6_addr.__u6_addr.__u6_addr32[0] = 
                ((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[0] |
                ~(((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[0]);
            sa6->sin6_addr.__u6_addr.__u6_addr32[1] = 
                ((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[1] |
                ~(((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[1]);
            sa6->sin6_addr.__u6_addr.__u6_addr32[2] = 
                ((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[2] |
                ~(((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[2]);
            sa6->sin6_addr.__u6_addr.__u6_addr32[3] = 
                ((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[3] |
                ~(((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[3]);
            sa2addr((struct sockaddr*)&ss, buffer.broad);
            /*
            printf("[0]:%x\n", sa6->sin6_addr.__u6_addr.__u6_addr32[0]);
            printf("[1]:%x\n", sa6->sin6_addr.__u6_addr.__u6_addr32[1]);
            printf("[2]:%x\n", sa6->sin6_addr.__u6_addr.__u6_addr32[2]);
            printf("[3]:%x\n", sa6->sin6_addr.__u6_addr.__u6_addr32[3]);
            printf("mask[0]:%x\n", (((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[0]));
            printf("mask[1]:%x\n", (((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[1]));
            printf("mask[2]:%x\n", (((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[2]));
            printf("mask[3]:%x\n", (((struct sockaddr_in6*)ifp->ifa_addr)->sin6_addr.__u6_addr.__u6_addr32[3]));
            printf("mask[0]:%x\n", (((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[0]));
            printf("mask[1]:%x\n", (((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[1]));
            printf("mask[2]:%x\n", (((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[2]));
            printf("mask[3]:%x\n", (((struct sockaddr_in6*)ifp->ifa_netmask)->sin6_addr.__u6_addr.__u6_addr32[3]));
            */
        } else {
            buffer.broad = "";
        }

        /*
        std::cout << buffer.name       << std::endl;
        std::cout << buffer.family     << std::endl;
        std::cout << buffer.addr       << std::endl;
        std::cout << buffer.mask       << std::endl;
        */

        if (buffer.family.compare("AF_INET")==0)  list4.push_back(buffer);
        if (buffer.family.compare("AF_INET6")==0) list6.push_back(buffer);
        list.push_back(buffer);
    }

    freeifaddrs(ifs);
    return;
}

const char* interface::fmtaddr(struct in_addr addr)
{
    static char buf[sizeof("255.255.255.255")];
    addr.s_addr = ntohl(addr.s_addr);

    sprintf(buf, "%d.%d.%d.%d",
            (addr.s_addr >> 24) & 0xFF,
            (addr.s_addr >> 16) & 0xFF,
            (addr.s_addr >>  8) & 0xFF,
            (addr.s_addr >>  0) & 0xFF);
    return buf;
}

void interface::sa2addr(struct sockaddr* sa, std::string &s)
{
    int err;
    char host[NI_MAXHOST];
    memset(host, 0, NI_MAXHOST);

    if (sa == NULL) {
        return;
    }

    switch (sa->sa_family)
    {
        case AF_INET:
            struct sockaddr_in* sain;
            sain = (struct sockaddr_in*)sa;
            memcpy(host, fmtaddr(sain->sin_addr), sizeof("255.255.255.255"));
            break;

        case AF_INET6:
            err = getnameinfo(sa, sizeof(struct sockaddr_in6), 
                              host, sizeof(host), NULL, 0,  NI_NUMERICHOST);
            if (err > 0) {
                PERROR("getnameinfo");
                return;
            }
            break;

        default:
            return;
    }
    s = host;
    return;
}
#endif // INTERFACE_HPP
