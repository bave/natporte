#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <netinet/in.h>

#include "structure.h"
#include "utils.hpp"


int main(int argc ,char** argv)
{
    s_init();
    int i;
    struct node* n;
    in_addr_t base_wan_ip;
    in_addr_t base_lan_ip;
    base_wan_ip = 0x96410001;
    base_lan_ip = 0xC0A80001;
    for (i=0; i<3; i++) { 
        struct node* tmp;
        n = (struct node*)malloc(sizeof(struct node));
        n->ctime = time(NULL); 
        n->atime = n->ctime;
        n->flags = 0;
        n->protocol = IPPROTO_TCP;
        n->lan.ip = htonl(base_lan_ip+i);
        n->lan.port = htons(i);
        n->wan.ip = htonl(base_wan_ip+i);
        n->wan.port = htons(i);
        n->l2w_key=var_lan(n);
        n->w2l_key=var_wan(n);
        tmp = s_insert(n);
        if (tmp != NULL) {
            printf("cant insert\n");
            //printf("var_lan :%llu\n", var_lan(n));
            //printf("var_wan :%llu\n", var_wan(n));
            free(n);
        }
    }

    i = 1;
    RB_FOREACH(n, l2w, &l2w_head){
        printf("count %d\n", i);
        atime(n);
        print_node(n);
        sleep(0);
        i++;
        printf("\n");
    }

    struct node rn;
    int bias = 1;
    memset(&rn, 0, sizeof(struct node));
    rn.lan.ip   = htonl(0xC0A80001 + bias);
    rn.lan.port = htons(bias);
    rn.protocol = IPPROTO_TCP;
    rn.l2w_key = var_lan(&rn);

    printf("find\n");
    n = find_l2w(&rn);
    print_node(n);

    printf("delete\n");
    n = s_delete_for_value(n);
    if (n == NULL) {
        printf("n is null pointer\n");
        exit(-1);
    } else {
    }

    printf("refind\n");
    n = find_l2w(&rn);
    if (n == NULL) {
        printf("cannot refine, n is null pointer\n");
    } else {
        exit(-1);
    }

    for (i=4; i<6; i++) { 
        struct node* tmp;
        n = (struct node*)malloc(sizeof(struct node));
        n->ctime = time(NULL); 
        n->atime = n->ctime;
        n->flags = 0;
        n->protocol = IPPROTO_TCP;
        n->lan.ip = htonl(base_lan_ip+i);
        n->lan.port = i;
        n->wan.ip = htonl(base_wan_ip+i);
        n->wan.port = i;
        n->l2w_key=var_lan(n);
        n->w2l_key=var_wan(n);
        tmp = s_insert(n);
        if (tmp != NULL) {
            printf("cant insert\n");
            //printf("var_lan :%llu\n", var_lan(n));
            //printf("var_wan :%llu\n", var_wan(n));
            free(n);
        }
    }

    i = 1;
    RB_FOREACH(n, l2w, &l2w_head){
        printf("count %d\n", i);
        /*
        printf("lan_ip  :%x\n", n->lan.ip);
        printf("lan_port:%d\n", n->lan.port);
        printf("wan_ip  :%x\n", n->wan.ip);
        printf("wan_port:%d\n", n->wan.port);
        */
        atime(n);
        print_node(n);
        sleep(0);
        i++;
        printf("\n");
    }

    return 0;
}
