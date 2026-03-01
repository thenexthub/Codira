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

#include "inspector_socket.h"

#include <algorithm>
#include <cstring>
#include <map>

#include "inspector/inspector_utils.h"
#include "llhttp.h"
#include "openssl/sha.h" // Sha-1 hash

#define ACCEPT_KEY_LENGTH Base64EncodeSize(20)

namespace jsvm {
namespace inspector {

class TcpHolder {
public:
    static void DisconnectAndDispose(TcpHolder* holder);
    using Pointer = DeleteFnPtr<TcpHolder, DisconnectAndDispose>;

    static Pointer Accept(uv_stream_t* server, InspectorSocket::DelegatePointer delegate);
    void SetHandler(ProtocolHandler* handler);
    int WriteRaw(const std::vector<char>& buffer, uv_write_cb writeCb);
    uv_tcp_t* GetTcp()
    {
        return &tcp;
    }
    InspectorSocket::Delegate* GetDelegate();

private:
    static TcpHolder* From(void* handle)
    {
        return jsvm::inspector::ContainerOf(&TcpHolder::tcp, reinterpret_cast<uv_tcp_t*>(handle));
    }
    static void OnClosed(uv_handle_t* handle);
    static void OnDataReceivedCb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    explicit TcpHolder(InspectorSocket::DelegatePointer delegate);
    ~TcpHolder() = default;
    void ReclaimUvBuf(const uv_buf_t* buf, ssize_t read);

    uv_tcp_t tcp;
    const InspectorSocket::DelegatePointer delegate;
    ProtocolHandler* handler;
    std::vector<char> buffer;
};

class ProtocolHandler {
public:
    ProtocolHandler(InspectorSocket* inspector, TcpHolder::Pointer tcp);

    virtual void AcceptUpgrade(const std::string& acceptKey) = 0;
    virtual void OnData(std::vector<char>* data) = 0;
    virtual void OnEof() = 0;
    virtual void Write(const std::vector<char> data) = 0;
    virtual void CancelHandshake() = 0;

    std::string GetHost() const;

    InspectorSocket* GetInspectorSocket()
    {
        return inspector;
    }
    virtual void Shutdown() = 0;

protected:
    virtual ~ProtocolHandler() = default;
    int WriteRaw(const std::vector<char>& buffer, uv_write_cb writeCb);
    InspectorSocket::Delegate* GetDelegate();

    InspectorSocket* const inspector;
    TcpHolder::Pointer tcp;
};

namespace {
class WriteRequest {
public:
    WriteRequest(ProtocolHandler* handler, const std::vector<char>& buffer)
        : handler(handler), storage(buffer), req(uv_write_t()), buf(uv_buf_init(storage.data(), storage.size()))
    {}

    static WriteRequest* FromWriteReq(uv_write_t* req)
    {
        return jsvm::inspector::ContainerOf(&WriteRequest::req, req);
    }

    static void Cleanup(uv_write_t* req, int status)
    {
        delete WriteRequest::FromWriteReq(req);
    }

