#ifndef STRUCTRUE_H
#define STRUCTRUE_H


#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/cdefs.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>

#include <pthread.h>

#include "utils.hpp"
#include "log.hpp"

static pthread_rwlock_t s_lock;
#define SWLOCK pthread_rwlock_wrlock(&s_lock)
#define SRLOCK pthread_rwlock_rdlock(&s_lock)
#define UNLOCK pthread_rwlock_unlock(&s_lock)

static pthread_rwlock_t buf_lock;
#define B_WLOCK pthread_rwlock_wrlock(&buf_lock)
#define B_RLOCK pthread_rwlock_rdlock(&buf_lock)
#define B_UNLOCK pthread_rwlock_unlock(&buf_lock)


#ifdef NATIVE_LOCK
int divert_recv_lock;
int divert_send_lock;
#define R_DIV_SOCK_LOCK   native_lock(&divert_recv_lock)
#define R_DIV_SOCK_UNLOCK native_unlock(&divert_recv_lock) 
#define S_DIV_SOCK_LOCK   native_lock(&divert_send_lock)
#define S_DIV_SOCK_UNLOCK native_unlock(&divert_send_lock)
#else
pthread_mutex_t divert_recv_lock;
pthread_mutex_t divert_send_lock;
#define R_DIV_SOCK_LOCK   pthread_mutex_lock(&divert_recv_lock)
#define R_DIV_SOCK_UNLOCK pthread_mutex_unlock(&divert_recv_lock) 
#define S_DIV_SOCK_LOCK   pthread_mutex_lock(&divert_send_lock)
#define S_DIV_SOCK_UNLOCK pthread_mutex_unlock(&divert_send_lock)
#endif


#define S_TF_ESTABLISHED       0x00
#define S_TF_SYN_SENT          0x01
#define S_TF_SYNACK_RECIVED    0x02
#define S_TF_FIN_WAIT_L2W      0x03
#define S_TF_FIN_WAIT_W2L      0x04
#define S_TF_FINACK_WAIT_L2W   0x05
#define S_TF_FINACK_WAIT_W2L   0x06
#define S_TF_CLOSING_L2W       0x07
#define S_TF_CLOSING_W2L       0x08
#define S_TF_CLOSED            0x09
#define S_TF_RESET             0x0A
#define S_TF_UNKNOWN           0x0F

#define S_TBL_CONNECTING 0x01
#define S_TBL_CONNECTED  0x02
#define S_TBL_CLOSING    0x04
#define S_TBL_CLOSED     0x08

#define S_TIMER_LOOP         0x00
#define S_TIMER_CLOSE        0x01
#define S_TIMER_SILENT_CLOSE 0x02

int kq;
int timeout_udp;
int timeout_tcp;

#define RB_HEAD(name, type) \
    struct name { \
        struct type *rbh_root; \
    }

#define RB_INITIALIZER(root) { NULL }

#define RB_INIT(root) do { \
    (root)->rbh_root = NULL;  \
} while (0)

#define RB_BLACK 0
#define RB_RED 1
#define RB_ENTRY(type) \
    struct { \
        struct type *rbe_left; \
        struct type *rbe_right; \
        struct type *rbe_parent; \
        int rbe_color; \
    }

#define RB_LEFT(elm, field)   (elm)->field.rbe_left
#define RB_RIGHT(elm, field)  (elm)->field.rbe_right
#define RB_PARENT(elm, field) (elm)->field.rbe_parent
#define RB_COLOR(elm, field)  (elm)->field.rbe_color
#define RB_ROOT(head)         (head)->rbh_root
#define RB_EMPTY(head)        (RB_ROOT(head) == NULL)

#define RB_SET(elm, parent, field) do { \
    RB_PARENT(elm, field) = parent; \
    RB_LEFT(elm, field) = RB_RIGHT(elm, field) = NULL; \
    RB_COLOR(elm, field) = RB_RED; \
} while (0)

#define RB_SET_BLACKRED(black, red, field) do { \
    RB_COLOR(black, field) = RB_BLACK; \
    RB_COLOR(red, field) = RB_RED; \
} while (0)

#ifndef RB_AUGMENT
#define RB_AUGMENT(x) do {} while (0)
#endif

