/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <cmath>
#include <thread>
#include <set>

#include "libarkbase/macros.h"
#include "libarkbase/os/exec.h"
#include "libarkbase/os/filesystem.h"
#include "libarkbase/os/kill.h"
#include "libarkbase/os/pipe.h"
#include "libarkbase/os/system_environment.h"
#include "libarkbase/os/time.h"
#include "plugins/ets/stdlib/native/core/Process.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

namespace ark::ets::stdlib {

enum Signals : uint32_t { SIG_INT = 2, SIG_QUIT = 3, SIG_KILL = 9, SIG_TERM = 15 };

#ifdef PANDA_TARGET_OHOS
// constexpr variables used by isAppUid & isIsolatedProcess, refered to arkts 1.0
// See: https://gitee.com/openharmony/js_sys_module/blob/master/process/js_process.cpp#L38
constexpr int PER_USER_RANGE = 100000;
// See: https://gitcode.com/openharmony/commonlibrary_ets_utils/blob/master/js_sys_module/process/js_process.cpp#L31
constexpr std::pair<int, int> APP_UID_RANGE = {10000, 65535};
// Only isolateuid numbers between 99000 and 99999.
// See: https://gitee.com/openharmony/js_sys_module/blob/master/process/js_process.cpp#L290
constexpr std::pair<int, int> ISOLATE_UID_RANGE = {99000, 99999};
// Only appuid numbers between 9000 and 98999.
// See: https://gitee.com/openharmony/js_sys_module/blob/master/process/js_process.cpp#L291
constexpr std::pair<int, int> GENERAL_UID_RANGE = {9000, 98999};
#endif

static auto GetPipeHandler(int stdOutFdIn, int stdOutFdOut, int stdErrFdIn, int stdErrFdOut)
{
    auto handlePipes = [stdOutFdIn, stdOutFdOut, stdErrFdIn, stdErrFdOut] {
        std::pair<ark::os::unique_fd::UniqueFd, ark::os::unique_fd::UniqueFd> out {stdOutFdIn, stdOutFdOut};
        std::pair<ark::os::unique_fd::UniqueFd, ark::os::unique_fd::UniqueFd> err {stdErrFdIn, stdErrFdOut};
        out.first.Reset();
        err.first.Reset();
        ark::os::unique_fd::UniqueFd defaultStdOut {1};
        ark::os::unique_fd::UniqueFd defaultStdErr {2};
        os::Dup2(out.second, defaultStdOut);
        os::Dup2(err.second, defaultStdErr);
        out.second.Release();
        err.second.Release();
        defaultStdOut.Release();
        defaultStdErr.Release();
    };
    return handlePipes;
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::Mutex g_terminatedChildSetLock {};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::set<ani_int> g_terminatedChildSet GUARDED_BY(g_terminatedChildSetLock) {};

static void SpawnChildProcess(ani_env *env, ani_object child, ani_string cmd, ani_int timeout, ani_int signal)
{
    auto stdOutFd = os::CreatePipe();
    if (!stdOutFd.first.IsValid()) {
        ThrowNewError(env, "std.core.RuntimeError", "Failed to create a child process", "C{std.core.String}:");
        return;
    }
    auto stdErrFd = os::CreatePipe();
    if (!stdErrFd.first.IsValid()) {
        ThrowNewError(env, "std.core.RuntimeError", "Failed to create a child process", "C{std.core.String}:");
        return;
    }

    auto pipeHandler =
        GetPipeHandler(stdOutFd.first.Get(), stdOutFd.second.Get(), stdErrFd.first.Get(), stdErrFd.second.Get());

    auto cmdString = ConvertFromAniString(env, cmd);
    auto result = ark::os::exec::ExecWithCallbackNoWait(pipeHandler, "/bin/sh", "-c", cmdString.c_str());

    stdOutFd.second.Reset();
    stdErrFd.second.Reset();

    if (result.HasValue()) {
        ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "pid", result.Value()));
    } else {
        stdOutFd.first.Reset();
        stdErrFd.first.Reset();
        ThrowNewError(env, "std.core.RuntimeError", "Failed to create a child process", "C{std.core.String}:");
        return;
    }

    ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "ppid", ark::os::thread::GetPid()));
    ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "outFd", stdOutFd.first.Release()));
    ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "errorFd", stdErrFd.first.Release()));

    if (timeout == 0) {
        return;
    }

    ani_int pidToTerminate;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "pid", &pidToTerminate));

    auto terminator = [timeout, signal, pidToTerminate] {
        ark::os::thread::NativeSleep(timeout);
        os::memory::LockHolder lock {g_terminatedChildSetLock};
        if (g_terminatedChildSet.count(pidToTerminate) == 0 && kill(pidToTerminate, signal) == 0) {
            auto signalType = std::array {SIG_INT, SIG_QUIT, SIG_KILL, SIG_TERM};
            if (std::find(signalType.begin(), signalType.end(), signal) != signalType.end()) {
                g_terminatedChildSet.insert(pidToTerminate);
            }
        }
    };

    std::thread timeoutListener {terminator};
    timeoutListener.detach();
}

