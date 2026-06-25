#ifndef _PLANE_LIST_H_
#define _PLANE_LIST_H_
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 全局链表头声明
extern struct list_head plane_list;
extern struct list_head myBull_list;
extern struct list_head enemyPlane_list;
extern struct list_head enemyBullet_list;
extern struct list_head bossBullet_list;
extern struct list_head explosion_list;
extern struct list_head supply_list;

// 实体类型
#define TYPE_ENEMY        1
#define TYPE_MY_BULLET    2
#define TYPE_ENEMY_BULLET 3
#define TYPE_BOSS_BULLET  4
#define TYPE_EXPLOSION    5
#define TYPE_SUPPLY       6

// 道具子类型
#define SUPPLY_BOMB   0  // 导弹道具
#define SUPPLY_BULLET 1  // 双弹道具

// 通用节点结构体
typedef struct NODE
{
    int x;
    int y;
    int speed;
    int hp;
    int maxHp;
    int damage;
    int type;
    int planeType;
    int shootTimer;
    double angle;
    int frame;
    int frameTimer;
    struct list_head list;
}Node;

void add_node(int x, int y, int speed);
#endif