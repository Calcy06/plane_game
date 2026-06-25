#include "airplay.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <conio.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "msimg32.lib")

#define PI 3.1415926

// ==================== 全局变量 ====================
IMAGE imgBg[5];
IMAGE imgPlayer[3];
IMAGE imgEnemy[7];
IMAGE imgMyBullet[8];
IMAGE imgEnemyBullet[6];
IMAGE imgBoss[TOTAL_LEVEL];
IMAGE imgExplosion[7];
IMAGE imgStartBg;
IMAGE imgSupply[2];
IMAGE imgSuperPlane;

Node myPlane;
Node myPlane2;
int isEnd;
int score;
int totalScore = 0;     // 累计总分
int level;
int playerHp;
int playerMaxHp;
int bossActive;
Node boss;
int bgOffset;
int enemySpawnTimer;
int lastFrameTime;
int currentBuff;
DWORD buffEndTime;
int currentPlane = 0;

// 合体系统全局变量
int g_fusionActive = 0;
int g_fusionEnergyP1 = 0;
int g_fusionEnergyP2 = 0;
int g_fusionHp = 0;
int g_fusionMaxHp = 0;
DWORD g_fusionEndTime = 0;
DWORD g_fusionShootTimer = 0;
SuperPlane g_superPlane;

// 全屏缩放参数
float g_scale = 1.0f;
int g_offsetX = 0;
int g_offsetY = 0;

// 网络相关
int g_netMode = 0;
NetManager* g_netMgr = NULL;
int g_player2Plane = 1;

// ==================== 工具函数 ====================
void setDrawScale()
{
    setaspectratio(g_scale, g_scale);
    setorigin(g_offsetX, g_offsetY);
}

void putimageTransparent(int x, int y, IMAGE* srcImg, COLORREF transColor)
{
    HDC dstDC = GetImageHDC(NULL);
    HDC srcDC = GetImageHDC(srcImg);
    int w = srcImg->getwidth();
    int h = srcImg->getheight();
    TransparentBlt(dstDC, x, y, w, h, srcDC, 0, 0, w, h, transColor);
}

void waitForKeyPress()
{
    while (true)
    {
        bool anyDown = false;
        for (int i = 0x30; i <= 0x5A; i++)
        {
            if (GetAsyncKeyState(i) & 0x8000) { anyDown = true; break; }
        }
        if (!anyDown)
        {
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) anyDown = true;
            if (GetAsyncKeyState(VK_RETURN) & 0x8000) anyDown = true;
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) anyDown = true;
            if (GetAsyncKeyState(VK_UP) & 0x8000) anyDown = true;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) anyDown = true;
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) anyDown = true;
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) anyDown = true;
        }
        if (!anyDown) break;
        Sleep(10);
    }
    while (true)
    {
        for (int i = 0x30; i <= 0x5A; i++)
        {
            if (GetAsyncKeyState(i) & 0x8000) { Sleep(120); return; }
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) { Sleep(120); return; }
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) { Sleep(120); return; }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { Sleep(120); return; }
        if (GetAsyncKeyState(VK_UP) & 0x8000) { Sleep(120); return; }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) { Sleep(120); return; }
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) { Sleep(120); return; }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { Sleep(120); return; }
        Sleep(10);
    }
}

// ==================== 加载素材 ====================
void loadAllResources()
{
    char path[64];
    for (int i = 0; i < 5; i++)
    {
        sprintf_s(path, "graph\\img_bg_level_%d.jpg", i + 1);
        loadimage(&imgBg[i], path, BGWEIGHT, BGHEIGHT);
    }
    loadimage(&imgPlayer[0], "graph\\hero.png", myAirWEIGHT, myAirHEIGHT);
    loadimage(&imgPlayer[1], "graph\\hero2.png", myAirWEIGHT, myAirHEIGHT);
    loadimage(&imgPlayer[2], "graph\\app.png", myAirWEIGHT, myAirHEIGHT);

    // 加载超级战机素材
    loadimage(&imgSuperPlane, "graph\\super.png", SUPER_WIDTH, SUPER_HEIGHT);

    for (int i = 0; i < 7; i++)
    {
        sprintf_s(path, "graph\\img-plane_%d.png", i + 1);
        loadimage(&imgEnemy[i], path, enemyPlaneWEIGHT, enemyPlaneHEIGHT);
    }
    for (int i = 0; i < 7; i++)
    {
        sprintf_s(path, "graph\\bullet_%d.png", i + 7);
        loadimage(&imgMyBullet[i], path, BulletWEIGHT, BulletHEIGHT);
    }
    sprintf_s(path, "graph\\bullet_14.png");
    loadimage(&imgMyBullet[7], path, MISSILE_WIDTH, MISSILE_HEIGHT);
    for (int i = 0; i < 6; i++)
    {
        sprintf_s(path, "graph\\bullet_%d.png", i + 1);
        loadimage(&imgEnemyBullet[i], path, BulletWEIGHT, BulletHEIGHT);
    }
    for (int i = 0; i < TOTAL_LEVEL; i++)
    {
        sprintf_s(path, "graph\\boss%d.png", i + 1);
        loadimage(&imgBoss[i], path, BOSS_WIDTH, BOSS_HEIGHT);
    }
    for (int i = 0; i < 7; i++)
    {
        sprintf_s(path, "graph\\bomb-%d.png", i + 1);
        loadimage(&imgExplosion[i], path, 60, 60);
    }
    loadimage(&imgStartBg, "graph\\bk.png", BGWEIGHT, BGHEIGHT);
    loadimage(&imgSupply[0], "graph\\bomb_supply.png", SUPPLY_WIDTH, SUPPLY_HEIGHT);
    loadimage(&imgSupply[1], "graph\\bullet_supply.png", SUPPLY_WIDTH, SUPPLY_HEIGHT);
    mciSendString("open graph\\bg.wav alias bgm", 0, 0, 0);
    mciSendString("open graph\\bomb.wav alias bomb", 0, 0, 0);
    mciSendString("play bgm", 0, 0, 0);
}

// ==================== 关卡初始化 ====================
void initLevel(int lv)
{
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &enemyPlane_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &myBull_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &enemyBullet_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &bossBullet_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &explosion_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &supply_list, Node, list) { list_del(&pos->list); free(pos); }

    myPlane.x = BGWEIGHT / 2 - myAirWEIGHT / 2 - 60;
    myPlane.y = BGHEIGHT - myAirHEIGHT - 10;
    myPlane.speed = 10;

    myPlane2.x = BGWEIGHT / 2 - myAirWEIGHT / 2 + 60;
    myPlane2.y = BGHEIGHT - myAirHEIGHT - 10;
    myPlane2.speed = 10;

    // 初始化合体相关变量
    g_fusionActive = 0;
    g_fusionEnergyP1 = 0;
    g_fusionEnergyP2 = 0;
    g_fusionHp = 0;
    g_fusionMaxHp = 0;
    g_fusionEndTime = 0;
    g_fusionShootTimer = 0;

    // ========== 血量上限逐关递增（增量越来越大） ==========
    // 第1关：500（基础）
    // 第2关：530（+30）
    // 第3关：580（+50，比上一关多+20）
    // 第4关：650（+70，比上一关多+20）
    // 第5关：740（+90，比上一关多+20）
    int totalHpAdd = (lv - 1) * 30 + (lv - 1) * (lv - 2) * 10;
    playerMaxHp = PLAYER_INIT_HP + totalHpAdd;

    if (lv == 1)
    {
        playerHp = PLAYER_INIT_HP;
    }
    else
    {
        // 计算上一关的总血量增量
        int prevTotalHpAdd = (lv - 2) * 30 + (lv - 2) * (lv - 3) * 10;
        // 这一关新增的血量 = 总增量 - 上一关总增量
        int thisLevelAdd = totalHpAdd - prevTotalHpAdd;
        // 当前血量加上本关新增的，不超过上限
        playerHp += thisLevelAdd;
        if (playerHp > playerMaxHp)
            playerHp = playerMaxHp;
    }
    // =====================================================

    score = 0;
    isEnd = 0;
    bossActive = 0;
    bgOffset = 0;
    enemySpawnTimer = 0;
    lastFrameTime = GetTickCount();
    currentBuff = 0;
    buffEndTime = 0;
    boss.x = BGWEIGHT / 2 - BOSS_WIDTH / 2;
    boss.y = 50;
    boss.hp = bossHpList[lv - 1];
    boss.maxHp = bossHpList[lv - 1];
    boss.shootTimer = 0;
    boss.speed = lv;
    boss.type = 0;
}

// ==================== 背景滚动 ====================
void updateBgScroll()
{
    bgOffset += 2;
    if (bgOffset >= BGHEIGHT)
        bgOffset = 0;
}

// ==================== 生成敌机 ====================
void createEnemy()
{
    // 生成间隔：第1关1050ms，每关减少190ms
    int spawnInterval = 1050 - (level - 1) * 190;
    if (bossActive) spawnInterval *= 3;
    if (spawnInterval < 300) spawnInterval = 300;

    int now = GetTickCount();
    if (now - enemySpawnTimer < spawnInterval) return;
    enemySpawnTimer = now;

    int typeCount = level + 2;
    if (typeCount > 7) typeCount = 7;
    int type = rand() % typeCount;

    Node* enemy = (Node*)malloc(sizeof(Node));
    assert(enemy);
    enemy->x = rand() % (BGWEIGHT - enemyPlaneWEIGHT);
    enemy->y = -enemyPlaneHEIGHT;

    // 敌机速度：大部分2~4，只有1/4概率出现快速敌机（比之前更少了）
    enemy->speed = rand() % 3 + 1 + level;
    if (rand() % 4 == 0)  // 从1/3降到1/4，快速敌机更少
        enemy->speed++;

    // 敌机血量倍率：第1关0.9倍，每关增加0.55倍
    double hpRatio = 0.9 + (level - 1) * 0.55;
    enemy->hp = (int)(enemyScore[type] * hpRatio);
    enemy->maxHp = enemy->hp;

    enemy->damage = 0;
    enemy->type = TYPE_ENEMY;
    enemy->planeType = type;
    enemy->shootTimer = GetTickCount();
    enemy->angle = 0;
    INIT_LIST_HEAD(&enemy->list);
    list_add_tail(&enemy->list, &enemyPlane_list);
}

// ==================== 敌机射击 ====================
void enemyShoot(Node* enemy)
{
    // 射击间隔：第1关1280ms，每关减少240ms
    int shootInterval = 1280 - (level - 1) * 240;
    if (shootInterval < 320) shootInterval = 320;

    int now = GetTickCount();
    if (now - enemy->shootTimer < shootInterval) return;
    enemy->shootTimer = now;

    int bulletType = enemy->planeType;
    if (bulletType >= 6) bulletType = 5;

    Node* bullet = (Node*)malloc(sizeof(Node));
    assert(bullet);
    bullet->x = enemy->x + enemyPlaneWEIGHT / 2 - BulletWEIGHT / 2;
    bullet->y = enemy->y + enemyPlaneHEIGHT;

    // 子弹速度：第1关5，每关增加1.3（比之前慢一点）
    bullet->speed = 4 + level + (level + 1) / 3;

    bullet->damage = enemyBulletDamage[bulletType];
    bullet->type = TYPE_ENEMY_BULLET;
    bullet->planeType = bulletType;
    bullet->angle = PI / 2;
    INIT_LIST_HEAD(&bullet->list);
    list_add_tail(&bullet->list, &enemyBullet_list);
}

// ==================== 道具生成 ====================
void createSupply(int x, int y, int type)
{
    Node* supply = (Node*)malloc(sizeof(Node));
    assert(supply);
    supply->x = x;
    supply->y = y;
    supply->speed = 2;
    supply->type = TYPE_SUPPLY;
    supply->planeType = type;
    INIT_LIST_HEAD(&supply->list);
    list_add_tail(&supply->list, &supply_list);
}

