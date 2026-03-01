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

#include "utils/logger.h"
#include "os/filesystem.h"
#include "utils/string_helpers.h"
#include "generated/base_options.h"

namespace panda {

Logger *Logger::logger = nullptr;
thread_local int Logger::nesting = 0;

#include <logger_impl_gen.inc>

void Logger::Initialize(const base_options::Options &options)
{
    panda::Logger::ComponentMask component_mask;
    auto load_components = [&component_mask](auto components) {
        for (const auto &s : components) {
            component_mask |= Logger::ComponentMaskFromString(s);
        }
    };
    Level level = Level::LAST;

    if (options.WasSetLogFatal()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        load_components(options.GetLogFatal());
        level = Level::FATAL;
    } else if (options.WasSetLogError()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        load_components(options.GetLogError());
        level = Level::ERROR;
    } else if (options.WasSetLogWarning()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        load_components(options.GetLogWarning());
        level = Level::WARNING;
    } else if (options.WasSetLogInfo()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        load_components(options.GetLogInfo());
        level = Level::INFO;
    } else if (options.WasSetLogDebug()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        load_components(options.GetLogDebug());
        level = Level::DEBUG;
    } else {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        load_components(options.GetLogComponents());
        level = Logger::LevelFromString(options.GetLogLevel());
    }

#ifdef ENABLE_HILOG
    Logger::InitializeHiLogging(level, component_mask);
    return;
#endif

    if (options.GetLogStream() == "std") {
        Logger::InitializeStdLogging(level, component_mask);
    } else if (options.GetLogStream() == "file" || options.GetLogStream() == "fast-file") {
        const std::string &file_name = options.GetLogFile();
        Logger::InitializeFileLogging(file_name, level, component_mask, options.GetLogStream() == "fast-file");
    } else if (options.GetLogStream() == "dummy") {
        Logger::InitializeDummyLogging(level, component_mask);
    } else {
        UNREACHABLE();
    }
}

#ifndef NDEBUG
/**
 * In debug builds this function allowes or disallowes proceeding with actual logging (i.e. creating Message{})
 */
/* static */
bool Logger::IsMessageSuppressed([[maybe_unused]] Level level, [[maybe_unused]] Component component)
{
    // Allowing only to log if it's not a nested log, or it's nested and it's severity is suitable
    return level >= Logger::logger->nested_allowed_level_ && nesting > 0;
}

/**
 * Increases log nesting (i.e. depth, or how many instances of Message{} is active atm) in a given thread
 */
/* static */
void Logger::LogNestingInc()
{
    nesting++;
}

/**
 * Decreases log nesting (i.e. depth, or how many instances of Message{} is active atm) in a given thread
 */
/* static */
void Logger::LogNestingDec()
{
    nesting--;
}
#endif  // NDEBUG

auto Logger::Buffer::printf(const char *format, ...) -> Buffer &
{
    va_list args;
    va_start(args, format);  // NOLINT(cppcoreguidelines-pro-type-vararg)

    [[maybe_unused]] int put = vsnprintf_s(this->data(), this->size(), this->size() - 1, format, args);
    ASSERT(put >= 0 && static_cast<size_t>(put) < BUFFER_SIZE);

    va_end(args);
    return *this;
}

os::memory::Mutex Logger::mutex;  // NOLINT(fuchsia-statically-constructed-objects)
FUNC_MOBILE_LOG_PRINT mlog_buf_print = nullptr;

Logger::Message::~Message()
{
    if (print_system_error_) {
        stream_ << ": " << os::Error(errno).ToString();
    }

    Logger::Log(level_, component_, stream_.str());
#ifndef NDEBUG
    panda::Logger::LogNestingDec();
#endif

    if (level_ == Level::FATAL) {
        std::cerr << "FATAL ERROR" << std::endl;
        std::cerr << "Backtrace [tid=" << os::thread::GetCurrentThreadId() << "]:\n";
        PrintStack(std::cerr);
        std::abort();
    }
}

/* static */
void Logger::Log(Level level, Component component, const std::string &str)
{
    if (!IsLoggingOn(level, component)) {
        return;
    }

    os::memory::LockHolder<os::memory::Mutex> lock(mutex);
    if (!IsLoggingOn(level, component)) {
        return;
    }

    size_t nl = str.find('\n');
    if (nl == std::string::npos) {
        logger->LogLineInternal(level, component, str);
        logger->WriteMobileLog(level, GetComponentTag(component), str.c_str());
    } else {
        size_t i = 0;
        while (nl != std::string::npos) {
            std::string line = str.substr(i, nl - i);
            logger->LogLineInternal(level, component, line);
            logger->WriteMobileLog(level, GetComponentTag(component), line.c_str());
            i = nl + 1;
            nl = str.find('\n', i);
        }

        logger->LogLineInternal(level, component, str.substr(i));
        logger->WriteMobileLog(level, GetComponentTag(component), str.substr(i).c_str());
    }
}

/* static */
std::string GetPrefix(Logger::Level level, Logger::Component component)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return helpers::string::Format("[TID %06x] %s/%s: ", os::thread::GetCurrentThreadId(), GetLevelTag(level),
                                   GetComponentTag(component));
}

