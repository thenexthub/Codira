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

#include <string_view>

#include "gtest/gtest.h"
#include "http.h"

using namespace OHOS::ArkCompiler::Toolchain;

namespace panda::test {
class HttpDecoderTest : public testing::Test {
public:
    static constexpr std::string_view REQUEST_HEADERS_ONE = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Origin: dvtls://dvtls\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersOne = std::string(REQUEST_HEADERS_ONE);

    static constexpr std::string_view REQUEST_HEADERS_TWO = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersTwo = std::string(REQUEST_HEADERS_TWO);

    static constexpr std::string_view REQUEST_HEADERS_THREE = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Origin: http://192.168.43.4:8000\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersThree = std::string(REQUEST_HEADERS_THREE);

    static constexpr std::string_view REQUEST_HEADERS_FOUR = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Origin: https://127.0.0.1:8000\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersFour = std::string(REQUEST_HEADERS_FOUR);

    static constexpr std::string_view REQUEST_HEADERS_FIVE = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Origin: https://localhost\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersFive = std::string(REQUEST_HEADERS_FIVE);

    static constexpr std::string_view REQUEST_HEADERS_SIX = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Origin: https://www.abc.com\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersSix = std::string(REQUEST_HEADERS_SIX);

    static constexpr std::string_view REQUEST_HEADERS_SEVEN = "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:19015\r\n"
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) Chrome/117.0.0.0 Safari/537.36\r\n"
        "Upgrade: websocket\r\n"
        "Origin: https://[::1]:8000\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Sec-WebSocket-Key: AyuTxzyBTJJdViDskomT0Q==\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n\r\n";
    std::string requestHeadersSeven = std::string(REQUEST_HEADERS_SEVEN);

    static constexpr std::string_view ERR_REQUEST_HEADERS = "GEY\r\n";
    std::string errRequestHeaders = std::string(ERR_REQUEST_HEADERS);

    static constexpr std::string_view NO_INFO_REQUEST_HEADERS = "GET\r\n";
    std::string noInfoRequestHeaders = std::string(NO_INFO_REQUEST_HEADERS);

    static constexpr std::string_view RESPONSE_HEADERS = "HTTP/1.1 101 Switching Protocols\r\n"
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Accept: AyuTxzyBTJJdViDskomT0Q==\r\n\r\n";
    std::string responseHeaders = std::string(RESPONSE_HEADERS);

    static constexpr std::string_view ERR_RESPONSE_HEADERS = "HTTB\r\n";
    std::string errResponseHeaders = std::string(ERR_RESPONSE_HEADERS);

    static constexpr std::string_view NO_INFO_RESPONSE_HEADERS = "HTTP\r\n";
    std::string noInfoResponseHeaders = std::string(NO_INFO_RESPONSE_HEADERS);

    static constexpr std::string_view EXPECTED_VERSION              = "HTTP/1.1";
    static constexpr std::string_view EXPECTED_STATUS               = "101";
    static constexpr std::string_view EXPECTED_CONNECTION           = "Upgrade";
    static constexpr std::string_view EXPECTED_UPGRADE              = "websocket";
    static constexpr std::string_view EXPECTED_SEC_WEBSOCKET_KEY    = "AyuTxzyBTJJdViDskomT0Q==";
};

HWTEST_F(HttpDecoderTest, TestRequestDecode_1, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersOne, parsed);

    ASSERT_TRUE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "dvtls://dvtls");
}

HWTEST_F(HttpDecoderTest, TestRequestDecode_2, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersTwo, parsed);

    ASSERT_TRUE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "");
}

HWTEST_F(HttpDecoderTest, TestRequestDecode_3, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersThree, parsed);

    ASSERT_FALSE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "http://192.168.43.4:8000");
}

HWTEST_F(HttpDecoderTest, TestRequestDecode_4, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersFour, parsed);

    ASSERT_TRUE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "https://127.0.0.1:8000");
}

HWTEST_F(HttpDecoderTest, TestRequestDecode_5, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersFive, parsed);

    ASSERT_TRUE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "https://localhost");
}

HWTEST_F(HttpDecoderTest, TestRequestDecode_6, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersSix, parsed);

    ASSERT_FALSE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "https://www.abc.com");
}

HWTEST_F(HttpDecoderTest, TestRequestDecode_7, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    auto succeeded = HttpRequest::Decode(requestHeadersSeven, parsed);

    ASSERT_TRUE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketKey, EXPECTED_SEC_WEBSOCKET_KEY);
    ASSERT_EQ(parsed.origin, "https://[::1]:8000");
}

HWTEST_F(HttpDecoderTest, TestAbnormalRequestDecode, testing::ext::TestSize.Level0)
{
    HttpRequest parsed;
    ASSERT_FALSE(HttpRequest::Decode(errRequestHeaders, parsed));
    ASSERT_TRUE(HttpRequest::Decode(noInfoRequestHeaders, parsed));

    ASSERT_EQ(parsed.version, "");
    ASSERT_EQ(parsed.connection, "");
    ASSERT_EQ(parsed.upgrade, "");
    ASSERT_EQ(parsed.secWebSocketKey, "");
}

HWTEST_F(HttpDecoderTest, TestResponseDecode, testing::ext::TestSize.Level0)
{
    HttpResponse parsed;
    auto succeeded = HttpResponse::Decode(responseHeaders, parsed);

    ASSERT_TRUE(succeeded);
    ASSERT_EQ(parsed.version, EXPECTED_VERSION);
    ASSERT_EQ(parsed.status, EXPECTED_STATUS);
    ASSERT_EQ(parsed.connection, EXPECTED_CONNECTION);
    ASSERT_EQ(parsed.upgrade, EXPECTED_UPGRADE);
    ASSERT_EQ(parsed.secWebSocketAccept, EXPECTED_SEC_WEBSOCKET_KEY);
}

HWTEST_F(HttpDecoderTest, TestAbnormalResponseDecode, testing::ext::TestSize.Level0)
{
    HttpResponse parsed;
    ASSERT_FALSE(HttpResponse::Decode(errResponseHeaders, parsed));
    ASSERT_TRUE(HttpResponse::Decode(noInfoResponseHeaders, parsed));

    ASSERT_EQ(parsed.version, "");
    ASSERT_EQ(parsed.status, "");
    ASSERT_EQ(parsed.connection, "");
    ASSERT_EQ(parsed.upgrade, "");
    ASSERT_EQ(parsed.secWebSocketAccept, "");
}

}  // namespace panda::test
