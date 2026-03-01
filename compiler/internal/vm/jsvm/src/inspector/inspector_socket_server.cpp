/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
#include "inspector_socket_server.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <map>
#include <securec.h>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>

#include "jsvm_dfx.h"
#include "jsvm_version.h"
#include "uv.h"
#include "v8_inspector_protocol_json.h"
#include "zlib.h"

namespace jsvm {
namespace inspector {

// Function is declared in inspector_io.h so the rest of the node does not
// depend on inspector_socket_server.h
std::string FormatWsAddress(const std::string& host, int port, const std::string& targetId, bool includeProtocol);
namespace {
std::string FormatHostPort(const std::string& host, int port)
{
    // Host is valid (socket was bound) so colon means it's a v6 IP address
    bool isIPV6 = (host.find(':') != std::string::npos);
    std::ostringstream url;
    if (isIPV6) {
        url << '[' << host << ']';
    } else {
        url << host;
    }
    url << ':' << port;
    return url.str();
}

void Escape(std::string* string)
{
    for (char& c : *string) {
        c = (c == '\"' || c == '\\') ? '_' : c;
    }
}

std::string FormatAddress(const std::string& host, const std::string& targetId, bool includeProtocol)
{
    std::ostringstream url;
    if (includeProtocol) {
        url << "ws://";
    }
    url << host << '/' << targetId;
    return url.str();
}

std::string MapToString(const std::map<std::string, std::string>& object)
{
    bool first = true;
    std::ostringstream json;
    json << "{\n";
    for (const auto& nameValue : object) {
        if (!first) {
            json << ",\n";
        }
        first = false;
        json << "  \"" << nameValue.first << "\": \"";
        json << nameValue.second << "\"";
    }
    json << "\n} ";
    return json.str();
}

std::string MapsToString(const std::vector<std::map<std::string, std::string>>& array)
{
    bool isFirst = true;
    std::ostringstream json;
    json << "[ ";
    for (const auto& object : array) {
        if (!isFirst) {
            json << ", ";
        }
        isFirst = false;
        json << MapToString(object);
    }
    json << "]\n\n";
    return json.str();
}

void SendHttpResponse(InspectorSocket* socket, const std::string& response, int code)
{
    const char headers[] = "HTTP/1.0 %d OK\r\n"
                           "Content-Type: application/json; charset=UTF-8\r\n"
                           "Cache-Control: no-cache\r\n"
                           "Content-Length: %zu\r\n"
                           "\r\n";
    char header[sizeof(headers) + 20];
    int headerLen = snprintf_s(header, sizeof(header), sizeof(header) - 1, headers, code, response.size());
    CHECK(headerLen >= 0);
    socket->Write(header, headerLen);
    socket->Write(response.data(), response.size());
}

const char* MatchPathSegment(const char* path, const char* expected)
{
    size_t len = strlen(expected);
    if (StringEqualNoCaseN(path, expected, len)) {
        if (path[len] == '/') {
            return path + len + 1;
        }
        if (path[len] == '\0') {
            return path + len;
        }
    }
    return nullptr;
}

enum HttpStatusCode : int {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
};

void SendHttpNotFound(InspectorSocket* socket)
{
    SendHttpResponse(socket, "", HttpStatusCode::HTTP_NOT_FOUND);
}

void SendVersionResponse(InspectorSocket* socket)
{
    std::map<std::string, std::string> response;
    response["Protocol-Version"] = "1.1";
    response["Browser"] = "jsvm/" JSVM_VERSION_STRING;
    SendHttpResponse(socket, MapToString(response), HttpStatusCode::HTTP_OK);
}

void SendProtocolJson(InspectorSocket* socket)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    CHECK_EQ(inflateInit(&strm), Z_OK);
    static const size_t decompressedSize = PROTOCOL_JSON[0] * 0x10000u + PROTOCOL_JSON[1] * 0x100u + PROTOCOL_JSON[2];
    strm.next_in = const_cast<uint8_t*>(PROTOCOL_JSON + ByteOffset::BYTE_3);
    strm.avail_in = sizeof(PROTOCOL_JSON) - ByteOffset::BYTE_3;
    std::string data(decompressedSize, '\0');
    strm.next_out = reinterpret_cast<Byte*>(data.data());
    strm.avail_out = data.size();
    CHECK_EQ(inflate(&strm, Z_FINISH), Z_STREAM_END);
    CHECK_EQ(strm.avail_out, 0);
    CHECK_EQ(inflateEnd(&strm), Z_OK);
    SendHttpResponse(socket, data, HttpStatusCode::HTTP_OK);
}
} // namespace

std::string FormatWsAddress(const std::string& host, int port, const std::string& targetId, bool includeProtocol)
{
    return FormatAddress(FormatHostPort(host, port), targetId, includeProtocol);
}

class SocketSession {
public:
    SocketSession(InspectorSocketServer* server, int id, int serverPort);
    void Close()
    {
        wsSocket.reset();
    }
    void Send(const std::string& message);
    void Own(InspectorSocket::Pointer wsSocketParam)
    {
        wsSocket = std::move(wsSocketParam);
    }
    int GetId() const
    {
        return id;
    }
    int ServerPort()
    {
        return serverPort;
    }
    InspectorSocket* GetWsSocket()
    {
        return wsSocket.get();
    }
    void Accept(const std::string& wsKey)
    {
        wsSocket->AcceptUpgrade(wsKey);
    }
    void Decline()
    {
        wsSocket->CancelHandshake();
    }