static void ReadFinalizer(ani_env *env, ani_object child, bool isStdErr)
{
    bool terminated = false;

    ani_int exitCode;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "exitCode", &exitCode));

    if (exitCode > -1) {
        terminated = true;
    } else {
        os::memory::LockHolder lock {g_terminatedChildSetLock};
        ani_int pid;
        ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "pid", &pid));
        terminated = g_terminatedChildSet.count(pid) != 0U;
    }

    if (terminated) {
        std::string fdName = isStdErr ? "errorFd" : "outFd";
        ani_int fd;
        ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, fdName.data(), &fd));
        ark::os::unique_fd::UniqueFd out {fd};
        out.Reset();
        ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, fdName.data(), -1));
    }
}

static void ReadChildProcessStdOut(ani_env *env, ani_object child)
{
    ani_int fd;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "outFd", &fd));
    if (fd == -1) {
        return;
    }

    ani_ref bufferObject;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(child, "outBuffer", &bufferObject));

    ani_ref outBuffer;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(bufferObject), "data", &outBuffer));

    while (true) {
        ani_int outBytesRead;
        ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "outBytesRead", &outBytesRead));

        ani_int outBufferLength;
        ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(reinterpret_cast<ani_object>(bufferObject), "_byteLength",
                                                          &outBufferLength));

        if (outBytesRead >= outBufferLength) {
            break;
        }

        constexpr int MAX_SIZE = 1024;
        std::array<int8_t, MAX_SIZE> buffer {};
        int bytesRead = read(fd, buffer.data(), sizeof(buffer.size()) - 1);
        if (bytesRead == 0) {
            ReadFinalizer(env, child, false);
            break;
        }

        if (bytesRead + outBytesRead > outBufferLength) {
            bytesRead = outBufferLength - outBytesRead;
            ark::os::unique_fd::UniqueFd out {fd};
            out.Reset();
            ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "outFd", -1));
        }

        ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Byte(reinterpret_cast<ani_fixedarray_byte>(outBuffer),
                                                          outBytesRead, bytesRead, buffer.data()));
        ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "outBytesRead", outBytesRead + bytesRead));
    }
}

static void ReadChildProcessStdErr(ani_env *env, ani_object child)
{
    ani_int fd;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "errorFd", &fd));
    if (fd == -1) {
        return;
    }

    ani_ref bufferObject;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(child, "errBuffer", &bufferObject));

    ani_ref errBuffer;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(bufferObject), "data", &errBuffer));

    ani_type childClassType;
    ANI_FATAL_IF_ERROR(env->Object_GetType(child, &childClassType));

    ani_field errBytesReadId;
    ANI_FATAL_IF_ERROR(env->Class_FindField(static_cast<ani_class>(childClassType), "errBytesRead", &errBytesReadId));

    while (true) {
        ani_int errBytesRead;
        ANI_FATAL_IF_ERROR(env->Object_GetField_Int(child, errBytesReadId, &errBytesRead));

        ani_int errBufferLength;
        ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(reinterpret_cast<ani_object>(bufferObject), "_byteLength",
                                                          &errBufferLength));

        if (errBytesRead >= errBufferLength) {
            break;
        }

        constexpr int MAX_SIZE = 1024;
        std::array<int8_t, MAX_SIZE> buffer {};
        int bytesRead = read(fd, buffer.data(), sizeof(buffer.size()) - 1);
        if (bytesRead == 0) {
            ReadFinalizer(env, child, true);
            break;
        }

        if (bytesRead + errBytesRead > errBufferLength) {
            bytesRead = errBufferLength - errBytesRead;

            ark::os::unique_fd::UniqueFd out {fd};
            out.Reset();
            ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Int(child, "errorFd", -1));
        }

        ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Byte(reinterpret_cast<ani_fixedarray_byte>(errBuffer),
                                                          errBytesRead, bytesRead, buffer.data()));
        ANI_FATAL_IF_ERROR(env->Object_SetField_Int(child, errBytesReadId, errBytesRead + bytesRead));
    }
}

