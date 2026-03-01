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

#include "libarkbase/os/native_stack.h"
#include "monitor.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/span.h"
#include "verification/plugins.h"
#include "verification/util/is_system.h"
#include "verification/util/optional_ref.h"
#ifdef ARK_HYBRID
#include <node_api.h>
#endif

// generated
#include "libarkbase/generated/ark_version.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"
#include "generated/verifier_options_gen.h"

#include <csignal>
#include <chrono>
#include <thread>

namespace ark::verifier {

size_t const MAX_THREADS = 64;

void Worker(PandaDeque<Method *> *queue, os::memory::Mutex *lock, size_t threadNum, std::atomic<bool> *result)
{
    LOG(DEBUG, VERIFIER) << "Verifier thread " << threadNum << " starting";
    {
        std::stringstream ss;
        ss << "Verifier #" << threadNum;
        if (os::thread::SetThreadName(os::thread::GetNativeHandle(), ss.str().c_str()) != 0) {
            LOG(ERROR, VERIFIER) << "Failed to set worker thread name " << ss.str();
        }
    }

    auto *service = Runtime::GetCurrent()->GetVerifierService();
    const auto &options = Runtime::GetCurrent()->GetOptions();
    auto mode = verifier::VerificationModeFromString(options.GetVerificationMode());

    ManagedThread *thread = nullptr;
    panda_file::SourceLang currentLang = panda_file::SourceLang::INVALID;
    for (;;) {
        Method *method;
        {
            os::memory::LockHolder lh {*lock};
            if (queue->empty()) {
                break;
            }
            method = queue->front();
            queue->pop_front();
        }
        auto methodLang = method->GetClass()->GetSourceLang();
        if (methodLang != currentLang) {
            if (thread != nullptr) {
                plugin::GetLanguagePlugin(currentLang)->DestroyManagedThread(thread);
            }
            thread = plugin::GetLanguagePlugin(methodLang)->CreateManagedThread();
            currentLang = methodLang;
        }
        if (ark::verifier::Verify(service, method, mode) != Status::OK) {
            *result = false;
            LOG(ERROR, VERIFIER) << "Error: method " << method->GetFullName(true) << " failed to verify";
        }
    }
    if (thread != nullptr) {
        plugin::GetLanguagePlugin(currentLang)->DestroyManagedThread(thread);
    }
    LOG(DEBUG, VERIFIER) << "Verifier thread " << threadNum << " finishing";
}

static void EnqueueMethod(Method *method, PandaDeque<Method *> &queue, PandaUnorderedSet<Method *> &methodsSet)
{
    if (methodsSet.count(method) == 0) {
        queue.push_back(method);
        methodsSet.insert(method);
    }
}

static void EnqueueClass(const Class &klass, bool verifyLibraries, PandaDeque<Method *> &queue,
                         PandaUnorderedSet<Method *> &methodsSet)
{
    if (!verifyLibraries && IsSystemClass(&klass)) {
        LOG(INFO, VERIFIER) << klass.GetName() << " is a system class, skipping";
        return;
    }
    LOG(INFO, VERIFIER) << "Begin verification of class " << klass.GetName();
    for (auto &method : klass.GetMethods()) {
        EnqueueMethod(&method, queue, methodsSet);
    }
}

static auto GetFileHandler(std::atomic<bool> &result, ClassLinker &classLinker, bool verifyLibraries,
                           PandaDeque<Method *> &queue, PandaUnorderedSet<Method *> &methodsSet)
{
    auto handleClass = [&result, &classLinker, verifyLibraries, &queue,
                        &methodsSet](const panda_file::File &file, const panda_file::File::EntityId &entityId) {
        if (!file.IsExternal(entityId)) {
            auto optLang = panda_file::ClassDataAccessor {file, entityId}.GetSourceLang();
            if (optLang.has_value() && !IsValidSourceLang(optLang.value())) {
                LOG(ERROR, VERIFIER) << "Unknown SourceLang";
                result = false;
                return false;
            }
            ClassLinkerExtension *ext =
                classLinker.GetExtension(optLang.value_or(panda_file::SourceLang::PANDA_ASSEMBLY));
            if (ext == nullptr) {
                LOG(ERROR, VERIFIER) << "Error: Class Linker Extension failed to initialize";
                result = false;
                return false;
            }
            const Class *klass = ext->GetClass(file, entityId);

            if (klass != nullptr) {
                EnqueueClass(*klass, verifyLibraries, queue, methodsSet);
            }
        }

        return true;
    };

    return [handleClass](const panda_file::File &file) {
        LOG(INFO, VERIFIER) << "Processing file" << file.GetFilename();
        for (auto id : file.GetClasses()) {
            panda_file::File::EntityId entityId {id};
            if (!handleClass(file, entityId)) {
                return false;
            }
        }

        return true;
    };
}

static void VerifyAllNames(std::atomic<bool> &result, bool verifyLibraries, Runtime &runtime,
                           PandaDeque<Method *> &queue, PandaUnorderedSet<Method *> &methodsSet)
{
    auto &classLinker = *runtime.GetClassLinker();
    PandaVector<const panda_file::File *> pfiles;
    classLinker.EnumeratePandaFiles([&pfiles](const panda_file::File &file) {
        pfiles.push_back(&file);
        return true;
    });

    auto handleFile = GetFileHandler(result, classLinker, verifyLibraries, queue, methodsSet);

    for (auto pf : pfiles) {
        if (!handleFile(*pf)) {
            break;
        }
    }

    if (verifyLibraries) {
        classLinker.EnumerateBootPandaFiles(handleFile);
    } else if (runtime.GetPandaFiles().empty()) {
        // in this case the last boot-panda-file and only it is actually not a system file and should be
        // verified
        OptionalConstRef<panda_file::File> file;
        classLinker.EnumerateBootPandaFiles([&file](const panda_file::File &pf) {
            file = std::cref(pf);
            return true;
        });
        if (file.HasRef()) {
            handleFile(*file);
        } else {
            LOG(ERROR, VERIFIER) << "No files given to verify";
        }
    }
}

static Class *GetClassByName(std::atomic<bool> &result, PandaUnorderedMap<std::string, Class *> &classesByName,
                             Runtime &runtime, const std::string &className)
{
    auto it = classesByName.find(className);
    if (it != classesByName.end()) {
        return it->second;
    }

    PandaString descriptor;
    auto &classLinker = *runtime.GetClassLinker();
    auto *ctx = classLinker.GetExtension(runtime.GetLanguageContext(runtime.GetRuntimeType()))->GetBootContext();
    const uint8_t *classNameBytes = ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor);

