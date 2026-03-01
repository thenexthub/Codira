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

#include "public.h"
#include "include/runtime_options.h"
#include "public_internal.h"
#include "runtime/include/mem/allocator.h"
#include "verification/config/config_load.h"
#include "verification/config/context/context.h"
#include "verification/cache/results_cache.h"
#include "verification/jobs/service.h"
#include "verification/jobs/job.h"

namespace ark::verifier {

Config *NewConfig()
{
    auto result = new Config;
    result->opts.Initialize();
    return result;
}

bool LoadConfigFile(Config *config, std::string_view configFileName)
{
    return ark::verifier::config::LoadConfig(config, configFileName);
}

void DestroyConfig(Config *config)
{
    config->opts.Destroy();
    delete config;
}

bool IsEnabled(Config const *config)
{
    ASSERT(config != nullptr);
    return config->opts.IsEnabled();
}

bool IsOnlyVerify(Config const *config)
{
    ASSERT(config != nullptr);
    return config->opts.IsOnlyVerify();
}

Service *CreateService(Config const *config, ark::mem::InternalAllocatorPtr allocator, ClassLinker *linker,
                       std::string const &cacheFileName)
{
    if (!cacheFileName.empty()) {
        VerificationResultCache::Initialize(cacheFileName);
    }
    auto res = allocator->New<Service>();
    if (res == nullptr) {
        LOG(ERROR, VERIFIER) << "Verifier Error: res is nullptr";
        return nullptr;
    }
    res->config = config;
    res->classLinker = linker;
    res->allocator = allocator;
    res->verifierService = allocator->New<VerifierService>(allocator, linker);
    if (res->verifierService == nullptr) {
        LOG(ERROR, VERIFIER) << "Verifier Error: res->verifierService is nullptr";
        return nullptr;
    }
    res->verifierService->Init();
    res->debugCtx.SetConfig(&config->debugCfg);
    return res;
}

void DestroyService(Service *service, bool updateCacheFile)
{
    if (service == nullptr) {
        return;
    }
    VerificationResultCache::Destroy(updateCacheFile);
    auto allocator = service->allocator;
    service->verifierService->Destroy();
    allocator->Delete(service->verifierService);
    allocator->Delete(service);
}

Config const *GetServiceConfig(Service const *service)
{
    return service->config;
}

// Do we really need this public/private distinction here?
inline Status ToPublic(VerificationResultCache::Status status)
{
    switch (status) {
        case VerificationResultCache::Status::OK:
            return Status::OK;
        case VerificationResultCache::Status::FAILED:
            return Status::FAILED;
        case VerificationResultCache::Status::UNKNOWN:
            return Status::UNKNOWN;
        default:
            UNREACHABLE();
    }
}

static void ReportStatus(Service const *service, Method const *method, std::string const &status)
{
    if (service->config->opts.show.status) {
        LOG(DEBUG, VERIFIER) << "Verification result for method " << method->GetFullName(true) << ": " << status;
    }
}

static bool VerifyClass(Class *clazz)
{
    auto *langPlugin = plugin::GetLanguagePlugin(clazz->GetSourceLang());
    auto result = langPlugin->CheckClass(clazz);
    if (!result.IsOk()) {
        LOG(ERROR, VERIFIER) << result.msg << " " << clazz->GetName();
        clazz->SetState(Class::State::ERRONEOUS);
        return false;
    }

    Class *parent = clazz->GetBase();
    if (parent != nullptr && parent->IsFinal()) {
        LOG(ERROR, VERIFIER) << "Cannot inherit from final class: " << clazz->GetName() << "->" << parent->GetName();
        clazz->SetState(Class::State::ERRONEOUS);
        return false;
    }

    clazz->SetState(Class::State::VERIFIED);
    return true;
}

static std::optional<Status> CheckBeforeVerification(Service *service, ark::Method *method, VerificationMode mode)
{
    using VStage = Method::VerificationStage;
    if (method->IsIntrinsic()) {
        return Status::OK;
    }

    /*
     *  Races are possible where the same method gets simultaneously verified on different threads.
     *  But those are harmless, so we ignore them.
     */
    auto stage = method->GetVerificationStage();
    if (stage == VStage::VERIFIED_OK) {
        return Status::OK;
    }
    if (stage == VStage::VERIFIED_FAIL) {
        return Status::FAILED;
    }

    if (!verifier::IsEnabled(mode)) {
        ReportStatus(service, method, "SKIP");
        return Status::OK;
    }
    if (method->GetInstructions() == nullptr) {
        LOG(DEBUG, VERIFIER) << method->GetFullName(true) << " has no code, no meaningful verification";
        return Status::OK;
    }
    service->debugCtx.AddMethod(*method, mode == VerificationMode::DEBUG);
    if (service->debugCtx.SkipVerification(method->GetUniqId())) {
        LOG(DEBUG, VERIFIER) << "Skipping verification of '" << method->GetFullName()
                             << "' according to verifier configuration";
        return Status::OK;
    }

    auto uniqId = method->GetUniqId();
    auto methodName = method->GetFullName();
    if (VerificationResultCache::Enabled()) {
        auto cachedStatus = ToPublic(VerificationResultCache::Check(uniqId));
        if (cachedStatus != Status::UNKNOWN) {
            LOG(DEBUG, VERIFIER) << "Verification result of method '" << methodName
                                 << "' was cached: " << (cachedStatus == Status::OK ? "OK" : "FAIL");
            return cachedStatus;
        }
    }

    return std::nullopt;
}

Status Verify(Service *service, ark::Method *method, VerificationMode mode)
{
    using VStage = Method::VerificationStage;
    ASSERT(service != nullptr);

    auto status = CheckBeforeVerification(service, method, mode);
    if (status) {
        return status.value();
    }

    auto uniqId = method->GetUniqId();
    auto methodName = method->GetFullName();

    auto lang = method->GetClass()->GetSourceLang();
    auto *processor = service->verifierService->GetProcessor(lang);

    if (processor == nullptr) {
        LOG(INFO, VERIFIER) << "Attempt to  verify " << method->GetFullName(true)
                            << "after service started shutdown, ignoring";
        return Status::FAILED;
    }

    auto const &methodOptions = service->config->opts.debug.GetMethodOptions();
    auto const &verifMethodOptions = methodOptions[methodName];
    LOG(DEBUG, VERIFIER) << "Verification config for '" << methodName << "': " << verifMethodOptions.GetName();
    LOG(INFO, VERIFIER) << "Started verification of method '" << method->GetFullName(true) << "'";

    // class verification can be called concurrently
    if ((method->GetClass()->GetState() < Class::State::VERIFIED) && !VerifyClass(method->GetClass())) {
        service->verifierService->ReleaseProcessor(processor);
        return Status::FAILED;
    }
    Job job {service, method, verifMethodOptions};
    bool result = job.DoChecks(processor->GetTypeSystem());

    LOG(DEBUG, VERIFIER) << "Verification result for '" << methodName << "': " << result;

    service->verifierService->ReleaseProcessor(processor);

    VerificationResultCache::CacheResult(uniqId, result);

    if (result) {
        method->SetVerificationStage(VStage::VERIFIED_OK);
        ReportStatus(service, method, "OK");
        return Status::OK;
    }
    method->SetVerificationStage(VStage::VERIFIED_FAIL);
    ReportStatus(service, method, "FAIL");
    return Status::FAILED;
}

bool IsEnabled(VerificationMode mode)
{
    return mode != VerificationMode::DISABLED;
}

VerificationMode VerificationModeFromString(const std::string &mode)
{
    if (mode == "on-the-fly") {
        return VerificationMode::ON_THE_FLY;
    }
    if (mode == "ahead-of-time") {
        return VerificationMode::AHEAD_OF_TIME;
    }
    if (mode == "debug") {
        return VerificationMode::DEBUG;
    }
    return VerificationMode::DISABLED;
}

}  // namespace ark::verifier