/* static */
void Logger::InitializeFileLogging(const std::string &log_file, Level level, const ComponentMask &component_mask,
                                   bool is_fast_logging)
{
    if (IsInitialized()) {
        return;
    }

    os::memory::LockHolder<os::memory::Mutex> lock(mutex);

    if (IsInitialized()) {
        return;
    }

    auto path = os::GetAbsolutePath(log_file);
    if (path == "") {
        logger = new StderrLogger(level, component_mask);
        std::string msg = helpers::string::Format("Fallback to stderr logging: resolve file path error");
        logger->LogLineInternal(Level::ERROR, Component::COMMON, msg);
        return;
    }

    std::ofstream stream(path);
    if (stream) {
        if (is_fast_logging) {
            logger = new FastFileLogger(std::move(stream), level, component_mask);
        } else {
            logger = new FileLogger(std::move(stream), level, component_mask);
        }
    } else {
        logger = new StderrLogger(level, component_mask);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        std::string msg = helpers::string::Format("Fallback to stderr logging: cannot open log file '%s': %s",
                                                  log_file.c_str(), os::Error(errno).ToString().c_str());
        logger->LogLineInternal(Level::ERROR, Component::COMMON, msg);
    }
}

#ifdef ENABLE_HILOG
/* static */
void Logger::InitializeHiLogging(Level level, const ComponentMask &component_mask)
{
    if (IsInitialized()) {
        return;
    }

    {
        os::memory::LockHolder<os::memory::Mutex> lock(mutex);

        if (IsInitialized()) {
            return;
        }

        logger = new HiLogger(level, component_mask);
    }
}
#endif

/* static */
void Logger::InitializeStdLogging(Level level, const ComponentMask &component_mask)
{
    if (IsInitialized()) {
        return;
    }

    {
        os::memory::LockHolder<os::memory::Mutex> lock(mutex);

        if (IsInitialized()) {
            return;
        }

        logger = new StderrLogger(level, component_mask);
    }
}

/* static */
void Logger::InitializeDummyLogging(Level level, const ComponentMask &component_mask)
{
    if (IsInitialized()) {
        return;
    }

    {
        os::memory::LockHolder<os::memory::Mutex> lock(mutex);

        if (IsInitialized()) {
            return;
        }

        logger = new DummyLogger(level, component_mask);
    }
}

/* static */
void Logger::Destroy()
{
    if (!IsInitialized()) {
        return;
    }

    Logger *l = nullptr;

    {
        os::memory::LockHolder<os::memory::Mutex> lock(mutex);

        if (!IsInitialized()) {
            return;
        }

        l = logger;
        logger = nullptr;
    }

    delete l;
}

/* static */
void Logger::ProcessLogLevelFromString(std::string_view s)
{
    if (Logger::IsInLevelList(s)) {
        Logger::SetLevel(Logger::LevelFromString(s));
    } else {
        LOG(ERROR, RUNTIME) << "Unknown level " << s;
    }
}

/* static */
void Logger::ProcessLogComponentsFromString(std::string_view s)
{
    Logger::ResetComponentMask();
    size_t last_pos = s.find_first_not_of(',', 0);
    size_t pos = s.find(',', last_pos);
    while (last_pos != std::string_view::npos) {
        std::string_view component_str = s.substr(last_pos, pos - last_pos);
        last_pos = s.find_first_not_of(',', pos);
        pos = s.find(',', last_pos);
        if (Logger::IsInComponentList(component_str)) {
            Logger::EnableComponent(Logger::ComponentMaskFromString(component_str));
        } else {
            LOG(ERROR, RUNTIME) << "Unknown component " << component_str;
        }
    }
}

void FileLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    std::string prefix = GetPrefix(level, component);
    stream_ << prefix << str << std::endl << std::flush;
}

void FastFileLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    std::string prefix = GetPrefix(level, component);
    stream_ << prefix << str << '\n';
}

#ifdef ENABLE_HILOG
void HiLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    std::string prefix = GetPrefix(level, component);
    stream_ << prefix << str;
    switch (level) {
        case Level::DEBUG:
            HILOG_DEBUG(LOG_CORE, "%{public}s", stream_.str().c_str());
            break;
        case Level::INFO:
            HILOG_INFO(LOG_CORE, "%{public}s", stream_.str().c_str());
            break;
        case Level::ERROR:
            HILOG_ERROR(LOG_CORE, "%{public}s", stream_.str().c_str());
            break;
        case Level::FATAL:
            HILOG_FATAL(LOG_CORE, "%{public}s", stream_.str().c_str());
            break;
        case Level::WARNING:
            HILOG_WARN(LOG_CORE, "%{public}s", stream_.str().c_str());
            break;
        default:
            UNREACHABLE();
    }
}
#endif

void StderrLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    std::string prefix = GetPrefix(level, component);
    std::cerr << prefix << str << std::endl << std::flush;
}

void FastFileLogger::SyncOutputResource()
{
    stream_ << std::flush;
}

}  // namespace panda
