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

#ifndef LIBPANDABASE_UTILS_LOGGER_H
#define LIBPANDABASE_UTILS_LOGGER_H

#include "macros.h"
#include "os/error.h"
#include "os/mutex.h"
#include "os/thread.h"

#include <array>
#include <cstdint>

#include <bitset>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

#include <atomic>

#ifdef ENABLE_HILOG
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#include <hilog/log.h>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD003F00

#undef LOG_TAG
#define LOG_TAG "ArkCompiler"
#endif

namespace panda {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_COMPONENT_ELEM(D, NAME, STR) D(NAME, NAME, STR)

using FUNC_MOBILE_LOG_PRINT = int (*)(int, int, const char *, const char *, const char *);
constexpr int LOG_ID_MAIN = 0;
extern FUNC_MOBILE_LOG_PRINT mlog_buf_print;

namespace base_options {
class Options;
}  // namespace base_options

class Logger {
public:
#include <logger_enum_gen.h>

    using ComponentMask = std::bitset<Component::LAST>;

    enum PandaLog2MobileLog : int {
        UNKNOWN = 0,
        DEFAULT,
        VERBOSE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        SILENT,
    };

    class Buffer {
    public:
        constexpr static size_t BUFFER_SIZE = 4096;

    public:
        Buffer() : buffer {} {}

    public:
        const char *data() const noexcept
        {
            return buffer.data();
        }
        char *data() noexcept
        {
            return buffer.data();
        }

    public:
        constexpr size_t size() const noexcept
        {
            return BUFFER_SIZE;
        }

    public:
        // always overwrites buffer data
        Buffer &printf(const char *format, ...);

    public:
        friend std::ostream &operator<<(std::ostream &os, const Buffer &b)
        {
            return os << b.data();
        }

    private:
        std::array<char, BUFFER_SIZE> buffer;
    };

    class Message {
    public:
        Message(Level level, Component component, bool print_system_error)
            : level_(level), component_(component), print_system_error_(print_system_error)
        {
#ifndef NDEBUG
            Logger::LogNestingInc();
#endif
        }

        PANDA_PUBLIC_API ~Message();

        std::ostream &GetStream()
        {
            return stream_;
        }

    private:
        Level level_;
        Component component_;
        bool print_system_error_;
        std::ostringstream stream_;

        NO_COPY_SEMANTIC(Message);
        NO_MOVE_SEMANTIC(Message);
    };

    static void Initialize(const base_options::Options &options);

    PANDA_PUBLIC_API static void InitializeFileLogging(const std::string &log_file, Level level,
                                                       const ComponentMask &component_mask,
                                                       bool is_fast_logging = false);
#ifdef ENABLE_HILOG
    static void InitializeHiLogging(Level level, const ComponentMask &component_mask);
#endif

    PANDA_PUBLIC_API static void InitializeStdLogging(Level level, const ComponentMask &component_mask);

    static void InitializeDummyLogging(Level level = Level::DEBUG, const ComponentMask &component_mask = 0);

    static void Destroy();

    static void SetMobileLogPrintEntryPointByPtr(void *mlog_buf_print_ptr)
    {
        mlog_buf_print = reinterpret_cast<FUNC_MOBILE_LOG_PRINT>(mlog_buf_print_ptr);
    }

    static uint32_t GetLevelNumber(Logger::Level level);

    void WriteMobileLog(Level level, const char *component, const char *message)
    {
        if (mlog_buf_print == nullptr || !is_mlog_opened_) {
            return;
        }
        PandaLog2MobileLog mlog_level = PandaLog2MobileLog::UNKNOWN;
        switch (level) {
            case Level::DEBUG:
                mlog_level = PandaLog2MobileLog::DEBUG;
                break;
            case Level::INFO:
                mlog_level = PandaLog2MobileLog::INFO;
                break;
            case Level::ERROR:
                mlog_level = PandaLog2MobileLog::ERROR;
                break;
            case Level::FATAL:
                mlog_level = PandaLog2MobileLog::FATAL;
                break;
            case Level::WARNING:
                mlog_level = PandaLog2MobileLog::WARN;
                break;
            default:
                UNREACHABLE();
        }
        std::string panda_component = "Ark " + std::string(component);
        mlog_buf_print(LOG_ID_MAIN, mlog_level, panda_component.c_str(), "%s", message);
    }

