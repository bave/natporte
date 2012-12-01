#ifndef ADDRPOOL_H
#define ADDRPOOL_H

#include "utils.hpp"
#include "log.hpp"

#include "ring.h"
#include "alias.h"
#include "structure.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <vector>

/*
#include <json/json.h>
#define json_object_free(obj) json_object_put(obj)
*/

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using namespace boost::property_tree;

//#include <boost/foreach.hpp>
//#include <boost/optional.hpp>

pthread_t pth_id_addrpool;

#ifdef NATIVE_LOCK
extern int divert_recv_lock;
extern int divert_send_lock;
#else
extern pthread_mutex_t divert_recv_lock;
extern pthread_mutex_t divert_send_lock;
#endif

extern struct ring_handler h_send;
extern struct ring_handler h_recv;
extern std::vector<uint32_t> pool_addr;

extern char* wan_if;
extern char* lan_if;
extern size_t wan_if_mtu;
extern size_t lan_if_mtu;
extern bool opt_nagete_df;

#define AP_LOW      0x00000000FFFFFFFF
#define AP_HIGH     0xFFFFFFFF00000000

#define AP_ZERO     0x0000000000000000
// :
#define AP_SEND_REQ 0x0000000000000001
#define AP_RECV_REQ 0x0000000000000002
#define AP_SEND_REL 0x0000000000000004
#define AP_RECV_REL 0x0000000000000008
#define AP_REL_L2W  0x0000000000000010
#define AP_REL_W2L  0x0000000000000020
#define AP_TIMEOUT  0x0000000000000040
#define AP_SILENT   0x0000000000000080
// :
#define AP_ADDRPOOL 0x0000000100000000
#define AP_DIVERT   0x0000000200000000
// :
#define AP_FULLFILL 0xFFFFFFFFFFFFFFFF
struct pth_addrpool_args {
    int addrpool_fd;
    int divert_fd;
    bool opt_reset;
};