static void WaitChildProcess(ani_env *env, ani_object child)
{
    ani_int pid;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "pid", &pid));

    ani_type childClassType;
    ANI_FATAL_IF_ERROR(env->Object_GetType(child, &childClassType));

    ani_field exitCodeId;
    ANI_FATAL_IF_ERROR(env->Class_FindField(static_cast<ani_class>(childClassType), "exitCode", &exitCodeId));

    ani_int exitCode;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Int(child, exitCodeId, &exitCode));

    if (exitCode < 0) {
        auto result = ark::os::exec::Wait(pid, false);
        if (result.HasValue()) {
            ANI_FATAL_IF_ERROR(env->Object_SetField_Int(child, exitCodeId, result.Value()));
        } else {
            ThrowNewError(env, "std.core.RuntimeError", "Wait failed", "C{std.core.String}:");
            return;
        }
    }

    ReadChildProcessStdOut(env, child);
    ReadChildProcessStdErr(env, child);

    {
        os::memory::LockHolder lock {g_terminatedChildSetLock};
        g_terminatedChildSet.erase(pid);
    }
}

static void KillChildProcess(ani_env *env, ani_object child, ani_int signal)
{
    auto signalType = std::array {SIG_INT, SIG_QUIT, SIG_KILL, SIG_TERM};

    auto intSignal = static_cast<int>(signal);

    ani_int pid;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "pid", &pid));

    if (ark::os::kill_process::Kill(pid, intSignal) == 0) {
        if (std::find(signalType.begin(), signalType.end(), intSignal) != signalType.end()) {
            os::memory::LockHolder lock {g_terminatedChildSetLock};
            g_terminatedChildSet.insert(pid);
        }
        ANI_FATAL_IF_ERROR(env->Object_SetFieldByName_Boolean(child, "killed", 1U));
        return;
    }

    ThrowNewError(env, "std.core.RuntimeError", "Kill failed", "C{std.core.String}:");
}

static void CloseChildProcess(ani_env *env, ani_object child)
{
    ani_type childClassType;
    ANI_FATAL_IF_ERROR(env->Object_GetType(child, &childClassType));

    ani_field exitCodeId;
    ANI_FATAL_IF_ERROR(env->Class_FindField(static_cast<ani_class>(childClassType), "exitCode", &exitCodeId));

    ani_int pid;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Int(child, "pid", &pid));

    ani_int exitCode;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Int(child, exitCodeId, &exitCode));

    if (exitCode > -1) {
        return;
    }

    {
        os::memory::LockHolder lock {g_terminatedChildSetLock};

        if (g_terminatedChildSet.count(pid) != 0U) {
            return;
        }
    }

    auto status = ark::os::kill_process::Close(pid);
    if (LIKELY(status != -1)) {
        ANI_FATAL_IF_ERROR(env->Object_SetField_Int(child, exitCodeId, status));
        return;
    }

    ThrowNewError(env, "std.core.RuntimeError", "Close failed", "C{std.core.String}:");
}

