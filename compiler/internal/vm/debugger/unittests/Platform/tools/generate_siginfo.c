//===-- generate_siginfo_linux.c ------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include <signal.h>
#include <stddef.h>
#include <stdio.h>

siginfo_t siginfo;

#define P(member)                                                              \
  printf("                   {\"%s\", %zd, %zd},\n", #member,   \
         offsetof(siginfo_t, member), sizeof(siginfo.member));

// undef annoying "POSIX friendliness" macros
#undef si_pid
#undef si_uid
#undef si_overrun
#undef si_status
#undef si_utime
#undef si_stime
#undef si_addr
#undef si_addr_lsb
#undef si_band
#undef si_fd

int main() {
  printf("  ExpectFields(siginfo_type,\n");
  printf("               {\n");

#if !defined(__NetBSD__)
  P(si_signo);
  P(si_errno);
  P(si_code);

#if defined(__GLIBC__)
  P(_sifields._kill.si_pid);
  P(_sifields._kill.si_uid);
  P(_sifields._timer.si_tid);
  P(_sifields._timer.si_overrun);
  P(_sifields._timer.si_sigval);
  P(_sifields._rt.si_pid);
  P(_sifields._rt.si_uid);
  P(_sifields._rt.si_sigval);
  P(_sifields._sigchld.si_pid);
  P(_sifields._sigchld.si_uid);
  P(_sifields._sigchld.si_status);
  P(_sifields._sigchld.si_utime);
  P(_sifields._sigchld.si_stime);
  P(_sifields._sigfault.si_addr);
  P(_sifields._sigfault.si_addr_lsb);
  P(_sifields._sigfault._bounds._addr_bnd._lower);
  P(_sifields._sigfault._bounds._addr_bnd._upper);
  P(_sifields._sigfault._bounds._pkey);
  P(_sifields._sigpoll.si_band);
  P(_sifields._sigpoll.si_fd);
  P(_sifields._sigsys._call_addr);
  P(_sifields._sigsys._syscall);
  P(_sifields._sigsys._arch);
#endif // defined(__GLIBC__)

#if defined(__FreeBSD__)
  // these are top-level fields on FreeBSD
  P(si_pid);
  P(si_uid);
  P(si_status);
  P(si_addr);
  P(si_value);
  P(_reason._fault._trapno);
  P(_reason._timer._timerid);
  P(_reason._timer._overrun);
  P(_reason._mesgq._mqd);
  P(_reason._poll._band);
#endif // defined(__FreeBSD__)

#else // defined(__NetBSD__)

  P(_info._signo);
  P(_info._code);
  P(_info._errno);
  P(_info._reason._rt._pid);
  P(_info._reason._rt._uid);
  P(_info._reason._rt._value);
  P(_info._reason._child._pid);
  P(_info._reason._child._uid);
  P(_info._reason._child._status);
  P(_info._reason._child._utime);
  P(_info._reason._child._stime);
  P(_info._reason._fault._addr);
  P(_info._reason._fault._trap);
  P(_info._reason._fault._trap2);
  P(_info._reason._fault._trap3);
  P(_info._reason._poll._band);
  P(_info._reason._poll._fd);
  P(_info._reason._syscall._sysnum);
  P(_info._reason._syscall._retval);
  P(_info._reason._syscall._error);
  P(_info._reason._syscall._args);
  P(_info._reason._ptrace_state._pe_report_event);
  P(_info._reason._ptrace_state._option._pe_other_pid);
  P(_info._reason._ptrace_state._option._pe_lwp);

#endif // defined(__NetBSD__)

  printf("               });\n");

  return 0;
}