    Class *klass = classLinker.GetClass(classNameBytes, true, ctx);
    if (klass == nullptr) {
        LOG(ERROR, VERIFIER) << "Error: Cannot resolve class with name " << className;
        result = false;
    }

    classesByName.emplace(className, klass);
    return klass;
}

static bool VeifyMethod(Class *klass, const std::string &fqMethodName, const std::string_view &unqualifiedMethodName,
                        PandaDeque<Method *> &queue, PandaUnorderedSet<Method *> &methodsSet)
{
    bool methodFound = false;
    for (auto &method : klass->GetMethods()) {
        const char *nameData = utf::Mutf8AsCString(method.GetName().data);
        if (std::string_view(nameData) == unqualifiedMethodName) {
            methodFound = true;
            LOG(INFO, VERIFIER) << "Verification of method '" << fqMethodName << "'";
            EnqueueMethod(&method, queue, methodsSet);
        }
    }
    return methodFound;
}

static void RunVerifierImpl(std::atomic<bool> &result, Runtime &runtime, const std::vector<std::string> &classNames,
                            const std::vector<std::string> &methodNames, PandaDeque<Method *> &queue)
{
    bool verifyLibraries = runtime.GetOptions().IsVerifyRuntimeLibraries();
    PandaUnorderedSet<Method *> methodsSet;

    // we need ScopedManagedCodeThread for the verifier since it can allocate objects
    ScopedManagedCodeThread managedObjThread(ManagedThread::GetCurrent());
    if (classNames.empty() && methodNames.empty()) {
        VerifyAllNames(result, verifyLibraries, runtime, queue, methodsSet);
    } else {
        PandaUnorderedMap<std::string, Class *> classesByName;

        for (const auto &className : classNames) {
            Class *klass = GetClassByName(result, classesByName, runtime, className);
            // the bad case is already handled in get_class_by_name
            if (klass != nullptr) {
                EnqueueClass(*klass, verifyLibraries, queue, methodsSet);
            }
        }

        for (const std::string &fqMethodName : methodNames) {
            size_t pos = fqMethodName.find_last_of("::");
            if (pos == std::string::npos) {
                LOG(ERROR, VERIFIER) << "Error: Fully qualified method name must contain '::', was " << fqMethodName;
                result = false;
                break;
            }
            std::string className = fqMethodName.substr(0, pos - 1);
            std::string_view unqualifiedMethodName = std::string_view(fqMethodName).substr(pos + 1);
            if (std::find(classNames.begin(), classNames.end(), className) != classNames.end()) {
                // this method was already verified while enumerating class_names
                continue;
            }
            Class *klass = GetClassByName(result, classesByName, runtime, className);
            if (klass == nullptr) {
                continue;
            }
            bool methodFound = VeifyMethod(klass, fqMethodName, unqualifiedMethodName, queue, methodsSet);
            if (!methodFound) {
                LOG(ERROR, VERIFIER) << "Error: Cannot resolve method with name " << unqualifiedMethodName
                                     << " in class " << className;
                result = false;
            }
        }
    }
}

