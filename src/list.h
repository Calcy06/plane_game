#ifndef _LIST_H_
#define _LIST_H_

#include <stddef.h>

struct list_head
{
    struct list_head* next;
    struct list_head* prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head* list)
{
    list->next = list;
    list->prev = list;
}

/* 获取包含链表节点的结构体地址 */
#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, type, member) \
    list_entry((pos)->member.next, type, member)

#define list_prev_entry(pos, type, member) \
    list_entry((pos)->member.prev, type, member)

/* 遍历链表节点 */
#define list_for_each(pos, head) \
    for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

/* 安全遍历 */
#define list_for_each_safe(pos, n, head) \
    for ((pos) = (head)->next, (n) = (pos)->next; \
         (pos) != (head); \
         (pos) = (n), (n) = (pos)->next)

/* 遍历结构体 */
#define list_for_each_entry(pos, head, type, member)             \
    for ((pos) = list_first_entry(head, type, member);           \
         &(pos)->member != (head);                               \
         (pos) = list_next_entry(pos, type, member))

/* 安全遍历结构体 */
#define list_for_each_entry_safe(pos, n, head, type, member)     \
    for ((pos) = list_first_entry(head, type, member),           \
         (n) = list_next_entry(pos, type, member);               \
         &(pos)->member != (head);                               \
         (pos) = (n),                                            \
         (n) = list_next_entry(n, type, member))

static inline void __list_add(
    struct list_head* new_node,
    struct list_head* prev,
    struct list_head* next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

static inline void list_add(
    struct list_head* new_node,
    struct list_head* head)
{
    __list_add(new_node, head, head->next);
}

static inline void list_add_tail(
    struct list_head* new_node,
    struct list_head* head)
{
    __list_add(new_node, head->prev, head);
}

static inline void __list_del(
    struct list_head* prev,
    struct list_head* next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head* entry)
{
    __list_del(entry->prev, entry->next);

    entry->next = NULL;
    entry->prev = NULL;
}

/* 判断链表是否为空 */
static inline int list_empty(const struct list_head* head)
{
    return head->next == head;
}

#endif