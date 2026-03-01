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

cd ${WORKSPACE}/codira_compiler;
python3 build.py clean;
python3 build.py build -t ${build_type} ${add_opts_buildpy};

export CMAKE_PREFIX_PATH=${MINGW_PATH}/x86_64-w64-mingw32;
python3 build.py build -t ${build_type} \
	--product codec \
  --no-tests \
	--target windows-x86_64 \
	--target-sysroot ${MINGW_PATH}/ \
	--target-toolchain ${MINGW_PATH}/bin \
	--build-codedb;
python3 build.py build -t ${build_type} \
	--product libs \
	--target windows-x86_64 \
	--target-sysroot ${MINGW_PATH}/ \
  --target-toolchain ${MINGW_PATH}/bin;
python3 build.py install --host windows-x86_64;
python3 build.py install;
cp -rf output-x86_64-w64-mingw32/* output;

source output/envsetup.sh;
codec -v;

# 编译运行时
cd ${WORKSPACE}/codira_runtime/runtime;
python3 build.py clean;
python3 build.py build -t ${build_type} \
  --target windows-x86_64 \
	--target-toolchain ${MINGW_PATH}/bin \
  -v ${codira_version};
python3 build.py install;
cp -rf ${WORKSPACE}/codira_runtime/runtime/output/common/windows_${build_type}_x86_64/{lib,runtime} ${WORKSPACE}/codira_compiler/output;
cp -rf ${WORKSPACE}/codira_runtime/runtime/output/common/windows_${build_type}_x86_64/{lib,runtime} ${WORKSPACE}/codira_compiler/output-x86_64-w64-mingw32;

# 编译标准库
cd ${WORKSPACE}/codira_runtime/std;
python3 build.py clean;
python3 build.py build -t ${build_type} \
  --target windows-x86_64 \
  --target-lib=${WORKSPACE}/codira_runtime/runtime/output \
  --target-lib=${MINGW_PATH}/x86_64-w64-mingw32/lib \
  --target-sysroot ${MINGW_PATH}/ \
  --target-toolchain ${MINGW_PATH}/bin;
python3 build.py install;
cp -rf ${WORKSPACE}/codira_runtime/std/output/* ${WORKSPACE}/codira_compiler/output/;
cp -rf ${WORKSPACE}/codira_runtime/std/output/* ${WORKSPACE}/codira_compiler/output-x86_64-w64-mingw32/;

# 编译STDX扩展库
cd ${WORKSPACE}/codira_stdx;
python3 build.py clean;
python3 build.py build -t ${build_type} \
	--include=$WORKSPACE/codira_compiler/include \
    --target-lib=${MINGW_PATH}/x86_64-w64-mingw32/lib \
	--target windows-x86_64 \
    --target-sysroot ${MINGW_PATH}/ \
    --target-toolchain ${MINGW_PATH}/bin;
python3 build.py install;
export CODIRA_STDX_PATH=${WORKSPACE}/codira_stdx/target/windows_x86_64_codenative/static/stdx;

# 编译codepm
cd ${WORKSPACE}/codira_tools/codepm/build;
python3 build.py clean;
python3 build.py build -t ${build_type} --target windows-x86_64;
python3 build.py install;

# 编译codefmt
cd ${WORKSPACE}/codira_tools/codefmt/build;
python3 build.py clean;
python3 build.py build -t ${build_type} --target windows-x86_64;
python3 build.py install;

# 编译hle
cd ${WORKSPACE}/codira_tools/hyperlangExtension/build;
python3 build.py clean;
python3 build.py build -t ${build_type} --target windows-x86_64;
python3 build.py install;

# 编译lsp
cd ${WORKSPACE}/codira_tools/codira-language-server/build;
python3 build.py clean;
python3 build.py build -t ${build_type} --target windows-x86_64 -j 16;
python3 build.py install;

# 清空历史构建
mkdir -p $WORKSPACE/software;
rm -rf $WORKSPACE/software/*;

# 打包Codira Frontend
cd $WORKSPACE/software;
mkdir -p codira/lib/windows_x86_64_codenative;
cp $WORKSPACE/codira_compiler/LICENSE codira;
cp $WORKSPACE/codira_compiler/Open_Source_Software_Notice.docx codira;
chmod -R 750 codira;
mv $WORKSPACE/codira_compiler/output-x86_64-w64-mingw32/lib/windows_x86_64_codenative/libcodira-ast-support.a codira/lib/windows_x86_64_codenative;
find codira -print0 | xargs -0r touch -t "$BEP_BUILD_TIME";
find codira -print0 | LC_ALL=C sort -z | xargs -0 zip -o -X $WORKSPACE/software/codira-frontend-windows-x64-${codira_version}.zip;

# 打包Codira SDK
rm -rf codira && cp -R $WORKSPACE/codira_compiler/output-x86_64-w64-mingw32 codira;
cp $WORKSPACE/codira_tools/codepm/dist/codepm.exe codira/tools/bin;
mkdir -p codira/tools/config;
cp $WORKSPACE/codira_tools/codefmt/build/build/bin/codefmt.exe codira/tools/bin;
cp $WORKSPACE/codira_tools/codefmt/config/*.toml codira/tools/config;
cp $WORKSPACE/codira_tools/hyperlangExtension/target/bin/main.exe codira/tools/bin/hle.exe;
cp -r $WORKSPACE/codira_tools/hyperlangExtension/src/dtsparser codira/tools;
rm -rf codira/tools/dtsparser/*.code;
cp $WORKSPACE/codira_tools/codira-language-server/output/bin/LSPServer.exe codira/tools/bin;
cp $WORKSPACE/codira_compiler/LICENSE codira;
cp $WORKSPACE/codira_compiler/Open_Source_Software_Notice.docx codira;
chmod -R 750 codira;
find codira -print0 | xargs -0r touch -t "$BEP_BUILD_TIME";
find codira -print0 | LC_ALL=C sort -z | xargs -0 zip -o -X $WORKSPACE/software/codira-sdk-windows-x64-${codira_version}.zip;

# 打包Codira STDX
cp -R $WORKSPACE/codira_stdx/target/windows_x86_64_codenative ./;
cp $WORKSPACE/codira_stdx/LICENSE windows_x86_64_codenative;
cp $WORKSPACE/codira_stdx/Open_Source_Software_Notice.docx windows_x86_64_codenative;
chmod -R 750 windows_x86_64_codenative;
find windows_x86_64_codenative -print0 | xargs -0r touch -t "$BEP_BUILD_TIME";
find windows_x86_64_codenative -print0 | LC_ALL=C sort -z | xargs -0 zip -o -X $WORKSPACE/software/codira-stdx-windows-x64-${codira_version}.${stdx_version}.zip;

chmod 550 *.zip;

ls -lh $WORKSPACE/software