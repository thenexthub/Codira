#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

FROM ubuntu:22.04

ARG TOOLS_BASE=/opt/tools
ARG NODE_VERSION=v18.20.2
ARG NODE_ARCH=node.tar.xz

ENV REQUESTS_CA_BUNDLE=/etc/ssl/certs

RUN apt-get update \
  && apt-get install -y --no-install-recommends \
    ca-certificates=2024* \
    python3-pip=22.0.* \
    tzdata=2024* \
    xz-utils=5.2.* \
  && rm -rf /var/lib/apt/lists/*

# fixes python-pip bug 1914239
RUN mkdir -p /etc/xdg/pip && ln -s /etc/pip.conf /etc/xdg/pip/pip.conf

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime \
  && echo $TZ > /etc/timezone

RUN apt-get update \
  && apt-get install -y --no-install-recommends \
    curl=7.81.* \
    dnsutils=1:9.18.* \
    git=1:2.34.* \
    gnupg=2.2.27* \
    iproute2=5.15.* \
    sudo=1.9.9* \
    unzip=6.* \ 
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p "${TOOLS_BASE:?}" \
    && curl -Lo "${TOOLS_BASE:?}/${NODE_ARCH:?}" "https://nodejs.org/dist/${NODE_VERSION:?}/node-${NODE_VERSION:?}-linux-x64.tar.xz" \
    && tar -xJf "${TOOLS_BASE:?}/${NODE_ARCH:?}" -C "${TOOLS_BASE:?}" \
    && ln -s "${TOOLS_BASE:?}/node-${NODE_VERSION:?}-linux-x64/bin/node" /usr/local/bin \
    && ln -s "${TOOLS_BASE:?}/node-${NODE_VERSION:?}-linux-x64/lib/node_modules/npm/bin/npm-cli.js" /usr/local/bin/npm \
    && ln -s "${TOOLS_BASE:?}/node-${NODE_VERSION:?}-linux-x64/lib/node_modules/npm/bin/npx-cli.js" /usr/local/bin/npx \
    && ln -s "${TOOLS_BASE:?}/node-${NODE_VERSION:?}-linux-x64/include/node" /usr/local/include/node \
    && rm -rf "${TOOLS_BASE:?}/${NODE_ARCH:?}"
 
WORKDIR /arkcompiler
COPY runtime_core/static_core/scripts /arkcompiler/runtime_core/static_core/scripts
RUN  ./runtime_core/static_core/scripts/install-deps-ubuntu \
    -i=dev \
    -i=test \
    -i=ohos-sdk \
    -i=llvm-prebuilts \
    -i=windows \
    -i=arm-all

ENTRYPOINT ["/bin/bash"]
