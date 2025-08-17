//===----------------------------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "Cygwin.h"
#include "language/Core/Config/config.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/VirtualFileSystem.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core;
using namespace toolchain::opt;

using tools::addPathIfExists;

Cygwin::Cygwin(const Driver &D, const toolchain::Triple &Triple, const ArgList &Args)
    : Generic_GCC(D, Triple, Args) {
  GCCInstallation.init(Triple, Args);
  std::string SysRoot = computeSysRoot();
  ToolChain::path_list &PPaths = getProgramPaths();

  Generic_GCC::PushPPaths(PPaths);

  path_list &Paths = getFilePaths();

  Generic_GCC::AddMultiarchPaths(D, SysRoot, "lib", Paths);

  // Similar to the logic for GCC above, if we are currently running Clang
  // inside of the requested system root, add its parent library path to those
  // searched.
  // FIXME: It's not clear whether we should use the driver's installed
  // directory ('Dir' below) or the ResourceDir.
  if (StringRef(D.Dir).starts_with(SysRoot))
    addPathIfExists(D, D.Dir + "/../lib", Paths);

  addPathIfExists(D, SysRoot + "/lib", Paths);
  addPathIfExists(D, SysRoot + "/usr/lib", Paths);
  addPathIfExists(D, SysRoot + "/usr/lib/w32api", Paths);
}

toolchain::ExceptionHandling Cygwin::GetExceptionModel(const ArgList &Args) const {
  if (getArch() == toolchain::Triple::x86_64 || getArch() == toolchain::Triple::aarch64 ||
      getArch() == toolchain::Triple::arm || getArch() == toolchain::Triple::thumb)
    return toolchain::ExceptionHandling::WinEH;
  return toolchain::ExceptionHandling::DwarfCFI;
}

void Cygwin::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                       ArgStringList &CC1Args) const {
  const Driver &D = getDriver();
  std::string SysRoot = computeSysRoot();

  if (DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nostdlibinc))
    addSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/local/include");

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> P(D.ResourceDir);
    toolchain::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // Check for configure-time C include directories.
  StringRef CIncludeDirs(C_INCLUDE_DIRS);
  if (CIncludeDirs != "") {
    SmallVector<StringRef, 5> Dirs;
    CIncludeDirs.split(Dirs, ":");
    for (StringRef Dir : Dirs) {
      StringRef Prefix =
          toolchain::sys::path::is_absolute(Dir) ? "" : StringRef(SysRoot);
      addExternCSystemInclude(DriverArgs, CC1Args, Prefix + Dir);
    }
    return;
  }

  // Lacking those, try to detect the correct set of system includes for the
  // target triple.

  AddMultilibIncludeArgs(DriverArgs, CC1Args);

  // On systems using multiarch, add /usr/include/$triple before
  // /usr/include.
  std::string MultiarchIncludeDir = getTriple().str();
  if (!MultiarchIncludeDir.empty() &&
      D.getVFS().exists(SysRoot + "/usr/include/" + MultiarchIncludeDir))
    addExternCSystemInclude(DriverArgs, CC1Args,
                            SysRoot + "/usr/include/" + MultiarchIncludeDir);

  // Add an include of '/include' directly. This isn't provided by default by
  // system GCCs, but is often used with cross-compiling GCCs, and harmless to
  // add even when Clang is acting as-if it were a system compiler.
  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/include");

  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/include");
  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/include/w32api");
}
