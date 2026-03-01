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

#include "logger.h"
#include "libarkbase/os/thread.h"
#include "string_helpers.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <string_view>

namespace ark {

Logger *Logger::logger_ = nullptr;
thread_local int Logger::nesting_ = 0;

#include <libarkbase/generated/logger_impl_gen.inc>

void Logger::Initialize(const logger::Options &options, Formatter formatter)
{
    if (formatter == nullptr) {
        formatter = DefaultFormatter;
    }

    ark::Logger::ComponentMask componentMask;
    auto loadComponents = [&componentMask](auto components) {
        for (const auto &s : components) {
            componentMask |= Logger::ComponentMaskFromString(s);
        }
    };
    Level level = Level::LAST;

    if (options.WasSetLogFatal()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        loadComponents(options.GetLogFatal());
        level = Level::FATAL;
    } else if (options.WasSetLogError()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        loadComponents(options.GetLogError());
        level = Level::ERROR;
    } else if (options.WasSetLogWarning()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        loadComponents(options.GetLogWarning());
        level = Level::WARNING;
    } else if (options.WasSetLogInfo()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        loadComponents(options.GetLogInfo());
        level = Level::INFO;
    } else if (options.WasSetLogDebug()) {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        loadComponents(options.GetLogDebug());
        level = Level::DEBUG;
    } else {
        ASSERT_PRINT(level == Level::LAST, "There are conflicting logger options");
        loadComponents(options.GetLogComponents());
        level = Logger::LevelFromString(options.GetLogLevel());
    }

    if (options.GetLogStream() == "std") {
        Logger::InitializeStdLogging(level, componentMask, formatter);
    } else if (options.GetLogStream() == "file" || options.GetLogStream() == "fast-file") {
        const std::string &fileName = options.GetLogFile();
        Logger::InitializeFileLogging(fileName, level, componentMask, options.GetLogStream() == "fast-file", formatter);
    } else if (options.GetLogStream() == "dummy") {
        Logger::InitializeDummyLogging(level, componentMask, formatter);
    } else {
        UNREACHABLE();
    }
}

#ifndef NDEBUG
/// In debug builds this function allowes or disallowes proceeding with actual logging (i.e. creating Message{})
/* static */
bool Logger::IsMessageSuppressed([[maybe_unused]] Level level, [[maybe_unused]] Component component)
{
    // Allowing only to log if it's not a nested log, or it's nested and it's severity is suitable
    return level >= Logger::logger_->nestedAllowedLevel_ && nesting_ > 0;
}

/// Increases log nesting (i.e. depth, or how many instances of Message{} is active atm) in a given thread
/* static */
void Logger::LogNestingInc()
{
    nesting_++;
}

/// Decreases log nesting (i.e. depth, or how many instances of Message{} is active atm) in a given thread
/* static */
void Logger::LogNestingDec()
{
    nesting_--;
}
#endif  // NDEBUG

auto Logger::Buffer::Printf(const char *format, ...) -> Buffer &
{
    va_list args;            // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(args, format);  // NOLINT(cppcoreguidelines-pro-type-vararg)

    [[maybe_unused]] int put = vsnprintf_s(this->Data(), this->Size(), this->Size() - 1, format, args);
    ASSERT(put >= 0 && static_cast<size_t>(put) < BUFFER_SIZE);

    va_end(args);
    return *this;
}

os::memory::Mutex Logger::mutex_;  // NOLINT(fuchsia-statically-constructed-objects)
FuncMobileLogPrint g_mlogBufPrint = nullptr;

Logger::Message::~Message()
{
    if (printSystemError_) {
        stream_ << ": " << os::Error(errno).ToString();
    }
    Logger::Log(level_, component_, stream_.str());
#ifndef NDEBUG
    ark::Logger::LogNestingDec();
#endif

    if (level_ == Level::FATAL) {
        std::cerr << "FATAL ERROR" << std::endl;
        std::cerr << "Backtrace [tid=" << os::thread::GetCurrentThreadId() << "]:\n";
        PrintStack(std::cerr);
        std::abort();  // CC-OFF(G.STD.16) fatal error
    }
}

/* static */
void Logger::Log(Level level, Component component, const std::string &str)
{
    if (!IsLoggingOn(level, component)) {
        return;
    }

    os::memory::LockHolder lock(mutex_);
    if (!IsLoggingOn(level, component)) {
        return;
    }

    size_t nl = str.find('\n');
    if (nl == std::string::npos) {
        logger_->LogLineInternal(level, component, str);
        logger_->WriteMobileLog(level, GetComponentTag(component), str.c_str());
    } else {
        size_t i = 0;
        while (nl != std::string::npos) {
            std::string line = str.substr(i, nl - i);
            logger_->LogLineInternal(level, component, line);
            logger_->WriteMobileLog(level, GetComponentTag(component), line.c_str());
            i = nl + 1;
            nl = str.find('\n', i);
        }

        logger_->LogLineInternal(level, component, str.substr(i));
        logger_->WriteMobileLog(level, GetComponentTag(component), str.substr(i).c_str());
    }
}

static std::string GetPrefix(Logger::Level level, const char *component)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return helpers::string::Format("[TID %06x] %s/%s: ", os::thread::GetCurrentThreadId(), GetLevelTag(level),
                                   component);
}

