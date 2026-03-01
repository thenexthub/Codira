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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_UTIL_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_UTIL_H

#include "test/utils/test_events.h"
#include "test/utils/test_extractor.h"

#include "agent/debugger_impl.h"

#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/debugger/js_debugger.h"
#include "os/mutex.h"

namespace panda::ecmascript::tooling::test {
template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
using CUnorderedMap = panda::ecmascript::CUnorderedMap<Key, T, Hash, KeyEqual>;
using TestMap = CUnorderedMap<std::string, std::unique_ptr<TestEvents>>;

class TestUtil {
public:
    static void RegisterTest(const std::string &testName, std::unique_ptr<TestEvents> test)
    {
        testMap_.insert({testName, std::move(test)});
    }

    static TestEvents *GetTest(const std::string &name)
    {
        auto iter = std::find_if(testMap_.begin(), testMap_.end(), [&name](auto &it) {
            return it.first == name;
        });
        if (iter != testMap_.end()) {
            return iter->second.get();
        }
        LOG_DEBUGGER(FATAL) << "Test " << name << " not found";
        return nullptr;
    }

    static void WaitForBreakpoint(JSPtLocation location)
    {
        auto predicate = [&location]() REQUIRES(eventMutex_) { return lastEventLocation_ == location; };
        auto onSuccess = []() REQUIRES(eventMutex_) {
            // Need to reset location, because we might want to stop at the same point
            lastEventLocation_ = JSPtLocation(nullptr, EntityId(0), 0);
        };

        WaitForEvent(DebugEvent::BREAKPOINT, predicate, onSuccess);
    }

    static bool WaitForDropframe()
    {
        auto predicate = []() REQUIRES(eventMutex_) { return lastEvent_ == DebugEvent::DROPFRAME; };
        return WaitForEvent(DebugEvent::DROPFRAME, predicate, [] {});
    }

    static bool WaitForCheckComplete()
    {
        auto predicate = []() REQUIRES(eventMutex_) { return lastEvent_ == DebugEvent::CHECK_COMPLETE; };
        return WaitForEvent(DebugEvent::CHECK_COMPLETE, predicate, [] {});
    }

    static bool WaitForExit()
    {
        return WaitForEvent(DebugEvent::VM_DEATH,
            []() REQUIRES(eventMutex_) {
                return lastEvent_ == DebugEvent::VM_DEATH;
            }, [] {});
    }

    static void WaitForStepComplete(JSPtLocation location)
    {
        auto predicate = [&location]() REQUIRES(eventMutex_) { return lastEventLocation_ == location; };
        auto onSuccess = []() REQUIRES(eventMutex_) {
            // Need to reset location, because we might want to stop at the same point
            lastEventLocation_ = JSPtLocation(nullptr, EntityId(0), 0);
        };

        WaitForEvent(DebugEvent::STEP_COMPLETE, predicate, onSuccess);
    }

    static bool WaitForException()
    {
        auto predicate = []() REQUIRES(eventMutex_) { return lastEvent_ == DebugEvent::EXCEPTION; };
        return WaitForEvent(DebugEvent::EXCEPTION, predicate, [] {});
    }

    static bool WaitForInit()
    {
        return WaitForEvent(DebugEvent::VM_START,
            []() REQUIRES(eventMutex_) {
                return initialized_;
            }, [] {});
    }

    static bool WaitForLoadModule()
    {
        auto predicate = []() REQUIRES(eventMutex_) { return lastEvent_ == DebugEvent::LOAD_MODULE; };
        return WaitForEvent(DebugEvent::LOAD_MODULE, predicate, [] {});
    }

    static void Event(DebugEvent event, JSPtLocation location = JSPtLocation(nullptr, EntityId(0), 0))
    {
        LOG_DEBUGGER(DEBUG) << "Occurred event " << event;
        os::memory::LockHolder holder(eventMutex_);
        lastEvent_ = event;
        lastEventLocation_ = location;
        if (event == DebugEvent::VM_START) {
            initialized_ = true;
        }
        eventCv_.Signal();
    }

    static void Reset()
    {
        os::memory::LockHolder lock(eventMutex_);
        initialized_ = false;
        lastEvent_ = DebugEvent::VM_START;
    }

    static TestMap &GetTests()
    {
        return testMap_;
    }

    static bool IsTestFinished()
    {
        os::memory::LockHolder lock(eventMutex_);
        return lastEvent_ == DebugEvent::VM_DEATH;
    }

    static JSPtLocation GetLocation(const char *sourceFile, int32_t line, int32_t column, const char *pandaFile)
    {
        auto jsPandaFile = ::panda::ecmascript::JSPandaFileManager::GetInstance()->FindJSPandaFile(pandaFile);
        if (jsPandaFile == nullptr) {
            LOG_DEBUGGER(FATAL) << "cannot find: " << pandaFile;
            UNREACHABLE();
        }
        TestExtractor extractor(jsPandaFile.get());
        auto [id, offset] = extractor.GetBreakpointAddress({jsPandaFile.get(), line, column});
        return JSPtLocation(jsPandaFile.get(), id, offset, sourceFile);
    }

    static SourceLocation GetSourceLocation(const JSPtLocation &location, const char *pandaFile)
    {
        auto jsPandaFile = ::panda::ecmascript::JSPandaFileManager::GetInstance()->FindJSPandaFile(pandaFile);
        if (jsPandaFile == nullptr) {
            LOG_DEBUGGER(FATAL) << "cannot find: " << pandaFile;
            UNREACHABLE();
        }
        TestExtractor extractor(jsPandaFile.get());
        return extractor.GetSourceLocation(jsPandaFile.get(), location.GetMethodId(), location.GetBytecodeOffset());
    }

