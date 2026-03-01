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


add_subdirectory(native)


set(STD_NET_SRCS
    ip_addess.code
    ip_prefix.code
    ip_socket_address.code
    ipv4_address.code
    ipv6_address.code
    result.code
    rune_ext.code
    socket_address.code
    unix_socket_address.code
    uint128.code
    address_family.code
    dns.code
    ip_address_ffi.code
    linger.code
    socket_ffi.code
    socket_ffi_sockaddr.code
    socket_enum.code
    socket_util.code
    socket_exception.code
    socket_raw_enum.code
    socket_keep_alive.code
    socket_holder.code
    socket_state.code
    socket_api.code
    socket_options.code
    mutex.code
    tcp.code
    tcp_server.code
    udp.code
    unix.code
    unix_datagram.code
    socket_impl_common.code
    CACHE INTERNAL "")

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(CODENATIVE_STD_NET_SRCS
        ${STD_NET_SRCS}
        socket_buffer.code
        socket_ffi_codenative.code
        socket_ffi_codenative_impl.code
        socket_raw_codenative.code
        CACHE INTERNAL "")
endif()