    class Delegate : public InspectorSocket::Delegate {
    public:
        Delegate(InspectorSocketServer* server, int sessionId) : server(server), usessionId(sessionId) {}
        ~Delegate() override
        {
            server->SessionTerminated(usessionId);
        }
        void OnHttpGet(const std::string& host, const std::string& path) override;
        void OnSocketUpgrade(const std::string& host, const std::string& path, const std::string& wsKey) override;
        void OnWsFrame(const std::vector<char>& data) override;

    private:
        SocketSession* Session()
        {
            return server->Session(usessionId);
        }

        InspectorSocketServer* server;
        int usessionId;
    };

private:
    const int id;
    InspectorSocket::Pointer wsSocket;
    const int serverPort;
};

class ServerSocket {
public:
    explicit ServerSocket(InspectorSocketServer* server)
        : tcpSocket(uv_tcp_t()), server(server), unixSocket(uv_pipe_t())
    {}
    int Listen(sockaddr* addr, uv_loop_t* loop, int pid = -1);
    void Close()
    {
        uv_close(reinterpret_cast<uv_handle_t*>(&tcpSocket), FreeOnCloseCallback);
    }
    void CloseUnix()
    {
        if (unixSocketOn) {
            uv_close(reinterpret_cast<uv_handle_t*>(&unixSocket), nullptr);
            unixSocketOn = false;
        }
    }
    int GetPort() const
    {
        return port;
    }

private:
    template<typename UvHandle>
    static ServerSocket* FromTcpSocket(UvHandle* socket)
    {
        return jsvm::inspector::ContainerOf(&ServerSocket::tcpSocket, reinterpret_cast<uv_tcp_t*>(socket));
    }
    static void SocketConnectedCallback(uv_stream_t* tcpSocket, int status);
    static void UnixSocketConnectedCallback(uv_stream_t* unixSocket, int status);
    static void FreeOnCloseCallback(uv_handle_t* tcpSocket)
    {
        delete FromTcpSocket(tcpSocket);
    }
    int DetectPort(uv_loop_t* loop, int pid);
    ~ServerSocket() = default;

