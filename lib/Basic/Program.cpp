//===--- Program.cpp - Implement OS Program Concept -----------------------===//
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

#include "language/Basic/Program.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/StringExtras.h"

#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Config/config.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/Program.h"
#if defined(_WIN32)
#include "toolchain/Support/Windows/WindowsSupport.h"
#endif

#include <memory>
#include <system_error>

#if HAVE_POSIX_SPAWN
#include <spawn.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>
#endif

using namespace language;

int language::ExecuteInPlace(const char *Program, const char **args,
                          const char **env) {
#if TOOLCHAIN_ON_UNIX
  int result;
  if (env)
    result = execve(Program, const_cast<char **>(args),
                    const_cast<char **>(env));
  else
    result = execv(Program, const_cast<char **>(args));

  return result;
#else
  std::optional<toolchain::ArrayRef<toolchain::StringRef>> Env = std::nullopt;
  if (env)
    Env = toolchain::toStringRefArray(env);
  int result =
      toolchain::sys::ExecuteAndWait(Program, toolchain::toStringRefArray(args), Env);
  if (result >= 0)
    exit(result);
  return result;
#endif
}

static const char **
toNullTerminatedCStringArray(ArrayRef<StringRef> array,
                             toolchain::BumpPtrAllocator &Alloc) {
  size_t size = array.size();
  const char **result = Alloc.Allocate<const char *>(size + 1);
  for (size_t i = 0; i < size; ++i) {
    result[i] = NullTerminatedStringRef(array[i], Alloc).data();
  }
  result[size] = nullptr;
  return result;
}

#if HAVE_UNISTD_H

namespace {
struct Pipe {
  int read;
  int write;

  Pipe() {
    int fds[2];
    if (pipe(fds) == -1) {
      read = write = -1; // '-1' to inidicate the failure.
    }
    read = fds[0];
    write = fds[1];
  }

  operator bool() const { return read != -1 || write != -1; }
};
} // namespace

toolchain::ErrorOr<language::ChildProcessInfo>
language::ExecuteWithPipe(toolchain::StringRef program,
                       toolchain::ArrayRef<toolchain::StringRef> args,
                       std::optional<toolchain::ArrayRef<toolchain::StringRef>> env) {
  Pipe p1; // Parent: write, child: read (child's STDIN).
  if (!p1)
    return std::error_code(errno, std::system_category());
  Pipe p2; // Parent: read, child: write (child's STDOUT).
  if (!p2)
    return std::error_code(errno, std::system_category());

  toolchain::BumpPtrAllocator Alloc;
  const char **argv = toNullTerminatedCStringArray(args, Alloc);
  const char **envp = nullptr;
  if (env.has_value()) {
    envp = toNullTerminatedCStringArray(*env, Alloc);
  }
  const char *progCStr = args[0] == program
                             ? argv[0]
                             : NullTerminatedStringRef(program, Alloc).data();

  pid_t pid;

#if HAVE_POSIX_SPAWN
  posix_spawn_file_actions_t FileActions;
  posix_spawn_file_actions_init(&FileActions);

  // Redirect file descriptors...
  posix_spawn_file_actions_adddup2(&FileActions, p1.read, STDIN_FILENO);
  posix_spawn_file_actions_adddup2(&FileActions, p2.write, STDOUT_FILENO);

  // Close all file descriptors, not needed as we duped them to the stdio.
  posix_spawn_file_actions_addclose(&FileActions, p1.read);
  posix_spawn_file_actions_addclose(&FileActions, p1.write);
  posix_spawn_file_actions_addclose(&FileActions, p2.read);
  posix_spawn_file_actions_addclose(&FileActions, p2.write);

  // Spawn the subtask.
  int error = posix_spawn(&pid, progCStr, &FileActions, nullptr,
                          const_cast<char **>(argv), const_cast<char **>(envp));

  posix_spawn_file_actions_destroy(&FileActions);

  close(p1.read);
  close(p2.write);

  if (error != 0 || pid == 0) {
    close(p1.write);
    close(p2.read);
    return std::error_code(error, std::system_category());
  }
#else
  // Create a child process.
  switch (pid = fork()) {
  // An error occurred:  Return to the caller.
  case -1:
    close(p1.read);
    close(p1.write);
    close(p2.read);
    close(p2.write);
    return std::error_code(errno, std::system_category());

  // Child process.
  case 0:
    // Redirect file descriptors...
    dup2(p1.read, STDIN_FILENO);
    dup2(p2.write, STDOUT_FILENO);

    // Close all file descriptors, not needed as we duped them to the stdio.
    close(p1.read);
    close(p1.write);
    close(p2.read);
    close(p2.write);

    // Execute the program.
    if (envp) {
      execve(progCStr, const_cast<char **>(argv), const_cast<char **>(envp));
    } else {
      execv(progCStr, const_cast<char **>(argv));
    }

    // If the execv()/execve() failed, we should exit. Follow Unix protocol and
    // return 127 if the executable was not found, and 126 otherwise. Use _exit
    // rather than exit so that atexit functions and static object destructors
    // cloned from the parent process aren't redundantly run, and so that any
    // data buffered in stdio buffers cloned from the parent aren't redundantly
    // written out.
    _exit(errno == ENOENT ? 127 : 126);

  // Parent process.
  default:
    close(p1.read);
    close(p2.write);
    break;
  }
#endif

  toolchain::sys::ProcessInfo proc;
  proc.Pid = pid;
  proc.Process = pid;
  return ChildProcessInfo(proc, p1.write, p2.read);
}

