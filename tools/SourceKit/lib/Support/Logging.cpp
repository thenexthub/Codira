//===--- Logging.cpp ------------------------------------------------------===//
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

#include "SourceKit/Support/Logging.h"
#include "SourceKit/Support/UIdent.h"
#include "toolchain/Config/config.h"
#include "toolchain/Support/Format.h"
#include "toolchain/Support/Mutex.h"
#include "toolchain/Support/Timer.h"

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#if __APPLE__
#include <asl.h>
#endif

using namespace SourceKit;

static toolchain::sys::Mutex LoggingMutex;

std::string Logger::LoggerName;
Logger::Level Logger::LoggingLevel = Logger::Level::None;

Logger &Logger::operator<<(UIdent UID) {
  *this << UID.getName();
  return *this;
}

Logger &Logger::operator<<(const toolchain::format_object_base &Fmt) {
  LogOS << Fmt;
  return *this;
}

Logger::~Logger() {
  toolchain::sys::ScopedLock L(LoggingMutex);

  static toolchain::TimeRecord sBeginTR = toolchain::TimeRecord::getCurrentTime();

  SmallString<64> LogMsg;
  toolchain::raw_svector_ostream LogMsgOS(LogMsg);
  raw_ostream &OS = LogMsgOS;

  OS << '[' << int(CurrLevel) << ':' << Name << ':';

  // FIXME: Portability.
#if HAVE_PTHREAD_H && __APPLE__
  mach_port_t tid = pthread_mach_thread_np(pthread_self());
  OS << tid << ':';
#endif

  toolchain::TimeRecord TR = toolchain::TimeRecord::getCurrentTime();
  OS << toolchain::format("%7.4f] ", TR.getWallTime() - sBeginTR.getWallTime());
  OS << Msg.str();

  fprintf(stderr, "%s: %s\n", LoggerName.c_str(), LogMsg.c_str());

#if __APPLE__
  // rdar://121076739
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  // Use the Apple System Log facility.
  aslclient asl = asl_open(LoggerName.c_str(), "com.apple.console",
                           ASL_OPT_NO_DELAY);
  aslmsg msg = asl_new(ASL_TYPE_MSG);
  static const char *levstr[] = {"0", "1", "2", "3", "4", "5", "6", "7"};
  asl_set(msg, ASL_KEY_LEVEL, levstr[int(CurrLevel)]);
  asl_set(msg, ASL_KEY_MSG, LogMsg.c_str());
  asl_send(asl, msg);
  asl_free(msg);
  asl_close(asl);
#pragma clang diagnostic pop
#endif
}
