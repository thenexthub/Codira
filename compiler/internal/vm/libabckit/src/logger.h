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

#ifndef LIBABCKIT_LOGGER
#define LIBABCKIT_LOGGER

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <map>
#include <cstdlib>

constexpr const char *LIBABCKIT_PREFIX = "[ LIBABCKIT ]";
#define LIBABCKIT_FUNC_NAME __func__
#define LIBABCKIT_LINE __LINE__

constexpr std::array<const char *, 7> const TRUE_VALUES = {"ON", "on", "1", "true", "TRUE", "enable", "ENABLE"};

namespace libabckit {

class NullBuffer : public std::streambuf {
public:
    int overflow(int c) override
    {
        return c;
    }
};
extern thread_local NullBuffer g_nB;
extern thread_local std::ostream g_nullStream;

struct AbckitComponent {
private:
    std::string name_;
    std::string prefix_;
    bool isEnabled_ = true;

public:
    explicit AbckitComponent(const std::string &componentName, bool enabled = true)
        : name_(componentName), isEnabled_(enabled)
    {
        prefix_ = "[" + componentName + "]\t";
    }
    explicit AbckitComponent(const std::string &componentName) : name_(componentName)
    {
        prefix_ = "[" + componentName + "]\t";
        isEnabled_ = true;
    }
    void Disable()
    {
        isEnabled_ = false;
    }
    void Enable()
    {
        isEnabled_ = true;
    }
    bool CheckStatus()
    {
        return isEnabled_;
    }
    std::string GetPrefix()
    {
        return prefix_;
    }
};

class Logger final {
public:
    enum LogLevel { DEBUG = 1, WARNING, ERROR, FATAL, UNIMPLEMENTED, IMPLEMENTED, UNKNOWN, INCORRECT_LOG_LVL };

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    inline static std::map<std::string, const LogLevel> levelsSet_ = {
        {"DEBUG", LogLevel::DEBUG},
        {"WARNING", LogLevel::WARNING},
        {"ERROR", LogLevel::ERROR},
        {"FATAL", LogLevel::FATAL},
        {"UNIMPLEMENTED", LogLevel::UNIMPLEMENTED},
        {"IMPLEMENTED", LogLevel::IMPLEMENTED},
        {"UNKNOWN", LogLevel::UNKNOWN},
        {"INCORRECT_LOG_LVL", LogLevel::INCORRECT_LOG_LVL},
    };
    enum MODE { DEBUG_MODE = 1, RELEASE_MODE, SILENCE_MODE };

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    inline static std::map<std::string, std::unique_ptr<AbckitComponent>> components_ = {};
    inline static bool isInitialized_ = false;

    static Logger *logger_;

    static void Initialize(MODE mode)
    {
        if (isInitialized_) {
            return;
        }
        if (const char *env = std::getenv("LIBABCKIT_DEBUG_MODE")) {
            if ((std::find(TRUE_VALUES.begin(), TRUE_VALUES.end(), static_cast<std::string>(env))) !=
                TRUE_VALUES.end()) {
                mode = MODE::DEBUG_MODE;
            }
        }
        logger_ = new Logger(mode);
        isInitialized_ = true;
    }

    explicit Logger(MODE mode) : loggerMode_(mode) {}

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    inline static std::string msgPrefix_;

    static bool CheckLoggingMode()
    {
        return !(logger_->loggerMode_ == MODE::SILENCE_MODE);
    }

    static bool CheckLogLevel(const std::string &level)
    {
        return (logger_->levelsSet_.find(level) != logger_->levelsSet_.end());
    }

    static bool CheckIfPermissible(LogLevel level)
    {
        if (logger_->loggerMode_ == MODE::DEBUG_MODE) {
            return true;  // permissible
        }
        if (logger_->loggerMode_ == MODE::RELEASE_MODE) {
            return (level == LogLevel::FATAL || level == LogLevel::ERROR);
        }
        return false;  // suppressed
    }

    static bool CheckIfPermissible(const LogLevel level, const std::string &componentName)
    {
        if (!logger_->CheckIfEnable(componentName)) {
            return false;
        }
        return logger_->CheckIfPermissible(level);
    }

    static bool CheckIfInComponents(const std::string &sideName)
    {
        return (logger_->components_.find(sideName) != logger_->components_.end());
    }

