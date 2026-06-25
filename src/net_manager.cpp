#include "net_manager.h"
#include <stdio.h>

NetManager::NetManager()
    : m_listenSocket(INVALID_SOCKET)
    , m_dataSocket(INVALID_SOCKET)
    , m_isHost(false)
    , m_connected(false)
    , m_recvBufLen(0)
{
    initWSA();
}

NetManager::~NetManager()
{
    close();
    cleanupWSA();
}

bool NetManager::initWSA()
{
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void NetManager::cleanupWSA()
{
    WSACleanup();
}

bool NetManager::hostGame(int port)
{
    m_isHost = true;

    // 创建监听socket
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET) {
        return false;
    }

    // 允许地址复用
    BOOL opt = TRUE;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // 绑定地址
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    // 开始监听
    if (listen(m_listenSocket, 1) == SOCKET_ERROR) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    // 阻塞等待客户端连接
    m_dataSocket = accept(m_listenSocket, NULL, NULL);
    if (m_dataSocket == INVALID_SOCKET) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    // 设置数据socket为非阻塞
    u_long mode = 1;
    ioctlsocket(m_dataSocket, FIONBIO, &mode);

    m_connected = true;
    m_recvBufLen = 0;
    return true;
}

bool NetManager::startListen(int port)
{
    m_isHost = true;

    // 创建监听socket
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET) {
        return false;
    }

    // 允许地址复用
    BOOL opt = TRUE;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // 绑定地址
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    // 开始监听
    if (listen(m_listenSocket, 1) == SOCKET_ERROR) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    // 设置为非阻塞
    u_long mode = 1;
    ioctlsocket(m_listenSocket, FIONBIO, &mode);

    return true;
}

bool NetManager::tryAccept()
{
    if (m_listenSocket == INVALID_SOCKET) {
        return false;
    }

    m_dataSocket = accept(m_listenSocket, NULL, NULL);
    if (m_dataSocket == INVALID_SOCKET) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            return false; // 还没有连接，正常
        }
        return false; // 真正的错误
    }

    // 设置数据socket为非阻塞
    u_long mode = 1;
    ioctlsocket(m_dataSocket, FIONBIO, &mode);

    m_connected = true;
    m_recvBufLen = 0;
    return true;
}

bool NetManager::joinGame(const char* ip, int port)
{
    m_isHost = false;

    m_dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_dataSocket == INVALID_SOCKET) {
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(m_dataSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_dataSocket);
        m_dataSocket = INVALID_SOCKET;
        return false;
    }

    // 设置为非阻塞
    u_long mode = 1;
    ioctlsocket(m_dataSocket, FIONBIO, &mode);

    m_connected = true;
    m_recvBufLen = 0;
    return true;
}

bool NetManager::sendMessage(BYTE msgType, const char* data, int dataLen)
{
    if (!m_connected || m_dataSocket == INVALID_SOCKET) {
        return false;
    }

    int totalLen = HEADER_SIZE + dataLen;
    char* sendBuf = new char[totalLen];

    PacketHeader* header = (PacketHeader*)sendBuf;
    header->magic = PACKET_MAGIC;
    header->dataLen = dataLen;
    header->msgType = msgType;

    if (dataLen > 0 && data != NULL) {
        memcpy(sendBuf + HEADER_SIZE, data, dataLen);
    }

    int sent = 0;
    while (sent < totalLen) {
        int ret = send(m_dataSocket, sendBuf + sent, totalLen - sent, 0);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                Sleep(1);
                continue;
            }
            m_connected = false;
            delete[] sendBuf;
            return false;
        }
        sent += ret;
    }

    delete[] sendBuf;
    return true;
}

int NetManager::recvMessage(BYTE& msgType, char* buffer, int maxLen)
{
    if (!m_connected || m_dataSocket == INVALID_SOCKET) {
        return -1;
    }

    // 先把socket里的数据读到缓冲区
    while (m_recvBufLen < RECV_BUF_SIZE) {
        int ret = recv(m_dataSocket, m_recvBuf + m_recvBufLen, RECV_BUF_SIZE - m_recvBufLen, 0);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                break; // 没有更多数据了
            }
            m_connected = false;
            return -1;
        }
        if (ret == 0) {
            m_connected = false;
            return -1;
        }
        m_recvBufLen += ret;
    }

    // 尝试解析一个完整的包
    if (m_recvBufLen < HEADER_SIZE) {
        return 0;
    }

    PacketHeader* header = (PacketHeader*)m_recvBuf;
    if (header->magic != PACKET_MAGIC) {
        // 数据错位，查找下一个起始标记
        for (int i = 1; i < m_recvBufLen - 1; i++) {
            if ((BYTE)m_recvBuf[i] == 0x55 && (BYTE)m_recvBuf[i + 1] == 0xAA) {
                memmove(m_recvBuf, m_recvBuf + i, m_recvBufLen - i);
                m_recvBufLen -= i;
                return recvMessage(msgType, buffer, maxLen);
            }
        }
        // 没找到有效标记，保留最后一个字节继续等
        if (m_recvBufLen > 0) {
            m_recvBuf[0] = m_recvBuf[m_recvBufLen - 1];
            m_recvBufLen = 1;
        }
        return 0;
    }

    int dataLen = header->dataLen;
    if (dataLen > 100000) { // 异常数据保护
        m_recvBufLen = 0;
        return 0;
    }
    if (m_recvBufLen < HEADER_SIZE + dataLen) {
        return 0; // 数据不完整
    }

    // 有完整的包了
    msgType = header->msgType;
    int copyLen = dataLen < maxLen ? dataLen : maxLen;
    if (copyLen > 0) {
        memcpy(buffer, m_recvBuf + HEADER_SIZE, copyLen);
    }

    // 移除已处理的数据
    int packetLen = HEADER_SIZE + dataLen;
    if (m_recvBufLen > packetLen) {
        memmove(m_recvBuf, m_recvBuf + packetLen, m_recvBufLen - packetLen);
    }
    m_recvBufLen -= packetLen;

    return copyLen;
}

void NetManager::close()
{
    if (m_dataSocket != INVALID_SOCKET) {
        closesocket(m_dataSocket);
        m_dataSocket = INVALID_SOCKET;
    }
    if (m_listenSocket != INVALID_SOCKET) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }
    m_connected = false;
    m_recvBufLen = 0;
}