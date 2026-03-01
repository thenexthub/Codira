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

#include "ArkASTWorker.h"
#include <pthread.h>
#include "ArkAST.h"
#include "Codira/Parse/Parser.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"
#include "logger/Logger.h"

using namespace Codira;
namespace {
// The files of codelibs only support SemanticTokens and Definition.
std::set<std::string> codelibSupportFeatures = {"SemanticTokens", "Definition"};
std::set<std::string> hierarchyIgnoreRequest = {"SubTypes", "SuperTypes", "OnIncomingCalls", "OnOutgoingCalls"};
}
namespace ark {
ArkASTWorker *ArkASTWorker::Create(AsyncTaskRunner &asyncTaskRunner, Semaphore &semaphore, Callbacks *c)
{
    auto worker = new (std::nothrow) ArkASTWorker(semaphore, c);
    if (worker == nullptr) {
        return nullptr;
    }
    auto task = [worker]() { worker->Run(); };
    asyncTaskRunner.RunAsync("worker:", std::move(task), worker);

    return worker;
}

ArkASTWorker::ArkASTWorker(Semaphore &semaphore, Callbacks *c) : barrier(semaphore), done(false), callback(c) {}

ArkASTWorker::~ArkASTWorker() {}

void ArkASTWorker::Update(const ParseInputs &inputs, NeedDiagnostics needDiag)
{
    Trace::Log("ArkASTWorker::Update in.");
    std::string taskName = "Update " + inputs.fileName;

    auto task = [this, inputs]() mutable {
        std::string realName = inputs.fileName;
        if (inputs.forceRebuild) {
            std::stringstream log;
            Logger &logger = Logger::Instance();
            // Check whether the newly opened file is in the current project.
            if (CompilerCodiraProject::GetInstance()->GetCodiraFileKind(realName).first == CodiraFileKind::MISSING) {
                return;
            }
            // ast is not in cache, need to compile separately
            CompilerCodiraProject::GetInstance()->CompilerOneFile(realName, inputs.contents);
        }

        std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(realName);
        callback->ReadyForDiagnostics(realName, inputs.version, diagnostics);
    };

    StartTask(taskName, std::move(task), needDiag);
}

void ArkASTWorker::Run()
{
    isRun = true;
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            for (auto dl = ScheduleLocked(); !dl.Expired(); dl = ScheduleLocked()) {
                if (done && requests.empty()) {
                    return;
                }
                Wait(lock, requestsCV, dl);
            }

            currentRequest = requests.front();
            requests.pop_front();
        }
        {
            std::unique_lock<Semaphore> sLock(barrier, std::try_to_lock);
            if (!sLock.owns_lock()) {
                sLock.lock();
            }
            currentRequest.action();
        }
    }
}

Deadline ArkASTWorker::ScheduleLocked()
{
    if (requests.empty()) {
        return Deadline::Infinity();
    }

    while (ShouldSkipHeadLocked()) {
        Logger &logger = Logger::Instance();
        logger.LogMessage(MessageType::MSG_INFO, "ASTWorker skipping " + requests.front().name);
        requests.pop_front();
    }
    return Deadline::Zero();
}

bool ArkASTWorker::ShouldSkipHeadLocked() const
{
    auto next = requests.begin();
    NeedDiagnostics updateType = next->updateType;
    ++next;

    if (next == requests.end()) {
        return false;
    }
    // The other way an update can be live is if its diagnostics might be used.
    switch (updateType) {
        case NeedDiagnostics::YES:
            return false; // Always used.
        case NeedDiagnostics::NO:
            return true; // Always dead.
        case NeedDiagnostics::AUTO:
            if (next->updateType == NeedDiagnostics::AUTO) {
                return true; // Prefer later diagnostics.
            }
            return false;
        default:
            return false;
    }
}

void ArkASTWorker::Stop() noexcept
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        done = true;
        requestsCV.notify_all();
    }
}