void* addrpool_receiver(void* args)
{
    pthread_detach(pthread_self());
    struct pth_addrpool_args* pla = (struct pth_addrpool_args*)args;
    int addrpool_fd = pla->addrpool_fd;
    int divert_fd = pla->divert_fd;
    bool opt_reset = pla->opt_reset;
    free(args);

    ssize_t ssize;
    char  mesg[BUFSIZ];
    ptree pt;

    int i;
    int vend;

    int ret;
    char* buf;
    size_t size;
    uint64_t flags;

    std::string seof = "\0";
    std::string string_buffer;
    std::stringstream ss;
    std::vector<std::string> string_vector;

    std::string type;
    std::string str_wan_ip;
    std::string str_lan_ip;
    uint32_t wan_ip;
    uint32_t lan_ip;
    uint16_t wan_port;
    uint16_t lan_port;
    uint8_t protocol;

    struct node n;
    struct node* table;

    struct sockaddr_in sin;
    socklen_t sin_len = sizeof(sin);
    memset(&sin, 0, sin_len);
    sin.sin_len = sin_len;
    sin.sin_family = AF_INET; 
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = 0xFFFE;

#ifdef OPT_HOSTID
    size_t prev_opt_size;
    size_t next_opt_size;
#endif

    string_buffer.clear();

    struct ring_buffer rb;

    for (;;) {

        ssize = recv(addrpool_fd, mesg, sizeof(mesg), 0);
        if (ssize == 0) {
            exit(0);
        } else if (ssize < 0) {
            PERROR("recv");
            exit(-1);
        }

        string_buffer.append(mesg, ssize);

        split(string_buffer, "\n", string_vector);
        vend = string_vector.size();
        for (i=0; i<vend-1; i++) {

            ss.clear();
            try {
                ss.write(string_vector[i].c_str(), string_vector[i].size());
                //ss.write(mesg, ssize);
                read_json(ss, pt);
            } catch (...) {
                l->output("parse error");
                exit(-1);
            }

            try {
                type   = pt.get<std::string>("type");
            } catch(...) {
                l->output("JSON parse error");
                exit(-1);
            }

            if (type.compare("request_ack") == 0) {

                try {
                    wan_ip = pt.get<uint32_t>("wan_ip");
                    lan_ip = pt.get<uint32_t>("lan_ip");
                    wan_port = pt.get<uint16_t>("wan_port");
                    lan_port = pt.get<uint16_t>("lan_port");
                    protocol = (uint8_t)pt.get<unsigned int>("protocol");
                } catch(...) {
                    l->output("JSON parse error");
                    exit(-1);
                }

                /*
                struct in_addr wi;
                struct in_addr li;
                wi.s_addr = htonl(wan_ip);
                li.s_addr = htonl(lan_ip);
                std::cout << type << std::endl;
                std::cout << inet_ntoa(wi) << std::endl;
                std::cout << inet_ntoa(li) << std::endl;
                std::cout << htons(wan_port) << std::endl;
                std::cout << htons(lan_port) << std::endl;
                std::cout << (uint16_t)protocol << std::endl;
                */


                ret = ring_pop_condblock(&h_recv, &buf, &size, &flags);
                if (ret != 0 || buf == NULL) continue;
                rb.buffer = buf;
                rb.size = size;
                rb.opaque = flags;


                table = (struct node*)malloc(sizeof(struct node));

                /*
                n.ctime    = time(NULL);
                n.atime    = n.ctime;
                n.protocol = (uint8_t)protocol;
                n.lan.ip   = htonl(lan_ip); 
                n.lan.port = htons(lan_port);
                n.wan.ip   = htonl(wan_ip); 
                n.wan.port = htons(wan_port);
                if (protocol == IPPROTO_TCP) {
                    n.flags = S_TF_SYN_SENT;
                } else if (protocol == IPPROTO_UDP) {
                    n.flags = 0;
                } else {
                    n.flags = 0;
                }
                */

                table->ctime    = time(NULL);
                table->atime    = table->ctime;
                table->protocol = (uint8_t)protocol;
                table->lan.ip   = htonl(lan_ip); 
                table->lan.port = htons(lan_port);
                table->wan.ip   = htonl(wan_ip); 
                table->wan.port = htons(wan_port);
                if (protocol == IPPROTO_TCP) {
                    table->flags = S_TF_SYN_SENT;
                    table->timer_flags = S_TIMER_LOOP;
                } else if (protocol == IPPROTO_UDP) {
                    table->flags = 0;
                    table->timer_flags = S_TIMER_LOOP;
                } else {
                    table->flags = 0;
                    table->timer_flags = S_TIMER_LOOP;
                }
                table->l2w_key = var_lan(table);
                table->w2l_key = var_wan(table);


                /*
                printf("compare protocol:%d\n", table->protocol);
                std::cout << "insert ctime   :" << table->ctime << endl;
                std::cout << "insert atime   :" << table->atime << endl;
                std::cout << "insert lan ip  :" << table->lan.ip << endl;
                std::cout << "insert lan port:" << table->lan.port << endl;
                std::cout << "insert wan ip  :" << table->wan.ip << endl;
                std::cout << "insert wan port:" << table->wan.port << endl;
                print_node(table);
                */


#ifdef DEBUGN
                B_WLOCK;
#endif
                if (s_insert(table) != NULL) {
#ifdef DEBUGN
                    B_UNLOCK;
#endif
                    print_node(table);
                    free(table);
                    free(buf);
                    continue;
                } else {
#ifdef DEBUGN
                    B_UNLOCK;
#endif
                    if (protocol == IPPROTO_TCP) {
                        add_timer_event_tcp(table->l2w_key);
                    } else if (protocol == IPPROTO_UDP) {
                        // for hairpin routing XXX
                        add_timer_event_udp(table->l2w_key);
                        struct ip* iphdr = (struct ip*)buf;
                        ret = chk_selfaddr(iphdr->ip_src.s_addr, iphdr->ip_dst.s_addr ,ALIAS_FROM_LAN2WAN);
                        if (ret == ALIAS_TO_ME) {
                            ret = wan2lan_alias(buf, size);
                            if (ret != ALIAS_DOALIAS) {
                                free(buf);
                                continue;
                            }
                        }
                    }
                }

#ifdef OPT_HOSTID
                if (opt_hostid) {
                    prev_opt_size = get_optsize((struct ip*)buf);
                }
#endif

                ret = lan2wan_alias(buf, size);
                if (ret != ALIAS_DOALIAS) {
                    free(buf);
                    continue;
                }

#ifdef OPT_HOSTID
                if (opt_hostid) {
                    next_opt_size= get_optsize((struct ip*)buf);
                    size += next_opt_size - prev_opt_size;
                }
#endif

                /*
                printf("%s\n", packet_info((struct ip*)buf));
                printf("%lu\n", size);
                memdump(buf, size);
                */

                if (opt_nagete_df) {
                    if (htons(((struct ip*)buf)->ip_off)&IP_DF && size>wan_if_mtu) {
                        struct ip* iphdr = (struct ip*)buf;
                        iphdr->ip_off = iphdr->ip_off & htons(0xbfff);
                    }
                }

                sin.sin_len = sin_len;
                sin.sin_family = AF_INET; 
                sin.sin_addr.s_addr = INADDR_ANY;
                sin.sin_port = 0xFFFE;
                S_DIV_SOCK_LOCK;
                if(sendto(divert_fd, buf, size, 0, (struct sockaddr*)&sin, sin_len) < 0) {
                    if (errno == EHOSTDOWN) {
                    } else if (errno == EHOSTUNREACH) {
                    } else {
                        PERROR("sendto");
                        exit(-1);
                    }
                }
                S_DIV_SOCK_UNLOCK;
                free(buf);

            } else if (type.compare("request_discard") == 0) {

                /*
                try {
                    wan_ip = pt.get<uint32_t>("wan_ip");
                    lan_ip = pt.get<uint32_t>("lan_ip");
                    wan_port = pt.get<uint16_t>("wan_port");
                    lan_port = pt.get<uint16_t>("lan_port");
                    protocol = (uint8_t)pt.get<unsigned int>("protocol");
                } catch(...) {
                    l->output("JSON parse error");
                    exit(-1);
                }
                */

                ret = ring_pop_condblock(&h_recv, &buf, &size, &flags);
                if (ret != 0 || buf == NULL) continue;

                if (opt_reset) {
                    struct ip* iphdr = (struct ip*)buf;
                    if (iphdr->ip_p == IPPROTO_TCP) {
                        tcp_reset_alias(buf, size);
                        sin.sin_len = sin_len;
                        sin.sin_family = AF_INET; 
                        sin.sin_addr.s_addr = INADDR_ANY;
                        sin.sin_port = 0xFFFE;
                        S_DIV_SOCK_LOCK;
                        if(sendto(divert_fd, buf, size, 0, (struct sockaddr*)&sin, sin_len) < 0) {
                            if (errno == EHOSTDOWN) {
                            } else if (errno == EHOSTUNREACH) {
                            } else {
                                PERROR("sendto");
                                exit(-1);
                            }
                        }
                        S_DIV_SOCK_UNLOCK;
                        free(buf);
                    } else {
                        free(buf);
                    }
                } else {
                    free(buf);
                }

            } else if (type.compare("release_ack") == 0) {

                try {
                    wan_ip = pt.get<uint32_t>("wan_ip");
                    lan_ip = pt.get<uint32_t>("lan_ip");
                    wan_port = pt.get<uint16_t>("wan_port");
                    lan_port = pt.get<uint16_t>("lan_port");
                    protocol = (uint8_t)pt.get<unsigned int>("protocol");
                } catch(...) {
                    l->output("JSON parse error");
                    exit(-1);
                }

                ret = ring_pop_condblock(&h_recv, &buf, &size, &flags);
                if (ret != 0 || buf == NULL) continue;

                if (flags == AP_TIMEOUT) {

                    // is release_ack information match with buffer informaton of node... 
                    // timeout and udp table closing
                    table = s_delete_for_point((struct node*)buf);
                    if (table == NULL) {
                        table = (struct node*)buf;
                        char error_buf[BUFSIZ];
                        struct in_addr wan_ip_tmp;
                        wan_ip_tmp.s_addr = table->wan.ip;
                        struct in_addr lan_ip_tmp;
                        lan_ip_tmp.s_addr = table->lan.ip;
                        memset(error_buf, 0, BUFSIZ);
                        sprintf(error_buf, "(%s:%d)nat table delete error:[%d][lan %s:%d][wan %s:%d]\n",
                                __FILE__, __LINE__,
                                table->protocol,
                                inet_ntoa(lan_ip_tmp),
                                htons(table->lan.port),
                                inet_ntoa(wan_ip_tmp),
                                htons(table->wan.port));
                        l->output(error_buf);
                        exit(-1);
                        //free(table);
                    }
                    free(table);

                } else {

                    // tcp state closing
                    n.flags    = 0;
                    n.ctime    = time(NULL);
                    n.atime    = n.ctime;
                    n.protocol = (uint8_t)protocol;
                    n.lan.ip   = htonl(lan_ip); 
                    n.lan.port = htons(lan_port);
                    n.wan.ip   = htonl(wan_ip); 
                    n.wan.port = htons(wan_port);
                    n.l2w_key = var_lan(&n);
                    n.w2l_key = var_wan(&n);

                    if (flags == AP_REL_L2W) {
                        ret = lan2wan_alias(buf, size);
                        if (ret != ALIAS_DOALIAS) {
                            free(buf);
                            continue;
                        }
                    } else if (flags == AP_REL_W2L) {
                        ret = wan2lan_alias(buf, size);
                        if (ret != ALIAS_DOALIAS) {
                            free(buf);
                            continue;
                        }
                    }

                    if (n.protocol == IPPROTO_TCP) {
                        //printf("delete for value:%d\n", __LINE__);
                        table = s_delete_for_value(&n);
                        if (table == NULL) {
                            char error_buf[BUFSIZ];
                            struct in_addr wan_ip_tmp;
                            wan_ip_tmp.s_addr = n.wan.ip;
                            struct in_addr lan_ip_tmp;
                            lan_ip_tmp.s_addr = n.lan.ip;
                            memset(error_buf, 0, BUFSIZ);
                            sprintf(error_buf, "(%s:%d)nat table delete error:[%d][lan %s:%d][wan %s:%d]\n",
                                    __FILE__, __LINE__,
                                    n.protocol,
                                    inet_ntoa(lan_ip_tmp),
                                    htons(n.lan.port),
                                    inet_ntoa(wan_ip_tmp),
                                    htons(n.wan.port));
                            l->output(error_buf);
                            exit(-1);
                        } else {
                            del_timer_event_tcp(table->l2w_key);
                            free(table);
                        }
                    } else if (n.protocol == IPPROTO_UDP) {
                        //printf("delete for value:%d\n", __LINE__);
                        table = s_delete_for_value(&n);
                        if (table == NULL) {
                            char error_buf[BUFSIZ];
                            struct in_addr wan_ip_tmp;
                            wan_ip_tmp.s_addr = n.wan.ip;
                            struct in_addr lan_ip_tmp;
                            lan_ip_tmp.s_addr = n.lan.ip;
                            memset(error_buf, 0, BUFSIZ);
                            sprintf(error_buf, "(%s:%d)nat table delete error:[%d][lan %s:%d][wan %s:%d]\n",
                                    __FILE__, __LINE__,
                                    n.protocol,
                                    inet_ntoa(lan_ip_tmp),
                                    htons(n.lan.port),
                                    inet_ntoa(wan_ip_tmp),
                                    htons(n.wan.port));
                            l->output(error_buf);
                            exit(-1);
                        } else {
                            del_timer_event_udp(table->l2w_key);
                            free(table);
                        }
                    } else {
                        ;
                    }
        
                    sin.sin_len = sin_len;
                    sin.sin_family = AF_INET; 
                    sin.sin_addr.s_addr = INADDR_ANY;
                    sin.sin_port = 0xFFFE;
                    S_DIV_SOCK_LOCK;
                    if(sendto(divert_fd, buf, size, 0, (struct sockaddr*)&sin, sin_len) < 0) {
                        if (errno == EHOSTDOWN) {
                        } else if (errno == EHOSTUNREACH) {
                        } else {
                            PERROR("sendto");
                            exit(-1);
                        }
                    }
                    S_DIV_SOCK_UNLOCK;
                    free(buf);
                }

            } else if (type.compare("release_discard") == 0) {

                /*
                try {
                    wan_ip = pt.get<uint32_t>("wan_ip");
                    lan_ip = pt.get<uint32_t>("lan_ip");
                    wan_port = pt.get<uint16_t>("wan_port");
                    lan_port = pt.get<uint16_t>("lan_port");
                    protocol = (uint8_t)pt.get<unsigned int>("protocol");
                } catch(...) {
                    l->output("JSON parse error");
                    exit(-1);
                }
                */

                ret = ring_pop_condblock(&h_recv, &buf, &size, &flags);
                if (ret != 0 || buf == NULL) continue;
                free(buf);

            } else if (type.compare("initialize") == 0) {

                // timeout_{tcp,udp} are declared in global
                timeout_tcp = pt.get<int>("timeout_tcp");
                timeout_udp = pt.get<int>("timeout_udp");

                /*
                ptree& results = pt.get_child("staticNAT");
                BOOST_FOREACH(const ptree::value_type& result, results)
                {
                    //std::string s_ip   = result.second.get<std::string>("forwardIP");
                    //std::string s_port = result.second.get<std::string>("forwardPport");
                    //std::string stable = boost::lexical_cast<std::string>(result.second.data());
                    //std::cout << stable << std::endl;
                }
                */

            } else {
                ;
            }
        }

        if (string_vector.back().size() == 0) {
            string_buffer.clear();
        } else {
            string_buffer = string_vector.back();
        }

    }
    return NULL;
}