void updateSupply()
{
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &supply_list, Node, list)
    {
        pos->y += pos->speed;
        if (pos->y > BGHEIGHT)
        {
            list_del(&pos->list);
            free(pos);
            continue;
        }
        // 合体状态下：超级战机拾取（对整体生效）
        if (g_fusionActive && g_netMode == 1)
        {
            int sx = pos->x + SUPPLY_WIDTH / 2;
            int sy = pos->y + SUPPLY_HEIGHT / 2;
            int px = g_superPlane.x + SUPER_WIDTH / 2;
            int py = g_superPlane.y + SUPER_HEIGHT / 2;
            if (abs(px - sx) < SUPER_WIDTH / 2 + SUPPLY_WIDTH / 2 &&
                abs(py - sy) < SUPER_HEIGHT / 2 + SUPPLY_HEIGHT / 2)
            {
                currentBuff = pos->planeType + 1;
                buffEndTime = GetTickCount() + BUFF_DURATION;
                list_del(&pos->list);
                free(pos);
                continue;
            }
        }
        else
        {
            // 玩家1拾取
            int px = myPlane.x + myAirWEIGHT / 2;
            int py = myPlane.y + myAirHEIGHT / 2;
            int sx = pos->x + SUPPLY_WIDTH / 2;
            int sy = pos->y + SUPPLY_HEIGHT / 2;
            if (abs(px - sx) < myAirWEIGHT / 2 + SUPPLY_WIDTH / 2 &&
                abs(py - sy) < myAirHEIGHT / 2 + SUPPLY_HEIGHT / 2)
            {
                currentBuff = pos->planeType + 1;
                buffEndTime = GetTickCount() + BUFF_DURATION;
                list_del(&pos->list);
                free(pos);
                continue;
            }
            // 玩家2拾取（仅主机模式检测）
            if (g_netMode == 1)
            {
                int px2 = myPlane2.x + myAirWEIGHT / 2;
                int py2 = myPlane2.y + myAirHEIGHT / 2;
                if (abs(px2 - sx) < myAirWEIGHT / 2 + SUPPLY_WIDTH / 2 &&
                    abs(py2 - sy) < myAirHEIGHT / 2 + SUPPLY_HEIGHT / 2)
                {
                    currentBuff = pos->planeType + 1;
                    buffEndTime = GetTickCount() + BUFF_DURATION;
                    list_del(&pos->list);
                    free(pos);
                }
            }
        }
    }
    if (currentBuff != 0 && GetTickCount() > buffEndTime)
    {
        currentBuff = 0;
    }
}

// ==================== 更新敌机子弹 ====================
void updateEnemyBullets()
{
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &enemyBullet_list, Node, list)
    {
        pos->x += (int)(cos(pos->angle) * pos->speed);
        pos->y += (int)(sin(pos->angle) * pos->speed);
        if (pos->y > BGHEIGHT || pos->x < -BulletWEIGHT || pos->x > BGWEIGHT)
        {
            list_del(&pos->list);
            free(pos);
            continue;
        }
        // 合体状态下：超级战机碰撞检测
        if (g_fusionActive && g_netMode == 1)
        {
            if (superPlane_bulletConflict(pos))
            {
                g_fusionHp -= pos->damage;
                list_del(&pos->list);
                free(pos);
                if (g_fusionHp <= 0)
                {
                    g_fusionHp = 0;
                    endFusion();
                }
                continue;
            }
        }
        else
        {
            // 玩家1碰撞
            int px = myPlane.x + myAirWEIGHT / 2;
            int py = myPlane.y + myAirHEIGHT / 2;
            int bx = pos->x + BulletWEIGHT / 2;
            int by = pos->y + BulletHEIGHT / 2;
            if (abs(px - bx) < myAirWEIGHT / 2 + BulletWEIGHT / 2 &&
                abs(py - by) < myAirHEIGHT / 2 + BulletHEIGHT / 2)
            {
                playerHp -= pos->damage;
                list_del(&pos->list);
                free(pos);
                if (playerHp <= 0)
                {
                    playerHp = 0;
                    isEnd = 1;
                }
                continue;
            }
            // 玩家2碰撞（仅主机模式）
            if (g_netMode == 1)
            {
                int px2 = myPlane2.x + myAirWEIGHT / 2;
                int py2 = myPlane2.y + myAirHEIGHT / 2;
                if (abs(px2 - bx) < myAirWEIGHT / 2 + BulletWEIGHT / 2 &&
                    abs(py2 - by) < myAirHEIGHT / 2 + BulletHEIGHT / 2)
                {
                    playerHp -= pos->damage;
                    list_del(&pos->list);
                    free(pos);
                    if (playerHp <= 0)
                    {
                        playerHp = 0;
                        isEnd = 1;
                    }
                }
            }
        }
    }
}

// ==================== Boss AI ====================
void bossAI()
{
    if (!bossActive) return;
    int now = GetTickCount();

    // Boss左右移动（第3关开始）
    if (level >= 3)
    {
        static int dir = 1;
        boss.x += boss.speed * dir;
        if (boss.x <= 0 || boss.x + BOSS_WIDTH >= BGWEIGHT)
            dir = -dir;
    }

    // 射击间隔：第1关950ms，每关减少170ms
    int shootInterval = 950 - (level - 1) * 170;
    if (shootInterval < 240) shootInterval = 240;

    // 第5关半血后狂暴模式
    if (level == 5 && boss.hp < boss.maxHp / 2)
        shootInterval = 180;  // 从150回调到180

    if (now - boss.shootTimer < shootInterval) return;
    boss.shootTimer = now;

    switch (level)
    {
    case 1:
    {
        Node* b = (Node*)malloc(sizeof(Node));
        b->x = boss.x + BOSS_WIDTH / 2 - BulletWEIGHT / 2;
        b->y = boss.y + BOSS_HEIGHT;
        b->speed = 6;  // 回调
        b->damage = enemyBulletDamage[level];
        b->type = TYPE_BOSS_BULLET;
        b->angle = PI / 2;
        INIT_LIST_HEAD(&b->list);
        list_add_tail(&b->list, &bossBullet_list);
        break;
    }
    case 2:
    {
        double angles[] = { PI / 2 - 0.26, PI / 2, PI / 2 + 0.26 };
        for (int i = 0; i < 3; i++)
        {
            Node* b = (Node*)malloc(sizeof(Node));
            b->x = boss.x + BOSS_WIDTH / 2 - BulletWEIGHT / 2;
            b->y = boss.y + BOSS_HEIGHT;
            b->speed = 6;
            b->damage = enemyBulletDamage[level];
            b->type = TYPE_BOSS_BULLET;
            b->angle = angles[i];
            INIT_LIST_HEAD(&b->list);
            list_add_tail(&b->list, &bossBullet_list);
        }
        break;
    }
    case 3:
    {
        double angles[] = { PI / 2 - 0.52, PI / 2 - 0.26, PI / 2, PI / 2 + 0.26, PI / 2 + 0.52 };
        for (int i = 0; i < 5; i++)
        {
            Node* b = (Node*)malloc(sizeof(Node));
            b->x = boss.x + BOSS_WIDTH / 2 - BulletWEIGHT / 2;
            b->y = boss.y + BOSS_HEIGHT;
            b->speed = 6;
            b->damage = enemyBulletDamage[level];
            b->type = TYPE_BOSS_BULLET;
            b->angle = angles[i];
            INIT_LIST_HEAD(&b->list);
            list_add_tail(&b->list, &bossBullet_list);
        }
        break;
    }
    case 4:
    {
        for (int i = -2; i <= 2; i++)
        {
            Node* b = (Node*)malloc(sizeof(Node));
            b->x = boss.x + BOSS_WIDTH / 2 - BulletWEIGHT / 2;
            b->y = boss.y + BOSS_HEIGHT;
            b->speed = 5;
            b->damage = enemyBulletDamage[level];
            b->type = TYPE_BOSS_BULLET;
            b->angle = PI / 2 + i * 0.15;
            INIT_LIST_HEAD(&b->list);
            list_add_tail(&b->list, &bossBullet_list);
        }
        // 追踪弹
        Node* b = (Node*)malloc(sizeof(Node));
        b->x = boss.x + BOSS_WIDTH / 2 - BulletWEIGHT / 2;
        b->y = boss.y + BOSS_HEIGHT;
        b->speed = 4;
        b->damage = enemyBulletDamage[level] + 8;
        b->type = TYPE_BOSS_BULLET;
        double dx, dy;
        if (g_fusionActive)
        {
            dx = (g_superPlane.x + SUPER_WIDTH / 2) - (boss.x + BOSS_WIDTH / 2);
            dy = (g_superPlane.y + SUPER_HEIGHT / 2) - (boss.y + BOSS_HEIGHT);
        }
        else
        {
            dx = (myPlane.x + myAirWEIGHT / 2) - (boss.x + BOSS_WIDTH / 2);
            dy = (myPlane.y + myAirHEIGHT / 2) - (boss.y + BOSS_HEIGHT);
        }
        b->angle = atan2(dy, dx);
        INIT_LIST_HEAD(&b->list);
        list_add_tail(&b->list, &bossBullet_list);
        break;
    }
    case 5:
    {
        int cnt = (boss.hp < boss.maxHp / 2) ? 14 : 9;  // 回调
        for (int i = 0; i < cnt; i++)
        {
            Node* b = (Node*)malloc(sizeof(Node));
            b->x = boss.x + BOSS_WIDTH / 2 - BulletWEIGHT / 2;
            b->y = boss.y + BOSS_HEIGHT / 2;
            b->speed = 5;
            b->damage = enemyBulletDamage[5];
            b->type = TYPE_BOSS_BULLET;
            b->angle = 2 * PI * i / cnt;
            INIT_LIST_HEAD(&b->list);
            list_add_tail(&b->list, &bossBullet_list);
        }
        break;
    }
    }
}

// ==================== 更新Boss子弹 ====================
void updateBossBullets()
{
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &bossBullet_list, Node, list)
    {
        pos->x += (int)(cos(pos->angle) * pos->speed);
        pos->y += (int)(sin(pos->angle) * pos->speed);
        if (pos->y > BGHEIGHT || pos->y < -BulletHEIGHT ||
            pos->x < -BulletWEIGHT || pos->x > BGWEIGHT)
        {
            list_del(&pos->list);
            free(pos);
            continue;
        }
        // 合体状态下：超级战机碰撞检测
        if (g_fusionActive && g_netMode == 1)
        {
            if (superPlane_bulletConflict(pos))
            {
                g_fusionHp -= pos->damage;
                list_del(&pos->list);
                free(pos);
                if (g_fusionHp <= 0)
                {
                    g_fusionHp = 0;
                    endFusion();
                }
                continue;
            }
        }
        else
        {
            // 玩家1碰撞
            int px = myPlane.x + myAirWEIGHT / 2;
            int py = myPlane.y + myAirHEIGHT / 2;
            int bx = pos->x + BulletWEIGHT / 2;
            int by = pos->y + BulletHEIGHT / 2;
            if (abs(px - bx) < myAirWEIGHT / 2 + BulletWEIGHT / 2 &&
                abs(py - by) < myAirHEIGHT / 2 + BulletHEIGHT / 2)
            {
                playerHp -= pos->damage;
                list_del(&pos->list);
                free(pos);
                if (playerHp <= 0)
                {
                    playerHp = 0;
                    isEnd = 1;
                }
                continue;
            }
            // 玩家2碰撞（仅主机模式）
            if (g_netMode == 1)
            {
                int px2 = myPlane2.x + myAirWEIGHT / 2;
                int py2 = myPlane2.y + myAirHEIGHT / 2;
                if (abs(px2 - bx) < myAirWEIGHT / 2 + BulletWEIGHT / 2 &&
                    abs(py2 - by) < myAirHEIGHT / 2 + BulletHEIGHT / 2)
                {
                    playerHp -= pos->damage;
                    list_del(&pos->list);
                    free(pos);
                    if (playerHp <= 0)
                    {
                        playerHp = 0;
                        isEnd = 1;
                    }
                }
            }
        }
    }
}

