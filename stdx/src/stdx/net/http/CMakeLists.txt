# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

set(HTTP_SRCS
        pool.code
        buffered_conn.code
        client.code
        collection_extend.code
        collection_multi_level_queue.code
        collection_queue.code
        constants.code
        cookie_jar.code 
        cookie.code
        coroutine_pool.code
        exception.code
        frame.code
        hpack_decoder.code
        hpack_encoder.code
        hpack_header_table.code
        hpack_quick_huffman.code
        hpack_utils.code
        http_body.code
        http_client1_1.code
        http_client2_0.code
        http_distributor.code
        http_engine_conn.code
        http_enum.code
        http_handler.code
        http_header.code
        http_logger.code
        http_request_context.code
        http_request.code
        http_response.code
        http_server1_1.code
        http_server2_0.code
        http_status_code.code
        protocol_service.code
        server.code
        str.code
        stream_client.code
        stream_server2_0.code
        transport.code
        utils.code
        utils2_0.code
        websocket_conn.code
        websocket_frame.code
        websocket_status_code.code
        websocket.code
        CACHE INTERNAL ""
)