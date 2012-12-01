#ifndef DIVERT_H
#define DIVERT_H


#include "utils.hpp"

#include "ring.h"
#include "alias.h"
#include "addrpool.h"


#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>

#include <netinet/in.h>

#include <string.h>

pthread_t pth_id_divert;
extern struct ring_handler h_send;
extern struct ring_handler h_recv;

extern size_t wan_if_mtu;
extern size_t lan_if_mtu;

#ifdef NATIVE_LOCK
extern int divert_recv_lock;
extern int divert_send_lock;
#else
extern pthread_mutex_t divert_recv_lock;
extern pthread_mutex_t divert_send_lock;
#endif


struct pth_divert_args {
    int fd;
    uint64_t lan_if_num;
    uint64_t wan_if_num;
};


void* loop_divert(void* args)
{
    struct pth_divert_args* pda = (struct pth_divert_args*)args;
    int divert_fd = pda->fd;
    uint64_t lan_if_num = pda->lan_if_num;
    uint64_t wan_if_num = pda->wan_if_num;
    free(args);
    pthread_detach(pthread_self());

    struct sockaddr_in sin;
    ssize_t size;
    socklen_t sin_len;

    int ret;
    char buf[65535];

    /*
    int kq;
    struct kevent kev;
    kq = kqueue();
    if (kq == -1) {
        PERROR("kqueue");
        exit(-1);
    }

    EV_SET(&kev, divert_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) {
        PERROR("kevent");
        exit(-1);
    }
    */

    uint64_t ifname;
    sin_len = sizeof(sin);

#ifdef OPT_HOSTID
    size_t prev_opt_size = 0;
    size_t next_opt_size = 0;
#endif

#ifdef DEBUG
    B_RLOCK;
#endif
    for (;;) {
        /*
        ret = kevent(kq, NULL, 0, &kev, 1, NULL);
        if (ret == -1) {
            PERROR("kevent");
            exit(-1);
        } else {

            if (kev.ident == (unsigned int)divert_fd) {
        */
#ifdef DEBUG
                B_UNLOCK;
#endif

                R_DIV_SOCK_LOCK;
                size = recvfrom(divert_fd, buf, sizeof(buf),
                                0, (struct sockaddr*)&sin, &sin_len);
                if (size < 0) {
                    PERROR("recvfrom");
                    exit(-1);
                }
                R_DIV_SOCK_UNLOCK;

#ifdef DEBUG
                B_RLOCK;
#endif
                // ------------------
                // check l2w or w2l
                // ------------------
                //printf("IF:%s\n", sin.sin_zero);
                //printf("IF:%llu\n", *(uint64_t*)&sin.sin_zero);
                ifname = *(uint64_t*)&sin.sin_zero;
                if (ifname == lan_if_num) {

#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[lan2wan]";
                    }
#endif

#ifdef OPT_HOSTID
                    if (opt_hostid) {
                        prev_opt_size = get_optsize((struct ip*)buf);
                    }
#endif

                    ret = lan2wan_alias(buf, size);



                } else if (ifname == wan_if_num) {

#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[wan2lan]";
                    }
#endif
                    ret = wan2lan_alias(buf, size);


                } else {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[other]";
                        if (sin.sin_zero[0] != 0) {
                            std::cout << "[" << &sin.sin_zero[0] << "]";
                        } else {
                            std::cout << "[FROM_ME]";
                        }
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif

                    other_if_check(buf, size);
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
                    continue;

                }

                // ------------------
                // filter process
                // ------------------
                if (ret == ALIAS_NO_TABLE && ifname == lan_if_num) {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[CONN_INIT]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
                    ring_push(&h_send, buf, size, AP_SEND_REQ|AP_ADDRPOOL);
                    continue;

                } else if (ret == ALIAS_TOO_BIG) {

                    size = mk_fn_packet(buf, size);
                    std::cout << packet_info((struct ip*)buf) << std::endl;
                    sin.sin_addr.s_addr = INADDR_ANY;
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
                    continue;

                } else if (ret == ALIAS_RESET_TABLE) {

                    for (;;) {
                        ret = ring_push(&h_send, buf, size, AP_SEND_REL|AP_ADDRPOOL);
                        if (ret == RING_OK) {
                            break;
                        }
                        //usleep(1);
                    }
                    continue;

                } else if (ret == ALIAS_DEL_TABLE) {
                    // for tcp only
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[DEL_TBL]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif

#ifdef WAIT_TIME_DELETE
                    struct node n;
                    struct node* table;
                    mk_node(buf, size, &n);
                    //print_node(&n);

                    if (ifname == lan_if_num) {
                        n.wan.ip   = n.lan.ip;
                        n.wan.port = n.lan.port;
                        n.w2l_key = var_wan(&n);
                        table = find_w2l(&n);
                    } else if (ifname == wan_if_num) {
                        n.lan.ip   = n.wan.ip;
                        n.lan.port = n.wan.port; 
                        n.l2w_key = var_lan(&n);
                        table = find_l2w(&n);
                    } else {
                        table = NULL;
                    }

                    if (table == NULL) {
                        continue;
                    }

                    timer_event_delete_for_node(table, 400);
                    sin.sin_addr.s_addr = INADDR_ANY;
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
#else
                    for (;;) {
                        ret = ring_push(&h_send, buf, size, AP_SEND_REL|AP_ADDRPOOL);
                        if (ret == RING_OK) {
                            break;
                        }
                        //usleep(1);
                    }
#endif
                    continue;

                } else if (ret == ALIAS_DOALIAS) {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[DO_ALIAS]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif

#ifdef OPT_HOSTID
                    if (opt_hostid && ifname==lan_if_num) {
                        next_opt_size = get_optsize((struct ip*)buf);
                        size += next_opt_size - prev_opt_size;
                    }
#endif

                    if (ifname == lan_if_num) {

                        if (opt_nagete_df) {
                            if (htons(((struct ip*)buf)->ip_off)&IP_DF || (size_t)size>wan_if_mtu) {
                                struct ip* iphdr = (struct ip*)buf;
                                iphdr->ip_off = iphdr->ip_off & htons(0xbfff);
                            }
                        }   

                    } else if (ifname == wan_if_num) {

                        if (opt_nagete_df) {
                            if (htons(((struct ip*)buf)->ip_off)&IP_DF || (size_t)size>wan_if_mtu) {
                                struct ip* iphdr = (struct ip*)buf;
                                iphdr->ip_off = iphdr->ip_off & htons(0xbfff);
                            }   

                        } else {
                            ;
                        }  

                    } else {
                        ;
                    }

                    sin.sin_addr.s_addr = INADDR_ANY;
                    S_DIV_SOCK_LOCK;
                    if(sendto(divert_fd, buf, size, 0, (struct sockaddr*)&sin, sin_len) < 0) {
                        if (errno == EHOSTDOWN) {
                        } else if (errno == EHOSTUNREACH) {
                        } else {
                            printf("error_number:%d", errno);
                            PERROR("sendto");
                            exit(-1);
                        }
                    }
                    S_DIV_SOCK_UNLOCK;
                    continue;

                } else if (ret == ALIAS_TOOSHORT) {
#ifdef DEBUG
                    if (verbose) { 
                        std::cout << "[TOO_SHORT]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
                    continue;
                } else if (ret == ALIAS_DODROP) {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[DO_DROP]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
                    continue;
                } else if (ret == ALIAS_NOT_SUPPORT) {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[NOT_SUPPORT]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
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
                    continue;

                } else if (ret == ALIAS_TO_ME) {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[TO_ME]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
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
                    continue;

                } else if (ret == ALIAS_FROM_ME) {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << "[FROM_ME]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
                    S_DIV_SOCK_LOCK;
                    if(sendto(divert_fd, buf, size, 0, (struct sockaddr*)&sin, sin_len) < 0) {
#ifdef DEBUG
                        if (verbose) {
                            std::cout << "[ERROR]";
                            /*
                            std::cout << "[ifname " << ifname << "]";
                            std::cout << "[size " << size << "]";
                            std::cout << "[port " << sin.sin_port << "]";
                            */
                        }
#endif
                        if (errno == EHOSTDOWN) {
                        } else if (errno == EHOSTUNREACH) {
                        } else {
                            PERROR("sendto");
                            exit(-1);
                        }
                    }
                    S_DIV_SOCK_UNLOCK;
                    continue;

                } else {
#ifdef DEBUG
                    if (verbose) {
                        std::cout << ret;
                        std::cout << "[unknown]";
                        std::cout << packet_info((struct ip*)buf) << std::endl;
                    }
#endif
                    continue;

                }
            }
    close(divert_fd);
    return NULL;
}