// ==================== 爆炸效果 ====================
void createExplosion(int x, int y)
{
    Node* exp = (Node*)malloc(sizeof(Node));
    exp->x = x - 30;
    exp->y = y - 30;
    exp->frame = 0;
    exp->frameTimer = GetTickCount();
    exp->type = TYPE_EXPLOSION;
    INIT_LIST_HEAD(&exp->list);
    list_add_tail(&exp->list, &explosion_list);
    mciSendString("play bomb from 0", 0, 0, 0);
}

void updateExplosion()
{
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &explosion_list, Node, list)
    {
        int now = GetTickCount();
        if (now - pos->frameTimer > 50)
        {
            pos->frame++;
            pos->frameTimer = now;
        }
        if (pos->frame >= 7)
        {
            list_del(&pos->list);
            free(pos);
        }
    }
}

// ==================== 合体系统核心函数 ====================
void addFusionEnergy(int amount)
{
    if (g_fusionActive) return;
    g_fusionEnergyP1 += amount;
    g_fusionEnergyP2 += amount;
    if (g_fusionEnergyP1 > FUSION_ENERGY_MAX) g_fusionEnergyP1 = FUSION_ENERGY_MAX;
    if (g_fusionEnergyP2 > FUSION_ENERGY_MAX) g_fusionEnergyP2 = FUSION_ENERGY_MAX;
}

int isFusionReady()
{
    return g_fusionEnergyP1 >= FUSION_ENERGY_MAX &&
        g_fusionEnergyP2 >= FUSION_ENERGY_MAX &&
        !g_fusionActive;
}

int superPlane_isConflict(Node* enemy)
{
    int px = g_superPlane.x + SUPER_WIDTH / 2;
    int py = g_superPlane.y + SUPER_HEIGHT / 2;
    int ex = enemy->x + enemyPlaneWEIGHT / 2;
    int ey = enemy->y + enemyPlaneHEIGHT / 2;
    return abs(px - ex) < SUPER_WIDTH / 2 + enemyPlaneWEIGHT / 2 &&
        abs(py - ey) < SUPER_HEIGHT / 2 + enemyPlaneHEIGHT / 2;
}

int superPlane_bulletConflict(Node* bullet)
{
    int bw = BulletWEIGHT;
    int bh = BulletHEIGHT;
    int px = g_superPlane.x + SUPER_WIDTH / 2;
    int py = g_superPlane.y + SUPER_HEIGHT / 2;
    int bx = bullet->x + bw / 2;
    int by = bullet->y + bh / 2;
    return abs(px - bx) < SUPER_WIDTH / 2 + bw / 2 &&
        abs(py - by) < SUPER_HEIGHT / 2 + bh / 2;
}

void playFusionAnimation()
{
    IMAGE saveScreen;
    getimage(&saveScreen, 0, 0, BGWEIGHT, BGHEIGHT);

    int startX1 = myPlane.x;
    int startY1 = myPlane.y;
    int startX2 = myPlane2.x;
    int startY2 = myPlane2.y;

    int targetX = BGWEIGHT / 2 - myAirWEIGHT / 2;
    int targetY = BGHEIGHT / 2 - myAirHEIGHT / 2;

    int totalFrames = 60;

    for (int frame = 0; frame < totalFrames; frame++)
    {
        if (g_netMode != 0 && !g_netMgr->isConnected())
        {
            MessageBox(GetForegroundWindow(), "连接断开！", "网络错误", MB_OK);
            isEnd = 1;
            return;
        }

        float t = (float)frame / totalFrames;

        int curX1 = startX1 + (int)((targetX - 60 - startX1) * t);
        int curY1 = startY1 + (int)((targetY - startY1) * t);
        int curX2 = startX2 + (int)((targetX + 60 - startX2) * t);
        int curY2 = startY2 + (int)((targetY - startY2) * t);

        BeginBatchDraw();
        setDrawScale();
        putimage(0, 0, &saveScreen);

        putimageTransparent(curX1, curY1, &imgPlayer[currentPlane]);
        putimageTransparent(curX2, curY2, &imgPlayer[g_player2Plane]);

        int alpha = (int)(200 * t);
        if (alpha > 200) alpha = 200;
        setfillcolor(WHITE);
        HDC hdc = GetImageHDC(NULL);
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)alpha, 0 };
        AlphaBlend(hdc, targetX - 30, targetY - 30,
            myAirWEIGHT + 60, myAirHEIGHT + 60,
            hdc, targetX - 30, targetY - 30,
            myAirWEIGHT + 60, myAirHEIGHT + 60, bf);

        if (frame > 30)
        {
            settextstyle(36, 0, "宋体");
            settextcolor(YELLOW);
            setbkmode(TRANSPARENT);
            int textX = (BGWEIGHT - textwidth("合体！")) / 2;
            outtextxy(textX, 150, "合体！");
        }

        EndBatchDraw();
        Sleep(20);
    }

    for (int i = 0; i < 10; i++)
    {
        BeginBatchDraw();
        setDrawScale();
        setfillcolor(WHITE);
        solidrectangle(0, 0, BGWEIGHT, BGHEIGHT);
        EndBatchDraw();
        Sleep(30);
    }
}

void startFusion()
{
    if (!isFusionReady() || g_fusionActive) return;

    playFusionAnimation();

    if (isEnd) return;

    g_superPlane.x = BGWEIGHT / 2 - SUPER_WIDTH / 2;
    g_superPlane.y = BGHEIGHT / 2 - SUPER_HEIGHT / 2;
    g_superPlane.speed = (int)(myPlane.speed * FUSION_SPEED_BOOST);

    g_fusionHp = playerHp * 2;
    g_fusionMaxHp = playerMaxHp * 2;

    g_fusionActive = 1;
    g_fusionEndTime = GetTickCount() + FUSION_DURATION;
    g_fusionShootTimer = 0;

    g_fusionEnergyP1 = 0;
    g_fusionEnergyP2 = 0;
}

void endFusion()
{
    if (!g_fusionActive) return;

    g_fusionActive = 0;

    int splitHp = g_fusionHp / 2;
    if (splitHp < 1) splitHp = 1;
    playerHp = splitHp;

    myPlane.x = g_superPlane.x - 30;
    myPlane.y = g_superPlane.y + SUPER_HEIGHT / 2 - myAirHEIGHT / 2;
    myPlane2.x = g_superPlane.x + SUPER_WIDTH + 30 - myAirWEIGHT;
    myPlane2.y = g_superPlane.y + SUPER_HEIGHT / 2 - myAirHEIGHT / 2;

    if (myPlane.x < 0) myPlane.x = 0;
    if (myPlane.x + myAirWEIGHT > BGWEIGHT) myPlane.x = BGWEIGHT - myAirWEIGHT;
    if (myPlane2.x < 0) myPlane2.x = 0;
    if (myPlane2.x + myAirWEIGHT > BGWEIGHT) myPlane2.x = BGWEIGHT - myAirWEIGHT;
}

void updateFusion()
{
    if (!g_fusionActive) return;

    if (GetTickCount() > g_fusionEndTime)
    {
        endFusion();
        return;
    }
}

void superPlaneShoot()
{
    if (!g_fusionActive) return;

    int now = GetTickCount();
    int shootInterval = (int)(150 / FUSION_FIRE_BOOST);

    if (now - g_fusionShootTimer < shootInterval) return;
    g_fusionShootTimer = now;

    int bulletType = level - 1;
    if (bulletType >= 8) bulletType = 7;

    if (currentBuff == 1)
    {
        Node* myBull = (Node*)malloc(sizeof(Node));
        myBull->x = g_superPlane.x + SUPER_WIDTH / 2 - MISSILE_WIDTH / 2;
        myBull->y = g_superPlane.y;
        myBull->speed = 8;
        myBull->damage = (int)(MISSILE_DAMAGE * FUSION_DAMAGE_BOOST);
        myBull->type = TYPE_MY_BULLET;
        myBull->planeType = 7;
        INIT_LIST_HEAD(&myBull->list);
        list_add_tail(&myBull->list, &myBull_list);
    }
    else if (currentBuff == 2)
    {
        Node* left = (Node*)malloc(sizeof(Node));
        left->x = g_superPlane.x + 10;
        left->y = g_superPlane.y;
        left->speed = 8;
        left->damage = (int)(myBulletDamage[bulletType] * FUSION_DAMAGE_BOOST);
        left->type = TYPE_MY_BULLET;
        left->planeType = bulletType;
        INIT_LIST_HEAD(&left->list);
        list_add_tail(&left->list, &myBull_list);

        Node* right = (Node*)malloc(sizeof(Node));
        right->x = g_superPlane.x + SUPER_WIDTH - 10 - BulletWEIGHT;
        right->y = g_superPlane.y;
        right->speed = 8;
        right->damage = (int)(myBulletDamage[bulletType] * FUSION_DAMAGE_BOOST);
        right->type = TYPE_MY_BULLET;
        right->planeType = bulletType;
        INIT_LIST_HEAD(&right->list);
        list_add_tail(&right->list, &myBull_list);
    }
    else
    {
        Node* myBull = (Node*)malloc(sizeof(Node));
        myBull->x = g_superPlane.x + SUPER_WIDTH / 2 - BulletWEIGHT / 2;
        myBull->y = g_superPlane.y;
        myBull->speed = 8;
        myBull->damage = (int)(myBulletDamage[bulletType] * FUSION_DAMAGE_BOOST);
        myBull->type = TYPE_MY_BULLET;
        myBull->planeType = bulletType;
        INIT_LIST_HEAD(&myBull->list);
        list_add_tail(&myBull->list, &myBull_list);
    }
}

// ==================== UI绘制 ====================
void drawUI()
{
    setbkcolor(0xcccccc);
    settextcolor(BLACK);
    settextstyle(16, 0, "宋体");
    setbkmode(OPAQUE);
    TCHAR str[64];
    wsprintf(str, "本关: %d", score);
    outtextxy(0, 0, str);
    wsprintf(str, "总分: %d", totalScore + score);
    outtextxy(0, 20, str);
    wsprintf(str, "关卡: %d", level);
    outtextxy(0, 40, str);

    if (g_fusionActive)
    {
        wsprintf(str, "合体HP: %d/%d", g_fusionHp, g_fusionMaxHp);
        outtextxy(0, 60, str);
        int remain = (g_fusionEndTime - GetTickCount()) / 1000;
        if (remain < 0) remain = 0;
        wsprintf(str, "合体剩余: %ds", remain);
        outtextxy(0, 80, str);
        settextcolor(RED);
        outtextxy(0, 100, "【合体模式】主机移动，客机射击");
    }
    else
    {
        wsprintf(str, "HP: %d/%d", playerHp, playerMaxHp);
        outtextxy(0, 60, str);
    }

    if (currentBuff != 0)
    {
        int remain = (buffEndTime - GetTickCount()) / 1000;
        if (remain < 0) remain = 0;
        if (currentBuff == 1)
            wsprintf(str, "导弹: %ds", remain);
        else
            wsprintf(str, "双弹: %ds", remain);
        outtextxy(0, 120, str);
    }

    if (g_netMode != 0)
    {
        settextcolor(BLUE);
        if (g_netMode == 1)
            outtextxy(0, 140, "模式: 主机");
        else
            outtextxy(0, 140, "模式: 客机");
    }

    // 绘制合体能量条（双人模式下显示）
    if (g_netMode != 0 && !g_fusionActive)
    {
        int barX = 280;
        int barY = 10;
        int barW = 110;
        int barH = 12;

        setfillcolor(0x333333);
        solidrectangle(barX, barY, barX + barW, barY + barH);
        int energyW1 = (int)(barW * ((double)g_fusionEnergyP1 / FUSION_ENERGY_MAX));
        setfillcolor(0x00BFFF);
        solidrectangle(barX, barY, barX + energyW1, barY + barH);
        settextcolor(WHITE);
        settextstyle(10, 0, "宋体");
        outtextxy(barX + 2, barY + 1, "P1能量");

        barY += 16;
        setfillcolor(0x333333);
        solidrectangle(barX, barY, barX + barW, barY + barH);
        int energyW2 = (int)(barW * ((double)g_fusionEnergyP2 / FUSION_ENERGY_MAX));
        setfillcolor(0xFF6347);
        solidrectangle(barX, barY, barX + energyW2, barY + barH);
        settextcolor(WHITE);
        outtextxy(barX + 2, barY + 1, "P2能量");

        if (isFusionReady())
        {
            settextcolor(YELLOW);
            settextstyle(12, 0, "宋体");
            outtextxy(barX, barY + 18, "按X键合体！");
        }
    }

    if (bossActive)
    {
        setfillcolor(RED);
        solidrectangle(50, 570, 350, 585);
        setfillcolor(GREEN);
        int hpW = (int)(300 * ((double)boss.hp / boss.maxHp));
        solidrectangle(50, 570, 50 + hpW, 585);
        settextcolor(WHITE);
        outtextxy(170, 570, "BOSS");
    }
}

