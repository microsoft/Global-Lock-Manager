#include <transport/server.h>

#include <spdlog/sinks/basic_file_sink.h>

#include <transport/message.h>

namespace transport {

template<typename T>
Server<T>::Server(uint32_t recvBufferSize, uint32_t sendBufferSize,
    uint32_t maxNumRequests, bool initTransport) :
    isInitialized{false}, stopServer{false},
    recvBufferSize{recvBufferSize}, sendBufferSize{sendBufferSize},
    recvBuffer{std::make_unique<uint8_t[]>(recvBufferSize)},
    sendBuffer{std::make_unique<uint8_t[]>(sendBufferSize)},
    requestsQueue{maxNumRequests}, responsesQueue{maxNumRequests},
    initializeServer{initTransport}
    {}

template<typename T>
void Server<T>::initServer(uint32_t serverPort) {
    serverSocketDesc = socket(AF_INET , SOCK_STREAM , 0);
    if (serverSocketDesc == -1) {
        spdlog::error("Could not create socket");
        throw;
    }

    sockaddr_in server{}, client{};
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(serverPort);

    if (bind(serverSocketDesc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        spdlog::error("binding a socket to a port failed");
        throw;
    }

    listen(serverSocketDesc , 3 /* listening mode */);

    // We expect the server to accept all client connections upfront.
    auto socketLen = sizeof(struct sockaddr_in);
    clientSocketDesc = accept(serverSocketDesc, (struct sockaddr *)& client,
            (socklen_t*)& socketLen);
    if (clientSocketDesc < 0) {
        spdlog::error("accepting the client connection failed");
        throw;
    }
}

template<typename T>
void Server<T>::recvThreadFunc() {
    if (!initializeServer) /* we don't initialize transport */ {
		return;
	}
    while(!isInitialized) { /* wait till server is initialized */ }
    T request;
    do {
        auto err = recvBytesFromBuffer();
        if (err <= 0) {
            auto emptyResponse = T();
            while (!responsesQueue.push(emptyResponse));
            // responsesQueue.push(T());
            break; // connection closed or error encountered.
        }
        deserializeRequestFromRecvBuffer(request);
        requestsQueue.push(request);
    } while (!stopServer);
}

template<typename T>
void Server<T>::sendThreadFunc() {
    if (!initializeServer) /* we don't initialize transport */ {
		return;
	}
    T response;
    do {
        while (!responsesQueue.pop(response));
        // auto response = responsesQueue.waitAndPop();
        if (response.signalsBreak()) {
            spdlog::info("send server thread signalled to break");
            break;
        }
        serializeResponseToSendBuffer(response);
        auto err = sendBytesInBuffer();
        if (err <= 0) {
            spdlog::info("send server thread errored to break");
            break; // connection closed or error encountered.
        }
    } while (!stopServer);
}

template<typename T>
int Server<T>::recvBytesFromBuffer() {
    auto buffPtr = recvBuffer.get();
    auto bytesLeft = recvBufferSize;
    int iResult;
    do {
        iResult = recv(clientSocketDesc, (char*) buffPtr, bytesLeft, 0);
        if (iResult > 0) {
            buffPtr += iResult;
            bytesLeft -= iResult;
        }
        else {
            if (iResult == 0) {
                spdlog::info("server connection closed");
            } else if (iResult == -1) {
                spdlog::error("recv failed with error: {}", errno);
            }
            return -1;
        }
    } while (bytesLeft > 0);
    return recvBufferSize;
}

template<typename T>
int Server<T>::sendBytesInBuffer() {
    auto buffPtr = (char*) sendBuffer.get();
    auto bytesLeft = sendBufferSize;
    while(bytesLeft > 0) {
        auto iResult = send(clientSocketDesc, buffPtr, bytesLeft, 0);
        if (iResult == -1) {
            spdlog::error("sending response failed with error: {}", errno);
            return -1;
        }
        bytesLeft -= iResult;
        buffPtr += iResult;
    }
    return sendBufferSize;
}

template class Server<request_msg_t>;

} // namespace transport