    ProtocolHandler* const handler;
    std::vector<char> storage;
    uv_write_t req;
    uv_buf_t buf;
};

void AllocateBuffer(uv_handle_t* stream, size_t len, uv_buf_t* buf)
{
    CHECK(len > 0);
    *buf = uv_buf_init(new char[len], len);
}

static void RemoveFromBeginning(std::vector<char>* buffer, size_t count)
{
    buffer->erase(buffer->begin(), buffer->begin() + count);
}

static const char CLOSE_FRAME[] = { '\x88', '\x00' };

enum WsDecodeResult { FRAME_OK, FRAME_INCOMPLETE, FRAME_CLOSE, FRAME_ERROR, FRAME_PING };

static void GenerateAcceptString(const std::string& clientKey, char (*buffer)[ACCEPT_KEY_LENGTH])
{
    // Magic string from websockets spec.
    static constexpr char wsMagic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string input(clientKey + wsMagic);
    char hash[SHA_DIGEST_LENGTH];
    USE(SHA1(reinterpret_cast<const unsigned char*>(input.data()), input.size(),
             reinterpret_cast<unsigned char*>(hash)));
    jsvm::inspector::Base64Encode(hash, sizeof(hash), *buffer, sizeof(*buffer));
}

static std::string TrimPort(const std::string& host)
{
    size_t lastColonPos = host.rfind(':');
    if (lastColonPos == std::string::npos) {
        return host;
    }
    size_t bracket = host.rfind(']');
    if (bracket == std::string::npos || lastColonPos > bracket) {
        return host.substr(0, lastColonPos);
    }
    return host;
}

static bool IsIPAddress(const std::string& host)
{
    // To avoid DNS rebinding attacks, we are aware of the following requirements:
    // * the host name must be an IP address (CVE-2018-7160, CVE-2022-32212),
    // * the IP address must be routable (hackerone.com/reports/1632921), and
    // * the IP address must be formatted unambiguously (CVE-2022-43548).

    // The logic below assumes that the string is null-terminated, so ensure that
    // we did not somehow end up with null characters within the string.
    if (host.find('\0') != std::string::npos) {
        return false;
    }

    // All IPv6 addresses must be enclosed in square brackets, and anything
    // enclosed in square brackets must be an IPv6 address.
    if (host.length() >= ByteSize::SIZE_4_BYTES && host.front() == '[' && host.back() == ']') {
        // INET6_ADDRSTRLEN is the maximum length of the dual format (including the
        // terminating null character), which is the longest possible representation
        // of an IPv6 address: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:ddd.ddd.ddd.ddd
        if (host.length() - ByteSize::SIZE_2_BYTES >= INET6_ADDRSTRLEN) {
            return false;
        }

        // Annoyingly, libuv's implementation of inet_pton() deviates from other
        // implementations of the function in that it allows '%' in IPv6 addresses.
        if (host.find('%') != std::string::npos) {
            return false;
        }

        // Parse the IPv6 address to ensure it is syntactically valid.
        char ipv6Str[INET6_ADDRSTRLEN];
        std::copy(host.begin() + 1, host.end() - 1, ipv6Str);
        ipv6Str[host.length()] = '\0';
        unsigned char ipv6[sizeof(struct in6_addr)];
        if (uv_inet_pton(AF_INET6, ipv6Str, ipv6) != 0) {
            return false;
        }

        // The only non-routable IPv6 address is ::/128. It should not be necessary
        // to explicitly reject it because it will still be enclosed in square
        // brackets and not even macOS should make DNS requests in that case, but
        // history has taught us that we cannot be careful enough.
        // Note that RFC 4291 defines both "IPv4-Compatible IPv6 Addresses" and
        // "IPv4-Mapped IPv6 Addresses", which means that there are IPv6 addresses
        // (other than ::/128) that represent non-routable IPv4 addresses. However,
        // this translation assumes that the host is interpreted as an IPv6 address
        // in the first place, at which point DNS rebinding should not be an issue.
        if (std::all_of(ipv6, ipv6 + sizeof(ipv6), [](auto b) { return b == 0; })) {
            return false;
        }

        // It is a syntactically valid and routable IPv6 address enclosed in square
        // brackets. No client should be able to misinterpret this.
        return true;
    }

    // Anything not enclosed in square brackets must be an IPv4 address. It is
    // important here that inet_pton() accepts only the so-called dotted-decimal
    // notation, which is a strict subset of the so-called numbers-and-dots
    // notation that is allowed by inet_aton() and inet_addr(). This subset does
    // not allow hexadecimal or octal number formats.
    unsigned char ipv4[sizeof(struct in_addr)];
    if (uv_inet_pton(AF_INET, host.c_str(), ipv4) != 0) {
        return false;
    }

    if (ipv4[0] == 0) {
        return false;
    }

    // It is a routable IPv4 address in dotted-decimal notation.
    return true;
}

// Constants for hybi-10 frame format.

typedef int OpCode;

const OpCode K_OP_CODE_CONTINUATION = 0x0;
const OpCode K_OP_CODE_TEXT = 0x1;
const OpCode K_OP_CODE_BINARY = 0x2;
const OpCode K_OP_CODE_CLOSE = 0x8;
const OpCode K_OP_CODE_PING = 0x9;
const OpCode K_OP_CODE_PONG = 0xA;

const unsigned char K_FINAL_BIT = 0x80;
const unsigned char K_RESERVED_1_BIT = 0x40;
const unsigned char K_RESERVED_2_BIT = 0x20;
const unsigned char K_RESERVED_3_BIT = 0x10;
const unsigned char K_OP_CODE_MASK = 0xF;
const unsigned char K_MASK_BIT = 0x80;
const unsigned char K_PAYLOAD_LENGTH_MASK = 0x7F;
const unsigned char K_PONG_FRAME_HEADER = 0x8A;

const size_t K_MAX_SINGLE_BYTE_PAYLOAD_LENGTH = 125;
const size_t K_TWO_BYTE_PAYLOAD_LENGTH_FIELD = 126;
const size_t K_EIGHT_BYTE_PAYLOAD_LENGTH_FIELD = 127;
const size_t K_MASKING_KEY_WIDTH_IN_BYTES = 4;

static std::vector<char> encode_frame_hybi17(const std::vector<char>& message)
{
    std::vector<char> frame;
    OpCode opCode = K_OP_CODE_TEXT;
    frame.push_back(K_FINAL_BIT | opCode);
    const size_t dataLength = message.size();
    if (dataLength <= K_MAX_SINGLE_BYTE_PAYLOAD_LENGTH) {
        frame.push_back(static_cast<char>(dataLength));
    } else if (dataLength <= 0xFFFF) {
        frame.push_back(K_TWO_BYTE_PAYLOAD_LENGTH_FIELD);
        frame.push_back((dataLength & 0xFF00) >> ByteOffset::BIT_8);
        frame.push_back(dataLength & 0xFF);
    } else {
        frame.push_back(K_EIGHT_BYTE_PAYLOAD_LENGTH_FIELD);
        constexpr size_t byteCount = 8;
        char extendedPayloadLength[byteCount];
        size_t remaining = dataLength;
        // Fill the length into extendedPayloadLength in the network byte order.
        for (int i = 0; i < byteCount; ++i) {
            extendedPayloadLength[byteCount - 1 - i] = remaining & 0xFF;
            remaining >>= byteCount;
        }
        frame.insert(frame.end(), extendedPayloadLength, extendedPayloadLength + byteCount);
        CHECK_EQ(0, remaining);
    }
    frame.insert(frame.end(), message.begin(), message.end());
    return frame;
}

static WsDecodeResult DecodeFrameHybi17(const std::vector<char>& buffer,
                                        bool clientFrame,
                                        int* bytesConsumed,
                                        std::vector<char>* output,
                                        bool* compressed)
{
    *bytesConsumed = 0;
    if (buffer.size() < ByteSize::SIZE_2_BYTES) {
        return FRAME_INCOMPLETE;
    }

    auto it = buffer.begin();

    unsigned char firstByte = *it++;
    unsigned char secondByte = *it++;

    bool final = (firstByte & K_FINAL_BIT) != 0;
    bool reserved1 = (firstByte & K_RESERVED_1_BIT) != 0;
    bool reserved2 = (firstByte & K_RESERVED_2_BIT) != 0;
    bool reserved3 = (firstByte & K_RESERVED_3_BIT) != 0;
    int opCode = firstByte & K_OP_CODE_MASK;
    bool masked = (secondByte & K_MASK_BIT) != 0;
    *compressed = reserved1;
    if (!final || reserved2 || reserved3) {
        return FRAME_ERROR; // Only compression extension is supported.
    }

    bool closed = false;
    uint64_t payloadLength64 = secondByte & K_PAYLOAD_LENGTH_MASK;
    switch (opCode) {
        case K_OP_CODE_CLOSE:
            closed = true;
            break;
        case K_OP_CODE_TEXT:
            break;
        case K_OP_CODE_PING: {
            output->push_back(K_PONG_FRAME_HEADER);
            output->push_back(static_cast<char>(payloadLength64));
            output->insert(output->end(), it, it + payloadLength64);
            return FRAME_PING;
        }
        case K_OP_CODE_BINARY:       // We don't support binary frames yet.
        case K_OP_CODE_CONTINUATION: // We don't support binary frames yet.
        case K_OP_CODE_PONG:         // We don't support binary frames yet.
        default:
            return FRAME_ERROR;
    }

    // In Hybi-17 spec client MUST mask its frame.
    if (clientFrame && !masked) {
        return FRAME_ERROR;
    }

    if (payloadLength64 > K_MAX_SINGLE_BYTE_PAYLOAD_LENGTH) {
        int extendedPayloadLengthSize;
        if (payloadLength64 == K_TWO_BYTE_PAYLOAD_LENGTH_FIELD) {
            extendedPayloadLengthSize = ByteSize::SIZE_2_BYTES;
        } else if (payloadLength64 == K_EIGHT_BYTE_PAYLOAD_LENGTH_FIELD) {
            extendedPayloadLengthSize = ByteSize::SIZE_8_BYTES;
        } else {
            return FRAME_ERROR;
        }
        if ((buffer.end() - it) < extendedPayloadLengthSize) {
            return FRAME_INCOMPLETE;
        }
        payloadLength64 = 0;
        for (int i = 0; i < extendedPayloadLengthSize; ++i) {
            payloadLength64 <<= ByteOffset::BIT_8;
            payloadLength64 |= static_cast<unsigned char>(*it++);
        }
    }

    static const uint64_t maxPayloadLength = 0x7FFFFFFFFFFFFFFF;
    static const size_t maxLength = SIZE_MAX;
    if (payloadLength64 > maxPayloadLength || payloadLength64 > maxLength - K_MASKING_KEY_WIDTH_IN_BYTES) {
        // WebSocket frame length too large.
        return FRAME_ERROR;
    }
    size_t payloadLength = static_cast<size_t>(payloadLength64);

    if (buffer.size() - K_MASKING_KEY_WIDTH_IN_BYTES < payloadLength) {
        return FRAME_INCOMPLETE;
    }

    std::vector<char>::const_iterator maskingKey = it;
    std::vector<char>::const_iterator payload = it + K_MASKING_KEY_WIDTH_IN_BYTES;
    for (size_t i = 0; i < payloadLength; ++i) { // Unmask the payload.
        output->insert(output->end(), payload[i] ^ maskingKey[i % K_MASKING_KEY_WIDTH_IN_BYTES]);
    }

    size_t pos = it + K_MASKING_KEY_WIDTH_IN_BYTES + payloadLength - buffer.begin();
    *bytesConsumed = pos;
    return closed ? FRAME_CLOSE : FRAME_OK;
}

// WS protocol
class WsHandler : public ProtocolHandler {
public:
    WsHandler(InspectorSocket* inspector, TcpHolder::Pointer tcp)
        : ProtocolHandler(inspector, std::move(tcp)), onCloseSent(&WsHandler::WaitForCloseReply),
          onCloseReceived(&WsHandler::CloseFrameReceived), dispose(false)
    {}

