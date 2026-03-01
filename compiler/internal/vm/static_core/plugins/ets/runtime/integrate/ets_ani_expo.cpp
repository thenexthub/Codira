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
#include "plugins/ets/runtime/integrate/ets_ani_expo.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/file_manager.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {
PANDA_PUBLIC_API void ETSAni::Prefork(ani_env *env, [[maybe_unused]] void *napienv)
{
    PandaEtsVM *vm = PandaEnv::FromAniEnv(env)->GetEtsVM();
    ProcessTaskpoolWorker(true);
    vm->PreZygoteFork();
#ifdef PANDA_ETS_INTEROP_JS
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    if (!interop::js::CreateMainInteropContext(coroutine, napienv)) {
        LOG(ERROR, RUNTIME) << "Cannot create interop context";
    }
#endif
}

PANDA_PUBLIC_API void ETSAni::Postfork(ani_env *env, const std::vector<ani_option> &options, bool postZygoteFork)
{
    bool loadAppAotFile = false;
    std::string appAotFileName;
    for (auto &opt : options) {
        std::string option(opt.option);
        if (option.rfind(AOT_FILE_OPTION_PREFIX, 0) == 0) {
            loadAppAotFile = true;
            appAotFileName = option.substr(AOT_FILE_OPTION_PREFIX.size());
        } else if (option == ENABLE_AN_OPTION) {
            TryLoadAotFileForBoot();
        } else {
            LOG(ERROR, RUNTIME) << "Unprocessed or invalid option parameter." << option;
        }
    }

    PandaEtsVM *vm = PandaEnv::FromAniEnv(env)->GetEtsVM();
    if (vm->GetLanguageContext().IsEnabledCHA()) {
        vm->GetRuntime()->GetClassLinker()->GetAotManager()->VerifyClassHierarchy();
    }

    if (loadAppAotFile) {
        LoadAotFileForApp(appAotFileName);
    }

    if (postZygoteFork) {
        vm->PostZygoteFork();
        ProcessTaskpoolWorker(false);
    }
}

void ETSAni::LoadAotFileForApp(std::string const &aotFileName)
{
    if (AotEscapeSignalHandler::IsEscapeSignalFlagExists()) {
        LOG(WARNING, AOT) << "LoadAotFileForApp: AOT mode has escaped and roll back to interpreter mode";
        return;
    }
    auto res = FileManager::LoadAnFile(aotFileName, true);
    if (!res) {
        LOG(ERROR, AOT) << "Failed to load AOT file: " << res.Error();
        return;
    }
    if (!res.Value()) {
        LOG(ERROR, AOT) << "Failed to load " << aotFileName << "with unkown reason";
        return;
    }
}

void ETSAni::TryLoadAotFileForBoot()
{
    if (AotEscapeSignalHandler::IsEscapeSignalFlagExists()) {
        LOG(WARNING, AOT) << "TryLoadAotFileForBoot: AOT mode has escaped and roll back to interpreter mode";
        return;
    }
    Runtime *runtime = Runtime::GetCurrent();
    ClassLinker *linker = runtime->GetClassLinker();
    const PandaVector<const panda_file::File *> bootPandaFiles = linker->GetBootPandaFiles();
    for (const auto *pf : bootPandaFiles) {
        const std::string location = pf->GetFilename();
        FileManager::TryLoadAnFileForLocation(location);
        const compiler::AotPandaFile *aotFile = linker->GetAotManager()->FindPandaFile(location);
        if (aotFile != nullptr) {
            pf->SetClassHashTable(aotFile->GetClassHashTable());
            // Issue 29288, temporary solution, need to support preload `an` files in hybridspawn nextly
            if (location == ETSSTDLIB_ABC) {
                linker->TryReLinkAotCodeForBoot(pf, aotFile, panda_file::SourceLang::ETS);
            }
        }
    }
}

void ETSAni::ProcessTaskpoolWorker(bool preFork)
{
    auto &taskpoolMode =
        Runtime::GetCurrent()->GetOptions().GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));

    const char *taskpoolEAWorkerMode = "eaworker";
    if (taskpoolMode != taskpoolEAWorkerMode) {
        return;
    }

    if (preFork) {
        bool res = DestroyExclusiveWorkerForTaskpoolIfExists();
        LOG(INFO, COROUTINES) << "DestroyExclusiveWorkerForTaskpoolIfExists done: " << res;
    } else {
        bool res = PreCreateExclusiveWorkerForTaskpool();
        LOG(INFO, COROUTINES) << "PreCreateExclusiveWorkerForTaskpool done: " << res;
    }
}

bool ETSAni::PreCreateExclusiveWorkerForTaskpool()
{
    auto *runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    ClassLinkerExtension *ext = classLinker->GetExtension(SourceLanguage::ETS);
    auto mutf8Name = reinterpret_cast<const uint8_t *>("Lstd/concurrency/taskpool;");
    auto *klass = ext->GetClass(mutf8Name);
    if (klass == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool failed in post zygote fork";
        return false;
    }
    auto method = ets::EtsClass::FromRuntimeClass(klass)->GetStaticMethod("initWorkerPool", ":V");
    if (method == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool initWorkerPool failed in post zygote fork";
        return false;
    }
    ScopedManagedCodeThread managedScope(Coroutine::GetCurrent());
    method->GetPandaMethod()->Invoke(Coroutine::GetCurrent(), nullptr);
    return true;
}

bool ETSAni::DestroyExclusiveWorkerForTaskpoolIfExists()
{
    auto *runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    ClassLinkerExtension *ext = classLinker->GetExtension(SourceLanguage::ETS);
    auto mutf8Name = reinterpret_cast<const uint8_t *>("Lstd/concurrency/taskpool;");
    auto *klass = ext->GetClass(mutf8Name);
    if (klass == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool failed in pre zygote fork";
        return false;
    }
    auto method = ets::EtsClass::FromRuntimeClass(klass)->GetStaticMethod("stopAllWorkers", ":V");
    if (method == nullptr) {
        LOG(ERROR, COROUTINES) << "Load taskpool::stopAllWorkers failed in pre zygote fork";
        return false;
    }
    ScopedManagedCodeThread managedScope(Coroutine::GetCurrent());
    method->GetPandaMethod()->Invoke(Coroutine::GetCurrent(), nullptr);
    return true;
}
}  // namespace ark::ets
