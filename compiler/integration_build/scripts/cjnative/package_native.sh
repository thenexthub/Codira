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

# 清空历史构建
mkdir -p $WORKSPACE/software;
rm -rf $WORKSPACE/software/*;

# 打包Codira Frontend
cd $WORKSPACE/software;
mkdir -p codira/lib/${kernel}_${cmake_arch}_codenative;
cp $WORKSPACE/codira_compiler/LICENSE codira;
cp $WORKSPACE/codira_compiler/Open_Source_Software_Notice.docx codira;
chmod -R 750 codira;
mv $WORKSPACE/codira_compiler/output/lib/${kernel}_${cmake_arch}_codenative/libcodira-ast-support.a codira/lib/${kernel}_${cmake_arch}_codenative;
$tar \
  --sort=name --mtime="@${SOURCE_DATE_EPOCH}" \
  --owner=0 \
  --group=0 \
  --numeric-owner \
  --pax-option=exthdr.name=$d/PaxHeaders/%f,delete=ctime \
  -cf \
  - codira | gzip -n > codira-frontend-${os}-${arch_name}-${codira_version}.tar.gz;

# 打包Codira SDK
rm -rf codira && cp -R $WORKSPACE/codira_compiler/output codira;
cp $WORKSPACE/codira_tools/codepm/dist/codepm codira/tools/bin/codepm;
mkdir -p codira/tools/config;
cp $WORKSPACE/codira_tools/codefmt/build/build/bin/codefmt codira/tools/bin;
cp $WORKSPACE/codira_tools/codefmt/config/*.toml codira/tools/config;
cp $WORKSPACE/codira_tools/hyperlangExtension/target/bin/main codira/tools/bin/hle;
cp -r $WORKSPACE/codira_tools/hyperlangExtension/src/dtsparser codira/tools;
rm -rf codira/tools/dtsparser/*.code;
cp $WORKSPACE/codira_tools/codira-language-server/output/bin/LSPServer codira/tools/bin;
cp $WORKSPACE/codira_compiler/LICENSE codira;
cp $WORKSPACE/codira_compiler/Open_Source_Software_Notice.docx codira;
chmod -R 750 codira;
$tar \
  --sort=name --mtime="@${SOURCE_DATE_EPOCH}" \
  --owner=0 \
  --group=0 \
  --numeric-owner \
  --pax-option=exthdr.name=$d/PaxHeaders/%f,delete=ctime \
  -cf \
  - codira | gzip -n > codira-sdk-${os}-${arch_name}-${codira_version}.tar.gz;

# 打包Codira STDX
cp -R $WORKSPACE/codira_stdx/target/${kernel}_${cmake_arch}_codenative ./;
cp $WORKSPACE/codira_stdx/LICENSE ${kernel}_${cmake_arch}_codenative;
cp $WORKSPACE/codira_stdx/Open_Source_Software_Notice.docx ${kernel}_${cmake_arch}_codenative;
chmod -R 750 ${kernel}_${cmake_arch}_codenative;
find ${kernel}_${cmake_arch}_codenative -print0 | xargs -0r touch -t "$BEP_BUILD_TIME";
find ${kernel}_${cmake_arch}_codenative -print0 | LC_ALL=C sort -z | xargs -0 zip -o -X $WORKSPACE/software/codira-stdx-${os}-${arch_name}-${codira_version}.${stdx_version}.zip;

chmod 550 *.tar.gz *.zip;

ls -lh $WORKSPACE/software