// あとでリングバッファのパケットコンテナをmallocしないように書き換える！！
void* addrpool_sender(void* args)
{
    pthread_detach(pthread_self());
    struct pth_addrpool_args* pla = (struct pth_addrpool_args*)args;
    int addrpool_fd = pla->addrpool_fd;
    //int divert_fd = pla->divert_fd;
    free(args);

    int ret;
    char mesg[BUFSIZ];
    memset(mesg, 0, BUFSIZ);

    /*
    struct sockaddr_in sin;
    socklen_t sin_len = sizeof(sin);
    memset(&sin, 0, sin_len);
    sin.sin_port = 0xFFFE;
    sin.sin_family = AF_INET; 
    */

    char* buf;
    size_t size;
    uint64_t flags;
    for (;;) {
        ret = ring_pop_condblock(&h_send, &buf, &size, &flags);
        if (ret != 0 || buf == NULL) {
            continue;
        }

        struct ring_buffer rb;
        rb.buffer = buf;
        rb.size   = size;
        rb.opaque = flags;


        // processing attribute
        switch (flags & AP_LOW)
        {
            case AP_SEND_REQ:
            {
                struct node  n;
                memset(&n, 0, sizeof(n));
                mk_node(rb.buffer, size, &n);
                time_t ctime = time(NULL);

                /*
                // debug
                printf("{ \"type\" :\"request\"," 
                       "  \"protocol\": %d," 
                       "  \"ctime\"   : %ld," 
                       "  \"ip_src\"  : %u," 
                       "  \"ip_dst\"  : %u," 
                       "  \"port_src\": %d," 
                       "  \"port_dst\": %d}",
                       n.protocol,
                       ctime,
                       htonl(n.lan.ip),
                       htonl(n.wan.ip),
                       htons(n.lan.port),
                       htons(n.wan.port));
                */

                sprintf(mesg , "{ \"type\" :\"request\"," 
                               "  \"protocol\": %d," 
                               "  \"ctime\"   : %ld," 
                               "  \"ip_src\"  : %u," 
                               "  \"ip_dst\"  : %u," 
                               "  \"port_src\": %d," 
                               "  \"port_dst\": %d}\n",
                               n.protocol,
                               ctime,
                               htonl(n.lan.ip),
                               htonl(n.wan.ip),
                               htons(n.lan.port),
                               htons(n.wan.port));

                rb.opaque = 0;
                for (;;) {
                    ret = ring_s_push(&h_recv, &rb);
                    if (ret == RING_OK) {
                        break;
                    }
                }
                break;
            }

            case AP_SEND_REL:
            {
                struct node  n;
                struct node* table;
                memset(&n, 0, sizeof(n));
                mk_node(rb.buffer, size, &n);
                n.l2w_key = var_lan(&n);
                n.w2l_key = var_wan(&n);
                table = find_l2w(&n);
                if (table == NULL) {
                    table = find_w2l(&n);
                    if (table == NULL) {
                        free(rb.buffer);
                        char error_buf[BUFSIZ];
                        struct in_addr wan_ip_tmp;
                        wan_ip_tmp.s_addr = n.wan.ip;
                        struct in_addr lan_ip_tmp;
                        lan_ip_tmp.s_addr = n.lan.ip;
                        memset(error_buf, 0, BUFSIZ);
                        sprintf(error_buf, "(%s:%d)cannot match the release node table:[%d][lan %s:%d][wan %s:%d]\n",
                                __FILE__, __LINE__,
                                table->protocol,
                                inet_ntoa(lan_ip_tmp),
                                htons(n.lan.port),
                                inet_ntoa(wan_ip_tmp),
                                htons(n.wan.port));
                        l->output(error_buf);
                        exit(-1);
                        continue;
                    } else {
                        rb.opaque = AP_REL_W2L;
                    }
                } else {
                    rb.opaque = AP_REL_L2W;
                }

                time_t ctime = time(NULL);
                sprintf(mesg , "{ \"type\" :\"release\"," 
                               "  \"protocol\": %d," 
                               "  \"ctime\"   : %ld," 
                               "  \"lan_ip\"  : %u," 
                               "  \"wan_ip\"  : %u," 
                               "  \"lan_port\": %d," 
                               "  \"wan_port\": %d}\n",
                               table->protocol,
                               ctime,
                               htonl(table->lan.ip),
                               htonl(table->wan.ip),
                               htons(table->lan.port),
                               htons(table->wan.port));

                for (;;) {
                    ret = ring_s_push(&h_recv, &rb);
                    if (ret == RING_OK) {
                        break;
                    }
                }
                break;
            }

            case AP_TIMEOUT:
            {
                struct node* table = (struct node*)rb.buffer;
                time_t ctime = time(NULL);
                sprintf(mesg , "{ \"type\" :\"release\"," 
                               "  \"protocol\": %d," 
                               "  \"ctime\"   : %ld," 
                               "  \"lan_ip\"  : %u," 
                               "  \"wan_ip\"  : %u," 
                               "  \"lan_port\": %d," 
                               "  \"wan_port\": %d}\n",
                               table->protocol,
                               ctime,
                               htonl(table->lan.ip),
                               htonl(table->wan.ip),
                               htons(table->lan.port),
                               htons(table->wan.port));
                rb.opaque = AP_TIMEOUT;
                for (;;) {
                    ret = ring_s_push(&h_recv, &rb);
                    if (ret == RING_OK) {
                        break;
                    }
                }
                break;
            }

            default:
            {
                break;
            }
        }

        switch (flags & AP_HIGH)
        {
            case AP_ADDRPOOL:
            {
                size_t mesg_size = strlen(mesg);
                if (send(addrpool_fd, mesg, mesg_size, 0) == -1) {
                    PERROR("send");
                    exit(-1);
                }
                break;
            }

            /*
            case AP_DIVERT:
            {
                //memdump(buf, size);
                // to outbound sending
                if(sendto(divert_fd, buf, size, 0, (struct sockaddr*)&sin, sin_len) < 0) {
                    if (errno == EHOSTDOWN) {
                    } else if (errno == EHOSTUNREACH) {
                    } else {
                        PERROR("sendto");
                        exit(-1);
                    }
                }
                free(buf);
                break;
            }
            */

            default:
            {
                break;
            }
        }

        /*
        time_t ctime = time(NULL);

        struct node  n;
        ret = mk_node(buf, size, &n);

        char mesg[BUFSIZ];

        struct node* table_l2w;
        table_l2w = find_l2w(&n);
        if (table_l2w != NULL) {
            sprintf(mesg, 
                    "{ 'message' :'release'," 
                    "  'protocol':'%d'," 
                    "  'ctime'   :'%ld'," 
                    "  'ip_src'  :'%d'," 
                    "  'ip_dst'  :'%d'," 
                    "  'port_src':'%d'," 
                    "  'port_dst':'%d'}",
                    table_l2w->protocol,
                    ctime,
                    htonl(table_l2w->lan.ip),
                    htonl(table_l2w->wan.ip),
                    htons(table_l2w->lan.port),
                    htons(table_l2w->wan.port));
        }

        struct node* table_w2l;
        table_w2l = find_w2l(&n);
        if (table_w2l != NULL) {
            sprintf(mesg, 
                    "{ 'message' :'release'," 
                    "  'protocol':'%d'," 
                    "  'ctime'   :'%ld'," 
                    "  'ip_src'  :'%d'," 
                    "  'ip_dst'  :'%d'," 
                    "  'port_src':'%d'," 
                    "  'port_dst':'%d'}",
                    table_w2l->protocol,
                    ctime,
                    htonl(table_w2l->lan.ip),
                    htonl(table_w2l->wan.ip),
                    htons(table_w2l->lan.port),
                    htons(table_w2l->wan.port));
        }

        if (table_l2w == NULL && table_w2l == NULL) {
            // request
            sprintf(mesg, 
                    "{ 'message' :'request'," 
                    "  'protocol':'%d'," 
                    "  'ctime'   :'%ld'," 
                    "  'ip_src'  :'%d'," 
                    "  'ip_dst'  :'%d'," 
                    "  'port_src':'%d'," 
                    "  'port_dst':'%d'}",
                    n.protocol,
                    ctime,
                    htonl(n.lan.ip),
                    htonl(n.wan.ip),
                    htons(n.lan.port),
                    htons(n.wan.port));
        }
        */

        /*
        struct ip* iphdr = (struct ip*)buf;
        uint8_t  protocol = iphdr->ip_p;
        uint32_t ip_src = htonl(iphdr->ip_src);
        uint32_t ip_dst = htonl(iphdr->ip_dst);
        switch (protocol)
        {
            case IPPROTO_TCP:
            {
                struct tcphdr* tcphdr = (struct tcphdr*)(buf+(iphdr->ip_hl<<2));
                uint16_t port_src = htons(tcphdr->th_sport);
                uint16_t port_dst = htons(tcphdr->th_dport);
                break;
            }

            case IPPROTO_UDP:
            {
                struct udphdr* udphdr = (struct udphdr*)(buf+(iphdr->ip_hl<<2));
                uint16_t port_src = htons(udphdr->uh_sport);
                uint16_t port_dst = htons(udphdr->uh_dport);
                break;
            }

            case IPPROTO_ICMP:
            {
                free(buf);
                continue;
            }

            default:
            {
                free(buf);
                continue;
            }
        }
        */

    }

    return NULL;
}

