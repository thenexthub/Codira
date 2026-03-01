#!/bin/bash
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -ex

do_fetch() {
    echo "skip."
}

do_patch() {
    echo "skip."
}

do_configure() {
    use_libz_inner=0

    if [ -f "${LIB_ROOT_DIR}/obj/vendor/${DEPENDENCY_TAG}/foundation/bundlemanager/zlib_override/libz_inner.a" ];then
        use_libz_inner=1
    fi

    ${CMAKE_PATH} \
        -DLIB_ROOT_DIR=${LIB_ROOT_DIR}\
        -DTARGET_CPU=${TARGET_CPU}\
        -DUSE_LIBZ_INNER=${use_libz_inner}\
        -B${TARGET_GEN_DIR}/build\
        -S${JSVM_PATH}\
        -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS}"\
        -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}"\
        -DDEPENDENCY_TAG="${DEPENDENCY_TAG}"
}

do_unstripped_copy() {
    JSVM_UNSTRIPPED_PATH=${TARGET_GEN_DIR}/../../../lib.unstripped/arkcompiler/jsvm/
    mkdir -p ${JSVM_UNSTRIPPED_PATH}
    cp -u ${workdir}/libjsvm.so ${JSVM_UNSTRIPPED_PATH}
    cp -u ${JS_ENGINE_URL}/v8/${TARGET_CPU}/lib.unstripped_v8/lib.unstripped/libv8_shared.so ${JSVM_UNSTRIPPED_PATH}
}

get_thread_num() {
    quota_us_file="/sys/fs/cgroup/cpu/cpu.cfs_quota_us"
    period_us_file="/sys/fs/cgroup/cpu/cpu.cfs_period_us"
    if [ -f "${quota_us_file}" ]; then
        cfs_quota_us=$(cat ${quota_us_file})
    fi
    if [ -f "${period_us_file}" ]; then
        cfs_period_us=$(cat ${period_us_file})
    fi
    # Set the default value when the variable is empty.
    cfs_quota_us=${cfs_quota_us:=-1}
    cfs_period_us=${cfs_period_us:=0}
    if [ "${cfs_quota_us}" != -1 -a "${cfs_period_us}" != 0 ]; then
        PROCESSORS=$(expr ${cfs_quota_us} / ${cfs_period_us})
        echo "cpu.cfs_quota_us: "$PROCESSORS
    else
        PROCESSORS=$(cat /proc/cpuinfo | grep "processor" | wc -l)
        echo "cpuinfo: "$PROCESSORS
    fi
}

do_compile() {
    get_thread_num
    cpu_num=$[PROCESSORS*2]
    make VERBOSE=1 -j${cpu_num} -C${workdir}
}

do_strip() {
    stripped_binary_path=${TARGET_GEN_DIR}/libjsvm.so
    binary=${stripped_binary_path}
    echo "${binary}"
    dynsyms_path="${stripped_binary_path}.dynsyms"
    funcsysms_path="${stripped_binary_path}.funcsyms"
    keep_path="${stripped_binary_path}.keep"
    debug_path="${stripped_binary_path}.debug"
    mini_debug_path="${stripped_binary_path}.minidebug"

    ${NM} -D ${binary} --format=posix --defined-only \
            | awk '{ print $1 }' | sort > ${dynsyms_path}
    ${NM} ${binary} --format=posix --defined-only \
            | awk '{ if ($2 == "T" || $2 == "t" || $2 == "D") print $1 }' \
            | sort > ${funcsysms_path}
    comm -13 ${dynsyms_path} ${funcsysms_path} > ${keep_path}

    ${OBJCOPY} --only-keep-debug ${binary} ${debug_path}
    ${OBJCOPY} -S --remove-section .gdb_index --remove-section .comment \
            --keep-symbols=${keep_path} ${debug_path} ${mini_debug_path}

    ${STRIP} --strip-all ${binary}

    xz ${mini_debug_path}
    ${OBJCOPY} --add-section .gnu_debugdata=${mini_debug_path}.xz ${binary}

    rm -f ${dynsyms_path}
    rm -f ${funcsysms_path}
    rm -f ${keep_path}
    rm -f ${debug_path}
    rm -f ${mini_debug_path}
    rm -f ${mini_debug_path}.xz
}

do_install () {
    cp -u ${workdir}/libjsvm.so ${TARGET_GEN_DIR}
}

