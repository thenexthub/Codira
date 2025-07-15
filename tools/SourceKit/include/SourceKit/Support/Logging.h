//===--- Logging.h - Logging Interface --------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SOURCEKIT_SUPPORT_LOGGING_H
#define TOOLCHAIN_SOURCEKIT_SUPPORT_LOGGING_H

#include "SourceKit/Core/Toolchain.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/raw_ostream.h"
#include <string>

namespace toolchain {
class format_object_base;
}

namespace SourceKit {
  class UIdent;
  class Logger;

typedef IntrusiveRefCntPtr<Logger> LogRef;

/// Collects logging output and writes it to stderr when it's destructed.
/// Common use case:
/// \code
///   if (LogRef Log = Logger::make(__func__, Logger::Level::Warning)) {
///     *Log << "stuff";
///   }
/// \endcode
class Logger : public toolchain::RefCountedBase<Logger> {
public:
  enum class Level : uint8_t {
    /// No logging.
    None = 0,
    /// Warning level.
    Warning = 1,
    /// Information level for high priority messages.
    InfoHighPrio = 2,
    /// Information level for medium priority messages.
    InfoMediumPrio = 3,
    /// Information level for low priority messages.
    InfoLowPrio = 4
  };

private:
  std::string Name;
  Level CurrLevel;
  SmallString<64> Msg;
  toolchain::raw_svector_ostream LogOS;

  static std::string LoggerName;
  static Level LoggingLevel;

public:
  static bool isLoggingEnabledForLevel(Level LogLevel) {
    return LoggingLevel >= LogLevel;
  }
  static void enableLogging(StringRef Name, Level LogLevel) {
    LoggerName = Name.str();
    LoggingLevel = LogLevel;
  }

  static LogRef make(toolchain::StringRef Name, Level LogLevel) {
    if (isLoggingEnabledForLevel(LogLevel)) return new Logger(Name, LogLevel);
    return nullptr;
  }

  Logger(toolchain::StringRef Name, Level LogLevel)
    : Name(Name), CurrLevel(LogLevel), LogOS(Msg) { }
  ~Logger();

  toolchain::raw_ostream &getOS() { return LogOS; }

  Logger &operator<<(SourceKit::UIdent UID);
  Logger &operator<<(toolchain::StringRef Str) { LogOS << Str; return *this; }
  Logger &operator<<(const char *Str) { if (Str) LogOS << Str; return *this; }
  Logger &operator<<(unsigned long N) { LogOS << N; return *this; }
  Logger &operator<<(long N) { LogOS << N ; return *this; }
  Logger &operator<<(unsigned N) { LogOS << N; return *this; }
  Logger &operator<<(int N) { LogOS << N; return *this; }
  Logger &operator<<(char C) { LogOS << C; return *this; }
  Logger &operator<<(unsigned char C) { LogOS << C; return *this; }
  Logger &operator<<(signed char C) { LogOS << C; return *this; }
  Logger &operator<<(const toolchain::format_object_base &Fmt);
};

} // namespace SourceKit

/// Macros to automate common uses of Logger. Like this:
/// \code
///   LOG_FUNC_SECTION_WARN {
///     *Log << "blah";
///   }
/// \endcode
#define LOG_SECTION(NAME, LEVEL) \
  if (LogRef Log = SourceKit::Logger::make(NAME, SourceKit::Logger::Level::LEVEL))
#define LOG_FUNC_SECTION(LEVEL) LOG_SECTION(__func__, LEVEL)
#define LOG_FUNC_SECTION_WARN LOG_FUNC_SECTION(Warning)

#define LOG(NAME, LEVEL, msg) LOG_SECTION(NAME, LEVEL) \
  do { *Log << msg; } while (0)
#define LOG_FUNC(LEVEL, msg) LOG_FUNC_SECTION(LEVEL) \
  do { *Log << msg; } while (0)
#define LOG_WARN(NAME, msg) LOG(NAME, Warning, msg)
#define LOG_WARN_FUNC(msg) LOG_FUNC(Warning, msg)
#define LOG_INFO_FUNC(PRIO, msg) LOG_FUNC(Info##PRIO##Prio, msg)
#define LOG_INFO(NAME, PRIO, msg) LOG(NAME, Info##PRIO##Prio, msg)

#endif