// ==================== 过关界面 ====================
void showLevelPass()
{
    // 通关时把本关分数累加到总分
    totalScore += score;

    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    settextstyle(36, 0, "宋体");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    TCHAR str[64];
    int x = (BGWEIGHT - textwidth("关卡通过！")) / 2;
    outtextxy(x, 170, "关卡通过！");

    settextstyle(24, 0, "宋体");
    wsprintf(str, "本关得分：%d", score);
    x = (BGWEIGHT - textwidth(str)) / 2;
    outtextxy(x, 240, str);

    settextcolor(YELLOW);
    wsprintf(str, "累计得分：%d", totalScore);
    x = (BGWEIGHT - textwidth(str)) / 2;
    outtextxy(x, 280, str);

    settextcolor(WHITE);
    settextstyle(20, 0, "宋体");
    wsprintf(str, "剩余血量：%d", playerHp);
    x = (BGWEIGHT - textwidth(str)) / 2;
    outtextxy(x, 330, str);

    settextstyle(20, 0, "宋体");
    x = (BGWEIGHT - textwidth("按任意键进入下一关")) / 2;
    outtextxy(x, 400, "按任意键进入下一关");
    EndBatchDraw();
    waitForKeyPress();
}

// ==================== 通关界面 ====================
void showGameWin()
{
    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    settextstyle(42, 0, "宋体");
    settextcolor(YELLOW);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth("恭喜通关！")) / 2;
    outtextxy(x, 160, "恭喜通关！");

    settextstyle(24, 0, "宋体");
    settextcolor(WHITE);
    TCHAR str[64];
    wsprintf(str, "最终得分：%d", totalScore);
    x = (BGWEIGHT - textwidth(str)) / 2;
    outtextxy(x, 240, str);

    settextstyle(18, 0, "宋体");
    x = (BGWEIGHT - textwidth("按任意键返回开始界面")) / 2;
    outtextxy(x, 330, "按任意键返回开始界面");
    EndBatchDraw();
    waitForKeyPress();
}

// ==================== 基础函数 ====================
void init()
{
    static bool resourceLoaded = false;
    if (!resourceLoaded)
    {
        loadAllResources();
        resourceLoaded = true;
    }
    srand((unsigned int)time(NULL));
    totalScore = 0;   // 新游戏重置累计分数
    level = 1;
    initLevel(1);
}

void create_myBull_from(int x, int y)
{
    int bulletType = level - 1;
    if (bulletType >= 8) bulletType = 7;
    if (currentBuff == 1)
    {
        Node* myBull = (Node*)malloc(sizeof(Node));
        myBull->x = x + myAirWEIGHT / 2 - MISSILE_WIDTH / 2;
        myBull->y = y;
        myBull->speed = 8;
        myBull->damage = MISSILE_DAMAGE;
        myBull->type = TYPE_MY_BULLET;
        myBull->planeType = 7;
        INIT_LIST_HEAD(&myBull->list);
        list_add_tail(&myBull->list, &myBull_list);
    }
    else if (currentBuff == 2)
    {
        Node* left = (Node*)malloc(sizeof(Node));
        left->x = x + 8;
        left->y = y;
        left->speed = 8;
        left->damage = myBulletDamage[bulletType];
        left->type = TYPE_MY_BULLET;
        left->planeType = bulletType;
        INIT_LIST_HEAD(&left->list);
        list_add_tail(&left->list, &myBull_list);
        Node* right = (Node*)malloc(sizeof(Node));
        right->x = x + myAirWEIGHT - 8 - BulletWEIGHT;
        right->y = y;
        right->speed = 8;
        right->damage = myBulletDamage[bulletType];
        right->type = TYPE_MY_BULLET;
        right->planeType = bulletType;
        INIT_LIST_HEAD(&right->list);
        list_add_tail(&right->list, &myBull_list);
    }
    else
    {
        Node* myBull = (Node*)malloc(sizeof(Node));
        myBull->x = x + myAirWEIGHT / 2 - BulletWEIGHT / 2;
        myBull->y = y;
        myBull->speed = 8;
        myBull->damage = myBulletDamage[bulletType];
        myBull->type = TYPE_MY_BULLET;
        myBull->planeType = bulletType;
        INIT_LIST_HEAD(&myBull->list);
        list_add_tail(&myBull->list, &myBull_list);
    }
}

void create_myBull()
{
    create_myBull_from(myPlane.x, myPlane.y);
}

// ==================== 绘制函数 ====================
void DrawMap()
{
    BeginBatchDraw();
    setDrawScale();
    putimage(0, bgOffset, &imgBg[level - 1]);
    putimage(0, bgOffset - BGHEIGHT, &imgBg[level - 1]);

    if (g_fusionActive)
    {
        putimageTransparent(g_superPlane.x, g_superPlane.y, &imgSuperPlane);
    }
    else
    {
        putimageTransparent(myPlane.x, myPlane.y, &imgPlayer[currentPlane]);
        if (g_netMode != 0)
        {
            putimageTransparent(myPlane2.x, myPlane2.y, &imgPlayer[g_player2Plane]);
        }
    }

    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &enemyPlane_list, Node, list)
        putimageTransparent(pos->x, pos->y, &imgEnemy[pos->planeType]);
    list_for_each_entry_safe(pos, n, &myBull_list, Node, list)
        putimageTransparent(pos->x, pos->y, &imgMyBullet[pos->planeType]);
    list_for_each_entry_safe(pos, n, &enemyBullet_list, Node, list)
        putimageTransparent(pos->x, pos->y, &imgEnemyBullet[pos->planeType]);
    list_for_each_entry_safe(pos, n, &bossBullet_list, Node, list)
        putimageTransparent(pos->x, pos->y, &imgEnemyBullet[level - 1]);
    list_for_each_entry_safe(pos, n, &supply_list, Node, list)
        putimageTransparent(pos->x, pos->y, &imgSupply[pos->planeType]);
    if (bossActive)
        putimageTransparent(boss.x, boss.y, &imgBoss[level - 1]);
    list_for_each_entry_safe(pos, n, &explosion_list, Node, list)
        putimageTransparent(pos->x, pos->y, &imgExplosion[pos->frame]);
    drawUI();
    EndBatchDraw();
}

// ==================== 单人模式输入处理 ====================
void play()
{
    static int space_pressed = 0;
    if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
    {
        if (myPlane.y >= 0) myPlane.y -= myPlane.speed;
    }
    if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        if (myPlane.y < BGHEIGHT - myAirHEIGHT) myPlane.y += myPlane.speed;
    }
    if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        if (myPlane.x >= 0) myPlane.x -= myPlane.speed;
    }
    if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        if (myPlane.x + myAirWEIGHT < BGWEIGHT) myPlane.x += myPlane.speed;
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        if (!space_pressed)
        {
            create_myBull();
            space_pressed = 1;
        }
    }
    else space_pressed = 0;
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        mciSendString("close all", 0, 0, 0);
        closegraph();
        if (g_netMgr) { g_netMgr->close(); delete g_netMgr; g_netMgr = NULL; }
        exit(0);
    }
}

// ==================== 碰撞检测 ====================
int Blt_isConflict(Node* p, Node* q)
{
    int bw = BulletWEIGHT;
    int bh = BulletHEIGHT;
    if (p->type == TYPE_MY_BULLET && p->planeType == 7)
    {
        bw = MISSILE_WIDTH;
        bh = MISSILE_HEIGHT;
    }
    int px = p->x + bw / 2;
    int py = p->y + bh / 2;
    int qx = q->x + enemyPlaneWEIGHT / 2;
    int qy = q->y + enemyPlaneHEIGHT / 2;
    return abs(px - qx) < enemyPlaneWEIGHT / 2 + bw / 2 &&
        abs(py - qy) < enemyPlaneHEIGHT / 2 + bh / 2;
}

int myPlane_isConflict(Node* p, Node* q)
{
    int px = p->x + myAirWEIGHT / 2;
    int py = p->y + myAirHEIGHT / 2;
    int qx = q->x + enemyPlaneWEIGHT / 2;
    int qy = q->y + enemyPlaneHEIGHT / 2;
    return abs(px - qx) < enemyPlaneWEIGHT / 2 + myAirWEIGHT / 2 &&
        abs(py - qy) < enemyPlaneHEIGHT / 2 + myAirHEIGHT / 2;
}

int bulletHitBoss(Node* p)
{
    int bw = BulletWEIGHT;
    int bh = BulletHEIGHT;
    if (p->planeType == 7)
    {
        bw = MISSILE_WIDTH;
        bh = MISSILE_HEIGHT;
    }
    int px = p->x + bw / 2;
    int py = p->y + bh / 2;
    int bx = boss.x + BOSS_WIDTH / 2;
    int by = boss.y + BOSS_HEIGHT / 2;
    return abs(px - bx) < BOSS_WIDTH / 2 + bw / 2 &&
        abs(py - by) < BOSS_HEIGHT / 2 + bh / 2;
}