void ArkASTWorker::StartTask(std::string name, std::function<void()> task, NeedDiagnostics needDiag)
{
    {
        std::lock_guard<std::mutex> lock(mutex);

        // remove old request with the same name
        requests.erase(
            std::remove_if(requests.begin(), requests.end(), [&name](const Request& req) {
                return req.name == name;
            }),
            requests.end()
        );

        // Allow this request to be cancelled if invalidated.
        requests.push_back({std::move(task), std::move(name), needDiag});
    }
    requestsCV.notify_all();
}

void ArkASTWorker::RunWithAST(const std::string &name,
    const std::string &file,
    std::function<void(InputsAndAST)> action,
    NeedDiagnostics needDiag)
{
    if (IsInCodelibDir(file) && codelibSupportFeatures.find(name) == codelibSupportFeatures.end()) {
        return;
    }

    if (name == "DocumentLink") {
        std::unique_lock<std::mutex> lock(editMutex);
        this->onEditName = file;
    }

    auto task = [this, file, action = std::move(action), name]() mutable {
        bool needReParser = this->callback->NeedReParser(file);
        bool useASTCache = true;
        ParseInputs inputs;
        inputs.fileName = file;
        inputs.contents = this->callback->GetContentsByFile(file);
        inputs.version = this->callback->GetVersionByFile(file);
        if (needReParser) {
            std::vector<TextDocumentContentChangeEvent> contentChanges;
            // Do incremental build for defined file first
            if (this->callback->isRenameDefined && this->callback->NeedReParser(this->callback->path) &&
                this->callback->path != file) {
                CompilerCodiraProject::GetInstance()->CompilerOneFile(
                    this->callback->path, this->callback->GetContentsByFile(this->callback->path));
                this->callback->UpdateDocNeedReparse(this->callback->path,
                    this->callback->GetVersionByFile(this->callback->path), false);
                this->callback->isRenameDefined = false;
                this->callback->path = "";
            }
            CompilerCodiraProject::GetInstance()->CompilerOneFile(file, this->callback->GetContentsByFile(file));
            this->callback->UpdateDocNeedReparse(file, inputs.version, false);
            std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
            callback->ReadyForDiagnostics(file, inputs.version, diagnostics);
            useASTCache = false;
        }
        if (hierarchyIgnoreRequest.find(name) == hierarchyIgnoreRequest.end() &&
            (!CompilerCodiraProject::GetInstance()->FileHasSemaCache(file) ||
                CompilerCodiraProject::GetInstance()->CheckNeedCompiler(file))) {
            CompilerCodiraProject::GetInstance()->IncrementOnePkgCompile(file, inputs.contents);
            std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
            callback->ReadyForDiagnostics(file, inputs.version, diagnostics);
            useASTCache = false;
        }
        ArkAST *ast = CompilerCodiraProject::GetInstance()->GetArkAST(file);
        // Run the user-provided action.
        std::string curOnEditName = file;
        {
            std::unique_lock<std::mutex> lock(editMutex);
            curOnEditName = this->onEditName;
        }
        Logger::Instance().CleanKernelLog(std::this_thread::get_id());
        if (name == "SemanticTokens") {
            if (FileUtil::FileExist(file)) {
                std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
                callback->ReadyForDiagnostics(file, inputs.version, diagnostics);
            } else {
                callback->ReadyForDiagnostics(file, inputs.version, {});
            }
        }
        action(InputsAndAST{inputs, ast, curOnEditName, useASTCache});
    };

    StartTask(name, std::move(task), needDiag);
}