static ani_boolean PManagerIsAppUid([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object process,
                                    [[maybe_unused]] ani_int uid)
{
#ifdef PANDA_TARGET_OHOS
    // refer to https://gitee.com/openharmony/js_sys_module/blob/master/process/js_process.cpp#L300
    int number = static_cast<int>(uid);
    if (number > 0) {
        const auto appId = number % PER_USER_RANGE;
        return (appId >= APP_UID_RANGE.first && appId <= APP_UID_RANGE.second) ? ANI_TRUE : ANI_FALSE;
    }

    return ANI_FALSE;
#else
    ThrowNewError(env, "std.core.RuntimeError", "not implemented for Non-OHOS target", "C{std.core.String}:");
    return ANI_FALSE;
#endif
}

static ani_int PManagerGetUidForName(ani_env *env, [[maybe_unused]] ani_object process, ani_string name)
{
    auto str = ConvertFromAniString(env, name);
    auto result = ark::os::system_environment::GetUidForName(str);
    return result;
}

static ani_int PManagerGetThreadPriority([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object process,
                                         ani_int tid)
{
    return ark::os::thread::GetPriority(tid);
}

static ani_long PManagerGetSystemConfig([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object process,
                                        ani_int name)
{
    return ark::os::system_environment::GetSystemConfig(name);
}

static ani_string PManagerGetEnvironmentVar(ani_env *env, [[maybe_unused]] ani_object process, ani_string name)
{
    auto str = ConvertFromAniString(env, name);
    auto envVar = ark::os::system_environment::GetEnvironmentVar(str.c_str());
    return CreateUtf8String(env, envVar.data(), envVar.size());
}

static void PManagerExit([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object process, ani_int code)
{
    PROCESS_EXIT(code);
}

static ani_boolean PManagerKill(ani_env *env, [[maybe_unused]] ani_object process, ani_int signal, ani_int pid)
{
    int integerPid = static_cast<int>(pid);
    auto ownPid = ark::os::thread::GetPid();
    if (integerPid == 0 || integerPid == -1 || integerPid == ownPid || integerPid == -ownPid) {
        ThrowNewError(env, "std.core.IllegalArgumentError", "Invalid pid argument", "C{std.core.String}:");
        return 0U;
    }

    constexpr int MIN_SIGNAL_VALUE = 1;
    constexpr int MAX_SINAL_VALUE = 64;

    if (std::trunc(signal) != signal || signal < MIN_SIGNAL_VALUE || signal > MAX_SINAL_VALUE) {
        ThrowNewError(env, "std.core.IllegalArgumentError", "Invalid signal argument", "C{std.core.String}:");
        return 0U;
    }

    return static_cast<ani_boolean>(ark::os::kill_process::Kill(integerPid, signal) == 0);
}

static ani_int GetTid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetCurrentThreadId();
}

static ani_boolean IsIsolatedProcImpl([[maybe_unused]] ani_env *env)
{
#ifdef PANDA_TARGET_OHOS
    // refer to https://gitee.com/openharmony/js_sys_module/blob/master/process/js_process.cpp#L284
    auto uid = ark::os::thread::GetUid();
    uid %= PER_USER_RANGE;
    return ((uid >= ISOLATE_UID_RANGE.first && uid <= ISOLATE_UID_RANGE.second) ||
            (uid >= GENERAL_UID_RANGE.first && uid <= GENERAL_UID_RANGE.second))
               ? ANI_TRUE
               : ANI_FALSE;
#else
    ThrowNewError(env, "std.core.RuntimeError", "not implemented for Non-OHOS target", "C{std.core.String}:");
    return ANI_FALSE;
#endif
}

static ani_int GetPid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetPid();
}

static ani_int GetPPid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetPPid();
}

static ani_int GetUid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetUid();
}

static ani_int GetEuid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetEuid();
}

static ani_int GetGid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetGid();
}

static ani_int GetEgid([[maybe_unused]] ani_env *env)
{
    return ark::os::thread::GetEgid();
}

static ani_array GetGroupIDs(ani_env *env)
{
    auto groups = ark::os::thread::GetGroups();
    auto groupIds = std::vector<ani_int>(groups.begin(), groups.end());

    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ani_array result;
    ANI_FATAL_IF_ERROR(env->Array_New(groups.size(), undefined, &result));

    if (groups.empty()) {
        ThrowNewError(env, "std.core.RuntimeError", "Failed to get process groups", "C{std.core.String}:");
        return result;
    }

    ani_class intClass {};
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Int", &intClass));

    ani_method intCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(intClass, "<ctor>", "i:", &intCtor));

    for (size_t i = 0; i < groups.size(); ++i) {
        ani_object boxedInt {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ANI_FATAL_IF_ERROR(env->Object_New(intClass, intCtor, &boxedInt, groupIds[i]));
        ANI_FATAL_IF_ERROR(env->Array_Set(result, i, boxedInt));
    }

    return result;
}

static ani_boolean Is64BitProcess([[maybe_unused]] ani_env *env)
{
    constexpr int SIZE_OF_64_BIT_PTR = 8;
    return static_cast<ani_boolean>(sizeof(char *) == SIZE_OF_64_BIT_PTR);
}

static ani_long GetProcessStartRealTime([[maybe_unused]] ani_env *env)
{
    return ark::os::time::GetStartRealTime<std::chrono::milliseconds>();
}

static ani_long GetProcessPastCpuTime([[maybe_unused]] ani_env *env)
{
    return ark::os::time::GetClockTime<std::chrono::milliseconds>(CLOCK_PROCESS_CPUTIME_ID);
}

static void AbortProcess([[maybe_unused]] ani_env *env)
{
    std::abort();  // CC-OFF(G.STD.16-CPP) solid logic
}

static ani_string GetCurrentWorkingDirectory(ani_env *env)
{
    auto workDir = ark::os::GetCurrentWorkingDirectory();
    return CreateUtf8String(env, workDir.data(), workDir.size());
}

static void ChangeCurrentWorkingDirectory(ani_env *env, ani_string path)
{
    auto str = ConvertFromAniString(env, path);
    os::ChangeCurrentWorkingDirectory(str);
}