// ==================== 主逻辑更新 ====================
void move()
{
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &myBull_list, Node, list)
    {
        pos->y -= pos->speed;
        if (pos->y + BulletHEIGHT < 0)
        {
            list_del(&pos->list);
            free(pos);
            continue;
        }
        Node* epos, * en;
        list_for_each_entry_safe(epos, en, &enemyPlane_list, Node, list)
        {
            if (Blt_isConflict(pos, epos))
            {
                epos->hp -= pos->damage;
                list_del(&pos->list);
                free(pos);
                if (epos->hp <= 0)
                {
                    score += enemyScore[epos->planeType];
                    createExplosion(epos->x + enemyPlaneWEIGHT / 2, epos->y + enemyPlaneHEIGHT / 2);
                    addFusionEnergy(enemyScore[epos->planeType]);
                    if (rand() % 100 < 15)
                    {
                        int type = rand() % 2;
                        createSupply(epos->x + enemyPlaneWEIGHT / 2 - SUPPLY_WIDTH / 2, epos->y, type);
                    }
                    list_del(&epos->list);
                    free(epos);
                }
                goto nextBullet;
            }
        }
        if (bossActive && bulletHitBoss(pos))
        {
            boss.hp -= pos->damage;
            addFusionEnergy((int)(pos->damage * 0.1));
            list_del(&pos->list);
            free(pos);
            if (boss.hp <= 0)
            {
                createExplosion(boss.x + BOSS_WIDTH / 2, boss.y + BOSS_HEIGHT / 2);
                bossActive = 0;
                isEnd = 2;
            }
        }
    nextBullet:;
    }
    list_for_each_entry_safe(pos, n, &enemyPlane_list, Node, list)
    {
        pos->y += pos->speed;
        enemyShoot(pos);
        if (pos->y > BGHEIGHT)
        {
            list_del(&pos->list);
            free(pos);
            continue;
        }
        // 合体状态下：超级战机与敌机碰撞
        if (g_fusionActive && g_netMode == 1)
        {
            if (superPlane_isConflict(pos))
            {
                g_fusionHp -= CRASH_DAMAGE;
                createExplosion(pos->x + enemyPlaneWEIGHT / 2, pos->y + enemyPlaneHEIGHT / 2);
                list_del(&pos->list);
                free(pos);
                if (g_fusionHp <= 0)
                {
                    g_fusionHp = 0;
                    endFusion();
                }
                continue;
            }
        }
        // 玩家1碰撞
        if (myPlane_isConflict(&myPlane, pos))
        {
            playerHp -= CRASH_DAMAGE;
            createExplosion(pos->x + enemyPlaneWEIGHT / 2, pos->y + enemyPlaneHEIGHT / 2);
            list_del(&pos->list);
            free(pos);
            if (playerHp <= 0)
            {
                playerHp = 0;
                isEnd = 1;
            }
            continue;
        }
        // 玩家2碰撞（仅主机模式，合体状态下不单独检测）
        if (g_netMode == 1 && !g_fusionActive)
        {
            if (myPlane_isConflict(&myPlane2, pos))
            {
                playerHp -= CRASH_DAMAGE;
                createExplosion(pos->x + enemyPlaneWEIGHT / 2, pos->y + enemyPlaneHEIGHT / 2);
                list_del(&pos->list);
                free(pos);
                if (playerHp <= 0)
                {
                    playerHp = 0;
                    isEnd = 1;
                }
            }
        }
    }
    updateEnemyBullets();
    bossAI();
    updateBossBullets();
    updateExplosion();
    updateSupply();
    if (!bossActive && score >= levelTargetScore[level - 1])
    {
        bossActive = 1;
        boss.hp = bossHpList[level - 1];
        boss.x = BGWEIGHT / 2 - BOSS_WIDTH / 2;
        boss.y = 50;
        boss.shootTimer = GetTickCount();
    }
}

// ==================== 网络模式：主机游戏循环 ====================
void netHostGame()
{
    static int space_pressed = 0;
    static int p2_space_pressed = 0;
    static int x_pressed = 0;
    char sendBuf[20000];
    char recvBuf[1024];
    BYTE msgType;
    while (1)
    {
        // 背景音乐循环
        char bgmStatus[16];
        mciSendString("status bgm mode", bgmStatus, 16, 0);
        if (strcmp(bgmStatus, "stopped") == 0)
        {
            mciSendString("play bgm from 0", 0, 0, 0);
        }
        // 1. 接收客机的玩家2输入
        int ret = g_netMgr->recvMessage(msgType, recvBuf, sizeof(recvBuf));
        if (ret == -1)
        {
            MessageBox(GetForegroundWindow(), "客机连接断开！", "网络错误", MB_OK);
            return;
        }
        if (ret > 0 && msgType == MSG_INPUT)
        {
            PlayerInput* input = (PlayerInput*)recvBuf;
            if (g_fusionActive)
            {
                if (input->keyState & KEY_SHOOT) {
                    superPlaneShoot();
                }
            }
            else
            {
                if (input->keyState & KEY_UP) {
                    if (myPlane2.y >= 0) myPlane2.y -= myPlane2.speed;
                }
                if (input->keyState & KEY_DOWN) {
                    if (myPlane2.y < BGHEIGHT - myAirHEIGHT) myPlane2.y += myPlane2.speed;
                }
                if (input->keyState & KEY_LEFT) {
                    if (myPlane2.x >= 0) myPlane2.x -= myPlane2.speed;
                }
                if (input->keyState & KEY_RIGHT) {
                    if (myPlane2.x + myAirWEIGHT < BGWEIGHT) myPlane2.x += myPlane2.speed;
                }
                if (input->keyState & KEY_SHOOT) {
                    if (!p2_space_pressed) {
                        create_myBull_from(myPlane2.x, myPlane2.y);
                        p2_space_pressed = 1;
                    }
                }
                else {
                    p2_space_pressed = 0;
                }
                if (input->keyState & KEY_FUSION) {
                    if (isFusionReady()) {
                        startFusion();
                    }
                }
            }
        }
        // 2. 玩家1（主机）本地输入
        if (g_fusionActive)
        {
            if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
            {
                if (g_superPlane.y >= 0) g_superPlane.y -= g_superPlane.speed;
            }
            if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                if (g_superPlane.y < BGHEIGHT - SUPER_HEIGHT) g_superPlane.y += g_superPlane.speed;
            }
            if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                if (g_superPlane.x >= 0) g_superPlane.x -= g_superPlane.speed;
            }
            if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                if (g_superPlane.x + SUPER_WIDTH < BGWEIGHT) g_superPlane.x += g_superPlane.speed;
            }
            // 合体状态下主机也可以按空格射击（双保险）
            if (GetAsyncKeyState(VK_SPACE) & 0x8000)
            {
                superPlaneShoot();
            }
        }
        else
        {
            if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
            {
                if (myPlane.y >= 0) myPlane.y -= myPlane.speed;
            }
            if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
            {
                if (myPlane.y < BGHEIGHT - myAirHEIGHT) myPlane.y += myPlane.speed;
            }
            if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                if (myPlane.x >= 0) myPlane.x -= myPlane.speed;
            }
            if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                if (myPlane.x + myAirWEIGHT < BGWEIGHT) myPlane.x += myPlane.speed;
            }
            if (GetAsyncKeyState(VK_SPACE) & 0x8000)
            {
                if (!space_pressed) {
                    create_myBull();
                    space_pressed = 1;
                }
            }
            else {
                space_pressed = 0;
            }
            // 按X键触发合体
            if (GetAsyncKeyState('X') & 0x8000)
            {
                if (!x_pressed && isFusionReady()) {
                    startFusion();
                    x_pressed = 1;
                }
            }
            else {
                x_pressed = 0;
            }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            mciSendString("close all", 0, 0, 0);
            closegraph();
            if (g_netMgr) { g_netMgr->close(); delete g_netMgr; g_netMgr = NULL; }
            exit(0);
        }

        // 3. 更新游戏逻辑
        updateBgScroll();
        createEnemy();
        move();
        updateFusion();

        // 4. 序列化游戏状态并发送给客机
        int len = serializeGameState(sendBuf, sizeof(sendBuf));
        g_netMgr->sendMessage(MSG_GAME_STATE, sendBuf, len);

        // 5. 绘制
        DrawMap();

        // 6. 判断游戏结束
        if (isEnd == 1)
        {
            // 游戏失败
            cleardevice();
            BeginBatchDraw();
            setDrawScale();
            settextstyle(36, 0, "宋体");
            settextcolor(RED);
            setbkmode(TRANSPARENT);
            int x = (BGWEIGHT - textwidth("游戏结束")) / 2;
            outtextxy(x, 250, "游戏结束");
            settextstyle(20, 0, "宋体");
            settextcolor(WHITE);
            TCHAR str[64];
            wsprintf(str, "最终分数：%d", score);
            x = (BGWEIGHT - textwidth(str)) / 2;
            outtextxy(x, 310, str);
            x = (BGWEIGHT - textwidth("按任意键返回")) / 2;
            outtextxy(x, 370, "按任意键返回");
            EndBatchDraw();
            waitForKeyPress();
            break;
        }
        if (isEnd == 2)
        {
            // Boss被击败，过关
            showLevelPass();
            if (level >= TOTAL_LEVEL)
            {
                showGameWin();
                break;
            }
            level++;
            initLevel(level);
        }

        Sleep(10);
    }
}

// ==================== 网络模式：客机游戏循环 ====================
void netClientGame()
{
    char sendBuf[1024];
    char recvBuf[20000];
    BYTE msgType;

    while (1)
    {
        // 1. 采集本地输入（玩家2）并发送给主机
        PlayerInput input;
        input.keyState = 0;

        // 合体状态下客机只负责射击
        if (g_fusionActive)
        {
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                input.keyState |= KEY_SHOOT;
            }
        }
        else
        {
            if (GetAsyncKeyState(VK_UP) & 0x8000)    input.keyState |= KEY_UP;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000)  input.keyState |= KEY_DOWN;
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)  input.keyState |= KEY_LEFT;
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) input.keyState |= KEY_RIGHT;
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) input.keyState |= KEY_SHOOT;
            if (GetAsyncKeyState('X') & 0x8000)      input.keyState |= KEY_FUSION;
        }

        memcpy(sendBuf, &input, sizeof(input));
        g_netMgr->sendMessage(MSG_INPUT, sendBuf, sizeof(input));

        // 2. 接收主机发来的游戏状态
        int ret = g_netMgr->recvMessage(msgType, recvBuf, sizeof(recvBuf));
        if (ret == -1)
        {
            MessageBox(GetForegroundWindow(), "主机连接断开！", "网络错误", MB_OK);
            return;
        }
        if (ret > 0 && msgType == MSG_GAME_STATE)
        {
            deserializeGameState(recvBuf, ret);
        }

        // 3. 绘制（客机只负责渲染，不做逻辑）
        DrawMap();

        // 4. 退出检测
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            mciSendString("close all", 0, 0, 0);
            closegraph();
            if (g_netMgr) { g_netMgr->close(); delete g_netMgr; g_netMgr = NULL; }
            exit(0);
        }

        // 5. 游戏结束判断（客机也显示结束画面）
        if (isEnd == 1)
        {
            cleardevice();
            BeginBatchDraw();
            setDrawScale();
            settextstyle(36, 0, "宋体");
            settextcolor(RED);
            setbkmode(TRANSPARENT);
            int x = (BGWEIGHT - textwidth("游戏结束")) / 2;
            outtextxy(x, 250, "游戏结束");
            settextstyle(20, 0, "宋体");
            settextcolor(WHITE);
            TCHAR str[64];
            wsprintf(str, "最终分数：%d", score);
            x = (BGWEIGHT - textwidth(str)) / 2;
            outtextxy(x, 310, str);
            x = (BGWEIGHT - textwidth("按任意键返回")) / 2;
            outtextxy(x, 370, "按任意键返回");
            EndBatchDraw();
            waitForKeyPress();
            break;
        }
        if (isEnd == 2)
        {
            showLevelPass();
            if (level >= TOTAL_LEVEL)
            {
                showGameWin();
                break;
            }
        }

        Sleep(10);
    }
}