bool RunVerifier(const Options &cliOptions)
{
    auto isPerfMeasure = cliOptions.IsPerfMeasure();
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;

    os::memory::Mutex lock;

    auto &runtime = *Runtime::GetCurrent();

    PandaDeque<Method *> queue;

    const std::vector<std::string> &classNames = cliOptions.GetClasses();
    const std::vector<std::string> &methodNames = cliOptions.GetMethods();

    std::atomic<bool> result = true;
    RunVerifierImpl(result, runtime, classNames, methodNames, queue);

    if (isPerfMeasure) {
        begin = std::chrono::steady_clock::now();
    }

    size_t nthreads = runtime.GetOptions().GetVerificationThreads();
    std::vector<std::thread *> threads;
    for (size_t i = 0; i < nthreads; i++) {
        auto *worker = runtime.GetInternalAllocator()->New<std::thread>(Worker, &queue, &lock, i, &result);
        threads.push_back(worker);
    }

    for (auto *thr : threads) {
        thr->join();
        runtime.GetInternalAllocator()->Delete(thr);
    }

    if (isPerfMeasure) {
        end = std::chrono::steady_clock::now();
        std::cout << "Verification time = "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " us" << std::endl;
    }

    return result;
}

void PrintHelp(const PandArgParser &paParser)
{
    std::string error = paParser.GetErrorString();
    if (!error.empty()) {
        error += "\n\n";
    }
    std::cerr << error << "Usage: verifier [option...] [file]\n"
              << "Verify specified Panda files (given by file and --panda-files) "
              << "or certain classes/methods in them.\n\n"
              << paParser.GetHelpString() << std::endl;
}

static int RunThreads(Options &cliOptions, RuntimeOptions &runtimeOptions)
{
    uint32_t threads = cliOptions.GetThreads();
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
        // hardware_concurrency can return 0 if the value is not computable or well defined
        if (threads == 0) {
            threads = 1;
        } else if (threads > MAX_THREADS) {
            threads = MAX_THREADS;
        }
    }
    runtimeOptions.SetVerificationThreads(threads);

    if (!Runtime::Create(runtimeOptions)) {
        std::cerr << "Error: cannot create runtime" << std::endl;
        return -1;
    }

    int ret = RunVerifier(cliOptions) ? 0 : -1;

    if (cliOptions.IsPrintMemoryStatistics()) {
        std::cout << Runtime::GetCurrent()->GetMemoryStatistics();
    }
    if (!Runtime::Destroy()) {
        std::cerr << "Error: cannot destroy runtime" << std::endl;
        return -1;
    }
    return ret;
}