    static bool IsLoggingOn(Level level, Component component)
    {
        return IsInitialized() && level <= logger->level_ &&
               (logger->component_mask_.test(component) || level == Level::FATAL);
    }

    static bool IsLoggingOnOrAbort(Level level, Component component)
    {
        if (IsLoggingOn(level, component)) {
            return true;
        }

        if (level == Level::FATAL) {
            std::abort();
        }

        return false;
    }

#ifndef NDEBUG
    PANDA_PUBLIC_API static void LogNestingInc();
    static void LogNestingDec();
    PANDA_PUBLIC_API static bool IsMessageSuppressed([[maybe_unused]] Level level,
                                                     [[maybe_unused]] Component component);
#endif

    static void Log(Level level, Component component, const std::string &str);

    static void Sync()
    {
        if (IsInitialized()) {
            logger->SyncOutputResource();
        }
    }

    static Level LevelFromString(std::string_view s);

    static ComponentMask ComponentMaskFromString(std::string_view s);

    static std::string StringfromDfxComponent(LogDfxComponent dfx_component);

    static void SetLevel(Level level)
    {
        ASSERT(IsInitialized());
        logger->level_ = level;
    }

    static Level GetLevel()
    {
        ASSERT(IsInitialized());
        return logger->level_;
    }

    static void EnableComponent(Component component)
    {
        ASSERT(IsInitialized());
        logger->component_mask_.set(component);
    }

    static void EnableComponent(const ComponentMask &component)
    {
        ASSERT(IsInitialized());
        logger->component_mask_ |= component;
    }

    static void DisableComponent(Component component)
    {
        ASSERT(IsInitialized());
        logger->component_mask_.reset(component);
    }

    static void ResetComponentMask()
    {
        ASSERT(IsInitialized());
        logger->component_mask_.reset();
    }

    static void SetMobileLogOpenFlag(bool is_mlog_opened)
    {
        ASSERT(IsInitialized());
        logger->is_mlog_opened_ = is_mlog_opened;
    }

    static bool IsInLevelList(std::string_view s);

    static bool IsInComponentList(std::string_view s);

    static void ProcessLogLevelFromString(std::string_view s);

    static void ProcessLogComponentsFromString(std::string_view s);

    static bool IsInitialized()
    {
        return logger != nullptr;
    }

protected:
    Logger(Level level, const ComponentMask &component_mask)
        : level_(level),
          component_mask_(component_mask)
#ifndef NDEBUG
          ,
          // Means all the LOGs are allowed just as usual
          nested_allowed_level_(Level::LAST)
#endif
    {
    }

    Logger(Level level, const ComponentMask &component_mask, [[maybe_unused]] Level nested_allowed_level)
        : level_(level),
          component_mask_(component_mask)
#ifndef NDEBUG
          ,
          nested_allowed_level_(nested_allowed_level)
#endif
    {
    }

    virtual void LogLineInternal(Level level, Component component, const std::string &str) = 0;

    /**
     * Flushes all the output buffers of LogLineInternal to the output resources
     * Sometimes nothinig shall be done, if LogLineInternal flushes everything by itself statelessl
     */
    virtual void SyncOutputResource() = 0;

    virtual ~Logger() = default;

    PANDA_PUBLIC_API static Logger *logger;

    static os::memory::Mutex mutex;

    static thread_local int nesting;

private:
    Level level_;
    ComponentMask component_mask_;
#ifndef NDEBUG
    // These are utilized by Fast* logger types.
    // For every thread, we trace events of staring shifting to a log (<<) and finishing doing it,
    // incrementing a log invocation depth variable bound to a thread, or decrementing it correspondingly.
    // Such variables we're doing as thread-local.
    // All the LOGs with levels < nested_allowed_level_ are only allowed to have depth of log == 1
    Level nested_allowed_level_;  // Log level to suppress LOG triggering within << to another LOG
#endif
    bool is_mlog_opened_ {true};

    NO_COPY_SEMANTIC(Logger);
    NO_MOVE_SEMANTIC(Logger);
};

static Logger::ComponentMask LoggerComponentMaskAll = ~Logger::ComponentMask();

class FileLogger : public Logger {
protected:
    FileLogger(std::ofstream &&stream, Level level, const ComponentMask &component_mask)
        : Logger(level, component_mask), stream_(std::forward<std::ofstream>(stream))
    {
    }

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override {}

    ~FileLogger() override = default;

