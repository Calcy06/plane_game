#include "plane_list.h"
#include <assert.h>

//// 定义全局链表头
//struct list_head plane_list;

// 添加节点
void add_node(int x, int y, int speed)
{
    // 分配内存并强转
    Node* s = (Node*)malloc(sizeof(Node));
    if (!s)
    {
        printf("malloc failed!\n");
        return;
    }
    assert(s);

    // 给节点赋值
    s->x = x;
    s->y = y;
    s->speed = speed;

    // 初始化节点内嵌链表节点
    INIT_LIST_HEAD(&s->list);

    // 尾插加入双向链表
    list_add_tail(&s->list, &plane_list);
}