// ==================== 游戏状态序列化 ====================
int serializeGameState(char* buf, int maxLen)
{
    int offset = 0;

    // 基础游戏状态
    memcpy(buf + offset, &score, sizeof(int));  offset += sizeof(int);
    memcpy(buf + offset, &level, sizeof(int));  offset += sizeof(int);
    memcpy(buf + offset, &playerHp, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &playerMaxHp, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &bossActive, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &isEnd, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &bgOffset, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &currentBuff, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &buffEndTime, sizeof(DWORD)); offset += sizeof(DWORD);
    memcpy(buf + offset, &currentPlane, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &g_player2Plane, sizeof(int)); offset += sizeof(int);

    // 合体状态
    memcpy(buf + offset, &g_fusionActive, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &g_fusionEnergyP1, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &g_fusionEnergyP2, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &g_fusionHp, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &g_fusionMaxHp, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &g_fusionEndTime, sizeof(DWORD)); offset += sizeof(DWORD);
    memcpy(buf + offset, &g_superPlane, sizeof(SuperPlane)); offset += sizeof(SuperPlane);

    // 玩家1飞机
    memcpy(buf + offset, &myPlane.x, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &myPlane.y, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &myPlane.speed, sizeof(int)); offset += sizeof(int);

    // 玩家2飞机
    memcpy(buf + offset, &myPlane2.x, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &myPlane2.y, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &myPlane2.speed, sizeof(int)); offset += sizeof(int);

    // Boss
    memcpy(buf + offset, &boss.x, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &boss.y, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &boss.hp, sizeof(int)); offset += sizeof(int);
    memcpy(buf + offset, &boss.maxHp, sizeof(int)); offset += sizeof(int);

    // 敌机列表
    int enemyCount = 0;
    Node* pos;
    list_for_each_entry(pos, &enemyPlane_list, Node, list) { enemyCount++; }
    memcpy(buf + offset, &enemyCount, sizeof(int)); offset += sizeof(int);
    list_for_each_entry(pos, &enemyPlane_list, Node, list)
    {
        memcpy(buf + offset, &pos->x, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->y, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->planeType, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->hp, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->maxHp, sizeof(int)); offset += sizeof(int);
    }

    // 我方子弹列表
    int myBulletCount = 0;
    list_for_each_entry(pos, &myBull_list, Node, list) { myBulletCount++; }
    memcpy(buf + offset, &myBulletCount, sizeof(int)); offset += sizeof(int);
    list_for_each_entry(pos, &myBull_list, Node, list)
    {
        memcpy(buf + offset, &pos->x, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->y, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->planeType, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->damage, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->speed, sizeof(int)); offset += sizeof(int);
    }

    // 敌机子弹列表
    int enemyBulletCount = 0;
    list_for_each_entry(pos, &enemyBullet_list, Node, list) { enemyBulletCount++; }
    memcpy(buf + offset, &enemyBulletCount, sizeof(int)); offset += sizeof(int);
    list_for_each_entry(pos, &enemyBullet_list, Node, list)
    {
        memcpy(buf + offset, &pos->x, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->y, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->planeType, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->damage, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->angle, sizeof(double)); offset += sizeof(double);
        memcpy(buf + offset, &pos->speed, sizeof(int)); offset += sizeof(int);
    }

    // Boss子弹列表
    int bossBulletCount = 0;
    list_for_each_entry(pos, &bossBullet_list, Node, list) { bossBulletCount++; }
    memcpy(buf + offset, &bossBulletCount, sizeof(int)); offset += sizeof(int);
    list_for_each_entry(pos, &bossBullet_list, Node, list)
    {
        memcpy(buf + offset, &pos->x, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->y, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->damage, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->angle, sizeof(double)); offset += sizeof(double);
        memcpy(buf + offset, &pos->speed, sizeof(int)); offset += sizeof(int);
    }

    // 爆炸效果列表
    int expCount = 0;
    list_for_each_entry(pos, &explosion_list, Node, list) { expCount++; }
    memcpy(buf + offset, &expCount, sizeof(int)); offset += sizeof(int);
    list_for_each_entry(pos, &explosion_list, Node, list)
    {
        memcpy(buf + offset, &pos->x, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->y, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->frame, sizeof(int)); offset += sizeof(int);
    }

    // 道具列表
    int supplyCount = 0;
    list_for_each_entry(pos, &supply_list, Node, list) { supplyCount++; }
    memcpy(buf + offset, &supplyCount, sizeof(int)); offset += sizeof(int);
    list_for_each_entry(pos, &supply_list, Node, list)
    {
        memcpy(buf + offset, &pos->x, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->y, sizeof(int)); offset += sizeof(int);
        memcpy(buf + offset, &pos->planeType, sizeof(int)); offset += sizeof(int);
    }

    return offset;
}

// ==================== 游戏状态反序列化 ====================
void deserializeGameState(const char* buf, int len)
{
    int offset = 0;

    // 清空现有链表
    Node* pos, * n;
    list_for_each_entry_safe(pos, n, &enemyPlane_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &myBull_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &enemyBullet_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &bossBullet_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &explosion_list, Node, list) { list_del(&pos->list); free(pos); }
    list_for_each_entry_safe(pos, n, &supply_list, Node, list) { list_del(&pos->list); free(pos); }

    // 基础游戏状态
    memcpy(&score, buf + offset, sizeof(int));  offset += sizeof(int);
    memcpy(&level, buf + offset, sizeof(int));  offset += sizeof(int);
    memcpy(&playerHp, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&playerMaxHp, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&bossActive, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&isEnd, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&bgOffset, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&currentBuff, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&buffEndTime, buf + offset, sizeof(DWORD)); offset += sizeof(DWORD);
    memcpy(&currentPlane, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&g_player2Plane, buf + offset, sizeof(int)); offset += sizeof(int);

    // 合体状态
    memcpy(&g_fusionActive, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&g_fusionEnergyP1, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&g_fusionEnergyP2, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&g_fusionHp, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&g_fusionMaxHp, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&g_fusionEndTime, buf + offset, sizeof(DWORD)); offset += sizeof(DWORD);
    memcpy(&g_superPlane, buf + offset, sizeof(SuperPlane)); offset += sizeof(SuperPlane);

    // 玩家1飞机
    memcpy(&myPlane.x, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&myPlane.y, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&myPlane.speed, buf + offset, sizeof(int)); offset += sizeof(int);

    // 玩家2飞机
    memcpy(&myPlane2.x, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&myPlane2.y, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&myPlane2.speed, buf + offset, sizeof(int)); offset += sizeof(int);

    // Boss
    memcpy(&boss.x, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&boss.y, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&boss.hp, buf + offset, sizeof(int)); offset += sizeof(int);
    memcpy(&boss.maxHp, buf + offset, sizeof(int)); offset += sizeof(int);

    // 敌机列表
    int enemyCount;
    memcpy(&enemyCount, buf + offset, sizeof(int)); offset += sizeof(int);
    for (int i = 0; i < enemyCount; i++)
    {
        Node* enemy = (Node*)malloc(sizeof(Node));
        memcpy(&enemy->x, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&enemy->y, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&enemy->planeType, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&enemy->hp, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&enemy->maxHp, buf + offset, sizeof(int)); offset += sizeof(int);
        enemy->type = TYPE_ENEMY;
        INIT_LIST_HEAD(&enemy->list);
        list_add_tail(&enemy->list, &enemyPlane_list);
    }

    // 我方子弹列表
    int myBulletCount;
    memcpy(&myBulletCount, buf + offset, sizeof(int)); offset += sizeof(int);
    for (int i = 0; i < myBulletCount; i++)
    {
        Node* bullet = (Node*)malloc(sizeof(Node));
        memcpy(&bullet->x, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->y, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->planeType, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->damage, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->speed, buf + offset, sizeof(int)); offset += sizeof(int);
        bullet->type = TYPE_MY_BULLET;
        INIT_LIST_HEAD(&bullet->list);
        list_add_tail(&bullet->list, &myBull_list);
    }

    // 敌机子弹列表
    int enemyBulletCount;
    memcpy(&enemyBulletCount, buf + offset, sizeof(int)); offset += sizeof(int);
    for (int i = 0; i < enemyBulletCount; i++)
    {
        Node* bullet = (Node*)malloc(sizeof(Node));
        memcpy(&bullet->x, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->y, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->planeType, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->damage, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->angle, buf + offset, sizeof(double)); offset += sizeof(double);
        memcpy(&bullet->speed, buf + offset, sizeof(int)); offset += sizeof(int);
        bullet->type = TYPE_ENEMY_BULLET;
        INIT_LIST_HEAD(&bullet->list);
        list_add_tail(&bullet->list, &enemyBullet_list);
    }

    // Boss子弹列表
    int bossBulletCount;
    memcpy(&bossBulletCount, buf + offset, sizeof(int)); offset += sizeof(int);
    for (int i = 0; i < bossBulletCount; i++)
    {
        Node* bullet = (Node*)malloc(sizeof(Node));
        memcpy(&bullet->x, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->y, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->damage, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&bullet->angle, buf + offset, sizeof(double)); offset += sizeof(double);
        memcpy(&bullet->speed, buf + offset, sizeof(int)); offset += sizeof(int);
        bullet->type = TYPE_BOSS_BULLET;
        bullet->planeType = level - 1;
        INIT_LIST_HEAD(&bullet->list);
        list_add_tail(&bullet->list, &bossBullet_list);
    }

    // 爆炸效果列表
    int expCount;
    memcpy(&expCount, buf + offset, sizeof(int)); offset += sizeof(int);
    for (int i = 0; i < expCount; i++)
    {
        Node* exp = (Node*)malloc(sizeof(Node));
        memcpy(&exp->x, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&exp->y, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&exp->frame, buf + offset, sizeof(int)); offset += sizeof(int);
        exp->type = TYPE_EXPLOSION;
        exp->frameTimer = GetTickCount();
        INIT_LIST_HEAD(&exp->list);
        list_add_tail(&exp->list, &explosion_list);
    }

    // 道具列表
    int supplyCount;
    memcpy(&supplyCount, buf + offset, sizeof(int)); offset += sizeof(int);
    for (int i = 0; i < supplyCount; i++)
    {
        Node* supply = (Node*)malloc(sizeof(Node));
        memcpy(&supply->x, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&supply->y, buf + offset, sizeof(int)); offset += sizeof(int);
        memcpy(&supply->planeType, buf + offset, sizeof(int)); offset += sizeof(int);
        supply->type = TYPE_SUPPLY;
        supply->speed = 2;
        INIT_LIST_HEAD(&supply->list);
        list_add_tail(&supply->list, &supply_list);
    }
}

// ==================== 网络模式选择界面 ====================
int showNetModeSelect()
{
    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    putimage(0, 0, &imgStartBg);

    settextstyle(28, 0, "宋体");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth("选择联机模式")) / 2;
    outtextxy(x, 120, "选择联机模式");

    // 按钮1：创建主机
    setfillcolor(BTN_COLOR_NORMAL);
    solidrectangle(BTN_X, 200, BTN_X + BTN_WIDTH, 200 + BTN_HEIGHT);
    settextcolor(BTN_TEXT_COLOR);
    settextstyle(18, 0, "宋体");
    x = (BGWEIGHT - textwidth("创建主机")) / 2;
    outtextxy(x, 208, "创建主机");

    // 按钮2：加入游戏
    setfillcolor(BTN_COLOR_NORMAL);
    solidrectangle(BTN_X, 270, BTN_X + BTN_WIDTH, 270 + BTN_HEIGHT);
    x = (BGWEIGHT - textwidth("加入游戏")) / 2;
    outtextxy(x, 278, "加入游戏");

    // 按钮3：返回
    setfillcolor(BTN_COLOR_NORMAL);
    solidrectangle(BTN_X, 340, BTN_X + BTN_WIDTH, 340 + BTN_HEIGHT);
    x = (BGWEIGHT - textwidth("返回")) / 2;
    outtextxy(x, 348, "返回");

    EndBatchDraw();

    while (true)
    {
        ExMessage msg = getmessage();
        if (msg.message == WM_LBUTTONDOWN)
        {
            int mx = (int)((msg.x - g_offsetX) / g_scale);
            int my = (int)((msg.y - g_offsetY) / g_scale);

            if (mx >= BTN_X && mx <= BTN_X + BTN_WIDTH &&
                my >= 200 && my <= 200 + BTN_HEIGHT)
            {
                return 1; // 主机
            }
            if (mx >= BTN_X && mx <= BTN_X + BTN_WIDTH &&
                my >= 270 && my <= 270 + BTN_HEIGHT)
            {
                return 2; // 客机
            }
            if (mx >= BTN_X && mx <= BTN_X + BTN_WIDTH &&
                my >= 340 && my <= 340 + BTN_HEIGHT)
            {
                return 0; // 返回
            }
        }
        Sleep(10);
    }
}

