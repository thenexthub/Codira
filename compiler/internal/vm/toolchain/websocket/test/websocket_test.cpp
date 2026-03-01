/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <securec.h>
#include <sys/un.h>

#include "gtest/gtest.h"
#include "client/websocket_client.h"
#include "server/websocket_server.h"

using namespace OHOS::ArkCompiler::Toolchain;

namespace panda::test {
class WebSocketTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
            GTEST_LOG_(ERROR) << "Reset SIGPIPE failed.";
        }
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

#if defined(OHOS_PLATFORM)
    static constexpr char UNIX_DOMAIN_PATH_1[] = "server.sock_1";
    static constexpr char UNIX_DOMAIN_PATH_2[] = "server.sock_2";
    static constexpr char UNIX_DOMAIN_PATH_3[] = "server.sock_3";
#endif
    static constexpr char HELLO_SERVER[]    = "hello server";
    static constexpr char HELLO_CLIENT[]    = "hello client";
    static constexpr char SERVER_OK[]       = "server ok";
    static constexpr char CLIENT_OK[]       = "client ok";
    static constexpr char QUIT[]            = "quit";
    static constexpr char PING[]            = "ping";
    static constexpr int TCP_PORT           = 9230;
    static const std::string LONG_MSG;
    static const std::string LONG_LONG_MSG;
};

const std::string WebSocketTest::LONG_MSG       = std::string(1000, 'f');
const std::string WebSocketTest::LONG_LONG_MSG  = std::string(0xfffff, 'f');

HWTEST_F(WebSocketTest, ConnectWebSocketTest, testing::ext::TestSize.Level0)
{
    WebSocketServer serverSocket;
    bool ret = false;
#if defined(OHOS_PLATFORM)
    int appPid = getpid();
    ret = serverSocket.InitUnixWebSocket(UNIX_DOMAIN_PATH_1 + std::to_string(appPid), 5);
#else
    ret = serverSocket.InitTcpWebSocket(TCP_PORT, 5);
#endif
    ASSERT_TRUE(ret);
    pid_t pid = fork();
    if (pid == 0) {
        // subprocess, handle client connect and recv/send message
        // note: EXPECT/ASSERT produce errors in subprocess that can not lead to failure of testcase in mainprocess,
        //       so testcase still success finally.
        WebSocketClient clientSocket;
        bool retClient = false;
#if defined(OHOS_PLATFORM)
        retClient = clientSocket.InitToolchainWebSocketForSockName(UNIX_DOMAIN_PATH_1 + std::to_string(appPid), 5);
#else
        retClient = clientSocket.InitToolchainWebSocketForPort(TCP_PORT, 5);
#endif
        ASSERT_TRUE(retClient);
        retClient = clientSocket.ClientSendWSUpgradeReq();
        ASSERT_TRUE(retClient);
        retClient = clientSocket.ClientRecvWSUpgradeRsp();
        ASSERT_TRUE(retClient);
        retClient = clientSocket.SendReply(HELLO_SERVER);
        EXPECT_TRUE(retClient);
        std::string recv = clientSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), HELLO_CLIENT), 0);
        if (strcmp(recv.c_str(), HELLO_CLIENT) == 0) {
            retClient = clientSocket.SendReply(CLIENT_OK);
            EXPECT_TRUE(retClient);
        }
        retClient = clientSocket.SendReply(LONG_MSG);
        EXPECT_TRUE(retClient);
        recv = clientSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), SERVER_OK), 0);
        if (strcmp(recv.c_str(), SERVER_OK) == 0) {
            retClient = clientSocket.SendReply(CLIENT_OK);
            EXPECT_TRUE(retClient);
        }
        retClient = clientSocket.SendReply(LONG_LONG_MSG);
        EXPECT_TRUE(retClient);
        recv = clientSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), SERVER_OK), 0);
        if (strcmp(recv.c_str(), SERVER_OK) == 0) {
            retClient = clientSocket.SendReply(CLIENT_OK);
            EXPECT_TRUE(retClient);
        }
        retClient = clientSocket.SendReply(PING, FrameType::PING); // send a ping frame and wait for pong frame
        EXPECT_TRUE(retClient);
        recv = clientSocket.Decode(); // get the pong frame
        EXPECT_EQ(strcmp(recv.c_str(), ""), 0); // pong frame has no data
        retClient = clientSocket.SendReply(QUIT);
        EXPECT_TRUE(retClient);
        clientSocket.Close();
        exit(0);
    } else if (pid > 0) {
        // mainprocess, handle server connect and recv/send message
        auto openCallBack = []() -> void {
            GTEST_LOG_(INFO) << "ConnectWebSocketTest connection is open.";
        };
        serverSocket.SetOpenConnectionCallback(openCallBack);

        ret = serverSocket.AcceptNewConnection();
        ASSERT_TRUE(ret);
        std::string recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), HELLO_SERVER), 0);
        serverSocket.SendReply(HELLO_CLIENT);
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), CLIENT_OK), 0);
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), LONG_MSG.c_str()), 0);
        serverSocket.SendReply(SERVER_OK);
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), CLIENT_OK), 0);
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), LONG_LONG_MSG.c_str()), 0);
        serverSocket.SendReply(SERVER_OK);
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), CLIENT_OK), 0);
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), PING), 0); // the ping frame has "PING" and send a pong frame
        recv = serverSocket.Decode();
        EXPECT_EQ(strcmp(recv.c_str(), QUIT), 0);
        serverSocket.Close();
    } else {
        std::cerr << "ConnectWebSocketTest::fork failed, error = "
                  << errno << ", desc = " << strerror(errno) << std::endl;
    }
}