do_install_asan() {
    # todo replace libv8_shared.so with hwasan
    mkdir -p ${TARGET_GEN_DIR}/asan
    cp -u ${workdir}/libjsvm.so ${TARGET_GEN_DIR}/asan
    cp -u ${JS_ENGINE_URL}/v8/${TARGET_CPU}/lib.unstripped_v8/lib.unstripped/libv8_shared.so ${TARGET_GEN_DIR}/asan

    mkdir -p ${TARGET_GEN_DIR}/../../../../../lib.unstripped/jsvm/
    cp -u ${workdir}/libjsvm.so ${TARGET_GEN_DIR}/../../../../../lib.unstripped/jsvm/
    cp -u ${JS_ENGINE_URL}/v8/${TARGET_CPU}/lib.unstripped_v8/lib.unstripped/libv8_shared.so ${TARGET_GEN_DIR}/../../../../../lib.unstripped/jsvm/
}

do_env() {
    # init workspace
    out_dir=${TARGET_GEN_DIR}/build
    workdir=${TARGET_GEN_DIR}/build
    # alway rebuild
    mkdir -p ${workdir}

    argurment+=" -D__MUSL__  -D_LIBCPP_HAS_MUSL_LIBC -DOHOS_JS_ENGINE -DENABLE_HISYSEVENT"
    argurment+=" -DV8_COMPRESS_POINTERS -DV8_COMPRESS_POINTERS_IN_SHARED_CAGE -DV8_31BIT_SMIS_ON_64BIT_ARCH -DV8_HAVE_TARGET_OS -DV8_TARGET_OS_OHOS"
    argurment+=" -fstack-protector-strong"
    argurment+=" -Wl,-z,noexecstack"
    argurment+=" -Wl,-z,relro"
    argurment+=" -Wl,-z,now"
    argurment+=" -pie"
    if [ $1 -eq 1 ]; then
        argurment+=" -ggdb3"
    fi
    if [[ "${TARGET_CPU}" = "arm" ]]; then
        cflags="  --target=arm-linux-ohos"
        cflags+=" --sysroot=${SYSROOT}"
        if [[ "${DEPENDENCY_TAG}" = "default" ]]; then
          cflags+=" -isystem ${SYSROOT}/usr/include/arm-linux-ohos"
        fi
        cflags+=" -march=armv7-a"
        cflags+=" -mfpu=neon"
        cflags+=" -mbranch-protection=pac-ret+b-key+bti"
        cflags_host="-m32"
        ARCH="arm"
    elif [[ "${TARGET_CPU}" = "arm64" ]]; then
        cflags="  --target=aarch64-linux-ohos"
        cflags+=" --sysroot=${SYSROOT}"
        if [[ "${DEPENDENCY_TAG}" = "default" ]]; then
          cflags+=" -isystem ${SYSROOT}/usr/include/aarch64-linux-ohos"
        fi
        cflags+=" -march=armv8-a"
        cflags+=" -DV8_OS_OH=1"
        cflags+=" -mfpu=neon"
        cflags+=" -mbranch-protection=pac-ret+b-key+bti"
        cflags_host="-m64"
        ARCH="aarch64"
    elif [[ "${TARGET_CPU}" = "x86_64" ]]; then
        argurment+=" -I${PREFIX}/../include/c++/v1"
        cflags="--target=x86_64-linux-ohos"
        cflags+=" --sysroot=${SYSROOT}"
        cflags+=" -isystem ${SYSROOT}/usr/include/x86_64-linux-ohos"
        cflags+=" -DV8_OS_OH=1"
        ARCH="x86_64"
    else
        die "not support target cpu"
    fi

    if [[ "${TARGET_CLANG_COVERAGE}" = "true" ]]; then
        cflags+=" --coverage"
    fi

    if [[ "$1" = 1 && "${IS_ASAN}" = "true" && "${USE_HWASEN}" = "true" ]]; then
        cflags+=" -shared-libasan"
        cflags+=" -fsanitize=hwaddress"
        cflags+=" -mllvm -hwasan-globals=0"
        cflags+=" -fno-emulated-tls"
        cflags+=" -fno-omit-frame-pointer"
    fi

    cflags+=" ${argurment}"

    # target env
    export CC="${CCACHE_EXEC} ${PREFIX}/clang ${INCLUDE_DIRS}"
    export CXX="${CCACHE_EXEC} ${PREFIX}/clang++ ${INCLUDE_DIRS}"
    export CMAKE_C_FLAGS="${cflags}"
    export CMAKE_CXX_FLAGS="${cflags}"
    export LD="${PREFIX}/ld.lld"
    export AS="${PREFIX}/llvm-as"
    export AR="${PREFIX}/llvm-ar"
    export STRIP="${PREFIX}/llvm-strip"
    export OBJCOPY="${PREFIX}/llvm-objcopy"
    export OBJDUMP="${PREFIX}/llvm-obidump"
    export RANLIB="${PREFIX}/llvm-ranlib"
    export NM="${PREFIX}/llvm-nm"
    export STRINGS="${PREFIX}/llvm-strings"
    export READELF="${PREFIX}/llvm-readelf"
    env > ${out_dir}/log.do_env
}