// ==================== IP输入界面 ====================
bool showIpInput(char* ipBuf, int bufLen)
{
    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    putimage(0, 0, &imgStartBg);

    settextstyle(24, 0, "宋体");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth("输入主机IP地址")) / 2;
    outtextxy(x, 150, "输入主机IP地址");

    settextstyle(16, 0, "宋体");
    x = (BGWEIGHT - textwidth("（格式：192.168.1.100）")) / 2;
    outtextxy(x, 190, "（格式：192.168.1.100）");

    // 输入框
    setfillcolor(WHITE);
    solidrectangle(60, 250, 340, 290);
    settextcolor(BLACK);
    setbkmode(OPAQUE);
    setbkcolor(WHITE);

    char input[32] = { 0 };
    int inputLen = 0;

    EndBatchDraw();

    while (true)
    {
        ExMessage msg = getmessage();
        if (msg.message == WM_CHAR)
        {
            if (msg.ch == '\r' || msg.ch == '\n')
            {
                // 回车确认
                if (inputLen > 0)
                {
                    strncpy_s(ipBuf, bufLen, input, inputLen);
                    ipBuf[inputLen] = '\0';
                    return true;
                }
            }
            else if (msg.ch == '\b')
            {
                // 退格
                if (inputLen > 0)
                {
                    inputLen--;
                    input[inputLen] = '\0';
                }
            }
            else if (msg.ch >= 32 && msg.ch <= 126 && inputLen < 31)
            {
                // 普通字符
                input[inputLen++] = msg.ch;
                input[inputLen] = '\0';
            }

            // 重绘输入框
            BeginBatchDraw();
            setDrawScale();
            setfillcolor(WHITE);
            solidrectangle(60, 250, 340, 290);
            settextcolor(BLACK);
            setbkmode(OPAQUE);
            setbkcolor(WHITE);
            outtextxy(70, 262, input);
            EndBatchDraw();
        }
        if (msg.message == WM_LBUTTONDOWN)
        {
            int mx = (int)((msg.x - g_offsetX) / g_scale);
            int my = (int)((msg.y - g_offsetY) / g_scale);
            // 取消按钮
            if (mx >= 60 && mx <= 180 && my >= 320 && my <= 360)
            {
                return false;
            }
            // 确认按钮
            if (mx >= 220 && mx <= 340 && my >= 320 && my <= 360)
            {
                if (inputLen > 0)
                {
                    strncpy_s(ipBuf, bufLen, input, inputLen);
                    ipBuf[inputLen] = '\0';
                    return true;
                }
            }
        }

        // 绘制按钮
        BeginBatchDraw();
        setDrawScale();
        setfillcolor(BTN_COLOR_NORMAL);
        solidrectangle(60, 320, 180, 360);
        solidrectangle(220, 320, 340, 360);
        settextcolor(BTN_TEXT_COLOR);
        setbkmode(TRANSPARENT);
        settextstyle(16, 0, "宋体");
        outtextxy(100, 332, "取消");
        outtextxy(265, 332, "确认");
        EndBatchDraw();

        Sleep(10);
    }
}

// ==================== 等待连接界面 ====================
void showWaitingScreen(const char* text)
{
    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    putimage(0, 0, &imgStartBg);
    settextstyle(24, 0, "宋体");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth(text)) / 2;
    outtextxy(x, 280, text);
    settextstyle(16, 0, "宋体");
    x = (BGWEIGHT - textwidth("按ESC取消")) / 2;
    outtextxy(x, 330, "按ESC取消");
    EndBatchDraw();
}

// ==================== 开始界面 ====================
bool showStartScreen()
{
    BeginBatchDraw();
    setDrawScale();
    putimage(0, 0, &imgStartBg);

    settextstyle(42, 0, "宋体");
    settextcolor(YELLOW);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth("飞机大战")) / 2;
    outtextxy(x, 100, "飞机大战");

    // 绘制按钮
    setfillcolor(BTN_COLOR_NORMAL);
    solidrectangle(BTN_X, BTN_START_Y, BTN_X + BTN_WIDTH, BTN_START_Y + BTN_HEIGHT);
    solidrectangle(BTN_X, BTN_SELECT_Y, BTN_X + BTN_WIDTH, BTN_SELECT_Y + BTN_HEIGHT);
    solidrectangle(BTN_X, BTN_HELP_Y, BTN_X + BTN_WIDTH, BTN_HELP_Y + BTN_HEIGHT);
    solidrectangle(BTN_X, BTN_EXIT_Y, BTN_X + BTN_WIDTH, BTN_EXIT_Y + BTN_HEIGHT);

    settextcolor(BTN_TEXT_COLOR);
    settextstyle(18, 0, "宋体");
    x = (BGWEIGHT - textwidth("开始游戏")) / 2;
    outtextxy(x, BTN_START_Y + 8, "开始游戏");
    x = (BGWEIGHT - textwidth("联机对战")) / 2;
    outtextxy(x, BTN_SELECT_Y + 8, "联机对战");
    x = (BGWEIGHT - textwidth("游戏帮助")) / 2;
    outtextxy(x, BTN_HELP_Y + 8, "游戏帮助");
    x = (BGWEIGHT - textwidth("退出游戏")) / 2;
    outtextxy(x, BTN_EXIT_Y + 8, "退出游戏");

    EndBatchDraw();

    while (true)
    {
        ExMessage msg = getmessage();
        if (msg.message == WM_LBUTTONDOWN)
        {
            int mx = (int)((msg.x - g_offsetX) / g_scale);
            int my = (int)((msg.y - g_offsetY) / g_scale);

            if (mx >= BTN_X && mx <= BTN_X + BTN_WIDTH)
            {
                if (my >= BTN_START_Y && my <= BTN_START_Y + BTN_HEIGHT)
                {
                    if (showPlaneSelect())  // 选飞机，返回true表示确认
                    {
                        return true;
                    }
                    // 返回false就是按了ESC，重绘开始界面继续循环
                    BeginBatchDraw();
                    setDrawScale();
                    putimage(0, 0, &imgStartBg);
                    settextstyle(42, 0, "宋体");
                    settextcolor(YELLOW);
                    setbkmode(TRANSPARENT);
                    int xx = (BGWEIGHT - textwidth("飞机大战")) / 2;
                    outtextxy(xx, 100, "飞机大战");
                    setfillcolor(BTN_COLOR_NORMAL);
                    solidrectangle(BTN_X, BTN_START_Y, BTN_X + BTN_WIDTH, BTN_START_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_SELECT_Y, BTN_X + BTN_WIDTH, BTN_SELECT_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_HELP_Y, BTN_X + BTN_WIDTH, BTN_HELP_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_EXIT_Y, BTN_X + BTN_WIDTH, BTN_EXIT_Y + BTN_HEIGHT);
                    settextcolor(BTN_TEXT_COLOR);
                    settextstyle(18, 0, "宋体");
                    xx = (BGWEIGHT - textwidth("开始游戏")) / 2;
                    outtextxy(xx, BTN_START_Y + 8, "开始游戏");
                    xx = (BGWEIGHT - textwidth("联机对战")) / 2;
                    outtextxy(xx, BTN_SELECT_Y + 8, "联机对战");
                    xx = (BGWEIGHT - textwidth("游戏帮助")) / 2;
                    outtextxy(xx, BTN_HELP_Y + 8, "游戏帮助");
                    xx = (BGWEIGHT - textwidth("退出游戏")) / 2;
                    outtextxy(xx, BTN_EXIT_Y + 8, "退出游戏");
                    EndBatchDraw();
                }
                if (my >= BTN_SELECT_Y && my <= BTN_SELECT_Y + BTN_HEIGHT)
                {
                    // 联机模式
                    int mode = showNetModeSelect();
                    if (mode == 1)
                    {
                        // 创建主机（非阻塞等待，支持ESC取消）
                        g_netMgr = new NetManager();
                        if (!g_netMgr->startListen(8888))
                        {
                            delete g_netMgr;
                            g_netMgr = NULL;
                            MessageBox(GetForegroundWindow(), "创建主机失败！", "错误", MB_OK);
                        }
                        else
                        {
                            bool connected = false;
                            while (!connected)
                            {
                                showWaitingScreen("等待客机连接...");
                                if (g_netMgr->tryAccept())
                                {
                                    connected = true;
                                }
                                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
                                {
                                    break; // ESC取消
                                }
                                Sleep(50);
                            }

                            if (connected)
                            {
                                g_netMode = 1;
                                return true;
                            }
                            else
                            {
                                g_netMgr->close();
                                delete g_netMgr;
                                g_netMgr = NULL;
                            }
                        }
                    }
                    else if (mode == 2)
                    {
                        // 加入游戏
                        char ip[32];
                        if (showIpInput(ip, sizeof(ip)))
                        {
                            g_netMgr = new NetManager();
                            showWaitingScreen("正在连接主机...");
                            if (g_netMgr->joinGame(ip, 8888))
                            {
                                g_netMode = 2;
                                return true;
                            }
                            else
                            {
                                delete g_netMgr;
                                g_netMgr = NULL;
                                MessageBox(GetForegroundWindow(), "连接失败！", "错误", MB_OK);
                            }
                        }
                    }
                    // 返回开始界面，重绘
                    BeginBatchDraw();
                    setDrawScale();
                    putimage(0, 0, &imgStartBg);
                    settextstyle(42, 0, "宋体");
                    settextcolor(YELLOW);
                    setbkmode(TRANSPARENT);
                    int xx = (BGWEIGHT - textwidth("飞机大战")) / 2;
                    outtextxy(xx, 100, "飞机大战");
                    setfillcolor(BTN_COLOR_NORMAL);
                    solidrectangle(BTN_X, BTN_START_Y, BTN_X + BTN_WIDTH, BTN_START_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_SELECT_Y, BTN_X + BTN_WIDTH, BTN_SELECT_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_HELP_Y, BTN_X + BTN_WIDTH, BTN_HELP_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_EXIT_Y, BTN_X + BTN_WIDTH, BTN_EXIT_Y + BTN_HEIGHT);
                    settextcolor(BTN_TEXT_COLOR);
                    settextstyle(18, 0, "宋体");
                    xx = (BGWEIGHT - textwidth("开始游戏")) / 2;
                    outtextxy(xx, BTN_START_Y + 8, "开始游戏");
                    xx = (BGWEIGHT - textwidth("联机对战")) / 2;
                    outtextxy(xx, BTN_SELECT_Y + 8, "联机对战");
                    xx = (BGWEIGHT - textwidth("游戏帮助")) / 2;
                    outtextxy(xx, BTN_HELP_Y + 8, "游戏帮助");
                    xx = (BGWEIGHT - textwidth("退出游戏")) / 2;
                    outtextxy(xx, BTN_EXIT_Y + 8, "退出游戏");
                    EndBatchDraw();
                }
                if (my >= BTN_HELP_Y && my <= BTN_HELP_Y + BTN_HEIGHT)
                {
                    showGameHelp();
                    // 返回后重绘
                    BeginBatchDraw();
                    setDrawScale();
                    putimage(0, 0, &imgStartBg);
                    settextstyle(42, 0, "宋体");
                    settextcolor(YELLOW);
                    setbkmode(TRANSPARENT);
                    int xx = (BGWEIGHT - textwidth("飞机大战")) / 2;
                    outtextxy(xx, 100, "飞机大战");
                    setfillcolor(BTN_COLOR_NORMAL);
                    solidrectangle(BTN_X, BTN_START_Y, BTN_X + BTN_WIDTH, BTN_START_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_SELECT_Y, BTN_X + BTN_WIDTH, BTN_SELECT_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_HELP_Y, BTN_X + BTN_WIDTH, BTN_HELP_Y + BTN_HEIGHT);
                    solidrectangle(BTN_X, BTN_EXIT_Y, BTN_X + BTN_WIDTH, BTN_EXIT_Y + BTN_HEIGHT);
                    settextcolor(BTN_TEXT_COLOR);
                    settextstyle(18, 0, "宋体");
                    xx = (BGWEIGHT - textwidth("开始游戏")) / 2;
                    outtextxy(xx, BTN_START_Y + 8, "开始游戏");
                    xx = (BGWEIGHT - textwidth("联机对战")) / 2;
                    outtextxy(xx, BTN_SELECT_Y + 8, "联机对战");
                    xx = (BGWEIGHT - textwidth("游戏帮助")) / 2;
                    outtextxy(xx, BTN_HELP_Y + 8, "游戏帮助");
                    xx = (BGWEIGHT - textwidth("退出游戏")) / 2;
                    outtextxy(xx, BTN_EXIT_Y + 8, "退出游戏");
                    EndBatchDraw();
                }
                if (my >= BTN_EXIT_Y && my <= BTN_EXIT_Y + BTN_HEIGHT)
                {
                    return false; // 退出
                }
            }
        }
        Sleep(10);
    }
}