ssize_t addrpool_initialize(int fd) 
{
    std::string buffer    = "";
    std::string prefix    = "{ \"type\" :\"initialize\", \"wan_addrs\":["; 
    std::string separator = ",";
    std::string suffix    = "] }\n";
    std::ostringstream array;

    std::vector<uint32_t>::iterator it;
    it = pool_addr.begin();
    while (it != pool_addr.end()) {
        array << *it;
        array << separator;
        it++;
    }

    buffer.append(prefix);
    buffer.append(array.str());
    buffer.erase(buffer.size()-1, 1);
    buffer.append(suffix);
    
    //std::cout << buffer << std::endl;

    size_t buffer_size = buffer.size();
    ssize_t ssize;
    ssize = send(fd, buffer.c_str(), buffer_size, 0);
    if (ssize < 0) {
        PERROR("send");
        exit(-1);
    }

    return ssize;
}


int open_addrpool(uint16_t port)
{
    int fd;

    struct addrinfo info;
    struct addrinfo *res;
    memset(&info, 0, sizeof(info));
    info.ai_socktype = SOCK_STREAM;


    int ret; 
    ret = getaddrinfo("127.0.0.1", "8668", &info, &res);
    if (ret != 0) {
        PERROR("getaddrinfo");
        exit(-1);
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        PERROR("socket");
        exit(-1);
    }

    ret = connect(fd, res->ai_addr, res->ai_addrlen);
    if (ret != 0) {
        PERROR("connect");
        exit(-1);
    }

    freeaddrinfo(res);

    addrpool_initialize(fd); 

    return fd;
}

int pthread_create_addrpool(int divert_fd, int addrpool_fd, bool opt_reset)
{
    struct pth_addrpool_args* pth_args_sender;

    pth_args_sender = (struct pth_addrpool_args*)malloc(sizeof(struct pth_addrpool_args));
    if (pth_args_sender == NULL) { 
        return -1;
    }

    pth_args_sender->divert_fd = divert_fd;
    pth_args_sender->addrpool_fd = addrpool_fd;
    pth_args_sender->opt_reset = opt_reset;
    if (pthread_create(&pth_id_addrpool, NULL, addrpool_sender, pth_args_sender) != 0 ) {
        return -1;
    }

    struct pth_addrpool_args* pth_args_receiver;
    pth_args_receiver = (struct pth_addrpool_args*)malloc(sizeof(struct pth_addrpool_args));
    if (pth_args_receiver == NULL) { 
        return -1;
    }

    pth_args_receiver->divert_fd = divert_fd;
    pth_args_receiver->addrpool_fd = addrpool_fd;
    pth_args_receiver->opt_reset = opt_reset;
    if (pthread_create(&pth_id_addrpool, NULL, addrpool_receiver, pth_args_receiver) != 0 ) {
        return -1;
    }
    return 0;
}

#endif // DIVERT_HPP