#define RB_ROTATE_LEFT(head, elm, tmp, field) do { \
    (tmp) = RB_RIGHT(elm, field); \
    if ((RB_RIGHT(elm, field) = RB_LEFT(tmp, field)) != NULL) { \
        RB_PARENT(RB_LEFT(tmp, field), field) = (elm); \
    } \
    RB_AUGMENT(elm); \
    if ((RB_PARENT(tmp, field) = RB_PARENT(elm, field)) != NULL) { \
        if ((elm) == RB_LEFT(RB_PARENT(elm, field), field)) \
        RB_LEFT(RB_PARENT(elm, field), field) = (tmp); \
        else \
        RB_RIGHT(RB_PARENT(elm, field), field) = (tmp); \
    } else \
    (head)->rbh_root = (tmp); \
    RB_LEFT(tmp, field) = (elm); \
    RB_PARENT(elm, field) = (tmp); \
    RB_AUGMENT(tmp); \
    if ((RB_PARENT(tmp, field))) \
    RB_AUGMENT(RB_PARENT(tmp, field)); \
} while (0)

#define RB_ROTATE_RIGHT(head, elm, tmp, field) do { \
    (tmp) = RB_LEFT(elm, field); \
    if ((RB_LEFT(elm, field) = RB_RIGHT(tmp, field)) != NULL) { \
        RB_PARENT(RB_RIGHT(tmp, field), field) = (elm); \
    } \
    RB_AUGMENT(elm); \
    if ((RB_PARENT(tmp, field) = RB_PARENT(elm, field)) != NULL) { \
        if ((elm) == RB_LEFT(RB_PARENT(elm, field), field)) \
        RB_LEFT(RB_PARENT(elm, field), field) = (tmp); \
        else \
        RB_RIGHT(RB_PARENT(elm, field), field) = (tmp); \
    } else \
    (head)->rbh_root = (tmp); \
    RB_RIGHT(tmp, field) = (elm); \
    RB_PARENT(elm, field) = (tmp); \
    RB_AUGMENT(tmp); \
    if ((RB_PARENT(tmp, field))) \
    RB_AUGMENT(RB_PARENT(tmp, field)); \
} while (0)

#define RB_PROTOTYPE(name, type, field, cmp) \
    RB_PROTOTYPE_INTERNAL(name, type, field, cmp,)
#define RB_PROTOTYPE_STATIC(name, type, field, cmp) \
    RB_PROTOTYPE_INTERNAL(name, type, field, cmp, __unused static)
#define RB_PROTOTYPE_INTERNAL(name, type, field, cmp, attr) \
static inline void name##_RB_INSERT_COLOR(struct name *, struct type *); \
static inline void name##_RB_REMOVE_COLOR(struct name *, struct type *, struct type *); \
static inline struct type *name##_RB_REMOVE(struct name *, struct type *); \
static inline struct type *name##_RB_INSERT(struct name *, struct type *); \
static inline struct type *name##_RB_FIND(struct name *, struct type *); \
static inline struct type *name##_RB_NFIND(struct name *, struct type *); \
static inline struct type *name##_RB_NEXT(struct type *); \
static inline struct type *name##_RB_PREV(struct type *); \
static inline struct type *name##_RB_MINMAX(struct name *, int); \

#define RB_GENERATE(name, type, field, cmp) RB_GENERATE_INTERNAL(name, type, field, cmp,)
#define RB_GENERATE_STATIC(name, type, field, cmp) RB_GENERATE_INTERNAL(name, type, field, cmp, __unused static)