static ani_long GetSystemUptime([[maybe_unused]] ani_env *env)
{
#ifdef PANDA_TARGET_MACOS
    return ark::os::time::GetClockTime<std::chrono::milliseconds>(CLOCK_MONOTONIC_RAW);
#else
    return ark::os::time::GetClockTime<std::chrono::milliseconds>(CLOCK_BOOTTIME);
#endif
}

void RegisterProcessNativeMethods(ani_env *env)
{
    const auto childProcessImpls =
        std::array {ani_native_function {"readOutput", ":", reinterpret_cast<void *>(ReadChildProcessStdOut)},
                    ani_native_function {"readErrorOutput", ":", reinterpret_cast<void *>(ReadChildProcessStdErr)},
                    ani_native_function {"close", ":", reinterpret_cast<void *>(CloseChildProcess)},
                    ani_native_function {"killImpl", "i:", reinterpret_cast<void *>(KillChildProcess)},
                    ani_native_function {"spawn", "C{std.core.String}ii:", reinterpret_cast<void *>(SpawnChildProcess)},
                    ani_native_function {"waitImpl", ":d", reinterpret_cast<void *>(WaitChildProcess)}};

    const auto processManagerImpls = std::array {
        ani_native_function {"isAppUid", "i:z", reinterpret_cast<void *>(PManagerIsAppUid)},
        ani_native_function {"getUidForName", "C{std.core.String}:i", reinterpret_cast<void *>(PManagerGetUidForName)},
        ani_native_function {"getThreadPriority", "i:i", reinterpret_cast<void *>(PManagerGetThreadPriority)},
        ani_native_function {"getSystemConfig", "i:l", reinterpret_cast<void *>(PManagerGetSystemConfig)},
        ani_native_function {"getEnvironmentVar", "C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(PManagerGetEnvironmentVar)},
        ani_native_function {"exit", "i:", reinterpret_cast<void *>(PManagerExit)},
        ani_native_function {"kill", "ii:z", reinterpret_cast<void *>(PManagerKill)},
    };

    const auto processImpls = std::array {
        ani_native_function {"tid", ":i", reinterpret_cast<void *>(GetTid)},
        ani_native_function {"pid", ":i", reinterpret_cast<void *>(GetPid)},
        ani_native_function {"ppid", ":i", reinterpret_cast<void *>(GetPPid)},
        ani_native_function {"uid", ":i", reinterpret_cast<void *>(GetUid)},
        ani_native_function {"euid", ":i", reinterpret_cast<void *>(GetEuid)},
        ani_native_function {"gid", ":i", reinterpret_cast<void *>(GetGid)},
        ani_native_function {"egid", ":i", reinterpret_cast<void *>(GetEgid)},
        ani_native_function {"groups", ":C{std.core.Array}", reinterpret_cast<void *>(GetGroupIDs)},
        ani_native_function {"is64Bit", ":z", reinterpret_cast<void *>(Is64BitProcess)},
        ani_native_function {"getStartRealtime", ":l", reinterpret_cast<void *>(GetProcessStartRealTime)},
        ani_native_function {"getPastCpuTime", ":l", reinterpret_cast<void *>(GetProcessPastCpuTime)},
        ani_native_function {"abort", ":", reinterpret_cast<void *>(AbortProcess)},
        ani_native_function {"cwd", ":C{std.core.String}", reinterpret_cast<void *>(GetCurrentWorkingDirectory)},
        ani_native_function {"chdir", "C{std.core.String}:", reinterpret_cast<void *>(ChangeCurrentWorkingDirectory)},
        ani_native_function {"uptime", ":l", reinterpret_cast<void *>(GetSystemUptime)},
        ani_native_function {"isIsolatedProcess", ":z", reinterpret_cast<void *>(IsIsolatedProcImpl)},
    };

    ani_class childProcessKlass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.StdProcess.ChildProcess", &childProcessKlass));
    ANI_FATAL_IF_ERROR(
        env->Class_BindNativeMethods(childProcessKlass, childProcessImpls.data(), childProcessImpls.size()));

    ani_class processManagerKlass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.StdProcess.ProcessManager", &processManagerKlass));
    ANI_FATAL_IF_ERROR(
        env->Class_BindNativeMethods(processManagerKlass, processManagerImpls.data(), processManagerImpls.size()));

    ani_namespace ns {};
    ANI_FATAL_IF_ERROR(env->FindNamespace("std.core.StdProcess", &ns));

    ANI_FATAL_IF_ERROR(env->Namespace_BindNativeFunctions(ns, processImpls.data(), processImpls.size()));
}

}  // namespace ark::ets::stdlib