    void SetCompOpt(std::string &sideName, bool enable)
    {
        if (!CheckIfInComponents(sideName)) {
            logger_->components_[sideName] = std::make_unique<AbckitComponent>(sideName, enable);
            return;
        }
        if (enable) {
            logger_->components_[sideName]->Enable();
        } else {
            logger_->components_[sideName]->Enable();
        }
    }
    void DisableComponent(std::string &sideName)
    {
        SetCompOpt(sideName, false);
    }
    void EnableComponent(std::string &sideName)
    {
        SetCompOpt(sideName, true);
    }

    static bool CheckIfEnable(const std::string &sideName)
    {
        if (!CheckIfInComponents(sideName)) {
            return true;
        }
        return logger_->components_[sideName]->CheckStatus();
    }

    static std::string MessageCompPrefix(const std::string &componentName)
    {
        return "[ LIBABCKIT " + componentName + " ] ";
    }

    static bool CheckPermission(const std::string &levelName)
    {
        if (!CheckLogLevel(levelName)) {
            std::cout << '\n' << LIBABCKIT_PREFIX << " INCORRECT LOG LEVEL: " << levelName << '\n';
            return false;
        }
        LogLevel msgLevel = logger_->levelsSet_[levelName];
        if (!CheckLoggingMode() || !CheckIfPermissible(msgLevel)) {
            return false;
        }
        if (msgLevel == LogLevel::INCORRECT_LOG_LVL) {
            return false;
        }
        return true;
    }

    static std::ostream *GetLoggerStream(const std::string &levelName)
    {
        if (!CheckPermission(levelName)) {
            return &g_nullStream;
        }
        return &std::cerr;
    }

    static std::ostream *Message(const std::string &levelName)
    {
        if (levelName == "FATAL") {
            std::cout << "FATAL: " << LIBABCKIT_FUNC_NAME << '\n';
            // CC-OFFNXT(G.FUU.08) fatal
            // CC-OFFNXT(G.STD.16) fatal error
            std::abort();
        }
        logger_->msgPrefix_ = LIBABCKIT_PREFIX;
        return GetLoggerStream(levelName);
    }

    static std::ostream *Message(const std::string &levelName, const std::string &componentName)
    {
        if (!CheckIfPermissible(logger_->levelsSet_[levelName], componentName)) {
            return &g_nullStream;
        }
        logger_->msgPrefix_ = MessageCompPrefix(componentName);
        return GetLoggerStream(levelName);
    }

private:
    MODE loggerMode_ = MODE::RELEASE_MODE;  // default
};
}  // namespace libabckit

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_STREAM(level) libabckit::Logger::Message(#level)

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG(level) LIBABCKIT_LOG_(level)

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_NO_FUNC(level) LIBABCKIT_LOG_NO_FUNC_(level)

// CC-OFFNXT(G.DCL.01) public API
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_(level)                                             \
    libabckit::Logger::Initialize(libabckit::Logger::MODE::RELEASE_MODE); \
    *LIBABCKIT_LOG_STREAM(level) << libabckit::Logger::msgPrefix_ << "[" << LIBABCKIT_FUNC_NAME << "] "

// CC-OFFNXT(G.DCL.01) public API
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_NO_FUNC_(level)                                     \
    libabckit::Logger::Initialize(libabckit::Logger::MODE::RELEASE_MODE); \
    *LIBABCKIT_LOG_STREAM(level) << libabckit::Logger::msgPrefix_

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_DUMP(dump, level)                   \
    do {                                                  \
        if (libabckit::Logger::CheckPermission(#level)) { \
            dump;                                         \
        }                                                 \
    } while (0)

// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_FUNC LIBABCKIT_LOG(DEBUG) << '\n'

// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_IMPLEMENTED LIBABCKIT_LOG(DEBUG) << "implemented\n"

// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_UNIMPLEMENTED                          \
    LIBABCKIT_LOG(DEBUG) << "is not implemented yet!\n"; \
    abort()  // CC-OFF(G.STD.16) fatal error

// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_UNREACHABLE                 \
    LIBABCKIT_LOG(DEBUG) << "unreachable!\n"; \
    abort()  // CC-OFF(G.STD.16) fatal error

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_PREFIX_TEST "[ LIBABCKIT TEST ]"

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_LOG_TEST(level)                                                               \
    libabckit::Logger::Initialize(libabckit::Logger::MODE::RELEASE_MODE);                       \
    *LIBABCKIT_LOG_STREAM(level) << LIBABCKIT_PREFIX_TEST << "[" << LIBABCKIT_FUNC_NAME << "] " \
                                 << " "

// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_UNREACHABLE_TEST(level)          \
    LIBABCKIT_LOG_TEST(level) << "UNREACHABLE!\n"; \
    abort()  // CC-OFF(G.STD.16) fatal error

#endif