    static bool SuspendUntilContinue(DebugEvent reason, JSPtLocation location = JSPtLocation(nullptr, EntityId(0), 0))
    {
        os::memory::LockHolder lock(suspendMutex_);
        suspended_ = true;

        // Notify the debugger thread about the suspend event
        Event(reason, location);

        // Wait for continue
        while (suspended_) {
            constexpr uint64_t TIMEOUT_MSEC = 300000U;
            bool timeExceeded = suspendCv_.TimedWait(&suspendMutex_, TIMEOUT_MSEC);
            if (timeExceeded) {
                LOG_DEBUGGER(FATAL) << "Time limit exceeded while suspend";
                return false;
            }
        }

        return true;
    }

    static bool Continue()
    {
        os::memory::LockHolder lock(suspendMutex_);
        suspended_ = false;
        suspendCv_.Signal();
        return true;
    }

private:
    template<class Predicate, class OnSuccessAction>
    static bool WaitForEvent(DebugEvent event, Predicate predicate, OnSuccessAction action)
    {
        os::memory::LockHolder holder(eventMutex_);
        while (!predicate()) {
            if (lastEvent_ == DebugEvent::VM_DEATH) {
                return false;
            }
            constexpr uint64_t TIMEOUT_MSEC = 300000U;
            bool timeExceeded = eventCv_.TimedWait(&eventMutex_, TIMEOUT_MSEC);
            if (timeExceeded) {
                LOG_DEBUGGER(FATAL) << "Time limit exceeded while waiting " << event;
                return false;
            }
        }
        action();
        return true;
    }

    static TestMap testMap_;
    static os::memory::Mutex eventMutex_;
    static os::memory::ConditionVariable eventCv_ GUARDED_BY(eventMutex_);
    static DebugEvent lastEvent_ GUARDED_BY(eventMutex_);
    static JSPtLocation lastEventLocation_ GUARDED_BY(eventMutex_);
    static os::memory::Mutex suspendMutex_;
    static os::memory::ConditionVariable suspendCv_ GUARDED_BY(suspendMutex_);
    static bool suspended_ GUARDED_BY(suspendMutex_);
    static bool initialized_ GUARDED_BY(eventMutex_);
};

std::ostream &operator<<(std::ostream &out, std::nullptr_t);

#define ASSERT_FAIL_(val1, val2, strval1, strval2, msg)                                    \
    do {                                                                                   \
        std::cerr << "Assertion failed at " << __FILE__ << ':' << __LINE__ << std::endl;   \
        std::cerr << "Expected that " strval1 " is " << (msg) << " " strval2 << std::endl; \
        std::cerr << "\t" strval1 ": " << (val1) << std::endl;                             \
        std::cerr << "\t" strval2 ": " << (val2) << std::endl;                             \
        std::abort();                                                                      \
    } while (0)

#define ASSERT_TRUE(cond)                                       \
    do {                                                        \
        auto res = (cond);                                      \
        if (!res) {                                             \
            ASSERT_FAIL_(res, true, #cond, "true", "equal to"); \
        }                                                       \
    } while (0)

#define ASSERT_FALSE(cond)                                        \
    do {                                                          \
        auto res = (cond);                                        \
        if (res) {                                                \
            ASSERT_FAIL_(res, false, #cond, "false", "equal to"); \
        }                                                         \
    } while (0)

#define ASSERT_EQ(lhs, rhs)                                   \
    do {                                                      \
        auto res1 = (lhs);                                    \
        decltype(res1) res2 = (rhs);                          \
        if (res1 != res2) {                                   \
            ASSERT_FAIL_(res1, res2, #lhs, #rhs, "equal to"); \
        }                                                     \
    } while (0)

#define ASSERT_NE(lhs, rhs)                                       \
    do {                                                          \
        auto res1 = (lhs);                                        \
        decltype(res1) res2 = (rhs);                              \
        if (res1 == res2) {                                       \
            ASSERT_FAIL_(res1, res2, #lhs, #rhs, "not equal to"); \
        }                                                         \
    } while (0)

#define ASSERT_STREQ(lhs, rhs)                                \
    do {                                                      \
        auto res1 = (lhs);                                    \
        decltype(res1) res2 = (rhs);                          \
        if (::strcmp(res1, res2) != 0) {                      \
            ASSERT_FAIL_(res1, res2, #lhs, #rhs, "equal to"); \
        }                                                     \
    } while (0)

#define ASSERT_SUCCESS(api_call)                                                                    \
    do {                                                                                            \
        auto error = api_call;                                                                      \
        if (error) {                                                                                \
            ASSERT_FAIL_(error.value().GetMessage(), "Success", "API call result", "Expected", ""); \
        }                                                                                           \
    } while (0)

#define ASSERT_EXITED()                                                                               \
    do {                                                                                              \
        bool res = TestUtil::WaitForExit();                                                           \
        if (!res) {                                                                                   \
            ASSERT_FAIL_(TestUtil::IsTestFinished(), true, "TestUtil::IsTestFinished()", "true", ""); \
        }                                                                                             \
    } while (0)

#define ASSERT_LOCATION_EQ(lhs, rhs)                                                 \
    do {                                                                             \
        ASSERT_EQ((lhs).GetJsPandaFile(), (rhs).GetJsPandaFile());                   \
        ASSERT_EQ((lhs).GetMethodId().GetOffset(), (rhs).GetMethodId().GetOffset()); \
        ASSERT_EQ((lhs).GetBytecodeOffset(), (rhs).GetBytecodeOffset());             \
    } while (0)
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_UTIL_H