    void AcceptUpgrade(const std::string& acceptKey) override {}
    void CancelHandshake() override {}

    void OnEof() override
    {
        tcp.reset();
        if (dispose) {
            delete this;
        }
    }

    void Write(const std::vector<char> data) override
    {
        std::vector<char> output = encode_frame_hybi17(data);
        WriteRaw(output, WriteRequest::Cleanup);
    }

    void OnData(std::vector<char>* data) override
    {
        int processed = 0;
        do {
            processed = ParseWsFrames(*data);
            if (processed > 0) {
                RemoveFromBeginning(data, processed);
            }
        } while (processed > 0 && !data->empty());
    }

protected:
    void Shutdown() override
    {
        if (tcp) {
            dispose = true;
            SendClose();
        } else {
            // if tcp is null, delete this
            delete this;
        }
    }

private:
    using Callback = void (WsHandler::*)();

    static void OnCloseFrameWritten(uv_write_t* req, int status)
    {
        WriteRequest* wr = WriteRequest::FromWriteReq(req);
        WsHandler* handler = static_cast<WsHandler*>(wr->handler);
        delete wr;
        Callback cb = handler->onCloseSent;
        (handler->*cb)();
    }

    void WaitForCloseReply()
    {
        onCloseReceived = &WsHandler::OnEof;
    }