void ArkASTWorker::RunWithASTCache(
    const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action)
{
    if (IsInCodelibDir(file)) {
        return;
    }

    auto task = [this, action = std::move(action), file, pos, name]() mutable {
        if (!Options::GetInstance().IsOptionSet("test")) {
            std::unique_lock<std::mutex> lock(completionMtx);
            isCompleteRunning = true;
        }
        auto inputs =
            ParseInputs(file, this->callback->GetContentsByFile(file), this->callback->GetVersionByFile(file));
        std::string absName = Codira::FileUtil::Normalize(file);
        auto fullPkgName = CompilerCodiraProject::GetInstance()->GetFullPkgName(file);
        bool shouldIncrementCompile = !CompilerCodiraProject::GetInstance()->pLRUCache->HasCache(fullPkgName);
        if (shouldIncrementCompile) {
            CompilerCodiraProject::GetInstance()->IncrementOnePkgCompile(absName, inputs.contents);
        }

        if (!IsFromCIMap(fullPkgName) && !IsFromCIMapNotInSrc(fullPkgName)) {
            return;
        }
        std::vector<TextDocumentContentChangeEvent> contentChanges;
        bool needReParser = this->callback->NeedReParser(file);
        this->callback->UpdateDocNeedReparse(file, inputs.version, needReParser);
        CompilerCodiraProject::GetInstance()->CompilerOneFile(
            file, this->callback->GetContentsByFile(file), pos, true, name);
        Logger::Instance().CleanKernelLog(std::this_thread::get_id());
        ArkAST *ast = CompilerCodiraProject::GetInstance()->GetParseArkAST(file);
        if (!ast) { return; }
        {
            std::unique_lock<std::recursive_mutex> lck(CompilerCodiraProject::GetInstance()->fileCacheMtx);
            ArkAST *astCache = CompilerCodiraProject::GetInstance()->GetArkAST(file);
            if (!astCache) {
                return;
            }
            ast->semaCache = astCache;
            action(InputsAndAST{inputs, ast, "", false});
        }
        if (Options::GetInstance().IsOptionSet("test")) {
            return;
        }

        {
            std::unique_lock<std::mutex> lock(completionMtx);
            if (waitingCompletionTask != nullptr) {
                std::thread thread(std::move(waitingCompletionTask));
                waitingCompletionTask = nullptr;
                thread.detach();
            } else {
                isCompleteRunning = false;
            }
        }
        if (CompilerCodiraProject::GetUseDB()) {
            lsp::BackgroundIndexDB *indexDB = CompilerCodiraProject::GetInstance()->GetBgIndexDB();
            if (!indexDB) {
                return;
            }
            auto &dbCache = indexDB->GetIndexDatabase().GetDatabaseCache();
            dbCache.EraseThreadCache();
        }
    };

    if (Options::GetInstance().IsOptionSet("test")) {
        std::thread thread(std::move(task));
        thread.join();
    } else {
        std::unique_lock<std::mutex> lock(completionMtx);
        if (isCompleteRunning) {
            waitingCompletionTask = std::move(task);
        } else {
            lock.unlock();
            std::thread thread(std::move(task));
            thread.detach();
        }
    }
}

AsyncTaskRunner::AsyncTaskRunner() : inFlightTasks(0) {}

AsyncTaskRunner::~AsyncTaskRunner() noexcept { Wait(); }

void AsyncTaskRunner::Wait(const Deadline &deadline) const
{
    std::unique_lock<std::mutex> lock(mutex);
    ark::Wait(lock, tasksReachedZero, deadline, [this] { return inFlightTasks == 0; });
}

void AsyncTaskRunner::RunAsync(const std::string &name, std::function<void()> action, ArkASTWorker *worker)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        ++inFlightTasks;
    }
#ifndef __APPLE__
    auto task = [name, action = std::move(action), worker, this]() mutable {
        action();
        // Make sure function stored by ThreadFunc is destroyed before Cleanup runs.
        if (worker) {
            delete worker;
            worker = nullptr;
        }
        action = nullptr;
        {
            std::lock_guard<std::mutex> lock(mutex);
            --inFlightTasks;
            if (!inFlightTasks) {
                tasksReachedZero.notify_all();
            }
        }
    };
    std::thread thread(std::move(task));
    thread.detach();
#else
    auto *data = new ThreadData();
    data->worker = worker;
    data->runner = this;
    data->action = std::move(action);
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, CONSTANTS::MAC_THREAD_STACK_SIZE);
    pthread_create(&thread, &attr, thread_routine, data);
    int waitTime = 1000;
    while (!worker->GetStatus()) {
        usleep(waitTime);
    }
    pthread_detach(thread);
#endif
}
} // namespace ark
