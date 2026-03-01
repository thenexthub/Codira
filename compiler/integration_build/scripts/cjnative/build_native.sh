#!/bin/bash

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

set -e;

# 编译Codira编译器 + codedb
cd ${WORKSPACE}/codira_compiler;
python3 build.py clean;
python3 build.py build -t ${build_type} --build-codedb ${add_opts_buildpy};
python3 build.py install;

source output/envsetup.sh;
# 验证安装
codec -v;

# 编译运行时
cd ${WORKSPACE}/codira_runtime/runtime;
python3 build.py clean;
python3 build.py build -t ${build_type} -v ${codira_version};
python3 build.py install;
cp -rf ${WORKSPACE}/codira_runtime/runtime/output/common/${kernel}_${build_type}_${cmake_arch}/{lib,runtime} ${WORKSPACE}/codira_compiler/output;

# 编译标准库
cd ${WORKSPACE}/codira_runtime/std;
python3 build.py clean;
python3 build.py build -t ${build_type} \
    --target-lib=${WORKSPACE}/codira_runtime/runtime/output \
    --target-lib=$OPENSSL_PATH;
python3 build.py install;
cp -rf ${WORKSPACE}/codira_runtime/std/output/* ${WORKSPACE}/codira_compiler/output/;

# 编译STDX扩展库
cd ${WORKSPACE}/codira_stdx;
python3 build.py clean;
python3 build.py build -t ${build_type} \
  --include=${WORKSPACE}/codira_compiler/include \
  --target-lib=$OPENSSL_PATH;
python3 build.py install;

export CODIRA_STDX_PATH=${WORKSPACE}/codira_stdx/target/${kernel}_${cmake_arch}_codenative/static/stdx;

# 编译codepm
cd ${WORKSPACE}/codira_tools/codepm/build;
python3 build.py clean;
python3 build.py build -t ${build_type} --set-rpath $RPATH;
python3 build.py install;

# 编译codefmt
cd ${WORKSPACE}/codira_tools/codefmt;
cd build;
python3 build.py clean;
python3 build.py build -t ${build_type};
python3 build.py install;

# 编译hle
cd ${WORKSPACE}/codira_tools/hyperlangExtension/build;
python3 build.py clean;
python3 build.py build -t ${build_type};
python3 build.py install;

# 编译lsp
cd ${WORKSPACE}/codira_tools/codira-language-server/build;
python3 build.py clean;
python3 build.py build -t ${build_type} -j 16;
python3 build.py install;