    void SendClose()
    {
        WriteRaw(std::vector<char>(CLOSE_FRAME, CLOSE_FRAME + sizeof(CLOSE_FRAME)), OnCloseFrameWritten);
    }

    void CloseFrameReceived()
    {
        onCloseSent = &WsHandler::OnEof;
        SendClose();
    }

    int ParseWsFrames(const std::vector<char>& buffer)
    {
        int bytesConsumed = 0;
        std::vector<char> output;
        bool compressed = false;

        WsDecodeResult r = DecodeFrameHybi17(buffer, true /* clientFrame */, &bytesConsumed, &output, &compressed);
        // Compressed frame means client is ignoring the headers and misbehaves
        if (compressed || r == FRAME_ERROR) {
            OnEof();
            bytesConsumed = 0;
        } else if (r == FRAME_CLOSE) {
            (this->*onCloseReceived)();
            bytesConsumed = 0;
        } else if (r == FRAME_OK) {
            GetDelegate()->OnWsFrame(output);
        } else if (r == FRAME_PING) {
            WriteRaw(output, WriteRequest::Cleanup);
        }
        return bytesConsumed;
    }

    Callback onCloseSent;
    Callback onCloseReceived;
    bool dispose;
};

// HTTP protocol
class HttpEvent {
public:
    HttpEvent(const std::string& path, bool upgrade, bool isGET, const std::string& wsKey, const std::string& host)
        : path(path), upgrade(upgrade), isGET(isGET), wsKey(wsKey), host(host)
    {}

