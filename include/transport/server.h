#pragma once

#include <chrono>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <boost/lockfree/queue.hpp>
#include <spdlog/spdlog.h>

#include <common/config.h>

namespace transport {

template<typename T>
class Server {
public:
    Server(uint32_t recvBufferSize, uint32_t sendBufferSize,
        uint32_t numMaxRequests, bool initTransport);

protected:
    void initServer(uint32_t serverPort);

    void recvThreadFunc();
    void sendThreadFunc();
    int recvBytesFromBuffer();
    int sendBytesInBuffer();

    virtual void deserializeRequestFromRecvBuffer(T& response) = 0;
    virtual void serializeResponseToSendBuffer(T& response) = 0;

protected:
    std::atomic<bool> isInitialized;
    std::atomic<bool> stopServer;
    uint32_t recvBufferSize;
    uint32_t sendBufferSize;
    std::unique_ptr<uint8_t[]> recvBuffer;
    std::unique_ptr<uint8_t[]> sendBuffer;
    boost::lockfree::queue<T> requestsQueue;
    boost::lockfree::queue<T> responsesQueue;
    const bool initializeServer;

    int serverSocketDesc, clientSocketDesc;
};

} // namespace transport
