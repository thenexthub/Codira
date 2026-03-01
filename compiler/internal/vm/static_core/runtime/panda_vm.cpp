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

#include <cstdlib>

#include "mem/lock_config_helper.h"
#include "runtime/default_debugger_agent.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"

#include "libarkfile/file.h"

namespace ark {
/* static */
PandaVM *PandaVM::Create(Runtime *runtime, const RuntimeOptions &options, std::string_view runtimeType)
{
    LanguageContext ctx = runtime->GetLanguageContext(std::string(runtimeType));
    return ctx.CreateVM(runtime, options);
}

Expected<int, Runtime::Error> PandaVM::InvokeEntrypoint(Method *entrypoint, const std::vector<std::string> &args)
{
    if (!CheckEntrypointSignature(entrypoint)) {
        LOG(ERROR, RUNTIME) << "Method '" << entrypoint->GetFullName(true) << "' has invalid signature";
        return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
    }
    Expected<int, Runtime::Error> ret = InvokeEntrypointImpl(entrypoint, args);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    bool hasException = false;
    {
        ScopedManagedCodeThread s(thread);
        hasException = thread->HasPendingException();
    }
    if (hasException) {
        HandleUncaughtException();
        ret = EXIT_FAILURE;
    }

    return ret;
}

void PandaVM::HandleLdaStr(Frame *frame, BytecodeId stringId)
{
    coretypes::String *str = ark::Runtime::GetCurrent()->ResolveString(this, *frame->GetMethod(), stringId.AsFileId());
    frame->GetAccAsVReg().SetReference(str);
}

std::unique_ptr<const panda_file::File> PandaVM::OpenPandaFile(std::string_view location)
{
    return panda_file::OpenPandaFile(location);
}

coretypes::String *PandaVM::GetNonMovableString(const panda_file::File &pf, panda_file::File::EntityId id) const
{
    auto cachedString = GetStringTable()->GetInternalStringFast(pf, id);
    if (cachedString == nullptr) {
        return nullptr;
    }

    if (!GetHeapManager()->IsObjectInNonMovableSpace(cachedString)) {
        return nullptr;
    }

    return cachedString;
}

bool PandaVM::ShouldEnableDebug()
{
    return !Runtime::GetOptions().GetDebuggerLibraryPath().empty() || Runtime::GetOptions().IsDebuggerEnable();
}

void PandaVM::FreeInternalResources()
{
    // Run monitor deflation first
    GetMonitorPool()->DeflateMonitors();
}

LoadableAgentHandle PandaVM::CreateDebuggerAgent()
{
    if (!Runtime::GetOptions().GetDebuggerLibraryPath().empty()) {
        return DefaultDebuggerAgent::LoadInstance();
    }

    return {};
}

PandaString PandaVM::GetClassesFootprint() const
{
    ASSERT(GetLanguageContext().GetLanguageType() == LangTypeT::LANG_TYPE_STATIC);
    PandaVector<Class *> classes;
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->EnumerateClasses([&classes](Class *cls) {
        classes.push_back(cls);
        return true;
    });

    PandaVector<uint64_t> footprintOfClasses(classes.size(), 0U);
    GetHeapManager()->CountInstances(classes, true, footprintOfClasses.data());

    PandaMultiMap<uint64_t, Class *> footprintToClass;
    for (size_t index = 0; index < classes.size(); ++index) {
        footprintToClass.insert({footprintOfClasses[index], classes[index]});
    }

    PandaStringStream statistic;
    PandaMultiMap<uint64_t, Class *>::reverse_iterator rit;
    for (rit = footprintToClass.rbegin(); rit != footprintToClass.rend(); ++rit) {
        if (rit->first == 0U) {
            break;
        }
        auto clazz = rit->second;
        statistic << "class: " << clazz->GetName() << ", footprint - " << rit->first << std::endl;
    }
    return statistic.str();
}

void PandaVM::ProcessReferenceFinalizers()
{
    GetReferenceProcessor()->ProcessFinalizers();
}

void PandaVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    auto *storage = GetGlobalObjectStorage();
    if (storage != nullptr) {
        storage->VisitObjects(visitor, mem::RootType::ROOT_NATIVE_GLOBAL);
    }
}

void PandaVM::UpdateAndSweepVmRefs(const ReferenceUpdater &updater)
{
    GetStringTable()->UpdateAndSweep(updater);
    auto globalStorage = GetGlobalObjectStorage();
    if (globalStorage != nullptr) {
        globalStorage->UpdateAndSweep(updater);
    }
}

}  // namespace ark