// ==================== 飞机选择界面 ====================
bool showPlaneSelect()
{
    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    putimage(0, 0, &imgStartBg);

    settextstyle(28, 0, "宋体");
    settextcolor(YELLOW);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth("选择你的战机")) / 2;
    outtextxy(x, 80, "选择你的战机");

    int planeY = 180;
    int planeGap = BGWEIGHT / 3;

    for (int i = 0; i < 3; i++)
    {
        int px = planeGap * i + planeGap / 2 - myAirWEIGHT / 2;

        // 选中高亮框
        if (i == currentPlane)
        {
            setfillcolor(YELLOW);
            solidrectangle(px - 5, planeY - 5, px + myAirWEIGHT + 5, planeY + myAirHEIGHT + 5);
        }

        putimageTransparent(px, planeY, &imgPlayer[i]);

        // 飞机名称
        settextstyle(16, 0, "宋体");
        settextcolor(WHITE);
        const char* names[3] = { "战机1号", "战机2号", "战机3号" };
        int nx = planeGap * i + planeGap / 2 - textwidth(names[i]) / 2;
        outtextxy(nx, planeY + myAirHEIGHT + 10, names[i]);
    }

    settextstyle(18, 0, "宋体");
    settextcolor(YELLOW);
    x = (BGWEIGHT - textwidth("点击选择，按回车开始，ESC返回")) / 2;
    outtextxy(x, 420, "点击选择，按回车开始，ESC返回");

    EndBatchDraw();

    while (true)
    {
        ExMessage msg = getmessage();
        if (msg.message == WM_LBUTTONDOWN)
        {
            int mx = (int)((msg.x - g_offsetX) / g_scale);
            int my = (int)((msg.y - g_offsetY) / g_scale);

            int planeGap = BGWEIGHT / 3;
            for (int i = 0; i < 3; i++)
            {
                int px = planeGap * i + planeGap / 2 - myAirWEIGHT / 2;
                if (mx >= px && mx <= px + myAirWEIGHT &&
                    my >= planeY && my <= planeY + myAirHEIGHT)
                {
                    currentPlane = i;

                    // 重绘选中状态
                    BeginBatchDraw();
                    setDrawScale();
                    putimage(0, 0, &imgStartBg);
                    settextstyle(28, 0, "宋体");
                    settextcolor(YELLOW);
                    setbkmode(TRANSPARENT);
                    int xx = (BGWEIGHT - textwidth("选择你的战机")) / 2;
                    outtextxy(xx, 80, "选择你的战机");

                    for (int j = 0; j < 3; j++)
                    {
                        int ppx = planeGap * j + planeGap / 2 - myAirWEIGHT / 2;
                        if (j == currentPlane)
                        {
                            setfillcolor(YELLOW);
                            solidrectangle(ppx - 5, planeY - 5, ppx + myAirWEIGHT + 5, planeY + myAirHEIGHT + 5);
                        }
                        putimageTransparent(ppx, planeY, &imgPlayer[j]);
                        const char* names[3] = { "战机1号", "战机2号", "战机3号" };
                        int nnx = planeGap * j + planeGap / 2 - textwidth(names[j]) / 2;
                        settextstyle(16, 0, "宋体");
                        settextcolor(WHITE);
                        outtextxy(nnx, planeY + myAirHEIGHT + 10, names[j]);
                    }

                    settextstyle(18, 0, "宋体");
                    settextcolor(YELLOW);
                    xx = (BGWEIGHT - textwidth("点击选择，按回车开始，ESC返回")) / 2;
                    outtextxy(xx, 420, "点击选择，按回车开始，ESC返回");
                    EndBatchDraw();
                    break;
                }
            }
        }
        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_RETURN)
        {
            return true;  // 确认选择
        }
        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
        {
            return false; // ESC返回上一页
        }
        Sleep(10);
    }
}

// ==================== 游戏帮助 ====================
void showGameHelp()
{
    cleardevice();
    BeginBatchDraw();
    setDrawScale();
    putimage(0, 0, &imgStartBg);

    settextstyle(28, 0, "宋体");
    settextcolor(YELLOW);
    setbkmode(TRANSPARENT);
    int x = (BGWEIGHT - textwidth("游戏帮助")) / 2;
    outtextxy(x, 60, "游戏帮助");

    settextstyle(16, 0, "宋体");
    settextcolor(WHITE);
    int y = 110;
    outtextxy(30, y, "【单人模式操作】"); y += 25;
    outtextxy(30, y, "WASD / 方向键：移动飞机"); y += 22;
    outtextxy(30, y, "空格键：发射子弹"); y += 22;
    outtextxy(30, y, "ESC键：退出游戏"); y += 30;

    outtextxy(30, y, "【双人联机操作】"); y += 25;
    outtextxy(30, y, "主机：WASD移动，空格射击"); y += 22;
    outtextxy(30, y, "客机：方向键移动，空格射击"); y += 22;
    outtextxy(30, y, "X键：能量满时触发合体"); y += 30;

    outtextxy(30, y, "【道具说明】"); y += 25;
    outtextxy(30, y, "炸弹补给：发射导弹（高伤害）"); y += 22;
    outtextxy(30, y, "子弹补给：双发子弹"); y += 30;

    outtextxy(30, y, "【合体系统】"); y += 25;
    outtextxy(30, y, "击杀敌机积累能量，满后按X合体"); y += 22;
    outtextxy(30, y, "合体后主机控制移动，客机控制射击"); y += 22;
    outtextxy(30, y, "合体状态HP翻倍，伤害提升10%"); y += 30;

    settextstyle(18, 0, "宋体");
    settextcolor(YELLOW);
    x = (BGWEIGHT - textwidth("按任意键返回")) / 2;
    outtextxy(x, 550, "按任意键返回");

    EndBatchDraw();
    waitForKeyPress();
}

// ==================== 淡入效果 ====================
void screenFadeIn(int step, int delay)
{
    for (int i = 0; i <= 255; i += step)
    {
        BeginBatchDraw();
        setDrawScale();
        putimage(0, 0, &imgStartBg);
        setfillcolor(BLACK);
        HDC hdc = GetImageHDC(NULL);
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)(255 - i), 0 };
        AlphaBlend(hdc, 0, 0, BGWEIGHT, BGHEIGHT, hdc, 0, 0, BGWEIGHT, BGHEIGHT, bf);
        EndBatchDraw();
        Sleep(delay);
    }
}

// ==================== 淡出效果 ====================
void screenFadeOut(int step, int delay)
{
    for (int i = 0; i <= 255; i += step)
    {
        BeginBatchDraw();
        setDrawScale();
        putimage(0, 0, &imgBg[0]);
        setfillcolor(BLACK);
        HDC hdc = GetImageHDC(NULL);
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)i, 0 };
        AlphaBlend(hdc, 0, 0, BGWEIGHT, BGHEIGHT, hdc, 0, 0, BGWEIGHT, BGHEIGHT, bf);
        EndBatchDraw();
        Sleep(delay);
    }
}

// ==================== 游戏主入口（全屏版） ====================
void start()
{
    // 获取屏幕尺寸
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // 计算缩放比例，保持游戏宽高比，选较小的那个比例
    float scaleW = (float)screenW / BGWEIGHT;
    float scaleH = (float)screenH / BGHEIGHT;
    g_scale = (scaleW < scaleH) ? scaleW : scaleH;

    // 计算游戏画面实际像素大小
    int gameW = (int)(BGWEIGHT * g_scale);
    int gameH = (int)(BGHEIGHT * g_scale);

    // 计算偏移量，让游戏画面在屏幕中居中
    g_offsetX = (screenW - gameW) / 2;
    g_offsetY = (screenH - gameH) / 2;

    // 创建和屏幕一样大的窗口
    initgraph(screenW, screenH, 1);

    // 去掉窗口边框和标题栏，实现真正全屏
    SetWindowLong(GetHWnd(), GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(GetHWnd(), HWND_TOP, 0, 0, screenW, screenH, SWP_FRAMECHANGED);

    SetWindowText(GetHWnd(), "飞机大战 - 双人联机版");

    // 初始化链表
    INIT_LIST_HEAD(&enemyPlane_list);
    INIT_LIST_HEAD(&myBull_list);
    INIT_LIST_HEAD(&enemyBullet_list);
    INIT_LIST_HEAD(&bossBullet_list);
    INIT_LIST_HEAD(&explosion_list);
    INIT_LIST_HEAD(&supply_list);

    loadAllResources();

    while (true)
    {
        g_netMode = 0;
        if (g_netMgr) { delete g_netMgr; g_netMgr = NULL; }

        screenFadeIn();

        if (!showStartScreen())
        {
            // 退出游戏
            mciSendString("close all", 0, 0, 0);
            closegraph();
            if (g_netMgr) { g_netMgr->close(); delete g_netMgr; g_netMgr = NULL; }
            return;
        }

        screenFadeOut();

        init();
        srand((unsigned int)time(NULL));

        if (g_netMode == 1)
        {
            // 主机模式
            netHostGame();
        }
        else if (g_netMode == 2)
        {
            // 客机模式
            netClientGame();
        }
        else
        {
            // 单人模式
            while (1)
            {
                char bgmStatus[16];
                mciSendString("status bgm mode", bgmStatus, 16, 0);
                if (strcmp(bgmStatus, "stopped") == 0)
                {
                    mciSendString("play bgm from 0", 0, 0, 0);
                }

                play();
                updateBgScroll();
                createEnemy();
                move();
                DrawMap();

                if (isEnd == 1)
                {
                    cleardevice();
                    BeginBatchDraw();
                    setDrawScale();
                    settextstyle(36, 0, "宋体");
                    settextcolor(RED);
                    setbkmode(TRANSPARENT);
                    int x = (BGWEIGHT - textwidth("游戏结束")) / 2;
                    outtextxy(x, 250, "游戏结束");
                    settextstyle(20, 0, "宋体");
                    settextcolor(WHITE);
                    TCHAR str[64];
                    wsprintf(str, "最终分数：%d", score);
                    x = (BGWEIGHT - textwidth(str)) / 2;
                    outtextxy(x, 310, str);
                    x = (BGWEIGHT - textwidth("按任意键返回")) / 2;
                    outtextxy(x, 370, "按任意键返回");
                    EndBatchDraw();
                    waitForKeyPress();
                    break;
                }
                if (isEnd == 2)
                {
                    showLevelPass();
                    if (level >= TOTAL_LEVEL)
                    {
                        showGameWin();
                        break;
                    }
                    level++;
                    initLevel(level);
                }
                Sleep(10);
            }
        }

        // 清理网络
        if (g_netMgr)
        {
            g_netMgr->close();
            delete g_netMgr;
            g_netMgr = NULL;
        }
    }
}