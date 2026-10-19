#pragma once

#include "client.h"
#include "server.h"

namespace transport {

template<typename T>
class Transport : public Server<T>, public Client {
public:
    Transport(int port, uint32_t recvServerBufSize, uint32_t sendServerBufSize,
        uint32_t maxNumRequests, std::string serverIp, int serverPort,
        uint32_t numClientSendThreads, uint32_t sendClientBufferSize,
        uint32_t recvClientBufferSize, bool initializeServerThenClient,
        const char* absolutePathLogFile, bool initTransport);
};

} // namespace transport
