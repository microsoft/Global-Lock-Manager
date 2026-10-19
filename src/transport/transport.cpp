#include <transport/transport.h>

namespace transport {

template<typename T>
Transport<T>::Transport(int port, uint32_t recvServerBufSize,
    uint32_t sendServerBufSize, uint32_t maxNumRequests, std::string serverIp,
    int serverPort, uint32_t numClientSendThreads, uint32_t sendClientBufferSize,
    uint32_t recvClientBufferSize, bool initializeServerThenClient,
    const char* absolutePathLogFile, bool initTransport) :
    Server<T>(recvServerBufSize, sendServerBufSize, maxNumRequests, initTransport),
    Client(numClientSendThreads, sendClientBufferSize, recvClientBufferSize,
        absolutePathLogFile != nullptr, initTransport) {
    if (initTransport) {
        if (initializeServerThenClient) {
            Server<T>::initServer(port);
            Client::initClient(serverIp, serverPort);
        } else {
            Client::initClient(serverIp, serverPort);
            Server<T>::initServer(port);
        }

        if (absolutePathLogFile != nullptr) {
            auto clientFilename = initializeServerThenClient ?
                "client-glm.log" : "client-nonGlm.log" ;
            auto absolutePathToClientFile = new char[
				strlen(absolutePathLogFile) + strlen(clientFilename) + 1];
            strcpy(absolutePathToClientFile, absolutePathLogFile);
            strcat(absolutePathToClientFile, clientFilename);
            Client::initLogger(absolutePathToClientFile);
        }
        spdlog::info("{} finished initializing transport",
            initializeServerThenClient ? "glm" : "nonGlm");
    }
}

template class Transport<request_msg_t>;

} // namespace transport