#define RB_GENERATE_INTERNAL(name, type, field, cmp, attr) \
static inline void name##_RB_INSERT_COLOR(struct name *head, struct type *elm) \
{ \
    struct type *parent, *gparent, *tmp; \
    while ((parent = RB_PARENT(elm, field)) != NULL && \
            RB_COLOR(parent, field) == RB_RED) { \
        gparent = RB_PARENT(parent, field); \
        if (parent == RB_LEFT(gparent, field)) { \
            tmp = RB_RIGHT(gparent, field); \
            if (tmp && RB_COLOR(tmp, field) == RB_RED) { \
                RB_COLOR(tmp, field) = RB_BLACK; \
                RB_SET_BLACKRED(parent, gparent, field); \
                elm = gparent; \
                continue; \
            } \
            if (RB_RIGHT(parent, field) == elm) { \
                RB_ROTATE_LEFT(head, parent, tmp, field); \
                tmp = parent; \
                parent = elm; \
                elm = tmp; \
            } \
            RB_SET_BLACKRED(parent, gparent, field); \
            RB_ROTATE_RIGHT(head, gparent, tmp, field); \
        } else { \
            tmp = RB_LEFT(gparent, field); \
            if (tmp && RB_COLOR(tmp, field) == RB_RED) { \
                RB_COLOR(tmp, field) = RB_BLACK; \
                RB_SET_BLACKRED(parent, gparent, field); \
                elm = gparent; \
                continue; \
            } \
            if (RB_LEFT(parent, field) == elm) { \
                RB_ROTATE_RIGHT(head, parent, tmp, field); \
                tmp = parent; \
                parent = elm; \
                elm = tmp; \
            } \
            RB_SET_BLACKRED(parent, gparent, field); \
            RB_ROTATE_LEFT(head, gparent, tmp, field); \
        } \
    } \
    RB_COLOR(head->rbh_root, field) = RB_BLACK; \
} \
\
static inline void \
name##_RB_REMOVE_COLOR(struct name *head, struct type *parent, struct type *elm) \
{ \
    struct type *tmp; \
    while ((elm == NULL || RB_COLOR(elm, field) == RB_BLACK) && \
            elm != RB_ROOT(head)) { \
        if (RB_LEFT(parent, field) == elm) { \
            tmp = RB_RIGHT(parent, field); \
            if (RB_COLOR(tmp, field) == RB_RED) { \
                RB_SET_BLACKRED(tmp, parent, field); \
                RB_ROTATE_LEFT(head, parent, tmp, field); \
                tmp = RB_RIGHT(parent, field); \
            } \
            if ((RB_LEFT(tmp, field) == NULL || \
                        RB_COLOR(RB_LEFT(tmp, field), field) == RB_BLACK) && \
                    (RB_RIGHT(tmp, field) == NULL || \
                     RB_COLOR(RB_RIGHT(tmp, field), field) == RB_BLACK)) { \
                RB_COLOR(tmp, field) = RB_RED; \
                elm = parent; \
                parent = RB_PARENT(elm, field); \
            } else { \
                if (RB_RIGHT(tmp, field) == NULL || \
                        RB_COLOR(RB_RIGHT(tmp, field), field) == RB_BLACK) { \
                    struct type *oleft; \
                    if ((oleft = RB_LEFT(tmp, field)) \
                            != NULL) \
                    RB_COLOR(oleft, field) = RB_BLACK; \
                    RB_COLOR(tmp, field) = RB_RED; \
                    RB_ROTATE_RIGHT(head, tmp, oleft, field); \
                    tmp = RB_RIGHT(parent, field); \
                } \
                RB_COLOR(tmp, field) = RB_COLOR(parent, field); \
                RB_COLOR(parent, field) = RB_BLACK; \
                if (RB_RIGHT(tmp, field)) \
                RB_COLOR(RB_RIGHT(tmp, field), field) = RB_BLACK; \
                RB_ROTATE_LEFT(head, parent, tmp, field); \
                elm = RB_ROOT(head); \
                break; \
            } \
        } else { \
            tmp = RB_LEFT(parent, field); \
            if (RB_COLOR(tmp, field) == RB_RED) { \
                RB_SET_BLACKRED(tmp, parent, field); \
                RB_ROTATE_RIGHT(head, parent, tmp, field); \
                tmp = RB_LEFT(parent, field); \
            } \
            if ((RB_LEFT(tmp, field) == NULL || \
                        RB_COLOR(RB_LEFT(tmp, field), field) == RB_BLACK) && \
                    (RB_RIGHT(tmp, field) == NULL || \
                     RB_COLOR(RB_RIGHT(tmp, field), field) == RB_BLACK)) { \
                RB_COLOR(tmp, field) = RB_RED; \
                elm = parent; \
                parent = RB_PARENT(elm, field); \
            } else { \
                if (RB_LEFT(tmp, field) == NULL || \
                        RB_COLOR(RB_LEFT(tmp, field), field) == RB_BLACK) { \
                    struct type *oright; \
                    if ((oright = RB_RIGHT(tmp, field)) \
                            != NULL) \
                    RB_COLOR(oright, field) = RB_BLACK; \
                    RB_COLOR(tmp, field) = RB_RED; \
                    RB_ROTATE_LEFT(head, tmp, oright, field); \
                    tmp = RB_LEFT(parent, field); \
                } \
                RB_COLOR(tmp, field) = RB_COLOR(parent, field); \
                RB_COLOR(parent, field) = RB_BLACK; \
                if (RB_LEFT(tmp, field)) \
                RB_COLOR(RB_LEFT(tmp, field), field) = RB_BLACK; \
                RB_ROTATE_RIGHT(head, parent, tmp, field); \
                elm = RB_ROOT(head); \
                break; \
            } \
        } \
    } \
    if (elm) \
    RB_COLOR(elm, field) = RB_BLACK; \
} \
\
static inline struct type * \
name##_RB_REMOVE(struct name *head, struct type *elm) \
{ \
    struct type *child, *parent, *old = elm; \
    int color; \
    if (RB_LEFT(elm, field) == NULL) \
    child = RB_RIGHT(elm, field); \
    else if (RB_RIGHT(elm, field) == NULL) \
    child = RB_LEFT(elm, field); \
    else { \
        struct type *left; \
        elm = RB_RIGHT(elm, field); \
        while ((left = RB_LEFT(elm, field)) != NULL) \
        elm = left; \
        child = RB_RIGHT(elm, field); \
        parent = RB_PARENT(elm, field); \
        color = RB_COLOR(elm, field); \
        if (child) \
        RB_PARENT(child, field) = parent; \
        if (parent) { \
            if (RB_LEFT(parent, field) == elm) \
            RB_LEFT(parent, field) = child; \
            else \
            RB_RIGHT(parent, field) = child; \
            RB_AUGMENT(parent); \
        } else \
        RB_ROOT(head) = child; \
        if (RB_PARENT(elm, field) == old) \
        parent = elm; \
        (elm)->field = (old)->field; \
        if (RB_PARENT(old, field)) { \
            if (RB_LEFT(RB_PARENT(old, field), field) == old) \
            RB_LEFT(RB_PARENT(old, field), field) = elm; \
            else \
            RB_RIGHT(RB_PARENT(old, field), field) = elm; \
            RB_AUGMENT(RB_PARENT(old, field)); \
        } else \
        RB_ROOT(head) = elm; \
        RB_PARENT(RB_LEFT(old, field), field) = elm; \
        if (RB_RIGHT(old, field)) \
        RB_PARENT(RB_RIGHT(old, field), field) = elm; \
        if (parent) { \
            left = parent; \
            do { \
                RB_AUGMENT(left); \
            } while ((left = RB_PARENT(left, field)) != NULL); \
        } \
        goto color; \
    } \
    parent = RB_PARENT(elm, field); \
    color = RB_COLOR(elm, field); \
    if (child) \
    RB_PARENT(child, field) = parent; \
    if (parent) { \
        if (RB_LEFT(parent, field) == elm) \
        RB_LEFT(parent, field) = child; \
        else \
        RB_RIGHT(parent, field) = child; \
        RB_AUGMENT(parent); \
    } else \
    RB_ROOT(head) = child; \
    color: \
    if (color == RB_BLACK) \
    name##_RB_REMOVE_COLOR(head, parent, child); \
    return (old); \
} \
\
static inline struct type * \
name##_RB_INSERT(struct name *head, struct type *elm) \
{ \
    struct type *tmp; \
    struct type *parent = NULL; \
    int comp = 0; \
    tmp = RB_ROOT(head); \
    while (tmp) { \
        parent = tmp; \
        comp = (cmp)(elm, parent); \
        if (comp < 0) \
        tmp = RB_LEFT(tmp, field); \
        else if (comp > 0) \
        tmp = RB_RIGHT(tmp, field); \
        else \
        return (tmp); \
    } \
    RB_SET(elm, parent, field); \
    if (parent != NULL) { \
        if (comp < 0) \
        RB_LEFT(parent, field) = elm; \
        else \
        RB_RIGHT(parent, field) = elm; \
        RB_AUGMENT(parent); \
    } else \
    RB_ROOT(head) = elm; \
    name##_RB_INSERT_COLOR(head, elm); \
    return (NULL); \
} \
\
static inline struct type * \
name##_RB_FIND(struct name *head, struct type *elm) \
{ \
    struct type *tmp = RB_ROOT(head); \
    int comp; \
    while (tmp) { \
        comp = cmp(elm, tmp); \
        if (comp < 0) { \
            tmp = RB_LEFT(tmp, field); \
        }  else if (comp > 0)  {\
            tmp = RB_RIGHT(tmp, field); \
        } else { \
            return (tmp); \
        } \
    } \
    return (NULL); \
} \
\
static inline struct type * \
name##_RB_NFIND(struct name *head, struct type *elm) \
{ \
    struct type *tmp = RB_ROOT(head); \
    struct type *res = NULL; \
    int comp; \
    while (tmp) { \
        comp = cmp(elm, tmp); \
        if (comp < 0) { \
            res = tmp; \
            tmp = RB_LEFT(tmp, field); \
        } else if (comp > 0) { \
            tmp = RB_RIGHT(tmp, field); \
        } else { \
            return (tmp); \
        } \
    } \
    return (res); \
} \
\
static inline struct type * \
name##_RB_NEXT(struct type *elm) \
{ \
    if (RB_RIGHT(elm, field)) { \
        elm = RB_RIGHT(elm, field); \
        while (RB_LEFT(elm, field)) \
        elm = RB_LEFT(elm, field); \
    } else { \
        if (RB_PARENT(elm, field) && \
                (elm == RB_LEFT(RB_PARENT(elm, field), field))) \
        elm = RB_PARENT(elm, field); \
        else { \
            while (RB_PARENT(elm, field) && \
                    (elm == RB_RIGHT(RB_PARENT(elm, field), field))) \
            elm = RB_PARENT(elm, field); \
            elm = RB_PARENT(elm, field); \
        } \
    } \
    return (elm); \
} \
\
static inline struct type * \
name##_RB_PREV(struct type *elm) \
{ \
    if (RB_LEFT(elm, field)) { \
        elm = RB_LEFT(elm, field); \
        while (RB_RIGHT(elm, field)) \
        elm = RB_RIGHT(elm, field); \
    } else { \
        if (RB_PARENT(elm, field) && \
                (elm == RB_RIGHT(RB_PARENT(elm, field), field))) \
        elm = RB_PARENT(elm, field); \
        else { \
            while (RB_PARENT(elm, field) && \
                    (elm == RB_LEFT(RB_PARENT(elm, field), field))) \
            elm = RB_PARENT(elm, field); \
            elm = RB_PARENT(elm, field); \
        } \
    } \
    return (elm); \
} \
\
static inline struct type * \
name##_RB_MINMAX(struct name *head, int val) \
{ \
    struct type *tmp = RB_ROOT(head); \
    struct type *parent = NULL; \
    while (tmp) { \
        parent = tmp; \
        if (val < 0) \
        tmp = RB_LEFT(tmp, field); \
        else \
        tmp = RB_RIGHT(tmp, field); \
    } \
    return (parent); \
}