    uv_tcp_t tcpSocket;
    InspectorSocketServer* server;
    uv_pipe_t unixSocket;
    int port = -1;
    bool unixSocketOn = false;
};

void PrintDebuggerReadyMessage(const std::string& host,
                               const std::vector<InspectorSocketServer::ServerSocketPtr>& serverSockets,
                               const std::vector<std::string>& ids,
                               const char* verb,
                               bool publishUidStderr,
                               FILE* out)
{
    if (!publishUidStderr || out == nullptr) {
        return;
    }
    for (const auto& serverSocket : serverSockets) {
        for (const std::string& id : ids) {
            if (fprintf(out, "Debugger %s on %s\n", verb,
                        FormatWsAddress(host, serverSocket->GetPort(), id, true).c_str()) < 0) {
                return;
            }
        }
    }
    if (fprintf(out, "For help, see: %s\n", "https://nodejs.org/en/docs/inspector") < 0) {
        return;
    }
    if (fflush(out) != 0) {
        return;
    }
}

InspectorSocketServer::InspectorSocketServer(std::unique_ptr<SocketServerDelegate> delegateParam,
                                             uv_loop_t* loop,
                                             const std::string& host,
                                             int port,
                                             const InspectPublishUid& inspectPublishUid,
                                             FILE* out,
                                             int pid)
    : loop(loop), delegate(std::move(delegateParam)), host(host), port(port), inspectPublishUid(inspectPublishUid),
      nextSessionId(0), out(out), pid(pid)
{
    delegate->AssignServer(this);
    state = ServerState::NEW;
}

InspectorSocketServer::~InspectorSocketServer() = default;

SocketSession* InspectorSocketServer::Session(int sessionId)
{
    auto it = connectedSessions.find(sessionId);
    return it == connectedSessions.end() ? nullptr : it->second.second.get();
}

void InspectorSocketServer::SessionStarted(int sessionId, const std::string& targetId, const std::string& wsKey)
{
    SocketSession* session = Session(sessionId);
    DCHECK(session != nullptr);
    if (!TargetExists(targetId)) {
        session->Decline();
        return;
    }
    connectedSessions[sessionId].first = targetId;
    session->Accept(wsKey);
    delegate->StartSession(sessionId, targetId);
}

void InspectorSocketServer::SessionTerminated(int sessionId)
{
    if (Session(sessionId) == nullptr) {
        return;
    }
    bool wasAttached = connectedSessions[sessionId].first != "";
    if (wasAttached) {
        delegate->EndSession(sessionId);
    }
    connectedSessions.erase(sessionId);
    if (connectedSessions.empty()) {
        if (wasAttached && state == ServerState::RUNNING && !serverSockets.empty()) {
            PrintDebuggerReadyMessage(host, serverSockets, delegate->GetTargetIds(), "ending",
                                      inspectPublishUid.console, out);
        }
        if (state == ServerState::STOPPED) {
            delegate.reset();
        }
    }
}

bool InspectorSocketServer::HandleGetRequest(int sessionId, const std::string& hostName, const std::string& path)
{
    SocketSession* session = Session(sessionId);
    DCHECK(session != nullptr);
    InspectorSocket* socket = session->GetWsSocket();
    if (!inspectPublishUid.http) {
        SendHttpNotFound(socket);
        return true;
    }
    const char* command = MatchPathSegment(path.c_str(), "/json");
    if (!command) {
        return false;
    }

    bool hasHandled = false;
    if (MatchPathSegment(command, "list") || command[0] == '\0') {
        SendListResponse(socket, hostName, session);
        hasHandled = true;
    } else if (MatchPathSegment(command, "protocol")) {
        SendProtocolJson(socket);
        hasHandled = true;
    } else if (MatchPathSegment(command, "version")) {
        SendVersionResponse(socket);
        hasHandled = true;
    }
    return hasHandled;
}

void InspectorSocketServer::SendListResponse(InspectorSocket* socket,
                                             const std::string& hostName,
                                             SocketSession* session)
{
    std::vector<std::map<std::string, std::string>> response;
    for (const std::string& id : delegate->GetTargetIds()) {
        response.push_back(std::map<std::string, std::string>());
        std::map<std::string, std::string>& targetMap = response.back();
        targetMap["description"] = "jsvm instance";
        targetMap["id"] = id;
        targetMap["title"] = delegate->GetTargetTitle(id);
        Escape(&targetMap["title"]);
        targetMap["type"] = "node";
        // This attribute value is a "best effort" URL that is passed as a JSON
        // string. It is not guaranteed to resolve to a valid resource.
        targetMap["url"] = delegate->GetTargetUrl(id);
        Escape(&targetMap["url"]);

        std::string detectedHost = hostName;
        if (detectedHost.empty()) {
            detectedHost = FormatHostPort(socket->GetHost(), session->ServerPort());
        }
        std::string formattedAddress = FormatAddress(detectedHost, id, false);
        targetMap["devtoolsFrontendUrl"] = GetFrontendURL(false, formattedAddress);
        // The compat URL is for Chrome browsers older than 66.0.3345.0
        targetMap["devtoolsFrontendUrlCompat"] = GetFrontendURL(true, formattedAddress);
        targetMap["webSocketDebuggerUrl"] = FormatAddress(detectedHost, id, true);
    }
    SendHttpResponse(socket, MapsToString(response), HttpStatusCode::HTTP_OK);
}

std::string InspectorSocketServer::GetFrontendURL(bool isCompat, const std::string& formattedAddress)
{
    std::ostringstream frontendUrl;
    frontendUrl << "devtools://devtools/bundled/";
    frontendUrl << (isCompat ? "inspector" : "js_app");
    frontendUrl << ".html?v8only=true&ws=";
    frontendUrl << formattedAddress;
    return frontendUrl.str();
}

bool InspectorSocketServer::Start()
{
    CHECK_NOT_NULL(delegate);
    CHECK_EQ(state, ServerState::NEW);
    std::unique_ptr<SocketServerDelegate> delegateHolder;
    // We will return it if startup is successful
    delegate.swap(delegateHolder);
    struct addrinfo hints;
    memset_s(&hints, sizeof(hints), 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_socktype = SOCK_STREAM;
    uv_getaddrinfo_t req;
    const std::string portString = std::to_string(port);
    int err = uv_getaddrinfo(loop, &req, nullptr, host.c_str(), portString.c_str(), &hints);
    if (err < 0) {
        if (out != nullptr) {
            if (fprintf(out, "Unable to resolve \"%s\": %s\n", host.c_str(), uv_strerror(err)) < 0) {
                return false;
            }
        }
        return false;
    }
    for (addrinfo* address = req.addrinfo; address != nullptr; address = address->ai_next) {
        auto serverSocket = ServerSocketPtr(new ServerSocket(this));
        err = serverSocket->Listen(address->ai_addr, loop, pid);
        if (err == 0) {
            serverSockets.push_back(std::move(serverSocket));
        }
    }
    uv_freeaddrinfo(req.addrinfo);

    // We only show error if we failed to start server on all addresses. We only
    // show one error, for the last address.
    if (serverSockets.empty()) {
        if (out != nullptr) {
            if (fprintf(out, "Starting inspector on %s:%d failed: %s\n", host.c_str(), port, uv_strerror(err)) < 0) {
                return false;
            }
            if (fflush(out) != 0) {
                return false;
            }
        }
        return false;
    }
    delegate.swap(delegateHolder);
    state = ServerState::RUNNING;
    PrintDebuggerReadyMessage(host, serverSockets, delegate->GetTargetIds(), "listening", inspectPublishUid.console,
                              out);
    return true;
}

void InspectorSocketServer::Stop()
{
    if (state == ServerState::STOPPED) {
        return;
    }
    CHECK_EQ(state, ServerState::RUNNING);
    state = ServerState::STOPPED;
    serverSockets.clear();
    if (Done()) {
        delegate.reset();
    }
}

void InspectorSocketServer::TerminateConnections()
{
    for (const auto& keyValue : connectedSessions) {
        keyValue.second.second->Close();
    }
}

bool InspectorSocketServer::TargetExists(const std::string& id)
{
    const std::vector<std::string>& targetIds = delegate->GetTargetIds();
    const auto& found = std::find(targetIds.begin(), targetIds.end(), id);
    return found != targetIds.end();
}

int InspectorSocketServer::GetPort() const
{
    if (!serverSockets.empty()) {
        return serverSockets[0]->GetPort();
    }
    return port;
}

void InspectorSocketServer::Accept(int serverPort, uv_stream_t* serverSocket)
{
    std::unique_ptr<SocketSession> session = std::make_unique<SocketSession>(this, nextSessionId++, serverPort);

    InspectorSocket::DelegatePointer delegatePointer =
        InspectorSocket::DelegatePointer(new SocketSession::Delegate(this, session->GetId()));

    InspectorSocket::Pointer inspector = InspectorSocket::Accept(serverSocket, std::move(delegatePointer));
    if (inspector) {
        session->Own(std::move(inspector));
        connectedSessions[session->GetId()].second = std::move(session);
    }
}

void InspectorSocketServer::Send(int sessionId, const std::string& message)
{
    SocketSession* session = Session(sessionId);
    if (session != nullptr) {
        session->Send(message);
    }
}

void InspectorSocketServer::CloseServerSocket(ServerSocket* server)
{
    server->Close();
    server->CloseUnix();
}

// InspectorSession tracking
SocketSession::SocketSession(InspectorSocketServer* server, int id, int serverPort) : id(id), serverPort(serverPort) {}

void SocketSession::Send(const std::string& message)
{
    wsSocket->Write(message.data(), message.length());
}

void SocketSession::Delegate::OnHttpGet(const std::string& host, const std::string& path)
{
    if (!server->HandleGetRequest(usessionId, host, path)) {
        SocketSession* session = Session();
        DCHECK(session != nullptr);
        session->GetWsSocket()->CancelHandshake();
    }
}

void SocketSession::Delegate::OnSocketUpgrade(const std::string& host,
                                              const std::string& path,
                                              const std::string& wsKey)
{
    std::string id = path.empty() ? path : path.substr(1);
    server->SessionStarted(usessionId, id, wsKey);
}

void SocketSession::Delegate::OnWsFrame(const std::vector<char>& data)
{
    server->MessageReceived(usessionId, std::string(data.data(), data.size()));
}

// ServerSocket implementation
int ServerSocket::DetectPort(uv_loop_t* loop, int pid)
{
    sockaddr_storage addr;
    int len = sizeof(addr);
    int err = uv_tcp_getsockname(&tcpSocket, reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (err != 0) {
        return err;
    }
    int portNum;
    if (addr.ss_family == AF_INET6) {
        portNum = reinterpret_cast<const sockaddr_in6*>(&addr)->sin6_port;
    } else {
        portNum = reinterpret_cast<const sockaddr_in*>(&addr)->sin_port;
    }
    port = ntohs(portNum);
    if (!unixSocketOn && pid != -1) {
        auto unixDomainSocketPath = "jsvm_devtools_remote_" + std::to_string(port) + "_" + std::to_string(pid);
        auto* abstract = new char[unixDomainSocketPath.length() + 2];
        abstract[0] = '\0';
        int ret = strcpy_s(abstract + 1, unixDomainSocketPath.length() + 2, unixDomainSocketPath.c_str());
        CHECK(ret == 0);
        auto status = uv_pipe_init(loop, &unixSocket, 0);
        if (status == 0) {
            status = uv_pipe_bind2(&unixSocket, abstract, unixDomainSocketPath.length() + 1, 0);
        }
        if (status == 0) {
            constexpr int unixBacklog = 128;
            status = uv_listen(reinterpret_cast<uv_stream_t*>(&unixSocket), unixBacklog,
                               ServerSocket::UnixSocketConnectedCallback);
        }
        unixSocketOn = status == 0;
        delete[] abstract;
    }
    return err;
}

int ServerSocket::Listen(sockaddr* addr, uv_loop_t* loop, int pid)
{
    uv_tcp_t* server = &tcpSocket;
    CHECK_EQ(0, uv_tcp_init(loop, server));
    int err = uv_tcp_bind(server, addr, 0);
    if (err == 0) {
        // 511 is the value used by a 'net' module by default
        err = uv_listen(reinterpret_cast<uv_stream_t*>(server), 511, ServerSocket::SocketConnectedCallback);
    }
    if (err == 0) {
        err = DetectPort(loop, pid);
    }
    return err;
}

// static
void ServerSocket::SocketConnectedCallback(uv_stream_t* tcpSocket, int status)
{
    if (status == 0) {
        ServerSocket* serverSocket = ServerSocket::FromTcpSocket(tcpSocket);
        // Memory is freed when the socket closes.
        serverSocket->server->Accept(serverSocket->port, tcpSocket);
    }
}

void ServerSocket::UnixSocketConnectedCallback(uv_stream_t* unixSocket, int status)
{
    if (status == 0) {
        (void)unixSocket;
    }
}
} // namespace inspector
} // namespace jsvm