HWTEST_F(WebSocketTest, ReConnectWebSocketTest, testing::ext::TestSize.Level0)
{
    WebSocketServer serverSocket;
    bool ret = false;
#if defined(OHOS_PLATFORM)
    int appPid = getpid();
    ret = serverSocket.InitUnixWebSocket(UNIX_DOMAIN_PATH_2 + std::to_string(appPid), 5);
#else
    ret = serverSocket.InitTcpWebSocket(TCP_PORT, 5);
#endif
    ASSERT_TRUE(ret);
    for (int i = 0; i < 5; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // subprocess, handle client connect and recv/send message
            // note: EXPECT/ASSERT produce errors in subprocess that can not lead to failure of testcase in mainprocess,
            //       so testcase still success finally.
            WebSocketClient clientSocket;
            bool retClient = false;
#if defined(OHOS_PLATFORM)
            retClient = clientSocket.InitToolchainWebSocketForSockName(UNIX_DOMAIN_PATH_2 + std::to_string(appPid), 5);
#else
            retClient = clientSocket.InitToolchainWebSocketForPort(TCP_PORT, 5);
#endif
            ASSERT_TRUE(retClient);
            retClient = clientSocket.ClientSendWSUpgradeReq();
            ASSERT_TRUE(retClient);
            retClient = clientSocket.ClientRecvWSUpgradeRsp();
            ASSERT_TRUE(retClient);
            retClient = clientSocket.SendReply(HELLO_SERVER + std::to_string(i));
            EXPECT_TRUE(retClient);
            std::string recv = clientSocket.Decode();
            EXPECT_EQ(strcmp(recv.c_str(), (HELLO_CLIENT + std::to_string(i)).c_str()), 0);
            if (strcmp(recv.c_str(), (HELLO_CLIENT + std::to_string(i)).c_str()) == 0) {
                retClient = clientSocket.SendReply(CLIENT_OK + std::to_string(i));
                EXPECT_TRUE(retClient);
            }
            clientSocket.Close();
            exit(0);
        } else if (pid > 0) {
            // mainprocess, handle server connect and recv/send message
            ret = serverSocket.AcceptNewConnection();
            ASSERT_TRUE(ret);
            std::string recv = serverSocket.Decode();
            EXPECT_EQ(strcmp(recv.c_str(), (HELLO_SERVER + std::to_string(i)).c_str()), 0);
            serverSocket.SendReply(HELLO_CLIENT + std::to_string(i));
            recv = serverSocket.Decode();
            EXPECT_EQ(strcmp(recv.c_str(), (CLIENT_OK + std::to_string(i)).c_str()), 0);
            while (serverSocket.IsConnected()) {
                serverSocket.Decode();
            }
        } else {
            std::cerr << "ReConnectWebSocketTest::fork failed, error = "
                      << errno << ", desc = " << strerror(errno) << std::endl;
        }
    }
    serverSocket.Close();
}

HWTEST_F(WebSocketTest, ClientAbnormalTest, testing::ext::TestSize.Level0)
{
    WebSocketClient clientSocket;
    ASSERT_STREQ(clientSocket.GetSocketStateString().c_str(), "closed");
    ASSERT_FALSE(clientSocket.ClientSendWSUpgradeReq());
    ASSERT_FALSE(clientSocket.ClientRecvWSUpgradeRsp());
    ASSERT_FALSE(clientSocket.SendReply(HELLO_SERVER));
}

