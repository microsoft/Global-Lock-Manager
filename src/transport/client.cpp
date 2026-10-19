#include <transport/client.h>

#include <cassert>
#include <unistd.h>

#include <common/thread_pin.h>
#include <common/wait_time.h>

#include <spdlog/sinks/basic_file_sink.h>

namespace transport {

Client::Client(uint32_t numSendThreads, uint32_t sendBufferSize,
    uint32_t recvBufferSize, uint32_t coreToPinTo, bool initTransport) :
    numSendThreads{numSendThreads}, stopClient{false},
    requestIdToSlotIdx{numSendThreads * 10}, sendBufferSize{sendBufferSize},
    recvBufferSize{recvBufferSize}, isInitialized{false},
    initializeClient{initTransport},
    receivingThread{&Client::handleResponses, this} {
    for (auto i = 0u; i < numSendThreads; i++) {
        requestSlots.emplace_back(std::make_unique<RequestSlot>(
            sendBufferSize));
    }
    pinThread(receivingThread, coreToPinTo);
}

void Client::shutdownClient() const {
    if (initializeClient) {
        close(socketDesc);
    }
    spdlog::info("shutdown client connection");
}

void Client::initLogger(const char* absolutePathLogFile) {
    clientLogger = spdlog::basic_logger_mt("client-logger", absolutePathLogFile);
}

void Client::initClient(const std::string& serverIp, int serverPort) {
    socketDesc = socket(AF_INET , SOCK_STREAM , 0);
    if (socketDesc == -1) {
        clientLogger->info("Could not create socket");
        throw;
    }

    struct sockaddr_in server{};
    server.sin_addr.s_addr = inet_addr("192.0.2.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );

    // Connect to remote server
    if (connect(socketDesc , (struct sockaddr *)&server , sizeof(server)) < 0) {
        spdlog::error("connecting to the server failed");
        throw;
    }

    isInitialized = true;
}

void Client::denyRequestsForTearDown() {
    for (auto i = 0u; i < numSendThreads; i++) {
        requestSlots[i]->responseStatus = response_status_t::FAILURE;
        requestSlots[i]->condVar.notify_one();
    }
}

void Client::sendRequest(RequestSlot* slot, uint32_t slotIdx)  {
    int iResult;
    {
        // add a flag to return directly.
        // set response to failure and return.
        std::unique_lock<std::mutex> lock(mtxSend);
        iResult = sendBytes(slot->sendBuffer.get());
        if (iResult == -1) {
            slot->responseStatus = response_status_t::FAILURE;
            return;
        }
    }

    auto requestId = ((uint64_t*) slot->sendBuffer.get())[0u];
    auto payload = slot->sendBuffer[8u];
    std::unique_lock<std::mutex> lock(slot->mtx);
    if (slot->responseStatus == response_status_t::WAITING) {
        slot->condVar.wait(lock, [&slot] {
            return slot->responseStatus != response_status_t::WAITING;
        });
    }
}

void Client::releaseRequestSlot(RequestSlot* slot, uint32_t slotIdx) {
    slot->responseStatus = response_status_t::WAITING;
    slot->isTaken = false; /* relase the slot */
    requestIdToSlotIdx.remove(slotIdx);
}

uint32_t Client::getRequestSlotIdx(uint64_t requestId) {
    auto currSlot = nextSlotIdx.load() % numSendThreads;
    do {
        std::unique_lock<std::mutex> lock(requestSlots[currSlot]->mtx, std::try_to_lock);
        if (lock.owns_lock() && !requestSlots[currSlot]->isTaken) {
            requestSlots[currSlot]->isTaken = true;
            requestIdToSlotIdx.put(requestId, currSlot);
            assert(currSlot == requestIdToSlotIdx.get(requestId, UINT8_MAX));
            nextSlotIdx++;
            return currSlot;
        } else {
            currSlot = nextSlotIdx++ % numSendThreads;
        }
    } while (true);
}

void Client::handleResponses() {
    if (!initializeClient) /* we don't initialize transport */ return;
    while(!isInitialized) { /* wait till client is initialized */ }
    auto recvBuffer = std::make_unique<uint8_t[]>(Client::recvBufferSize);
    auto recvBuffAsIntArr = (uint64_t*) recvBuffer.get();
    do {
        auto iResult = Client::recvBytes(recvBuffer.get()); // recvBuffer
        if (iResult > 0) {
            // pull the data and give the response to the requesting thread.
            auto requestId = recvBuffAsIntArr[0u];
            auto slotIdx = requestIdToSlotIdx.get(requestId, UINT8_MAX);
            assert(slotIdx != UINT8_MAX);
            auto slot = requestSlots[slotIdx].get();
            /* if (enableLogging) {
                clientLogger->info("Received response (RequestId:{}) : {}",
                    requestId, ResponseStatusAsStr[response]);
            } */
            assert(requestId == recvBuffAsIntArr[0u]);
            slot->responseServerTimestamp = recvBuffAsIntArr[1u];
            slot->responseStatus = (response_status_t) recvBuffer[16u];
            slot->condVar.notify_one();
        } else {
            break;
        }
    } while (!stopClient);
}

int Client::sendBytes(const uint8_t* buffer) const {
    auto bytesLeft = sendBufferSize;
    while(bytesLeft > 0) {
        auto iResult = send(socketDesc, (const char*) buffer, bytesLeft, 0);
        if (iResult == -1) {
            spdlog::error("sending response failed with error: {}", errno);
            return errno;
        }
        bytesLeft -= iResult;
        buffer += iResult;
    }
    return 0;
}

int Client::recvBytes(const uint8_t* buffer) const {
    auto bytesLeft = recvBufferSize;
    ssize_t iResult;
    do {
        iResult = recv(socketDesc, (char*) buffer, bytesLeft, 0);
        if (iResult > 0) {
            buffer += iResult;
            bytesLeft -= iResult;
        } else {
            if (iResult == 0) {
                spdlog::info("server connection closed");
            } else if (iResult == -1) {
                spdlog::error("recv failed with error: {}", errno);
            }
            return -1;
        }
    } while (bytesLeft > 0);
    return (int) iResult;
}

} // namespace transport