    std::string path;
    bool upgrade;
    bool isGET;
    std::string wsKey;
    std::string host;
};

class HttpHandler : public ProtocolHandler {
public:
    explicit HttpHandler(InspectorSocket* inspector, TcpHolder::Pointer tcp)
        : ProtocolHandler(inspector, std::move(tcp)), parsingValue(false)
    {
        llhttp_init(&parser, HTTP_REQUEST, &parserSettings);
        llhttp_settings_init(&parserSettings);
        parserSettings.on_header_field = OnHeaderField;
        parserSettings.on_header_value = OnHeaderValue;
        parserSettings.on_message_complete = OnMessageComplete;
        parserSettings.on_url = OnPath;
    }

    void AcceptUpgrade(const std::string& acceptKey) override
    {
        char acceptString[ACCEPT_KEY_LENGTH];
        GenerateAcceptString(acceptKey, &acceptString);
        const char acceptWsPrefix[] = "HTTP/1.1 101 Switching Protocols\r\n"
                                      "Upgrade: websocket\r\n"
                                      "Connection: Upgrade\r\n"
                                      "Sec-WebSocket-Accept: ";
        const char acceptWsSuffix[] = "\r\n\r\n";
        std::vector<char> reply(acceptWsPrefix, acceptWsPrefix + sizeof(acceptWsPrefix) - 1);
        reply.insert(reply.end(), acceptString, acceptString + sizeof(acceptString));
        reply.insert(reply.end(), acceptWsSuffix, acceptWsSuffix + sizeof(acceptWsSuffix) - 1);
        if (WriteRaw(reply, WriteRequest::Cleanup) >= 0) {
            inspector->SwitchProtocol(new WsHandler(inspector, std::move(tcp)));
        } else {
            tcp.reset();
        }
    }

    void CancelHandshake() override
    {
        const char handshakeFailedResponse[] = "HTTP/1.0 400 Bad Request\r\n"
                                               "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                                               "WebSockets request was expected\r\n";
        WriteRaw(std::vector<char>(handshakeFailedResponse,
                                   handshakeFailedResponse + sizeof(handshakeFailedResponse) - 1),
                 ThenCloseAndReportFailure);
    }

    void OnEof() override
    {
        tcp.reset();
    }

    void Write(const std::vector<char> data) override
    {
        WriteRaw(data, WriteRequest::Cleanup);
    }

