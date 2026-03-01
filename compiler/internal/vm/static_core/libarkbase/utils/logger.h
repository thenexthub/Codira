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

#ifndef PANDA_LIBPANDABASE_UTILS_LOGGER_H_
#define PANDA_LIBPANDABASE_UTILS_LOGGER_H_

#include "libarkbase/macros.h"
#include "libarkbase/globals.h"
#include "libarkbase/os/error.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/dfx.h"

#include <cstdio>
#include <cstdint>

#include <bitset>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

#include <atomic>
#include <array>

namespace ark {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_COMPONENT_ELEM(D, NAME, STR) D(NAME, NAME, STR)

using FuncMobileLogPrint = int (*)(int, int, const char *, const char *, const char *);
inline constexpr int LOG_ID_MAIN = 0;
PANDA_PUBLIC_API extern FuncMobileLogPrint g_mlogBufPrint;

namespace logger {
class Options;
}  // namespace logger

class Logger {
public:
#include <libarkbase/generated/logger_enum_gen.h>

    using ComponentMask = std::bitset<Component::LAST>;
    using Formatter = void (*)(FILE *stream, int level, const char *component, const char *message);

    // NOTE:
    //  The Level enumeration is represented as int in the public interface,
    //  so we must provide it unchanged in the future.
    static_assert(static_cast<int>(Level::FATAL) == 0);
    static_assert(static_cast<int>(Level::ERROR) == 1);
    static_assert(static_cast<int>(Level::WARNING) == 2);
    static_assert(static_cast<int>(Level::INFO) == 3);
    static_assert(static_cast<int>(Level::DEBUG) == 4);
    static_assert(static_cast<int>(Level::LAST) == 5);

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
        Buffer() = default;

    public:
        const char *Data() const noexcept
        {
            return buffer_.data();
        }
        char *Data() noexcept
        {
            return buffer_.data();
        }

    public:
        constexpr size_t Size() const noexcept
        {
            return BUFFER_SIZE;
        }

    public:
        // always overwrites buffer data
        PANDA_PUBLIC_API Buffer &Printf(const char *format, ...);

    public:
        friend std::ostream &operator<<(std::ostream &os, const Buffer &b)
        {
            return os << b.Data();
        }

    private:
        std::array<char, BUFFER_SIZE> buffer_ {};
    };

    class Message {
    public:
        PANDA_PUBLIC_API Message(Level level, Component component, bool printSystemError)
            : level_(level), component_(component), printSystemError_(printSystemError)
        {
#ifndef NDEBUG
            Logger::LogNestingInc();
#endif
        }

        PANDA_PUBLIC_API ~Message();

        PANDA_PUBLIC_API std::ostream &GetStream()
        {
            return stream_;
        }

    private:
        Level level_;
        Component component_;
        bool printSystemError_;
        std::ostringstream stream_;

        NO_COPY_SEMANTIC(Message);
        NO_MOVE_SEMANTIC(Message);
    };

    PANDA_PUBLIC_API static void Initialize(const logger::Options &options, Formatter formatter = nullptr);

    PANDA_PUBLIC_API static void InitializeFileLogging(const std::string &logFile, Level level,
                                                       ComponentMask componentMask, bool isFastLogging = false,
                                                       Formatter formatter = nullptr);

    PANDA_PUBLIC_API static void InitializeStdLogging(Level level, ComponentMask componentMask,
                                                      Formatter formatter = nullptr);

    PANDA_PUBLIC_API static void InitializeDummyLogging(Level level = Level::DEBUG, ComponentMask componentMask = 0,
                                                        Formatter formatter = nullptr);

    PANDA_PUBLIC_API static void Destroy();

    static void SetMobileLogPrintEntryPointByPtr(void *mlogBufPrintPtr)
    {
        g_mlogBufPrint = reinterpret_cast<FuncMobileLogPrint>(mlogBufPrintPtr);
    }

    static uint32_t GetLevelNumber(Logger::Level level);

    void WriteMobileLog(Level level, const char *component, const char *message)
    {
        if (g_mlogBufPrint == nullptr || !isMlogOpened_) {
            return;
        }
        PandaLog2MobileLog mlogLevel = PandaLog2MobileLog::UNKNOWN;
        switch (level) {
            case Level::DEBUG:
                mlogLevel = PandaLog2MobileLog::DEBUG;
                break;
            case Level::INFO:
                mlogLevel = PandaLog2MobileLog::INFO;
                break;
            case Level::ERROR:
                mlogLevel = PandaLog2MobileLog::ERROR;
                break;
            case Level::FATAL:
                mlogLevel = PandaLog2MobileLog::FATAL;
                break;
            case Level::WARNING:
                mlogLevel = PandaLog2MobileLog::WARN;
                break;
            default:
                UNREACHABLE();
        }
        std::string pandaComponent = "Ark " + std::string(component);
        g_mlogBufPrint(LOG_ID_MAIN, mlogLevel, pandaComponent.c_str(), "%s", message);
    }

