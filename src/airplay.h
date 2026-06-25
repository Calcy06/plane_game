#ifndef _AIRPLAY_H_
#define _AIRPLAY_H_

#include "plane_list.h"
#include "net_manager.h"
#include <easyx.h>
#include <conio.h>
#include <stdlib.h>

// 游戏逻辑基准尺寸
#define BGWEIGHT 400
#define BGHEIGHT 600

// 基础尺寸
#define myAirWEIGHT   50
#define myAirHEIGHT   70
#define enemyPlaneWEIGHT 50
#define enemyPlaneHEIGHT 70
#define BulletWEIGHT  10
#define BulletHEIGHT  10
#define BOSS_WIDTH    120
#define BOSS_HEIGHT   120

// 导弹专属尺寸
#define MISSILE_WIDTH   22
#define MISSILE_HEIGHT  34

// 道具参数
#define SUPPLY_WIDTH    45
#define SUPPLY_HEIGHT   60
#define BUFF_DURATION   15000
#define MISSILE_DAMAGE  120

// 超级战机尺寸
#define SUPER_WIDTH         80
#define SUPER_HEIGHT        100

// 合体系统常量
#define FUSION_ENERGY_MAX       500     // 能量条满值
#define FUSION_DURATION         10000   // 合体持续时间（毫秒）
#define FUSION_SPEED_BOOST      1.1f    // 移动速度加成10%
#define FUSION_DAMAGE_BOOST     1.1f    // 伤害加成10%
#define FUSION_FIRE_BOOST       1.1f    // 射速加成10%

// 开始界面常量
#define BTN_WIDTH       120
#define BTN_HEIGHT      40
#define BTN_X           ((BGWEIGHT - BTN_WIDTH) / 2)
#define BTN_START_Y     220
#define BTN_SELECT_Y    280
#define BTN_HELP_Y      340
#define BTN_EXIT_Y      400
#define BTN_COLOR_NORMAL 0xCCCCCC
#define BTN_COLOR_HOVER  0x888888
#define BTN_TEXT_COLOR   BLACK

// 关卡常量
#define TOTAL_LEVEL     5
#define PLAYER_INIT_HP  500     // 从300加到500，容错空间更大
//#define PLAYER_INIT_HP  5000000
#define CRASH_DAMAGE    50      // 碰撞伤害调回50

// 目标分数
const int levelTargetScore[TOTAL_LEVEL] = { 200, 400, 700, 1100, 1600 };
//const int levelTargetScore[TOTAL_LEVEL] = { 10, 20, 30, 40, 50 };

// Boss血量（比之前降一点）
const int bossHpList[TOTAL_LEVEL] = { 350, 700, 1200, 1850, 2800 };
//const int bossHpList[TOTAL_LEVEL] = { 100, 200, 300, 400, 500 };

// 敌机分值
const int enemyScore[7] = { 10, 20, 30, 50, 80, 120, 200 };

// 我方子弹伤害（调回一点）
const int myBulletDamage[8] = { 8, 11, 16, 24, 33, 46, 60, 95 };

// 敌方子弹伤害（比之前降一点）
const int enemyBulletDamage[6] = { 5, 8, 12, 16, 21, 27 };

// 超级战机结构体
typedef struct {
    int x;
    int y;
    int speed;
} SuperPlane;

// 全局变量声明（都是extern，不能赋值！）
extern IMAGE imgBg[5];
extern IMAGE imgPlayer[3];
extern IMAGE imgEnemy[7];
extern IMAGE imgMyBullet[8];
extern IMAGE imgEnemyBullet[6];
extern IMAGE imgBoss[TOTAL_LEVEL];
extern IMAGE imgExplosion[7];
extern IMAGE imgStartBg;
extern IMAGE imgSupply[2];
extern IMAGE imgSuperPlane;

extern Node myPlane;
extern Node myPlane2;
extern int isEnd;
extern int score;
extern int totalScore;  // 累计总分（所有关卡叠加）
extern int level;
extern int playerHp;
extern int playerMaxHp;
extern int bossActive;
extern Node boss;
extern int bgOffset;
extern int enemySpawnTimer;
extern int lastFrameTime;
extern int currentBuff;
extern DWORD buffEndTime;
extern int currentPlane;

// 合体系统全局变量
extern int g_fusionActive;
extern int g_fusionEnergyP1;
extern int g_fusionEnergyP2;
extern int g_fusionHp;
extern int g_fusionMaxHp;
extern DWORD g_fusionEndTime;
extern DWORD g_fusionShootTimer;
extern SuperPlane g_superPlane;

// 全屏缩放相关
extern float g_scale;
extern int g_offsetX;
extern int g_offsetY;

// 网络模式：0=单机，1=主机，2=客机
extern int g_netMode;
extern NetManager* g_netMgr;
extern int g_player2Plane;

// 函数声明
void init();
void DrawMap();
void play();
void start();
bool showStartScreen();
void showGameHelp();
bool showPlaneSelect();
void screenFadeIn(int step = 5, int delay = 10);
void screenFadeOut(int step = 5, int delay = 10);
void putimageTransparent(int x, int y, IMAGE* srcImg, COLORREF transColor = RGB(0, 0, 0));
void loadAllResources();
void initLevel(int lv);
void updateBgScroll();
void createEnemy();
void enemyShoot(Node* enemy);
void updateEnemyBullets();
void bossAI();
void updateBossBullets();
void createExplosion(int x, int y);
void updateExplosion();
void drawUI();
void showLevelPass();
void showGameWin();
void waitForKeyPress();
void createSupply(int x, int y, int type);
void updateSupply();
void setDrawScale();

// 合体系统函数声明
void addFusionEnergy(int amount);
int isFusionReady();
void startFusion();
void endFusion();
void updateFusion();
void superPlaneShoot();
void playFusionAnimation();
int superPlane_isConflict(Node* enemy);
int superPlane_bulletConflict(Node* bullet);

// 双人联机相关函数
void create_myBull_from(int x, int y);
void netHostGame();
void netClientGame();
int serializeGameState(char* buf, int maxLen);
void deserializeGameState(const char* buf, int len);
int showNetModeSelect();
bool showIpInput(char* ipBuf, int bufLen);
void showWaitingScreen(const char* text);

#endif