    void OnData(std::vector<char>* data) override
    {
        llhttp_errno_t err = llhttp_execute(&parser, data->data(), data->size());
        if (err == HPE_PAUSED_UPGRADE) {
            err = HPE_OK;
            llhttp_resume_after_upgrade(&parser);
        }
        data->clear();
        if (err != HPE_OK) {
            CancelHandshake();
        }
        // Event handling may delete *this
        std::vector<HttpEvent> httpEvents;
        std::swap(httpEvents, events);
        for (const HttpEvent& event : httpEvents) {
            if (!IsAllowedHost(event.host) || !event.isGET) {
                CancelHandshake();
                return;
            } else if (!event.upgrade) {
                GetDelegate()->OnHttpGet(event.host, event.path);
            } else if (event.wsKey.empty()) {
                CancelHandshake();
                return;
            } else {
                GetDelegate()->OnSocketUpgrade(event.host, event.path, event.wsKey);
            }
        }
    }

protected:
    void Shutdown() override
    {
        delete this;
    }

private:
    static void ThenCloseAndReportFailure(uv_write_t* req, int status)
    {
        ProtocolHandler* handler = WriteRequest::FromWriteReq(req)->handler;
        WriteRequest::Cleanup(req, status);
        handler->GetInspectorSocket()->SwitchProtocol(nullptr);
    }

    static int OnHeaderValue(llhttp_t* parser, const char* at, size_t length)
    {
        HttpHandler* handler = From(parser);
        handler->parsingValue = true;
        handler->headers[handler->currentHeader].append(at, length);
        return 0;
    }

    static int OnHeaderField(llhttp_t* parser, const char* at, size_t length)
    {
        HttpHandler* handler = From(parser);
        if (handler->parsingValue) {
            handler->parsingValue = false;
            handler->currentHeader.clear();
        }
        handler->currentHeader.append(at, length);
        return 0;
    }

    static int OnPath(llhttp_t* parser, const char* at, size_t length)
    {
        HttpHandler* handler = From(parser);
        handler->path.append(at, length);
        return 0;
    }

    static HttpHandler* From(llhttp_t* parser)
    {
        return jsvm::inspector::ContainerOf(&HttpHandler::parser, parser);
    }

    static int OnMessageComplete(llhttp_t* parser)
    {
        // Event needs to be fired after the parser is done.
        HttpHandler* handler = From(parser);
        handler->events.emplace_back(handler->path, parser->upgrade, parser->method == HTTP_GET,
                                     handler->HeaderValue("Sec-WebSocket-Key"), handler->HeaderValue("Host"));
        handler->path = "";
        handler->parsingValue = false;
        handler->headers.clear();
        handler->currentHeader = "";
        return 0;
    }

    std::string HeaderValue(const std::string& header) const
    {
        bool headerFound = false;
        std::string value;
        for (const auto& header_value : headers) {
            if (jsvm::inspector::StringEqualNoCaseN(header_value.first.data(), header.data(), header.length())) {
                if (headerFound) {
                    return "";
                }
                value = header_value.second;
                headerFound = true;
            }
        }
        return value;
    }

    bool IsAllowedHost(const std::string& hostWithPort) const
    {
        std::string host = TrimPort(hostWithPort);
        return host.empty() || IsIPAddress(host) || jsvm::inspector::StringEqualNoCase(host.data(), "localhost");
    }