    static bool IsLoggingOn(Level level, Component component)
    {
        return IsInitialized() && level <= logger_->level_ &&
               (logger_->componentMask_.test(component) || level == Level::FATAL);
    }

    PANDA_PUBLIC_API static bool IsLoggingOnOrAbort(Level level, Component component)
    {
        if (IsLoggingOn(level, component)) {
            return true;
        }

        if (level == Level::FATAL) {
            std::abort();  // CC-OFF(G.FUU.08) fatal error
        }

        return false;
    }

#ifndef NDEBUG
    PANDA_PUBLIC_API static void LogNestingInc();
    static void LogNestingDec();
    PANDA_PUBLIC_API static bool IsMessageSuppressed([[maybe_unused]] Level level,
                                                     [[maybe_unused]] Component component);
#endif

    static bool IsLoggingDfxOn()
    {
        if (!DfxController::IsInitialized() || !IsInitialized()) {
            return false;
        }
        return (DfxController::GetOptionValue(DfxOptionHandler::DFXLOG) == 1);
    }

    static void Log(Level level, Component component, const std::string &str);

    static void Sync()
    {
        if (IsInitialized()) {
            logger_->SyncOutputResource();
        }
    }

    PANDA_PUBLIC_API static Level LevelFromString(std::string_view s);

    PANDA_PUBLIC_API static ComponentMask ComponentMaskFromString(std::string_view s);

    static std::string StringfromDfxComponent(LogDfxComponent dfxComponent);

    static void SetLevel(Level level)
    {
        ASSERT(IsInitialized());
        logger_->level_ = level;
    }

    static Level GetLevel()
    {
        ASSERT(IsInitialized());
        return logger_->level_;
    }

    static void EnableComponent(Component component)
    {
        ASSERT(IsInitialized());
        logger_->componentMask_.set(component);
    }

    static void EnableComponent(ComponentMask component)
    {
        ASSERT(IsInitialized());
        logger_->componentMask_ |= component;
    }

    static void DisableComponent(Component component)
    {
        ASSERT(IsInitialized());
        logger_->componentMask_.reset(component);
    }

    static void ResetComponentMask()
    {
        ASSERT(IsInitialized());
        logger_->componentMask_.reset();
    }

    static void SetMobileLogOpenFlag(bool isMlogOpened)
    {
        ASSERT(IsInitialized());
        logger_->isMlogOpened_ = isMlogOpened;
    }

    static bool IsInLevelList(std::string_view s);

    static bool IsInComponentList(std::string_view s);

    PANDA_PUBLIC_API static void ProcessLogLevelFromString(std::string_view s);

    PANDA_PUBLIC_API static void ProcessLogComponentsFromString(std::string_view s);

    static bool IsInitialized()
    {
        return logger_ != nullptr;
    }

protected:
    Logger(FILE *stream, Level level, ComponentMask componentMask, Formatter formatter)
        : stream_(stream),
          formatter_(formatter),
          level_(level),
          componentMask_(componentMask)
#ifndef NDEBUG
          ,
          nestedAllowedLevel_(Level::LAST)
    // Means all the LOGs are allowed just as usual
#endif
    {
    }

    Logger(FILE *stream, Level level, ComponentMask componentMask, Formatter formatter,
           [[maybe_unused]] Level nestedAllowedLevel)
        : stream_(stream),
          formatter_(formatter),
          level_(level),
          componentMask_(componentMask)
#ifndef NDEBUG
          ,
          nestedAllowedLevel_(nestedAllowedLevel)
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

    PANDA_PUBLIC_API static Logger *logger_;

    static os::memory::Mutex mutex_;

    static thread_local int nesting_;

protected:
    FILE *stream_;         // NOLINT(misc-non-private-member-variables-in-classes)
    Formatter formatter_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    static void DefaultFormatter(FILE *stream, int level, const char *component, const char *message);

    Level level_;
    ComponentMask componentMask_;
#ifndef NDEBUG
    // These are utilized by Fast* logger types.
    // For every thread, we trace events of staring shifting to a log (<<) and finishing doing it,
    // incrementing a log invocation depth variable bound to a thread, or decrementing it correspondingly.
    // Such variables we're doing as thread-local.
    // All the LOGs with levels < nested_allowed_level_ are only allowed to have depth of log == 1
    Level nestedAllowedLevel_;  // Log level to suppress LOG triggering within << to another LOG
#endif
    bool isMlogOpened_ {true};

    NO_COPY_SEMANTIC(Logger);
    NO_MOVE_SEMANTIC(Logger);
};

