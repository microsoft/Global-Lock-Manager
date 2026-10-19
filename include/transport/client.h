#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <common/config.h>
#include <common/map.h>
#include <common/typedefs.h>
#include <storage/lock_manager/locks/lock_mode.h>
#include <transport/message.h>

#include <spdlog/spdlog.h>

namespace transport {

/**
 * This is a generic client class which supports sending requests by a calling
 * thread using the sendRequest() method. The client has a 'receiving' thread
 * which receives the response and provides it to the sending thread which
 * sleeps when the request is in-flight.
 */
class Client {
public:
    Client(uint32_t numSendThreads, uint32_t sendBufferSize,
        uint32_t recvBufferSize, uint32_t coreToPinTo, bool initTransport);

    void denyRequestsForTearDown();

protected:
    struct RequestSlot {
        explicit RequestSlot(uint32_t sendBufferSize) :
            sendBuffer{std::make_unique<uint8_t[]>(sendBufferSize)},
            isTaken{false}, responseStatus{response_status_t::WAITING},
            responseServerTimestamp{getTimestamp()}
            {}

        std::unique_ptr<uint8_t[]> sendBuffer;
        bool isTaken;
        response_status_t responseStatus;
        uint64_t responseServerTimestamp;
        std::mutex mtx;
        std::condition_variable condVar;
    };

    void initLogger(const char* absolutePathLogFile);

    void initClient(const std::string& serverIp, int serverPort);
    void shutdownClient() const;

    void sendRequest(RequestSlot* slot, uint32_t slotIdx);
    void handleResponses();

    uint32_t getRequestSlotIdx(uint64_t requestId);
    void releaseRequestSlot(RequestSlot* slot, uint32_t slotIdx);

    int sendBytes(const uint8_t* buffer) const;
    int recvBytes(const uint8_t* buffer) const;

protected:
    uint32_t numSendThreads;
    std::atomic<bool> stopClient;

    Map<uint64_t, uint8_t> requestIdToSlotIdx;
    const uint32_t sendBufferSize, recvBufferSize;
    std::atomic<bool> isInitialized;
    const bool initializeClient;
    std::thread receivingThread;

    std::vector<std::unique_ptr<RequestSlot>> requestSlots;

    int socketDesc;
    std::mutex mtxSend;
    std::atomic<uint32_t> nextSlotIdx;

    std::shared_ptr<spdlog::logger> clientLogger;
};

} // namespace transport