#elif defined(_WIN32)

toolchain::ErrorOr<language::ChildProcessInfo>
language::ExecuteWithPipe(toolchain::StringRef program,
                       toolchain::ArrayRef<toolchain::StringRef> args,
                       std::optional<toolchain::ArrayRef<toolchain::StringRef>> env) {
  using unique_handle = std::unique_ptr<void, decltype(&CloseHandle)>;
  enum { PI_READ, PI_WRITE };

  unique_handle input[2] = {
    {INVALID_HANDLE_VALUE, CloseHandle},
    {INVALID_HANDLE_VALUE, CloseHandle},
  };
  unique_handle output[2] = {
    {INVALID_HANDLE_VALUE, CloseHandle},
    {INVALID_HANDLE_VALUE, CloseHandle},
  };
  unique_handle error{INVALID_HANDLE_VALUE, CloseHandle};
  HANDLE hRead = INVALID_HANDLE_VALUE, hWrite = INVALID_HANDLE_VALUE;
  SECURITY_ATTRIBUTES saAttrs{sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  if (!CreatePipe(&hRead, &hWrite, &saAttrs, 0))
    return std::error_code(GetLastError(), std::system_category());
  output[PI_READ].reset(hRead);
  output[PI_WRITE].reset(hWrite);

  if (!SetHandleInformation(output[PI_READ].get(), HANDLE_FLAG_INHERIT, FALSE))
    return std::error_code(GetLastError(), std::system_category());

  if (!CreatePipe(&hRead, &hWrite, &saAttrs, 0))
    return std::error_code(GetLastError(), std::system_category());
  input[PI_READ].reset(hRead);
  input[PI_WRITE].reset(hWrite);

  if (!SetHandleInformation(input[PI_WRITE].get(), HANDLE_FLAG_INHERIT, FALSE))
    return std::error_code(GetLastError(), std::system_category());

  if (!DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_ERROR_HANDLE),
                       GetCurrentProcess(), &hWrite, DUPLICATE_SAME_ACCESS,
                       TRUE, DUPLICATE_SAME_ACCESS))
    return std::error_code(GetLastError(), std::system_category());
  error.reset(hWrite);

  STARTUPINFO si = {0};
  si.cb = sizeof(si);
  si.hStdInput = input[PI_READ].get();
  si.hStdOutput = output[PI_WRITE].get();
  si.hStdError = error.get();
  si.dwFlags = STARTF_USESTDHANDLES;

  toolchain::SmallVector<wchar_t, MAX_PATH> executable;
  if (std::error_code ec = toolchain::sys::windows::widenPath(program, executable))
    return ec;

  std::vector<StringRef> components;
  components.push_back(program);
  components.assign(args.begin(), args.end());
  toolchain::ErrorOr<std::wstring> commandline =
      toolchain::sys::flattenWindowsCommandLine(components);
  if (!commandline)
    return commandline.getError();

  std::vector<wchar_t> command(commandline->size() + 1, 0);
  std::copy(commandline->begin(), commandline->end(), command.begin());

  PROCESS_INFORMATION pi = {0};
  if (!CreateProcessW(executable.data(),
                      command.data(), nullptr, nullptr, TRUE, 0, nullptr,
                      nullptr, &si, &pi))
    return std::error_code(GetLastError(), std::system_category());

  unique_handle hThread{pi.hThread, CloseHandle};
  unique_handle hProcess{pi.hProcess, CloseHandle};

  int ifd = _open_osfhandle(reinterpret_cast<intptr_t>(input[PI_WRITE].get()), 0);
  if (ifd < 0)
    return std::error_code(errno, std::system_category());
  input[PI_WRITE].release();

  int ofd = _open_osfhandle(reinterpret_cast<intptr_t>(output[PI_READ].get()), 0);
  if (ofd < 0) {
    _close(ifd);
    return std::error_code(errno, std::system_category());
  }
  output[PI_READ].release();

  toolchain::sys::ProcessInfo proc;
  proc.Pid = pi.dwProcessId;
  proc.Process = pi.hProcess;
  return ChildProcessInfo(proc, ifd, ofd);
}

#else // HAVE_UNISTD_H

toolchain::ErrorOr<language::ChildProcessInfo>
language::ExecuteWithPipe(toolchain::StringRef program,
                       toolchain::ArrayRef<toolchain::StringRef> args,
                       std::optional<toolchain::ArrayRef<toolchain::StringRef>> env) {
  // Not supported.
  return std::errc::not_supported;
}

#endif