#define RB_NEGINF -1
#define RB_INF 1

#define RB_INSERT(name, x, y) name##_RB_INSERT(x, y)
#define RB_REMOVE(name, x, y) name##_RB_REMOVE(x, y)
#define RB_FIND(name, x, y) name##_RB_FIND(x, y)
#define RB_NFIND(name, x, y) name##_RB_NFIND(x, y)
#define RB_NEXT(name, x, y) name##_RB_NEXT(y)
#define RB_PREV(name, x, y) name##_RB_PREV(y)
#define RB_MIN(name, x) name##_RB_MINMAX(x, RB_NEGINF)
#define RB_MAX(name, x) name##_RB_MINMAX(x, RB_INF)

#define RB_FOREACH(x, name, head) \
    for ((x) = RB_MIN(name, head); \
            (x) != NULL; \
            (x) = name##_RB_NEXT(x))

#define RB_FOREACH_FROM(x, name, y) \
    for ((x) = (y); \
            ((x) != NULL) && ((y) = name##_RB_NEXT(x), (x) != NULL); \
            (x) = (y))

#define RB_FOREACH_SAFE(x, name, head, y) \
    for ((x) = RB_MIN(name, head); \
            ((x) != NULL) && ((y) = name##_RB_NEXT(x), (x) != NULL); \
            (x) = (y))

#define RB_FOREACH_REVERSE(x, name, head) \
    for ((x) = RB_MAX(name, head); \
            (x) != NULL; \
            (x) = name##_RB_PREV(x))

#define RB_FOREACH_REVERSE_FROM(x, name, y) \
    for ((x) = (y); \
            ((x) != NULL) && ((y) = name##_RB_PREV(x), (x) != NULL); \
            (x) = (y))

#define RB_FOREACH_REVERSE_SAFE(x, name, head, y) \
    for ((x) = RB_MAX(name, head); \
            ((x) != NULL) && ((y) = name##_RB_PREV(x), (x) != NULL); \
            (x) = (y))

struct node
{
    //TAILQ_ENTRY(node) q_next;
    RB_ENTRY(node) l2w_next;
    RB_ENTRY(node) w2l_next;

    //create_time ctime
    time_t ctime;
    //access_time atime
    time_t atime;

    uint8_t flags;
    uint8_t timer_flags;

    uint8_t protocol;

    // wan id's 48bits
    struct {
        in_port_t port;
        in_addr_t ip;
    } wan;

    // wan id's 48bits
    struct {
        in_port_t port;
        in_addr_t ip;
    } lan;

    uint64_t l2w_key;
    uint64_t w2l_key;

}; //__packed;

struct icmp_bucket {

    uint32_t local;
    uint32_t foreign;

} icmp_bucket;

static inline uint8_t get_protocol_from_var_lan(uint64_t var)
{
    return ((uint8_t)((var & 0xFFFF000000000000)>>48));
}

static inline uint16_t get_port_from_var_lan(uint64_t var)
{
    return ((uint16_t)((var & 0x0000FFFF00000000)>>32));
}

static inline uint32_t get_ip_from_var_lan(uint64_t var)
{
    return ((uint32_t)(var & 0x00000000FFFFFFFF));
}

static inline uint64_t var_lan(const struct node* n)
{
    uint64_t a = 0xFFFF000000000000 & (((uint64_t)n->protocol)<<48);
    uint64_t b = 0x0000FFFF00000000 & (((uint64_t)n->lan.port)<<32);
    uint64_t c = 0x00000000FFFFFFFF & ((uint64_t)n->lan.ip);
    return  (a | b | c);
}

static inline uint64_t var_wan(const struct node* n)
{
    uint64_t a = 0xFFFF000000000000 & (((uint64_t)n->protocol)<<48);
    uint64_t b = 0x0000FFFF00000000 & (((uint64_t)n->wan.port)<<32);
    uint64_t c = 0x00000000FFFFFFFF & ((uint64_t)n->wan.ip);
    return  (a | b | c);
}

static inline int cmp_l2w(const struct node* lhs, const struct node* rhs)
{
    /*
    uint64_t lhs_cmp = var_lan(lhs);
    uint64_t rhs_cmp = var_lan(rhs);
    */

    uint64_t lhs_cmp = lhs->l2w_key;
    uint64_t rhs_cmp = rhs->l2w_key;
    if (lhs_cmp < rhs_cmp) {
        return -1;
    } else if (lhs_cmp > rhs_cmp) {
        return 1;
    } else {
        return 0;
    }
    return 0;
}

static inline int cmp_w2l(const struct node* lhs, const struct node* rhs)
{
    /*
    uint64_t lhs_cmp = var_wan(lhs);
    uint64_t rhs_cmp = var_wan(rhs);
    */
    uint64_t lhs_cmp = lhs->w2l_key;
    uint64_t rhs_cmp = rhs->w2l_key;
    if (lhs_cmp < rhs_cmp) {
        return -1;
    } else if (lhs_cmp > rhs_cmp) {
        return 1;
    } else {
        return 0;
    }
    return 0;
}

TAILQ_HEAD(queue, node) q_head;

RB_HEAD(l2w, node) l2w_head;
RB_PROTOTYPE(l2w, node, l2w_next, cmp_l2w);
//RB_PROTOTYPE_INTERNAL(l2w, node, l2w_next, cmp_l2w, static inline);
RB_GENERATE(l2w, node, l2w_next, cmp_l2w);

RB_HEAD(w2l, node) w2l_head;
RB_PROTOTYPE(w2l, node, w2l_next, cmp_w2l);
//RB_PROTOTYPE_INTERNAL(w2l, node, w2l_next, cmp_w2l, static inline);
RB_GENERATE(w2l, node, w2l_next, cmp_w2l);

static inline void s_init(void)
{
    pthread_rwlock_init(&buf_lock, 0);
    pthread_rwlock_init(&s_lock, 0);
    RB_INIT(&l2w_head);
    RB_INIT(&w2l_head);
    //TAILQ_INIT(&q_head);
    return;
}

static inline struct node* s_insert(struct node* node)
{
    SWLOCK;
    struct node* p;
    p = RB_INSERT(l2w, &l2w_head, node);
    if (p != NULL) {
        fprintf(stderr,"l2w insert fail\n");
        l->output("l2w insert fail");
        UNLOCK;
        return p;
    }
    p = RB_INSERT(w2l, &w2l_head, node);
    if (p != NULL) {
        fprintf(stderr,"w2l insert fail\n");
        l->output("w2l insert fail");
        RB_REMOVE(l2w, &l2w_head, node);
        UNLOCK;
        return p;
    }
    UNLOCK;
    return NULL;
}

static inline struct node* s_delete_for_point(struct node* node)
{
    struct node* node_ret = NULL;
    struct node* node_w2l;
    struct node* node_l2w;

    SWLOCK;
    node_l2w = RB_REMOVE(l2w, &l2w_head, node);
    node_w2l = RB_REMOVE(w2l, &w2l_head, node);
    UNLOCK;
    if (node_l2w == node_w2l) {
        node_ret = node_l2w;
    }
    return node_ret;
}


static inline struct node* s_delete_for_value(struct node* node)
{
    struct node* node_ret = NULL;
    struct node* node_l2w;
    struct node* node_w2l;

    SRLOCK;
    node_l2w = RB_FIND(l2w, &l2w_head, node);
    node_w2l = RB_FIND(w2l, &w2l_head, node);
    UNLOCK;

    if (node_l2w == NULL || node_w2l == NULL) {
        return NULL;
    }

    if (node_l2w == node_w2l) {
        node_ret = s_delete_for_point(node_l2w);
    }
    return node_ret;
}


static inline struct node* find_l2w(struct node* node)
{
    SRLOCK;
    struct node* ret = RB_FIND(l2w, &l2w_head, node);
    UNLOCK;
    return ret;
}

static inline struct node* find_w2l(struct node* node)
{
    SRLOCK;
    struct node* ret = RB_FIND(w2l, &w2l_head, node);
    UNLOCK;
    return ret;
}

static inline struct node* s_head(void)
{
    return TAILQ_FIRST(&q_head);
}

static inline struct node* s_tail(void)
{
    return TAILQ_LAST(&q_head, queue);
}

static inline void atime(struct node* node)
{
    node->atime = time(NULL);
    return;
}

static inline void attach(struct node* node)
{
    /*
    SWLOCK;
    TAILQ_REMOVE(&q_head, node, q_next);
    TAILQ_INSERT_HEAD(&q_head, node, q_next);
    atime(node);
    UNLOCK;
    */
    atime(node);
    return;
}
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


void print_node(struct node* n)
{
    if (n == NULL) {
        printf("struct node* n is null\n");
        return;
    }

    struct tm* date;
    char my_date[BUFSIZ];

    printf("protocol :%d\n", n->protocol);

    date = localtime(&n->ctime);
    strftime(my_date, sizeof(my_date), "%Y/%m/%d_%H:%M:%S", date);
    printf("ctime    :%s(%ld)\n", my_date, n->ctime);

    date = localtime(&n->atime);
    strftime(my_date, sizeof(my_date), "%Y/%m/%d_%H:%M:%S", date);
    printf("atime    :%s(%ld)\n", my_date, n->atime);

    if (n->protocol == IPPROTO_TCP) {
        printf("flags    :%s\n", table_tcp_status(n->flags));
    } else {
        printf("flags    :%d\n", n->flags);
    }

    struct in_addr wan_ip;
    wan_ip.s_addr = n->wan.ip;
    printf("WAN      :%s:%d\n", inet_ntoa(wan_ip), htons(n->wan.port));

    struct in_addr lan_ip;
    lan_ip.s_addr = n->lan.ip;
    printf("LAN      :%s:%d\n", inet_ntoa(lan_ip), htons(n->lan.port));

    return;
}

#define STRUCTURE_ETH_FRAME_MIN 20
static inline int mk_node(char* buf, size_t size, struct node* n)
{
    if (size <= STRUCTURE_ETH_FRAME_MIN) { 
        return -1;
    }

    struct ip* iphdr = (struct ip*)buf;

    n->protocol = iphdr->ip_p;
    n->lan.ip   = iphdr->ip_src.s_addr;
    n->wan.ip   = iphdr->ip_dst.s_addr;
    //printf("\n%x->%x\n", n->lan.ip, n->wan.ip);

    switch (iphdr->ip_p)
    {
        case IPPROTO_TCP:
        {
            struct tcphdr* tcphdr = (struct tcphdr*)((char*)iphdr+(iphdr->ip_hl<<2));
            n->lan.port = tcphdr->th_sport;
            n->wan.port = tcphdr->th_dport;
            //printf("\n0x%d->0x%d\n", htons(n->lan.port), htons(n->wan.port));
            break;
        }

        case IPPROTO_UDP:
        {
            struct udphdr* udphdr = (struct udphdr*)((char*)iphdr+(iphdr->ip_hl<<2));
            n->lan.port = udphdr->uh_sport;
            n->wan.port = udphdr->uh_dport;
            //printf("\n0x%d->0x%d\n", htons(n->lan.port), htons(n->wan.port));
            break;
        }

        default:
        {
            return -1;
        }
    }
    return 0;
}

// timer is [ms]
// reset_timer_evetn_delete??
static inline void timer_event_delete(uint64_t ident, int timer)
{
    int ret;
    struct kevent kev;
    EV_SET(&kev, ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
#ifdef __MACH__
    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD, 0x02, (timer*1000), NULL);
#else
    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD, 0x02, timer, NULL);
#endif
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) {
        PERROR("kevent");
        uint8_t proto = get_protocol_from_var_lan(ident);
        uint16_t port = get_port_from_var_lan(ident);
        struct in_addr ip;
        ip.s_addr = get_ip_from_var_lan(ident);
        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "(%s:%d)protocol:%d:%s:%d\n", __FILE__, __LINE__, proto, inet_ntoa(ip), htons(port));
        l->output(buf);
        exit(-1);
    }
    return;
}

// timer is [ms]
// reset_timer_evetn_delete??
static inline void timer_event_delete_for_node(struct node* table, int timer)
{
    uint64_t ident;
    ident = table->l2w_key;
    table->timer_flags = S_TIMER_CLOSE;
    timer_event_delete(ident, timer);
    return;
}

static inline void add_timer_event_udp(uint64_t ident)
{
    int ret;
    struct kevent kev;
#ifdef __MACH__
    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD, 0x02, (timeout_udp*1000)/2, NULL);
#else
    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD, 0x02, (timeout_udp)/2, NULL);
#endif
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) {
        uint8_t proto = get_protocol_from_var_lan(ident);
        uint16_t port = get_port_from_var_lan(ident);
        struct in_addr ip;
        ip.s_addr = get_ip_from_var_lan(ident);
        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "(%s:%d)protocol:%d:%s:%d\n", __FILE__, __LINE__, proto, inet_ntoa(ip), htons(port));
        l->output(buf);
        exit(-1);
    }
    return;
}

