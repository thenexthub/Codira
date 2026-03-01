#!/bin/bash

set -e;

export CMAKE_PREFIX_PATH=${MINGW_PATH}/x86_64-w64-mingw32;
cd ${WORKSPACE}/codira_compiler;
python3 build.py build -t ${build_type} \
	--product libs \
	--target windows-x86_64 \
	--target-sysroot ${MINGW_PATH}/ \
	--target-toolchain ${MINGW_PATH}/bin;
python3 build.py install;

source output/envsetup.sh;
# 验证codec可用
codec -v;

# 编译windows 运行时
cd ${WORKSPACE}/codira_runtime/runtime;
python3 build.py clean;
python3 build.py build -t ${build_type} \
    --target windows-x86_64 \
	  --target-toolchain ${MINGW_PATH}/bin \
    -v ${codira_version};
python3 build.py install;
cp -rf ${WORKSPACE}/codira_runtime/runtime/output/common/windows_${build_type}_x86_64/{lib,runtime} ${WORKSPACE}/codira_compiler/output;

# 编译windows 标准库
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