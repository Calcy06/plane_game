#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "airplay.h"

// 初始化所有链表头
LIST_HEAD(plane_list);
LIST_HEAD(myBull_list);
LIST_HEAD(enemyPlane_list);
LIST_HEAD(enemyBullet_list);
LIST_HEAD(bossBullet_list);
LIST_HEAD(explosion_list);
LIST_HEAD(supply_list);

int main()
{
    start();
    return 0;
}