static int Run(Options &cliOptions, RuntimeOptions &runtimeOptions, logger::Options &loggerOptions,
               PandArg<std::string> &file)
{
    auto bootPandaFiles = cliOptions.GetBootPandaFiles();
    auto pandaFiles = cliOptions.GetPandaFiles();
    std::string mainFileName = file.GetValue();
    if (!mainFileName.empty()) {
        if (pandaFiles.empty()) {
            bootPandaFiles.push_back(mainFileName);
            runtimeOptions.SetLoadInBoot(true);
        } else {
            auto foundIter = std::find_if(pandaFiles.begin(), pandaFiles.end(),
                                          [&mainFileName](auto &fileName) { return mainFileName == fileName; });
            if (foundIter == pandaFiles.end()) {
                pandaFiles.push_back(mainFileName);
            }
        }
    }

    runtimeOptions.SetBootPandaFiles(bootPandaFiles);
    runtimeOptions.SetPandaFiles(pandaFiles);
    runtimeOptions.SetLoadRuntimes(cliOptions.GetLoadRuntimes());
    runtimeOptions.SetGcType(cliOptions.GetGcType());

    loggerOptions.SetLogComponents(cliOptions.GetLogComponents());
    loggerOptions.SetLogLevel(cliOptions.GetLogLevel());
    loggerOptions.SetLogStream(cliOptions.GetLogStream());
    loggerOptions.SetLogFile(cliOptions.GetLogFile());
    Logger::Initialize(loggerOptions);

    runtimeOptions.SetLimitStandardAlloc(cliOptions.IsLimitStandardAlloc());
    runtimeOptions.SetInternalAllocatorType(cliOptions.GetInternalAllocatorType());
    runtimeOptions.SetInternalMemorySizeLimit(cliOptions.GetInternalMemorySizeLimit());

    runtimeOptions.SetVerificationMode(cliOptions.IsDebugMode() ? "debug" : "ahead-of-time");
    runtimeOptions.SetVerificationConfigFile(cliOptions.GetConfigFile());
    runtimeOptions.SetVerificationCacheFile(cliOptions.GetCacheFile());
    runtimeOptions.SetVerificationUpdateCache(cliOptions.IsUpdateCache());
    runtimeOptions.SetVerifyRuntimeLibraries(cliOptions.IsVerifyRuntimeLibraries());
    return RunThreads(cliOptions, runtimeOptions);
}

int Main(int argc, const char **argv)
{
    Span<const char *> sp(argv, argc);
    RuntimeOptions runtimeOptions(sp[0]);
    Options cliOptions(sp[0]);
    PandArgParser paParser;
    logger::Options loggerOptions("");

    PandArg<bool> help("help", false, "Print this message and exit");
    PandArg<bool> options("options", false, "Print verifier options");
    // tail argument
    PandArg<std::string> file("file", "", "path to pandafile");

    cliOptions.AddOptions(&paParser);

    paParser.Add(&help);
    paParser.Add(&options);
    paParser.PushBackTail(&file);
    paParser.EnableTail();

#ifdef ARK_HYBRID
    // This workaround is needed to define weak symbols of napi in hybrid libarkruntime.so
    // It will be removed after #26269 fix
    volatile bool initNapi = false;
    if (initNapi) {
        napi_module_register(nullptr);
    }
#endif

    if (!paParser.Parse(argc, argv)) {
        PrintHelp(paParser);
        return 1;
    }

    if (runtimeOptions.IsVersion()) {
        PrintPandaVersion();
        return 0;
    }

    if (help.GetValue()) {
        PrintHelp(paParser);
        return 0;
    }

    if (options.GetValue()) {
        std::cout << paParser.GetRegularArgs() << std::endl;
        return 0;
    }

    auto cliOptionsErr = cliOptions.Validate();
    if (cliOptionsErr) {
        std::cerr << "Error: " << cliOptionsErr.value().GetMessage() << std::endl;
        return 1;
    }

    return Run(cliOptions, runtimeOptions, loggerOptions, file);
}

}  // namespace ark::verifier

int main(int argc, const char **argv)
{
    return ark::verifier::Main(argc, argv);
}
