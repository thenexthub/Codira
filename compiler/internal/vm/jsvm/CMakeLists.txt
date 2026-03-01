cmake_minimum_required(VERSION 3.28)

# 设置项目名称
project(JSVM)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加编译选项
add_compile_options(
  -fstack-protector-strong
  -msign-return-address=all
  -pthread
  -Wall
  -Wextra
  -Wno-unused-parameter
  -fPIC
  -Werror=unused-result
  -O3
  -fno-omit-frame-pointer
  -fno-rtti
  -fno-exceptions
  -fvisibility=hidden
)

if ((DEFINED TARGET_CPU) AND (${TARGET_CPU} MATCHES "arm"))
  add_compile_options(
    -mbranch-protection=pac-ret+b-key+bti
  )
endif()

# 设置宏定义
add_compile_definitions(
  -DENABLE_INSPECTOR
  -DHAVE_OPENSSL=1
  -DTARGET_OHOS
  # -DENABLE_HISYSEVENT
)

# 添加头文件
include_directories(jsvm
  "src"
  "interface/kits"
)

# TODO: samgr:samgr_proxy module, indirect dependencies cannot be passed
if (DEPENDENCY_TAG STREQUAL "default")
include_directories(jsvm
  "../../foundation/systemabilitymgr/samgr/interfaces/innerkits/samgr_proxy/include"
)
endif()

# 设置源文件
set (jsvm_sources
  "src/js_native_api_v8.cpp"
  "src/jsvm_env.cpp"
  "src/jsvm_reference.cpp"
)

set (jsvm_inspector_sources
  "src/inspector/inspector_socket.cpp"
  "src/inspector/inspector_socket_server.cpp"
  "src/inspector/inspector_utils.cpp"
  "src/inspector/js_native_api_v8_inspector.cpp"
)

set (platform_sources
  "src/platform/platform_ohos.cpp"
)

# 添加动态库搜索路径
set (LIB_DIRS
  "${LIB_ROOT_DIR}/obj/arkcompiler/jsvm"
  "${LIB_ROOT_DIR}/obj/third_party/zlib"
  "${LIB_ROOT_DIR}/hiviewdfx/hilog_override"
  "${LIB_ROOT_DIR}/hiviewdfx/hilog"
  "${LIB_ROOT_DIR}/resourceschedule/resource_schedule_service"
  "${LIB_ROOT_DIR}/startup/init"
  "${LIB_ROOT_DIR}/hiviewdfx/hitrace/"
  "${LIB_ROOT_DIR}/thirdparty/libuv/"
  "${LIB_ROOT_DIR}/thirdparty/icu/"
  "${LIB_ROOT_DIR}/thirdparty/openssl/"
  "${LIB_ROOT_DIR}/obj/vendor/${DEPENDENCY_TAG}/foundation/bundlemanager/zlib_override/"
)
set(CMAKE_SKIP_RPATH TRUE)
link_directories(${LIB_DIRS})

# 添加目标
add_library(jsvm
  SHARED
  ${platform_sources}
  ${jsvm_sources}
  ${jsvm_inspector_sources}
)

target_link_options(jsvm
  PRIVATE
  "-Wl,--exclude-libs=libc++_static.a"
)

if (NOT DEPENDENCY_TAG STREQUAL "default")
if ((DEFINED USE_LIBZ_INNER) AND USE_LIBZ_INNER)
target_link_libraries(jsvm
  libz_inner.a
  libllhttp.a
  libressched_client.z.so
  libbegetutil.z.so
  libhitrace_meter.so
  libuv.so
  libhmicuuc.z.so
  libhmicui18n.z.so
  libcrypto_openssl.z.so
  libssl_openssl.z.so
  libv8_shared.so
  libhilog.so
  c++_static
)
else()
target_link_libraries(jsvm
  libz.a
  libllhttp.a
  libressched_client.z.so
  libbegetutil.z.so
  libhitrace_meter.so
  libuv.so
  libhmicuuc.z.so
  libhmicui18n.z.so
  libcrypto_openssl.z.so
  libssl_openssl.z.so
  libv8_shared.so
  libhilog.so
  c++_static
)
endif()
endif()
