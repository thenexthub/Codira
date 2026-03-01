/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "plugins/ets/runtime/ets_native_library_provider.h"

#include "ets_native_library.h"
#include "include/mem/panda_containers.h"
#include "include/mem/panda_string.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_namespace_manager_impl.h"

namespace ark::ets {

namespace {

Expected<EtsNativeLibrary, os::Error> LoadFromPath(const PandaVector<PandaString> &pathes, const PandaString &name)
{
    if (name.find('/') == std::string::npos) {
        for (const auto &path : pathes) {
            if (path.empty()) {
                continue;
            }
            PandaStringStream s;
            s << path << '/' << name;
            if (auto loadRes = EtsNativeLibrary::Load(s.str())) {
                return loadRes;
            }
        }
    }
    return EtsNativeLibrary::Load(name);
}

Expected<EtsNativeLibrary, os::Error> LoadNativeLibraryFromNamespace(const char *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    std::string abcPath;
    for (auto stack = StackWalker::Create(coroutine); stack.HasFrame(); stack.NextFrame()) {
        auto *method = stack.GetMethod();
        if (LIKELY(method != nullptr)) {
            if (method->GetPandaFile() == nullptr) {
                continue;
            }
            auto *ctx = method->GetClass()->GetLoadContext();
            ASSERT(ctx != nullptr);
            LOG(INFO, RUNTIME) << "NativeLibraryProvider::LoadNativeLibraryFromNamespace: ctx:"
                               << reinterpret_cast<uint64_t>(ctx)
                               << ", isBootContext:" << ((ctx->IsBootContext()) ? "true" : "false");
            if (!ctx->IsBootContext()) {
                abcPath = method->GetPandaFile()->GetFullFileName();
                break;
            }
        }
    }
    EtsNamespaceManagerImpl &instance = EtsNamespaceManagerImpl::GetInstance();
    return instance.LoadNativeLibraryFromNs(abcPath, name);
}
}  // namespace

std::optional<std::string> NativeLibraryProvider::LoadLibrary(ani_env *env, const PandaString &name,
                                                              bool shouldVerifyPermission, const PandaString &fileName)
{
    ASSERT(env != nullptr);
    if (shouldVerifyPermission && !CheckLibraryPermission(env, fileName)) {
        return "NativeLibraryProvider::CheckLibraryPermission failed";
    }
    {
        os::memory::ReadLockHolder lock(lock_);
        auto it =
            std::find_if(libraries_.begin(), libraries_.end(), [&name](auto &lib) { return lib.GetName() == name; });
        if (it != libraries_.end()) {
            return {};
        }
    }
    auto loadRes = LoadFromPath(GetLibraryPath(), name);
    if (!shouldVerifyPermission && !loadRes) {
        LOG(WARNING, RUNTIME) << "Failed to load System library: " << loadRes.Error().ToString()
                              << "; Attempting to load application library instead.";
        loadRes = LoadNativeLibraryFromNamespace(name.c_str());
    }
    if (!loadRes) {
        return loadRes.Error().ToString();
    }
    const EtsNativeLibrary *lib = nullptr;
    {
        os::memory::WriteLockHolder lock(lock_);
        auto [it, inserted] = libraries_.emplace(std::move(loadRes.Value()));
        if (!inserted) {
            return {};
        }
        lib = &*it;
    }
    ASSERT(lib != nullptr);
    return CallAniCtor(env, lib);
}

std::optional<std::string> NativeLibraryProvider::CallAniCtor(ani_env *env, const EtsNativeLibrary *lib)
{
    if (auto res = lib->FindSymbol("ANI_Constructor")) {
        using AniCtor = ani_status (*)(ani_vm *, uint32_t *);
        auto ctor = reinterpret_cast<AniCtor>(res.Value());
        uint32_t version;
        ani_vm *vm = PandaEnv::FromAniEnv(env)->GetEtsVM();
        ani_status status = ctor(vm, &version);
        if (status != ANI_OK) {
            return "ANI_Constructor returns an error: " + std::to_string(status);
        }
        if (!ani::IsVersionSupported(version)) {
            return "Unsupported ANI version: " + std::to_string(version);
        }
    } else {
        std::string message = std::string(lib->GetName()) + " doesn't contain ANI_Constructor";
        LOG(ERROR, ANI) << message;
        return message;
    }
    return {};
}

std::optional<std::string> NativeLibraryProvider::GetCallerClassName(ani_env *env)
{
    auto coroutine = PandaEnv::FromAniEnv(env)->GetEtsCoroutine();
    if (coroutine == nullptr) {
        LOG(ERROR, RUNTIME) << "Coroutine is null, failed to get class name.";
        return std::nullopt;
    }
    auto stack = StackWalker::Create(coroutine);
    if (!stack.HasFrame()) {
        LOG(ERROR, RUNTIME) << "No valid method in stack, failed to determine class.";
        return std::nullopt;
    }
    auto *method = stack.GetMethod();
    if (method == nullptr) {
        LOG(ERROR, RUNTIME) << "Method is null, failed to get class name.";
        return std::nullopt;
    }
    auto *cls = method->GetClass();
    auto *ctx = cls->GetLoadContext();
    if (ctx == nullptr || !ctx->IsBootContext()) {
        LOG(ERROR, RUNTIME) << "load context is not a boot context.";
        return std::nullopt;
    }
    return cls->GetName();
}

bool NativeLibraryProvider::CheckLibraryPermission([[maybe_unused]] ani_env *env,
                                                   [[maybe_unused]] const PandaString &fileName)
{
#if defined(PANDA_TARGET_OHOS)
    auto classNameOpt = GetCallerClassName(env);
    if (!classNameOpt.has_value()) {
        LOG(ERROR, RUNTIME) << "Failed to retrieve class name during permission check.";
        return false;
    }
    auto className = classNameOpt.value();
    auto &instance = EtsNamespaceManagerImpl::GetInstance();
    const auto cb = instance.GetExtensionApiCheckCallback();
    if (cb == nullptr) {
        LOG(DEBUG, RUNTIME) << "ExtensionApiCheckCallback is not registered";
        return true;
    }
    if (!cb(className, fileName.c_str())) {
        LOG(ERROR, RUNTIME) << "CheckLibraryPermission failed: class name: " << className
                            << " is not in the API allowed list, loading prohibited";
        return false;
    }
    LOG(INFO, RUNTIME) << "NativeLibraryProvider::CheckLibraryPermission: class name detection passed";
#endif
    return true;
}

void *NativeLibraryProvider::ResolveSymbol(const PandaString &name) const
{
    os::memory::ReadLockHolder lock(lock_);

    for (auto &lib : libraries_) {
        if (auto ptr = lib.FindSymbol(name)) {
            return ptr.Value();
        }
    }

    return nullptr;
}

PandaVector<PandaString> NativeLibraryProvider::GetLibraryPath() const
{
    os::memory::ReadLockHolder lock(lock_);
    return libraryPath_;
}

void NativeLibraryProvider::SetLibraryPath(const PandaVector<PandaString> &pathes)
{
    os::memory::WriteLockHolder lock(lock_);
    libraryPath_ = pathes;
}

void NativeLibraryProvider::AddLibraryPath(const PandaString &path)
{
    os::memory::WriteLockHolder lock(lock_);
    libraryPath_.emplace_back(path);
}

}  // namespace ark::ets
