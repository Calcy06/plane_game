#ifndef _GAME_PROTOCOL_H_
#define _GAME_PROTOCOL_H_
#include <windows.h>

// ========== 网络协议定义 ==========

// 消息类型
#define MSG_INPUT       1   // 客机→主机：玩家按键输入
#define MSG_GAME_STATE  2   // 主机→客机：游戏状态快照
#define MSG_HELLO       3   // 客机→主机：连接确认
#define MSG_START       4   // 主机→客机：游戏开始
#define MSG_EVENT       5   // 主机→客机：游戏事件（过关/失败/胜利）
#define MSG_FUSION_SYNC 6   // 主机→客机：合体状态同步

// 按键位标志
#define KEY_UP      0x01
#define KEY_DOWN    0x02
#define KEY_LEFT    0x04
#define KEY_RIGHT   0x08
#define KEY_SHOOT   0x10
#define KEY_FUSION  0x20  // X键：触发合体

// 玩家输入数据包
struct PlayerInput {
    BYTE keyState;   // 按键状态，按位存储
};

// 游戏事件类型
#define EVENT_LEVEL_PASS    1   // 过关
#define EVENT_GAME_OVER     2   // 游戏失败
#define EVENT_GAME_WIN      3   // 游戏胜利

// 游戏事件数据包
struct GameEventMsg {
    int eventType;
    int param1;  // 附加参数1（如分数）
    int param2;  // 附加参数2（如关卡）
};

// 合体状态同步数据包
struct FusionSyncMsg {
    int energy1;        // 玩家1能量
    int energy2;        // 玩家2能量
    int fusionActive;   // 是否处于合体状态
    int fusionRemain;   // 合体剩余时间（毫秒）
    int superX;         // 超级战机X坐标
    int superY;         // 超级战机Y坐标
    int fusionHp;       // 合体血量
    int fusionMaxHp;    // 合体最大血量
};

// 数据包头部
struct PacketHeader {
    WORD magic;      // 起始标记 0x55AA
    WORD dataLen;    // 数据长度（不包括头部）
    BYTE msgType;    // 消息类型
};

#define PACKET_MAGIC    0x55AA
#define HEADER_SIZE     (sizeof(PacketHeader))

#endif