#ifndef _NET_MANAGER_H_
#define _NET_MANAGER_H_

#include <winsock2.h>
#include "game_protocol.h"
#pragma comment(lib, "ws2_32.lib")

#define RECV_BUF_SIZE   65536

class NetManager {
public:
    NetManager();
    ~NetManager();

    // 主机模式：创建房间，监听指定端口，等待客户端连接（阻塞）
    bool hostGame(int port = 8888);

    // 主机模式分步：开始监听（非阻塞，配合 tryAccept 使用）
    bool startListen(int port = 8888);
    // 主机模式分步：尝试接受连接（非阻塞），返回 true 表示有客户端连上了
    bool tryAccept();

    // 客机模式：加入房间，连接到指定IP和端口
    bool joinGame(const char* ip, int port = 8888);

    // 发送消息
    bool sendMessage(BYTE msgType, const char* data, int dataLen);

    // 尝试接收一条消息（非阻塞）
    int recvMessage(BYTE& msgType, char* buffer, int maxLen);

    // 是否连接成功
    bool isConnected() const { return m_connected; }

    // 是否是主机
    bool isHost() const { return m_isHost; }

    // 关闭连接
    void close();

private:
    SOCKET m_listenSocket;  // 主机的监听socket
    SOCKET m_dataSocket;    // 数据传输socket
    bool m_isHost;
    bool m_connected;

    char m_recvBuf[RECV_BUF_SIZE];  // 接收缓冲区（处理粘包）
    int m_recvBufLen;               // 缓冲区中已有数据长度

    bool initWSA();
    void cleanupWSA();
};

#endif