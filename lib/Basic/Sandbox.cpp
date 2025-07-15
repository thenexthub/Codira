#include "language/Basic/Sandbox.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/StringExtras.h"
#include "toolchain/ADT/SmallString.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

using namespace language;
using namespace Sandbox;

#if defined(__APPLE__) && TARGET_OS_OSX
static StringRef sandboxProfile(toolchain::BumpPtrAllocator &Alloc) {
  toolchain::SmallString<256> contents;
  contents += "(version 1)\n";

  // Deny everything by default.
  contents += "(deny default)\n";

  // Import the system sandbox profile.
  contents += "(import \"system.sb\")\n";

  // Allow reading file metadata of any files.
  contents += "(allow file-read-metadata)\n";

  // Allow reading dylibs.
  contents += "(allow file-read* (regex #\"\\.dylib$\"))\n";

  // This is required to launch any processes (execve(2)).
  contents += "(allow process-exec*)\n";

  return NullTerminatedStringRef(StringRef(contents), Alloc);
}
#endif

bool language::Sandbox::apply(toolchain::SmallVectorImpl<toolchain::StringRef> &command,
                           toolchain::BumpPtrAllocator &Alloc) {
#if defined(__APPLE__) && TARGET_OS_OSX
  auto profile = sandboxProfile(Alloc);
  command.insert(command.begin(), {"/usr/bin/sandbox-exec", "-p", profile});
#endif
  return false;
}