/* static */
void Logger::InitializeFileLogging(const std::string &logFile, Level level, ComponentMask componentMask,
                                   bool isFastLogging, Formatter formatter)
{
    if (formatter == nullptr) {
        formatter = DefaultFormatter;
    }

    if (IsInitialized()) {
        return;
    }

    os::memory::LockHolder lock(mutex_);

    if (IsInitialized()) {
        return;
    }

    FILE *file = nullptr;
    if (!logFile.empty()) {
#ifdef PANDA_TARGET_WINDOWS
        constexpr char const *MODE = "w";
#else
        constexpr char const *MODE = "we";
#endif
        file = fopen(logFile.c_str(), MODE);
    }
    if (file != nullptr) {
        if (isFastLogging) {
            logger_ = new FastFileLogger(file, level, componentMask, formatter);
        } else {
            logger_ = new FileLogger(file, level, componentMask, formatter);
        }
    } else {
        logger_ = new StderrLogger(level, componentMask, formatter);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        std::string msg = helpers::string::Format("Fallback to stderr logging: cannot open log file '%s': %s",
                                                  logFile.c_str(), os::Error(errno).ToString().c_str());
        logger_->LogLineInternal(Level::ERROR, Component::COMMON, msg);
    }
#ifdef PANDA_TARGET_UNIX
    if (DfxController::IsInitialized() && DfxController::GetOptionValue(DfxOptionHandler::MOBILE_LOG) == 0) {
        Logger::SetMobileLogOpenFlag(false);
    }
#endif
}

/* static */
void Logger::InitializeStdLogging(Level level, ComponentMask componentMask, Formatter formatter)
{
    if (formatter == nullptr) {
        formatter = DefaultFormatter;
    }
    if (IsInitialized()) {
        return;
    }

    {
        os::memory::LockHolder lock(mutex_);

        if (IsInitialized()) {
            return;
        }
        logger_ = new StderrLogger(level, componentMask, formatter);
#ifdef PANDA_TARGET_UNIX
        if (DfxController::IsInitialized() && DfxController::GetOptionValue(DfxOptionHandler::MOBILE_LOG) == 0) {
            Logger::SetMobileLogOpenFlag(false);
        }
#endif
    }
}

/* static */
void Logger::InitializeDummyLogging(Level level, ComponentMask componentMask, Formatter formatter)
{
    if (formatter == nullptr) {
        formatter = DefaultFormatter;
    }

    if (IsInitialized()) {
        return;
    }

    {
        os::memory::LockHolder lock(mutex_);

        if (IsInitialized()) {
            return;
        }

        logger_ = new DummyLogger(level, componentMask, formatter);
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
        os::memory::LockHolder lock(mutex_);

        if (!IsInitialized()) {
            return;
        }

        l = logger_;
        logger_ = nullptr;
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
    size_t lastPos = s.find_first_not_of(',', 0);
    size_t pos = s.find(',', lastPos);
    while (lastPos != std::string_view::npos) {
        std::string_view componentStr = s.substr(lastPos, pos - lastPos);
        lastPos = s.find_first_not_of(',', pos);
        pos = s.find(',', lastPos);
        if (Logger::IsInComponentList(componentStr)) {
            Logger::EnableComponent(Logger::ComponentMaskFromString(componentStr));
        } else {
            LOG(ERROR, RUNTIME) << "Unknown component " << componentStr;
        }
    }
}

void Logger::DefaultFormatter(FILE *stream, int level, const char *component, const char *message)
{
    std::string prefix = GetPrefix(static_cast<Level>(level), component);
    std::stringstream streamBuf;
    streamBuf << prefix << message << std::endl << std::flush;
    std::fwrite(streamBuf.str().c_str(), streamBuf.str().size(), 1, stream);
    std::fflush(stream);
}

void FileLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    formatter_(stream_, static_cast<int>(level), GetComponentTag(component), str.c_str());
}

void FastFileLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    formatter_(stream_, static_cast<int>(level), GetComponentTag(component), str.c_str());
}

void StderrLogger::LogLineInternal(Level level, Component component, const std::string &str)
{
    formatter_(stderr, static_cast<int>(level), GetComponentTag(component), str.c_str());
}

void FastFileLogger::SyncOutputResource()
{
    std::fflush(stream_);
}

}  // namespace ark
