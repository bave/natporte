
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <iostream>
#include <set>

#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.hpp"
#include "interface.hpp"

#include "hostid.h"
#include "divert.h"
#include "addrpool.h"
#include "alias.h"
#include "ring.h"
#include "structure.h"

#include "utils.hpp"

// global declaration
extern pthread_t pth_id_divert;
extern pthread_t pth_id_addrpool;
extern const char* pname;
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

extern std::vector<uint32_t> lan_addr;
extern std::vector<uint32_t> wan_addr;
extern std::vector<uint32_t> pool_addr;


extern int kq;
extern int timeout_udp;
extern int timeout_tcp;

struct ring_handler h_send;
struct ring_handler h_recv;

char* wan_if = NULL;
char* lan_if = NULL;

size_t wan_if_mtu;
size_t lan_if_mtu;


uint32_t icmp_wan_addr;
class interface* iflist;
class log* l;

// prototype
void usage();

bool opt_nagete_df;
bool opt_thread;

bool verbose;
#ifdef DEBUG
#endif

bool opt_hostid;



int main(int argc, char** argv)
{
    int ret;

    pname = argv[0];
    bool opt_reset;
    bool opt_node_table_dump;

    int divert_port;
    int divert_fd;

    int addrpool_port;
    int addrpool_fd;

    uint64_t wan_if_num = 0;
    uint64_t lan_if_num = 0;

    timeout_udp = 0;
    timeout_tcp = 0;

    int rb_size = 50;

    struct kevent kev;

    wan_if = NULL;
    lan_if = NULL;
    wan_if_mtu = 0;
    lan_if_mtu = 0;

    icmp_wan_addr = 0;
    opt_reset = false;
    opt_node_table_dump = false;
    divert_port = 8668;
    addrpool_port = 8668;

    verbose = false;
    opt_hostid = false;
    opt_thread = false; 
    opt_nagete_df = false;

    int i;
    std::set<std::string> has_interface;

    int ch;
    while ((ch = getopt(argc, argv, "a:b:d:hil:nrtvw:")) != -1) {
        switch (ch) {
            case 'a':
                addrpool_port = atoi(optarg);
                if (addrpool_port == 0) {
                    return -1; 
                }
                break;
            case 'b':
                rb_size = atoi(optarg);
                if (rb_size < 1) {
                    return -1; 
                }
                break;
            case 'd':
                divert_port = atoi(optarg);
                if (divert_port == 0) {
                    return -1; 
                }
                break;
            case 'n':
                opt_nagete_df = true;
                break;
            case 'w':
                wan_if = optarg;
                wan_if_num = check_wan_if(wan_if);
                //printf("wlan_IF:%llu\n", wan_if_num);
                break;
            case 'l':
                lan_if = optarg;
                lan_if_num = check_lan_if(lan_if);
                //printf("wlan_IF:%llu\n", lan_if_num);
                break;
            case 'h':
#ifdef OPT_HOSTID
                opt_hostid = true;
#endif
                break;
            case 'r':
                opt_reset = true;
                break;
            case 't':
                opt_thread = true;
                break;
#ifdef DEBUG
            case 'i':
                opt_node_table_dump = true;
                break;
            case 'v':
                verbose = true;
                break;
#endif
            default:
                usage();
                return -1;
        }
    }
    argc -= optind;
    argv += optind;

    l = new log::log((char *)pname, LOG_LOCAL0, LOG_DEBUG);

    iflist = new interface::interface();
    for (i=0; i<iflist->list4size(); i++) {
        has_interface.insert(iflist->name4(i));
    }
    /*
    std::set<std::string>::iterator it = has_interface.begin();
    while( it != has_interface.end() ) { cout << *it << endl; ++it; }
    */


    if (wan_if == NULL) {
        usage();
        return -1;
    } else if ( has_interface.find(wan_if) == has_interface.end()) {
        std::cout << wan_if
                  << " was not found!!"
                  << std::endl;
        usage();
        return -1;
    }


    if (lan_if == NULL) {
        usage();
        return -1;
    } else if ( has_interface.find(lan_if) == has_interface.end()) {
        std::cout << lan_if 
                  << " was not found!!"
                  << std::endl;
        usage();
        return -1;
    }


    if (wan_if_num == lan_if_num) {
        std::cout << "cant set same interface between WanIF and LanIF" << std::endl;
        usage();
        return -1;
    }

    if(!verbose && !opt_node_table_dump) {
        if(daemon(0, 1) != 0) {
            std::cout << pname << " fail to run as a daemon mode." << std::endl;
            usage();
            return -1;
        }
    }


    wan_if_mtu = get_mut(wan_if);
    lan_if_mtu = get_mut(lan_if);

    for (i=0; i<iflist->list4size(); i++) {

        if (iflist->name4(i).compare(lan_if) == 0) {

            std::string addr  = iflist->addr4(i);
            std::string broad = iflist->broad4(i);
            bool uniq = true;
            uint32_t int_broad = htonl(string2addr(broad));
            std::vector<uint32_t>::iterator it = lan_addr.begin();
            while (it != lan_addr.end()) {
                if (*it == int_broad) {
                    uniq = false;
                    break;
                } 
                it++;
            }
            if (uniq) {
                lan_addr.push_back(int_broad);
            }
            lan_addr.push_back(htonl(string2addr(addr)));

        } else if (iflist->name4(i).compare(wan_if) == 0) {

            std::string addr  = iflist->addr4(i);
            std::string broad = iflist->broad4(i);

            bool uniq = true;
            uint32_t int_broad = htonl(string2addr(broad));
            std::vector<uint32_t>::iterator it;
            it = wan_addr.begin();
            while (it != wan_addr.end()) {
                if (*it == int_broad) {
                    uniq = false;
                    break;
                } 
                it++;
            }
            if (uniq) {
                wan_addr.push_back(int_broad);
            }
            wan_addr.push_back(htonl(string2addr(addr)));
            pool_addr.push_back(htonl(string2addr(addr)));
            if (icmp_wan_addr == 0) {
                icmp_wan_addr = string2addr(addr);
            }
        }
        
        /*
        std::string name   = iflist->name4(i);
        std::string addr   = iflist->addr4(i);
        std::string mask   = iflist->mask4(i);
        std::string family = iflist->family4(i);
        */
    }

#ifdef DEBUG
    if (verbose) {
        std::cout << "WAN's addrs..." << std::endl;
        std::vector<uint32_t>::iterator it;
        it = wan_addr.begin();
        while (it != wan_addr.end()) {
            std::cout << "	" << addr2string(ntohl(*it)) << std::endl;
            it++;
        }
    }
    if (verbose) std::cout << std::endl;
    if (verbose) {
        std::cout << "LAN's addrs..." << std::endl;
        std::vector<uint32_t>::iterator it;
        it = lan_addr.begin();
        while(it != lan_addr.end()) {
            std::cout << "	"<< addr2string(ntohl(*it)) << std::endl;
            it++;
        }
    }
    if (verbose) std::cout << std::endl;
#endif

    /*
    std::cout << "lan_addr" << std::endl;
    for (i=0; i<(int32_t)lan_addr.size(); i++) {
        cout << "0x" << std::hex <<lan_addr[i] << std::endl;
    }
    std::cout << "wan_addr" << std::endl;
    for (i=0; i<(int32_t)wan_addr.size(); i++) {
        cout << "0x" << std::hex <<wan_addr[i] << std::endl;
    }
    */

    s_init();

    ring_init(&h_send, rb_size);
    ring_init(&h_recv, rb_size*100);

    addrpool_fd = open_addrpool(addrpool_port);
    divert_fd = open_divert(divert_port);

    if (pthread_create_addrpool(divert_fd, addrpool_fd, opt_reset)) return -1;
    if (pthread_create_divert(divert_fd, lan_if_num, wan_if_num)) return -1;

    while (1) {
        if ( timeout_udp != 0 && 
             timeout_tcp != 0 )
        {
            break;
        }
        usleep(1000);
    }

#ifdef DEBUG
    if (verbose) {
        printf("tcp_timeout=%d[ms]\n", timeout_tcp);
        printf("udp_timeout=%d[ms]\n", timeout_udp);
        printf("\n");
    }
#endif
    
    kq = kqueue();
    if (kq == -1) {
        PERROR("kqueue");
        exit(-1);
    }

#ifdef DEBUG
    if (opt_node_table_dump) {
        int table_dump_interval = 1000;
#ifdef __MACH__
        // timer unit is micro second.
        EV_SET(&kev, 1, EVFILT_TIMER, EV_ADD, 0x02, (table_dump_interval*1000), NULL);
#else
        // timer unit is milli second.
        EV_SET(&kev, 1, EVFILT_TIMER, EV_ADD, 0x02, (table_dump_interval), NULL);
#endif
        ret = kevent(kq, &kev, 1, NULL, 0, NULL);
        if (ret == -1) {
            PERROR("kevent");
            exit(-1);
        }

    }
#endif

    for (;;) {
        ret = kevent(kq, NULL, 0, &kev, 1, NULL);
        if (ret == -1) {
            PERROR("kevent");
            exit(-1);
        } else if (kev.ident == 1) {
#ifdef DEBUG
            printf("----------------------------------------\n");
            B_RLOCK;
            //dump_tree();
            int count = count_node();
            printf("count=%d\n", count);
            B_UNLOCK;
            printf("----------------------------------------\n");
#endif
        } else {

            struct node n;
            struct node* table;
            struct ring_buffer rb;

            n.protocol = get_protocol_from_var_lan(kev.ident);
            n.lan.ip   = get_ip_from_var_lan(kev.ident);
            n.wan.ip   = 0x000000;
            n.lan.port = get_port_from_var_lan(kev.ident);
            n.wan.port = 0x0000;
            n.l2w_key  = var_lan(&n);
            n.w2l_key  = 0;

            table = find_l2w(&n);
            if (table == NULL) {
                char buf[BUFSIZ];
                memset(buf, 0, BUFSIZ);
                l->output("cannot found kve");
                continue;
            }

            time_t now = time(NULL);
            time_t elapse = now - table->atime;
            bool timeout = false;


            if (table->protocol == IPPROTO_TCP) {
                if (elapse > (timeout_tcp/1000)) {
                    del_timer_event_tcp(kev.ident);
                    timeout = true;
                } else {
                    timeout = false;
                }
            } else if (table->protocol == IPPROTO_UDP) {
                if (elapse > (timeout_udp/1000)) {
                    del_timer_event_udp(kev.ident);
                    timeout = true;
                } else {
                    timeout = false;
                }
            } else {
                continue;
            }

#ifdef WAIT_TIME_DELETE
            if ((timeout == false) && (table->timer_flags!=S_TIMER_LOOP)) {
                del_timer_event_tcp(kev.ident);
                timeout = true;
            }
#endif

            if (timeout) {
                rb.buffer = (char*)table;
                rb.size = sizeof(struct node);
                rb.opaque = AP_TIMEOUT|AP_ADDRPOOL;
                for (;;) {
                    ret = ring_s_push(&h_send, &rb);
                    if (ret == RING_OK) {
                        break;
                    }
                }
            } else {
                if (n.protocol == IPPROTO_TCP) {
                    ;
                } else if (n.protocol == IPPROTO_UDP) {
                    ;
                } else {
                    continue;
                }
            }
        }
    }
    return 0;
}

void usage()
{
    std::cout << "usage:\n"
              << ""
              << pname
              << " -w ifname -l ifname                                       \n"
              << "    w: Wan interface name   [must]                         \n"
              << "    l: Lan interface name   [must]                         \n"
              << "    a: Addrpool port number [default:(tcp)8668]            \n"
              << "    d: Divert port number   [default:(divert)8668]         \n"
              << "    r: send back to Reset when tcp session already limited \n"
              << "    b: ring Buffer size using by pool server communitacion \n"
              << "    n: force Negated the DF flag effect                    \n"
              << "    t: running nat Thread number [default:1](experiment)   \n"
#ifdef OPT_HOSTID
              << "    h: adding Hostid (src addr within ip option)           \n"
#endif
#ifdef DEBUG
              << "    v: Verbose mode          (run as a front-end mode)     \n"
              << "    i: Interval table dump   (run as a front-end mode)     \n"
#endif
              << std::endl;
    return;
}