    NO_COPY_SEMANTIC(FileLogger);
    NO_MOVE_SEMANTIC(FileLogger);

private:
    std::ofstream stream_;

    friend Logger;
};

class FastFileLogger : public Logger {
protected:
    // Uses advanced Logger constructor, so we tell to suppress all nested messages below WARNING severity
    FastFileLogger(std::ofstream &&stream, Level level, const ComponentMask &component_mask)
        : Logger(level, component_mask, Logger::Level::WARNING), stream_(std::forward<std::ofstream>(stream))
    {
    }

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override;

    ~FastFileLogger() override = default;

    NO_COPY_SEMANTIC(FastFileLogger);
    NO_MOVE_SEMANTIC(FastFileLogger);

private:
    std::ofstream stream_;

    friend Logger;
};

#ifdef ENABLE_HILOG
class HiLogger : public Logger {
protected:
    HiLogger(Level level, const ComponentMask &component_mask) : Logger(level, component_mask) {}

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override {}

    ~HiLogger() override = default;

    NO_COPY_SEMANTIC(HiLogger);
    NO_MOVE_SEMANTIC(HiLogger);

private:
    std::ostringstream stream_;

    friend Logger;
};
#endif

class StderrLogger : public Logger {
private:
    StderrLogger(Level level, const ComponentMask &component_mask) : Logger(level, component_mask) {}

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override {}

    friend Logger;

    ~StderrLogger() override = default;

    NO_COPY_SEMANTIC(StderrLogger);
    NO_MOVE_SEMANTIC(StderrLogger);
};

class DummyLogger : public Logger {
private:
    DummyLogger(Level level, const ComponentMask &component_mask) : Logger(level, component_mask) {}

    void LogLineInternal([[maybe_unused]] Level level, [[maybe_unused]] Component component,
                         [[maybe_unused]] const std::string &str) override
    {
    }

    void SyncOutputResource() override {}

    friend Logger;

    ~DummyLogger() override = default;

    NO_COPY_SEMANTIC(DummyLogger);
    NO_MOVE_SEMANTIC(DummyLogger);
};

class DummyStream {
public:
    explicit operator bool() const
    {
        return true;
    }
};

template <class T>
DummyStream operator<<(DummyStream s, [[maybe_unused]] const T &v)
{
    return s;
}

class LogOnceHelper {
public:
    bool IsFirstCall()
    {
        flag_ >>= 1U;
        return flag_ != 0;
    }

private:
    uint8_t flag_ = 0x03;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ONCE_HELPER() static LogOnceHelper MERGE_WORDS(log_once_helper, __LINE__);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ONCE(level, component) \
    LOG_ONCE_HELPER()              \
    MERGE_WORDS(log_once_helper, __LINE__).IsFirstCall() && LOG(level, component)

#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_SUPPRESSION_CHECK(level, component) \
    !panda::Logger::IsMessageSuppressed(panda::Logger::Level::level, panda::Logger::Component::component)
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_SUPPRESSION_CHECK(level, component) true
#endif

// Explicit namespace is specified to allow using the logger out of panda namespace.
// For example, in the main function.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG(level, component, p)                                                                          \
    panda::Logger::IsLoggingOnOrAbort(panda::Logger::Level::level, panda::Logger::Component::component) && \
        _LOG_SUPPRESSION_CHECK(level, component) &&                                                        \
        panda::Logger::Message(panda::Logger::Level::level, panda::Logger::Component::component, p).GetStream()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_TRANSFORM(level, component) _LOG_##level(component, false)
#define LOG(level, component) LOG_TRANSFORM(level, component)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_LOG_STREAM(level, component) \
    panda::Logger::Message(panda::Logger::Level::level, panda::Logger::Component::component, false).GetStream()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PLOG(level, component) _LOG_##level(component, true)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_IF(cond, level, component) (cond) && LOG(level, component)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PLOG_IF(cond, level, component) (cond) && PLOG(level, component)

#ifndef NDEBUG

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_DEBUG(component, p) _LOG(DEBUG, component, p)

#else

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_DEBUG(component, p) false && panda::DummyStream()

#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_INFO(component, p) _LOG(INFO, component, p)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_WARNING(component, p) _LOG(WARNING, component, p)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_ERROR(component, p) _LOG(ERROR, component, p)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define _LOG_FATAL(component, p) _LOG(FATAL, component, p)

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_LOGGER_H