    bool parsingValue;
    llhttp_t parser;
    llhttp_settings_t parserSettings;
    std::vector<HttpEvent> events;
    std::string currentHeader;
    std::map<std::string, std::string> headers;
    std::string path;
};

} // namespace

// Any protocol
ProtocolHandler::ProtocolHandler(InspectorSocket* inspector, TcpHolder::Pointer tcpParam)
    : inspector(inspector), tcp(std::move(tcpParam))
{
    CHECK_NOT_NULL(tcp);
    tcp->SetHandler(this);
}

int ProtocolHandler::WriteRaw(const std::vector<char>& buffer, uv_write_cb writeCb)
{
    return tcp->WriteRaw(buffer, writeCb);
}

InspectorSocket::Delegate* ProtocolHandler::GetDelegate()
{
    return tcp->GetDelegate();
}

std::string ProtocolHandler::GetHost() const
{
    char ip[INET6_ADDRSTRLEN];
    sockaddr_storage addr;
    int len = sizeof(addr);
    int err = uv_tcp_getsockname(tcp->GetTcp(), reinterpret_cast<struct sockaddr*>(&addr), &len);
    if (err) {
        return "";
    }
    if (addr.ss_family == AF_INET6) {
        // using ipv6
        const sockaddr_in6* v6 = reinterpret_cast<const sockaddr_in6*>(&addr);
        err = uv_ip6_name(v6, ip, sizeof(ip));
    } else {
        // using ipv4
        const sockaddr_in* v4 = reinterpret_cast<const sockaddr_in*>(&addr);
        err = uv_ip4_name(v4, ip, sizeof(ip));
    }
    if (err) {
        return "";
    }
    return ip;
}

// RAII uv_tcp_t wrapper
TcpHolder::TcpHolder(InspectorSocket::DelegatePointer delegate) : tcp(), delegate(std::move(delegate)), handler(nullptr)
{}

// static
TcpHolder::Pointer TcpHolder::Accept(uv_stream_t* server, InspectorSocket::DelegatePointer delegate)
{
    TcpHolder* result = new TcpHolder(std::move(delegate));
    uv_stream_t* tcp = reinterpret_cast<uv_stream_t*>(&result->tcp);
    int err = uv_tcp_init(server->loop, &result->tcp);
    if (err == 0) {
        err = uv_accept(server, tcp);
    }
    if (err == 0) {
        err = uv_read_start(tcp, AllocateBuffer, OnDataReceivedCb);
    }
    if (err == 0) {
        return TcpHolder::Pointer(result);
    } else {
        delete result;
        return nullptr;
    }
}

void TcpHolder::SetHandler(ProtocolHandler* protocalHandler)
{
    handler = protocalHandler;
}

int TcpHolder::WriteRaw(const std::vector<char>& buffer, uv_write_cb writeCb)
{
    // Freed in write_request_cleanup
    WriteRequest* wr = new WriteRequest(handler, buffer);
    uv_stream_t* stream = reinterpret_cast<uv_stream_t*>(&tcp);
    int err = uv_write(&wr->req, stream, &wr->buf, 1, writeCb);
    if (err < 0) {
        delete wr;
    }
    return err < 0;
}

InspectorSocket::Delegate* TcpHolder::GetDelegate()
{
    return delegate.get();
}

// static
void TcpHolder::OnClosed(uv_handle_t* handle)
{
    delete From(handle);
}

void TcpHolder::OnDataReceivedCb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
    TcpHolder* holder = From(tcp);
    holder->ReclaimUvBuf(buf, nread);
    if (nread < 0 || nread == UV_EOF) {
        holder->handler->OnEof();
    } else {
        holder->handler->OnData(&holder->buffer);
    }
}

// static
void TcpHolder::DisconnectAndDispose(TcpHolder* holder)
{
    uv_handle_t* handle = reinterpret_cast<uv_handle_t*>(&holder->tcp);
    uv_close(handle, OnClosed);
}

void TcpHolder::ReclaimUvBuf(const uv_buf_t* buf, ssize_t read)
{
    if (read > 0) {
        DCHECK(read <= buf.len);
        // insert buffer
        buffer.insert(buffer.end(), buf->base, buf->base + read);
    }
    delete[] buf->base;
}

InspectorSocket::~InspectorSocket() = default;

// static method
void InspectorSocket::Shutdown(ProtocolHandler* handler)
{
    handler->Shutdown();
}

// static method
InspectorSocket::Pointer InspectorSocket::Accept(uv_stream_t* server, DelegatePointer delegate)
{
    auto tcp = TcpHolder::Accept(server, std::move(delegate));
    InspectorSocket* inspector = nullptr;
    if (tcp) {
        // If accept tcp, create new inspector socket
        inspector = new InspectorSocket();
        inspector->SwitchProtocol(new HttpHandler(inspector, std::move(tcp)));
        return InspectorSocket::Pointer(inspector);
    }
    return InspectorSocket::Pointer(nullptr);
}

void InspectorSocket::AcceptUpgrade(const std::string& acceptKey)
{
    protocolHandler->AcceptUpgrade(acceptKey);
}

void InspectorSocket::CancelHandshake()
{
    protocolHandler->CancelHandshake();
}

std::string InspectorSocket::GetHost()
{
    return protocolHandler->GetHost();
}

void InspectorSocket::SwitchProtocol(ProtocolHandler* handler)
{
    protocolHandler.reset(std::move(handler));
}

void InspectorSocket::Write(const char* data, size_t len)
{
    protocolHandler->Write(std::vector<char>(data, data + len));
}

} // namespace inspector
} // namespace jsvm