HWTEST_F(WebSocketTest, ServerAbnormalTest, testing::ext::TestSize.Level0)
{
    WebSocketServer serverSocket;
    // No connection established, the function returns directly.
    serverSocket.Close();
    ASSERT_FALSE(serverSocket.AcceptNewConnection());

#if defined(OHOS_PLATFORM)
    int appPid = getpid();
    ASSERT_TRUE(serverSocket.InitUnixWebSocket(UNIX_DOMAIN_PATH_3 + std::to_string(appPid), 5));
#else
    ASSERT_TRUE(serverSocket.InitTcpWebSocket(TCP_PORT, 5));
#endif
    pid_t pid = fork();
    if (pid == 0) {
        WebSocketClient clientSocket;
        auto closeCallBack = []() -> void {
            GTEST_LOG_(INFO) << "ServerAbnormalTest client connection is closed.";
        };
        clientSocket.SetCloseConnectionCallback(closeCallBack);

#if defined(OHOS_PLATFORM)
        ASSERT_TRUE(clientSocket.InitToolchainWebSocketForSockName(UNIX_DOMAIN_PATH_3 + std::to_string(appPid), 5));
        // state is not UNITED, the function returns directly.
        ASSERT_TRUE(clientSocket.InitToolchainWebSocketForSockName(UNIX_DOMAIN_PATH_3 + std::to_string(appPid), 5));
#else
        ASSERT_TRUE(clientSocket.InitToolchainWebSocketForPort(TCP_PORT, 5));
        // state is not UNITED, the function returns directly.
        ASSERT_TRUE(clientSocket.InitToolchainWebSocketForPort(TCP_PORT, 5));
#endif
        ASSERT_TRUE(clientSocket.ClientSendWSUpgradeReq());
        ASSERT_FALSE(clientSocket.ClientRecvWSUpgradeRsp());
        exit(0);
    } else if (pid > 0) {
        auto failCallBack = []() -> void {
            GTEST_LOG_(INFO) << "ServerAbnormalTest server connection is failed.";
        };
        serverSocket.SetFailConnectionCallback(failCallBack);
        auto notValidCallBack = [](const HttpRequest&) -> bool {
            GTEST_LOG_(INFO) << "ServerAbnormalTest server connection request is not valid.";
            return false;
        };
        serverSocket.SetValidateConnectionCallback(notValidCallBack);
        ASSERT_FALSE(serverSocket.AcceptNewConnection());
    } else {
        std::cerr << "ServerAbnormalTest::fork failed, error = "
                  << errno << ", desc = " << strerror(errno) << std::endl;
    }
}

HWTEST_F(WebSocketTest, InitUnixWebSocketWithValidSocketfdTest, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    WebSocketServer serverSocket;
    int sockfd[2];
    bool ret = false;
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd), 0);
    ret = serverSocket.InitUnixWebSocket(sockfd[0]);
    ASSERT_TRUE(ret);
    ret = serverSocket.InitUnixWebSocket(sockfd[1]);
    ASSERT_TRUE(ret);
    serverSocket.Close();
    close(sockfd[0]);
    close(sockfd[1]);
#endif
}

HWTEST_F(WebSocketTest, InitUnixWebSocketWithInvalidFdTest, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    WebSocketServer invalidFdServerSocket;
    bool ret = invalidFdServerSocket.InitUnixWebSocket(-1);
    ASSERT_FALSE(ret);
#endif
}

HWTEST_F(WebSocketTest, InitUnixWebSocketNormalInitCloseTest, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    WebSocketServer normalServerSocket;
    int sockfd[2];
    bool ret = false;
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd), 0);
    ret = normalServerSocket.InitUnixWebSocket(sockfd[0]);
    ASSERT_TRUE(ret);
    normalServerSocket.Close();
    close(sockfd[0]);
    close(sockfd[1]);
#endif
}

HWTEST_F(WebSocketTest, InitUnixWebSocketWithClosedFdTest, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    WebSocketServer closedFdServerSocket;
    int closedFd = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_GE(closedFd, 0);
    close(closedFd);
    bool ret = closedFdServerSocket.InitUnixWebSocket(closedFd);
    ASSERT_FALSE(ret);
#endif
}