static inline void add_timer_event_tcp(uint64_t ident)
{
    int ret;
    struct kevent kev;
#ifdef __MACH__
    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD, 0x02, (timeout_tcp*1000)/2, NULL);
#else
    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD, 0x02, (timeout_tcp)/2, NULL);
#endif
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) {
        PERROR("kevent");
        uint8_t proto = get_protocol_from_var_lan(ident);
        uint16_t port = get_port_from_var_lan(ident);
        struct in_addr ip;
        ip.s_addr = get_ip_from_var_lan(ident);
        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "(%s:%d)protocol:%d:%s:%d\n", __FILE__, __LINE__, proto, inet_ntoa(ip), htons(port));
        l->output(buf);
        exit(-1);
    }
    return;
}

static inline void del_timer_event_udp(uint64_t ident)
{
    int ret;
    struct kevent kev;
    EV_SET(&kev, ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) {
        uint8_t proto = get_protocol_from_var_lan(ident);
        uint16_t port = get_port_from_var_lan(ident);
        struct in_addr ip;
        ip.s_addr = get_ip_from_var_lan(ident);
        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "(%s:%d)protocol:%d:%s:%d\n", __FILE__, __LINE__, proto, inet_ntoa(ip), htons(port));
        l->output(buf);
        PERROR("kevent");
        exit(-1);
    }
    return;
}

static inline void del_timer_event_tcp(uint64_t ident)
{
    int ret;
    struct kevent kev;
    EV_SET(&kev, ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    ret = kevent(kq, &kev, 1, NULL, 0, NULL);
    if (ret == -1) {
        PERROR("kevent");
        uint8_t proto = get_protocol_from_var_lan(ident);
        uint16_t port = get_port_from_var_lan(ident);
        struct in_addr ip;
        ip.s_addr = get_ip_from_var_lan(ident);
        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "(%s:%d)protocol:%d:%s:%d\n", __FILE__, __LINE__, proto, inet_ntoa(ip), htons(port));
        l->output(buf);
        exit(-1);
    }
    return;
}

static int count_node()
{
    int i = 0;
    struct node* table;
    RB_FOREACH(table, l2w, &l2w_head){
        i++;
    }
    return i;
}


static void dump_tree()
{
    int i = 1;
    struct node* table;
    printf("----------------------------------------\n");
    RB_FOREACH(table, l2w, &l2w_head){
        printf("count %d\n", i);
        print_node(table);
        i++;
    }

    if(i == 1) {
        printf("no entry\n");
    }

    printf("----------------------------------------\n\n");
}

#endif // STRUCTURE_H