int open_divert(uint16_t port)
{
    struct sockaddr_in bind_port;
    int fd;

#ifdef NATIVE_LOCK
    divert_recv_lock = 0;
    divert_send_lock = 0;
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&divert_recv_lock, &attr);
    pthread_mutex_init(&divert_send_lock, &attr);
#endif

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT);
    if (fd < 0) {
        PERROR("socket");
        exit(-1);
    }

    memset(&bind_port, 0,sizeof(bind_port));
    bind_port.sin_family = AF_INET;
    bind_port.sin_port   = htons(port);

    if (bind(fd, (struct sockaddr*)&bind_port, sizeof(bind_port)) < 0) {
        close(fd);
        PERROR("bind");
        exit(-1);
    }

    return fd;
}

int pthread_create_divert(int fd, uint64_t lan_if_num, uint64_t wan_if_num)
{

    pthread_attr_t attr = NULL;

/*
    int ret;
    int newprio = 30;
    sched_param param;

    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        PERROR("pthread_attr_init");
        exit(-1);
    }

    param.sched_priority = newprio;
    ret = pthread_attr_setschedparam (&attr, &param);
    if (ret != 0) {
        PERROR("pthread_attr_setschedparam");
        exit(-1);
    }
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret != 0) {
        PERROR("pthread_attr_setschedpolicy");
        exit(-1);
    }
*/

    struct pth_divert_args* pth_args;
    pth_args = (struct pth_divert_args*)malloc(sizeof(struct pth_divert_args));
    if (pth_args == NULL) { 
        return -1;
    }
    pth_args->fd = fd;
    pth_args->lan_if_num = lan_if_num;
    pth_args->wan_if_num = wan_if_num;
    if (pthread_create(&pth_id_divert, &attr, loop_divert, pth_args) != 0 ) {
        PERROR("pthread_create");
        exit(-1);
    }

    /*
    // 2kome
    struct pth_divert_args* pth_args2;
    pth_args2 = (struct pth_divert_args*)malloc(sizeof(struct pth_divert_args));
    if (pth_args2 == NULL) { 
        return -1;
    }
    pth_args2->fd = fd;
    pth_args2->lan_if_num = lan_if_num;
    pth_args2->wan_if_num = wan_if_num;
    if (pthread_create(&pth_id_divert, &attr, loop_divert, pth_args2) != 0 ) {
        return -1;
    }

    // 3kome
    struct pth_divert_args* pth_args3;
    pth_args3 = (struct pth_divert_args*)malloc(sizeof(struct pth_divert_args));
    if (pth_args3 == NULL) { 
        return -1;
    }
    pth_args3->fd = fd;
    pth_args3->lan_if_num = lan_if_num;
    pth_args3->wan_if_num = wan_if_num;
    if (pthread_create(&pth_id_divert, &attr, loop_divert, pth_args3) != 0 ) {
        return -1;
    }

    */



    return 0;
}

#endif // DIVERT_H
