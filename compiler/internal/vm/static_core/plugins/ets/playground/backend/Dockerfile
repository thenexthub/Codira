# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM python:3.10
WORKDIR /playground


COPY ./build/ ./build
COPY ./plugins/ets/stdlib ./stdlib
COPY ./plugins/ets/playground/backend/dist ./dist
COPY ./plugins/ets/playground/backend/docker/config.yaml ./config.yaml
COPY ./plugins/ets/playground/backend/docker/arktsconfig.json ./build/bin/arktsconfig.json
COPY ./third_party/icu/ohos_icu4j/data/icudt72l.dat ./icudt72l.dat

# System libraries required by ark binaries
RUN apt-get update -qq && apt-get install -yqq \
    libatomic1 \
    libdwarf1 \
    libelf1 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

# Set library path for ark_aot and other binaries
ENV LD_LIBRARY_PATH=/playground/build/lib

# Compile stdlib in AOT mode for better performance
RUN cd /playground && \
    /playground/build/bin/ark_aot \
    --boot-panda-files=/playground/build/plugins/ets/etsstdlib.abc \
    --load-runtimes=ets \
    --paoc-panda-files=/playground/build/plugins/ets/etsstdlib.abc \
    --paoc-output=/playground/build/plugins/ets/etsstdlib.an \
    --paoc-mode=aot \
    --gc-type=g1-gc \
    --compiler-ignore-failures=false

RUN python3 -m pip install --no-cache-dir /playground/dist/*.whl
CMD ["arkts-playground", "-c", "/playground/config.yaml"]