inline constexpr uint64_t MASK_ALL = 0xFFFF'FFFF'FFFF'FFFFUL;
static_assert(sizeof(MASK_ALL) * BITS_PER_BYTE > Logger::Component::LAST);
inline constexpr Logger::ComponentMask LOGGER_COMPONENT_MASK_ALL = Logger::ComponentMask(MASK_ALL);

class FileLogger : public Logger {
protected:
    FileLogger(FILE *stream, Level level, ComponentMask componentMask, Formatter formatter)
        : Logger(stream, level, componentMask, formatter)
    {
    }

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override {}

    ~FileLogger() override
    {
        if (stream_ != stdout && stream_ != stderr) {
            fclose(stream_);
        }
    }
    NO_COPY_SEMANTIC(FileLogger);
    NO_MOVE_SEMANTIC(FileLogger);

    friend Logger;
};

class FastFileLogger : public Logger {
protected:
    // Uses advanced Logger constructor, so we tell to suppress all nested messages below WARNING severity
    FastFileLogger(FILE *stream, Level level, ComponentMask componentMask, Formatter formatter)
        : Logger(stream, level, componentMask, formatter, Logger::Level::WARNING)
    {
    }

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override;

    ~FastFileLogger() override
    {
        if (stream_ != stdout && stream_ != stderr) {
            fclose(stream_);
        }
    }

    NO_COPY_SEMANTIC(FastFileLogger);
    NO_MOVE_SEMANTIC(FastFileLogger);

private:
    friend Logger;
};

class StderrLogger : public Logger {
private:
    StderrLogger(Level level, ComponentMask componentMask, Formatter formatter)
        : Logger(stderr, level, componentMask, formatter)
    {
    }

    void LogLineInternal(Level level, Component component, const std::string &str) override;
    void SyncOutputResource() override {}

    friend Logger;

    ~StderrLogger() override = default;

    NO_COPY_SEMANTIC(StderrLogger);
    NO_MOVE_SEMANTIC(StderrLogger);
};

class DummyLogger : public Logger {
private:
    DummyLogger(Level level, ComponentMask componentMask, Formatter formatter)
        : Logger(nullptr, level, componentMask, formatter)
    {
    }

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
#define LOG_ONCE_HELPER() static LogOnceHelper MERGE_WORDS(log_once_helper, __LINE__)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ONCE(level, component) \
    LOG_ONCE_HELPER();             \
    MERGE_WORDS(log_once_helper, __LINE__).IsFirstCall() && LOG(level, component)

#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_SUPPRESSION_CHECK(level, component) \
    /* CC-OFFNXT(G.PRE.02) namespace member */  \
    !ark::Logger::IsMessageSuppressed(ark::Logger::Level::level, ark::Logger::Component::component)
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_SUPPRESSION_CHECK(level, component) true
#endif

// Explicit namespace is specified to allow using the logger out of panda namespace.
// For example, in the main function.
// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define IMPL_LOG(level, component, p)                                                                \
    /* CC-OFFNXT(G.PRE.02) namespace member */                                                       \
    ark::Logger::IsLoggingOnOrAbort(ark::Logger::Level::level, ark::Logger::Component::component) && \
        LOG_SUPPRESSION_CHECK(level, component) &&                                                   \
        /* CC-OFFNXT(G.PRE.02) namespace member */                                                   \
        ark::Logger::Message(ark::Logger::Level::level, ark::Logger::Component::component, p).GetStream()
// clang-format on

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG(level, component) LOG_##level(component, false)

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DFX(dfx_component)                                                               \
    ark::Logger::IsLoggingDfxOn() &&                                                         \
        ark::Logger::Message(ark::Logger::Level::ERROR, ark::Logger::DFX, false).GetStream() \
            /* CC-OFFNXT(G.PRE.02) namespace member */                                       \
            << ark::Logger::StringfromDfxComponent(ark::Logger::LogDfxComponent::dfx_component) << " log:"
// clang-format on

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_LOG_STREAM(level, component)       \
    /* CC-OFFNXT(G.PRE.02) namespace member */ \
    ark::Logger::Message(ark::Logger::Level::level, ark::Logger::Component::component, false).GetStream()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PLOG(level, component) LOG_##level(component, true)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_IF(cond, level, component) (cond) && LOG(level, component)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PLOG_IF(cond, level, component) (cond) && PLOG(level, component)

#ifndef NDEBUG

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG(component, p) IMPL_LOG(DEBUG, component, p)

#else

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG(component, p) false && ark::DummyStream()

#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INFO(component, p) IMPL_LOG(INFO, component, p)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_WARNING(component, p) IMPL_LOG(WARNING, component, p)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ERROR(component, p) IMPL_LOG(ERROR, component, p)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_FATAL(component, p) IMPL_LOG(FATAL, component, p)

}  // namespace ark

#endif  // PANDA_LIBPANDABASE_UTILS_LOGGER_H_