HWTEST_F(WebSocketTest, ConnectUnixWebSocketBySocketpairTest, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    WebSocketServer serverSocket;
    int sockfd[2];
    bool ret = false;
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd), 0);
    ASSERT_TRUE(serverSocket.InitUnixWebSocket(sockfd[0]));
    pid_t pid = fork();
    if (pid == 0) {
        const char* upgradeRequest = "GET / HTTP/1.1\r\n"
                                     "Host: localhost\r\n"
                                     "Upgrade: websocket\r\n"
                                     "Connection: Upgrade\r\n"
                                     "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                                     "Sec-WebSocket-Version: 13\r\n"
                                     "\r\n";
        ssize_t sent = send(sockfd[1], upgradeRequest, strlen(upgradeRequest), 0);
        ASSERT_EQ(sent, static_cast<ssize_t>(strlen(upgradeRequest)));
        char buffer[1024] = {0};
        ssize_t received = recv(sockfd[1], buffer, sizeof(buffer) - 1, 0);
        ASSERT_GT(received, 0);
        close(sockfd[1]);
        exit(0);
    } else if (pid > 0) {
        ret = serverSocket.ConnectUnixWebSocketBySocketpair();
        ASSERT_TRUE(ret);
        int status;
        waitpid(pid, &status, 0);
        ASSERT_EQ(status, 0);
        serverSocket.Close();
    } else {
        std::cerr << "ConnectUnixWebSocketBySocketpairTest::fork failed, error = "
                  << errno << ", desc = " << strerror(errno) << std::endl;
    }
    close(sockfd[0]);
#endif
}

HWTEST_F(WebSocketTest, ConnectUnixWebSocketBySocketpairFailedTest, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    WebSocketServer serverSocket;
    int sockfd[2];
    bool ret = false;
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd), 0);
    ASSERT_TRUE(serverSocket.InitUnixWebSocket(sockfd[0]));
    pid_t pid = fork();
    if (pid == 0) {
        const char* invalidRequest = "GET / HTTP/1.1\r\n"
                                     "Host: localhost\r\n"
                                     "\r\n";
        ssize_t sent = send(sockfd[1], invalidRequest, strlen(invalidRequest), 0);
        ASSERT_EQ(sent, static_cast<ssize_t>(strlen(invalidRequest)));
        char buffer[1024] = {0};
        ssize_t received = recv(sockfd[1], buffer, sizeof(buffer) - 1, 0);
        ASSERT_GT(received, 0);
        close(sockfd[1]);
        exit(0);
    } else if (pid > 0) {
        ret = serverSocket.ConnectUnixWebSocketBySocketpair();
        ASSERT_FALSE(ret);
        int status;
        waitpid(pid, &status, 0);
        ASSERT_EQ(status, 0);
        serverSocket.Close();
    } else {
        std::cerr << "ConnectUnixWebSocketBySocketpairHandshakeFailedTest::fork failed, error = "
                  << errno << ", desc = " << strerror(errno) << std::endl;
    }
    close(sockfd[0]);
#endif
}

HWTEST_F(WebSocketTest, InitTcpWebSocketInvalidPortTest, testing::ext::TestSize.Level0)
{
    WebSocketServer serverSocket;
    bool ret = false;
    ret = serverSocket.InitTcpWebSocket(-1, 5);
    ASSERT_FALSE(ret);
}

HWTEST_F(WebSocketTest, InitTcpWebSocketAlreadyInitedTest, testing::ext::TestSize.Level0)
{
    WebSocketServer serverSocket;
    bool ret = false;
    ret = serverSocket.InitTcpWebSocket(TCP_PORT, 5);
    ASSERT_TRUE(ret);
    ret = serverSocket.InitTcpWebSocket(TCP_PORT, 5);
    ASSERT_TRUE(ret);
    serverSocket.Close();
}

HWTEST_F(WebSocketTest, InitTcpWebSocketPortInUseTest, testing::ext::TestSize.Level0)
{
    WebSocketServer serverSocket1;
    WebSocketServer serverSocket2;
    bool ret = false;
    ret = serverSocket1.InitTcpWebSocket(TCP_PORT + 2, 5);
    ASSERT_TRUE(ret);
    ret = serverSocket2.InitTcpWebSocket(TCP_PORT + 2, 5);
    ASSERT_FALSE(ret);
    serverSocket1.Close();
}

HWTEST_F(WebSocketTest, InitToolchainWebSocketForPortTest, testing::ext::TestSize.Level0)
{
    bool ret = false;
    WebSocketClient clientSocket1;
    ret = clientSocket1.InitToolchainWebSocketForPort(-1, 5);
    ASSERT_FALSE(ret);
    WebSocketClient clientSocket2;
    struct rlimit originalLimit;
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &originalLimit), 0);
    struct rlimit newLimit;
    newLimit.rlim_cur = 2;
    newLimit.rlim_max = originalLimit.rlim_max;
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &newLimit), 0);
    std::vector<int> dummyFds;
    int fd;
    while ((fd = open("/dev/null", O_RDONLY)) >= 0) {
        dummyFds.push_back(fd);
    }
    ret = clientSocket2.InitToolchainWebSocketForPort(8080, 5);
    ASSERT_FALSE(ret);
    for (int dummyFd : dummyFds) {
        close(dummyFd);
    }
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &originalLimit), 0);
}

}  // namespace panda::test