//===--- Linux.h - Linux ToolChain Implementations --------------*- C++ -*-===//
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

#include "Linux.h"
#include "Arch/ARM.h"
#include "Arch/LoongArch.h"
#include "Arch/Mips.h"
#include "Arch/PPC.h"
#include "Arch/RISCV.h"
#include "language/Core/Config/config.h"
#include "language/Core/Driver/CommonArgs.h"
#include "language/Core/Driver/Distro.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/SanitizerArgs.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/ProfileData/InstrProf.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/ScopedPrinter.h"
#include "toolchain/Support/VirtualFileSystem.h"

using namespace language::Core::driver;
using namespace language::Core::driver::toolchains;
using namespace language::Core;
using namespace toolchain::opt;

using tools::addPathIfExists;

/// Get our best guess at the multiarch triple for a target.
///
/// Debian-based systems are starting to use a multiarch setup where they use
/// a target-triple directory in the library and header search paths.
/// Unfortunately, this triple does not align with the vanilla target triple,
/// so we provide a rough mapping here.
std::string Linux::getMultiarchTriple(const Driver &D,
                                      const toolchain::Triple &TargetTriple,
                                      StringRef SysRoot) const {
  toolchain::Triple::EnvironmentType TargetEnvironment =
      TargetTriple.getEnvironment();
  bool IsAndroid = TargetTriple.isAndroid();
  bool IsMipsR6 = TargetTriple.getSubArch() == toolchain::Triple::MipsSubArch_r6;
  bool IsMipsN32Abi = TargetTriple.getEnvironment() == toolchain::Triple::GNUABIN32;

  // For most architectures, just use whatever we have rather than trying to be
  // clever.
  switch (TargetTriple.getArch()) {
  default:
    break;

  // We use the existence of '/lib/<triple>' as a directory to detect some
  // common linux triples that don't quite match the Clang triple for both
  // 32-bit and 64-bit targets. Multiarch fixes its install triples to these
  // regardless of what the actual target triple is.
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
    if (IsAndroid)
      return "arm-linux-androideabi";
    if (TargetEnvironment == toolchain::Triple::GNUEABIHF ||
        TargetEnvironment == toolchain::Triple::MuslEABIHF ||
        TargetEnvironment == toolchain::Triple::EABIHF)
      return "arm-linux-gnueabihf";
    return "arm-linux-gnueabi";
  case toolchain::Triple::armeb:
  case toolchain::Triple::thumbeb:
    if (TargetEnvironment == toolchain::Triple::GNUEABIHF ||
        TargetEnvironment == toolchain::Triple::MuslEABIHF ||
        TargetEnvironment == toolchain::Triple::EABIHF)
      return "armeb-linux-gnueabihf";
    return "armeb-linux-gnueabi";
  case toolchain::Triple::x86:
    if (IsAndroid)
      return "i686-linux-android";
    return "i386-linux-gnu";
  case toolchain::Triple::x86_64:
    if (IsAndroid)
      return "x86_64-linux-android";
    if (TargetEnvironment == toolchain::Triple::GNUX32)
      return "x86_64-linux-gnux32";
    return "x86_64-linux-gnu";
  case toolchain::Triple::aarch64:
    if (IsAndroid)
      return "aarch64-linux-android";
    if (hasEffectiveTriple() &&
        getEffectiveTriple().getEnvironment() == toolchain::Triple::PAuthTest)
      return "aarch64-linux-pauthtest";
    return "aarch64-linux-gnu";
  case toolchain::Triple::aarch64_be:
    return "aarch64_be-linux-gnu";

  case toolchain::Triple::loongarch64: {
    const char *Libc;
    const char *FPFlavor;

    if (TargetTriple.isGNUEnvironment()) {
      Libc = "gnu";
    } else if (TargetTriple.isMusl()) {
      Libc = "musl";
    } else {
      return TargetTriple.str();
    }

    switch (TargetEnvironment) {
    default:
      return TargetTriple.str();
    case toolchain::Triple::GNUSF:
    case toolchain::Triple::MuslSF:
      FPFlavor = "sf";
      break;
    case toolchain::Triple::GNUF32:
    case toolchain::Triple::MuslF32:
      FPFlavor = "f32";
      break;
    case toolchain::Triple::GNU:
    case toolchain::Triple::GNUF64:
    case toolchain::Triple::Musl:
      // This was going to be "f64" in an earlier Toolchain Conventions
      // revision, but starting from Feb 2023 the F64 ABI variants are
      // unmarked in their canonical forms.
      FPFlavor = "";
      break;
    }

    return (Twine("loongarch64-linux-") + Libc + FPFlavor).str();
  }

  case toolchain::Triple::m68k:
    return "m68k-linux-gnu";

  case toolchain::Triple::mips:
    return IsMipsR6 ? "mipsisa32r6-linux-gnu" : "mips-linux-gnu";
  case toolchain::Triple::mipsel:
    return IsMipsR6 ? "mipsisa32r6el-linux-gnu" : "mipsel-linux-gnu";
  case toolchain::Triple::mips64: {
    std::string MT = std::string(IsMipsR6 ? "mipsisa64r6" : "mips64") +
                     "-linux-" + (IsMipsN32Abi ? "gnuabin32" : "gnuabi64");
    if (D.getVFS().exists(concat(SysRoot, "/lib", MT)))
      return MT;
    if (D.getVFS().exists(concat(SysRoot, "/lib/mips64-linux-gnu")))
      return "mips64-linux-gnu";
    break;
  }
  case toolchain::Triple::mips64el: {
    std::string MT = std::string(IsMipsR6 ? "mipsisa64r6el" : "mips64el") +
                     "-linux-" + (IsMipsN32Abi ? "gnuabin32" : "gnuabi64");
    if (D.getVFS().exists(concat(SysRoot, "/lib", MT)))
      return MT;
    if (D.getVFS().exists(concat(SysRoot, "/lib/mips64el-linux-gnu")))
      return "mips64el-linux-gnu";
    break;
  }
  case toolchain::Triple::ppc:
    if (D.getVFS().exists(concat(SysRoot, "/lib/powerpc-linux-gnuspe")))
      return "powerpc-linux-gnuspe";
    return "powerpc-linux-gnu";
  case toolchain::Triple::ppcle:
    return "powerpcle-linux-gnu";
  case toolchain::Triple::ppc64:
    return "powerpc64-linux-gnu";
  case toolchain::Triple::ppc64le:
    return "powerpc64le-linux-gnu";
  case toolchain::Triple::riscv64:
    if (IsAndroid)
      return "riscv64-linux-android";
    return "riscv64-linux-gnu";
  case toolchain::Triple::sparc:
    return "sparc-linux-gnu";
  case toolchain::Triple::sparcv9:
    return "sparc64-linux-gnu";
  case toolchain::Triple::systemz:
    return "s390x-linux-gnu";
  }
  return TargetTriple.str();
}

static StringRef getOSLibDir(const toolchain::Triple &Triple, const ArgList &Args) {
  if (Triple.isMIPS()) {
    // lib32 directory has a special meaning on MIPS targets.
    // It contains N32 ABI binaries. Use this folder if produce
    // code for N32 ABI only.
    if (tools::mips::hasMipsAbiArg(Args, "n32"))
      return "lib32";
    return Triple.isArch32Bit() ? "lib" : "lib64";
  }

  // It happens that only x86, PPC and SPARC use the 'lib32' variant of
  // oslibdir, and using that variant while targeting other architectures causes
  // problems because the libraries are laid out in shared system roots that
  // can't cope with a 'lib32' library search path being considered. So we only
  // enable them when we know we may need it.
  //
  // FIXME: This is a bit of a hack. We should really unify this code for
  // reasoning about oslibdir spellings with the lib dir spellings in the
  // GCCInstallationDetector, but that is a more significant refactoring.
  if (Triple.getArch() == toolchain::Triple::x86 || Triple.isPPC32() ||
      Triple.getArch() == toolchain::Triple::sparc)
    return "lib32";

  if (Triple.getArch() == toolchain::Triple::x86_64 && Triple.isX32())
    return "libx32";

  if (Triple.getArch() == toolchain::Triple::riscv32)
    return "lib32";

  return Triple.isArch32Bit() ? "lib" : "lib64";
}

Linux::Linux(const Driver &D, const toolchain::Triple &Triple, const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {
  GCCInstallation.init(Triple, Args);
  Multilibs = GCCInstallation.getMultilibs();
  SelectedMultilibs.assign({GCCInstallation.getMultilib()});
  toolchain::Triple::ArchType Arch = Triple.getArch();
  std::string SysRoot = computeSysRoot();
  ToolChain::path_list &PPaths = getProgramPaths();

  Generic_GCC::PushPPaths(PPaths);

  Distro Distro(D.getVFS(), Triple);

  if (Distro.IsAlpineLinux() || Triple.isAndroid()) {
    ExtraOpts.push_back("-z");
    ExtraOpts.push_back("now");
  }

  if (Distro.IsOpenSUSE() || Distro.IsUbuntu() || Distro.IsAlpineLinux() ||
      Triple.isAndroid()) {
    ExtraOpts.push_back("-z");
    ExtraOpts.push_back("relro");
  }

  // Note, lld from 11 onwards default max-page-size to 65536 for both ARM and
  // AArch64.
  if (Triple.isAndroid()) {
    if (Triple.isARM()) {
      // Android ARM uses max-page-size=4096 to reduce VMA usage.
      ExtraOpts.push_back("-z");
      ExtraOpts.push_back("max-page-size=4096");
    } else if (Triple.isAArch64() || Triple.getArch() == toolchain::Triple::x86_64) {
      // Android AArch64 uses max-page-size=16384 to support 4k/16k page sizes.
      // Android emulates a 16k page size for app testing on x86_64 machines.
      ExtraOpts.push_back("-z");
      ExtraOpts.push_back("max-page-size=16384");
    }
    if (Triple.isAndroidVersionLT(29)) {
      // https://github.com/android/ndk/issues/1196
      // The unwinder used by the crash handler on versions of Android prior to
      // API 29 did not correctly handle binaries built with rosegment, which is
      // enabled by default for LLD. Android only supports LLD, so it's not an
      // issue that this flag is not accepted by other linkers.
      ExtraOpts.push_back("--no-rosegment");
    }
    if (!Triple.isAndroidVersionLT(28)) {
      // Android supports relr packing starting with API 28 and had its own
      // flavor (--pack-dyn-relocs=android) starting in API 23.
      // TODO: It's possible to use both with --pack-dyn-relocs=android+relr,
      // but we need to gather some data on the impact of that form before we
      // can know if it's a good default.
      // On the other hand, relr should always be an improvement.
      ExtraOpts.push_back("--use-android-relr-tags");
      ExtraOpts.push_back("--pack-dyn-relocs=relr");
    }
  }

  if (GCCInstallation.getParentLibPath().contains("opt/rh/"))
    // With devtoolset on RHEL, we want to add a bin directory that is relative
    // to the detected gcc install, because if we are using devtoolset gcc then
    // we want to use other tools from devtoolset (e.g. ld) instead of the
    // standard system tools.
    PPaths.push_back(Twine(GCCInstallation.getParentLibPath() +
                     "/../bin").str());

  if (Arch == toolchain::Triple::arm || Arch == toolchain::Triple::thumb)
    ExtraOpts.push_back("-X");

  const bool IsAndroid = Triple.isAndroid();
  const bool IsMips = Triple.isMIPS();
  const bool IsHexagon = Arch == toolchain::Triple::hexagon;
  const bool IsRISCV = Triple.isRISCV();
  const bool IsCSKY = Triple.isCSKY();

  if (IsCSKY && !SelectedMultilibs.empty())
    SysRoot = SysRoot + SelectedMultilibs.back().osSuffix();

  if ((IsMips || IsCSKY) && !SysRoot.empty())
    ExtraOpts.push_back("--sysroot=" + SysRoot);

  // Do not use 'gnu' hash style for Mips targets because .gnu.hash
  // and the MIPS ABI require .dynsym to be sorted in different ways.
  // .gnu.hash needs symbols to be grouped by hash code whereas the MIPS
  // ABI requires a mapping between the GOT and the symbol table.
  // Android loader does not support .gnu.hash until API 23.
  // Hexagon linker/loader does not support .gnu.hash
  if (!IsMips && !IsHexagon) {
    if (Distro.IsOpenSUSE() || Distro == Distro::UbuntuLucid ||
        Distro == Distro::UbuntuJaunty || Distro == Distro::UbuntuKarmic ||
        (IsAndroid && Triple.isAndroidVersionLT(23)))
      ExtraOpts.push_back("--hash-style=both");
    else
      ExtraOpts.push_back("--hash-style=gnu");
  }

#ifdef ENABLE_LINKER_BUILD_ID
  ExtraOpts.push_back("--build-id");
#endif

  // The selection of paths to try here is designed to match the patterns which
  // the GCC driver itself uses, as this is part of the GCC-compatible driver.
  // This was determined by running GCC in a fake filesystem, creating all
  // possible permutations of these directories, and seeing which ones it added
  // to the link paths.
  path_list &Paths = getFilePaths();

  const std::string OSLibDir = std::string(getOSLibDir(Triple, Args));
  const std::string MultiarchTriple = getMultiarchTriple(D, Triple, SysRoot);

  // mips32: Debian multilib, we use /libo32, while in other case, /lib is
  // used. We need add both libo32 and /lib.
  if (Arch == toolchain::Triple::mips || Arch == toolchain::Triple::mipsel) {
    Generic_GCC::AddMultilibPaths(D, SysRoot, "libo32", MultiarchTriple, Paths);
    addPathIfExists(D, concat(SysRoot, "/libo32"), Paths);
    addPathIfExists(D, concat(SysRoot, "/usr/libo32"), Paths);
  }
  Generic_GCC::AddMultilibPaths(D, SysRoot, OSLibDir, MultiarchTriple, Paths);

  addPathIfExists(D, concat(SysRoot, "/lib", MultiarchTriple), Paths);
  addPathIfExists(D, concat(SysRoot, "/lib/..", OSLibDir), Paths);

  if (IsAndroid) {
    // Android sysroots contain a library directory for each supported OS
    // version as well as some unversioned libraries in the usual multiarch
    // directory.
    addPathIfExists(
        D,
        concat(SysRoot, "/usr/lib", MultiarchTriple,
               toolchain::to_string(Triple.getEnvironmentVersion().getMajor())),
        Paths);
  }

  addPathIfExists(D, concat(SysRoot, "/usr/lib", MultiarchTriple), Paths);
  addPathIfExists(D, concat(SysRoot, "/usr", OSLibDir), Paths);
  if (IsRISCV) {
    StringRef ABIName = tools::riscv::getRISCVABI(Args, Triple);
    addPathIfExists(D, concat(SysRoot, "/", OSLibDir, ABIName), Paths);
    addPathIfExists(D, concat(SysRoot, "/usr", OSLibDir, ABIName), Paths);
  }

  Generic_GCC::AddMultiarchPaths(D, SysRoot, OSLibDir, Paths);

  addPathIfExists(D, concat(SysRoot, "/lib"), Paths);
  addPathIfExists(D, concat(SysRoot, "/usr/lib"), Paths);
}

ToolChain::RuntimeLibType Linux::GetDefaultRuntimeLibType() const {
  if (getTriple().isAndroid())
    return ToolChain::RLT_CompilerRT;
  return Generic_ELF::GetDefaultRuntimeLibType();
}

unsigned Linux::GetDefaultDwarfVersion() const {
  if (getTriple().isAndroid())
    return 4;
  return ToolChain::GetDefaultDwarfVersion();
}

ToolChain::CXXStdlibType Linux::GetDefaultCXXStdlibType() const {
  if (getTriple().isAndroid())
    return ToolChain::CST_Libcxx;
  return ToolChain::CST_Libstdcxx;
}

bool Linux::HasNativeLLVMSupport() const { return true; }

Tool *Linux::buildLinker() const { return new tools::gnutools::Linker(*this); }

Tool *Linux::buildStaticLibTool() const {
  return new tools::gnutools::StaticLibTool(*this);
}

Tool *Linux::buildAssembler() const {
  return new tools::gnutools::Assembler(*this);
}

std::string Linux::computeSysRoot() const {
  if (!getDriver().SysRoot.empty())
    return getDriver().SysRoot;

  if (getTriple().isAndroid()) {
    // Android toolchains typically include a sysroot at ../sysroot relative to
    // the clang binary.
    const StringRef ClangDir = getDriver().Dir;
    std::string AndroidSysRootPath = (ClangDir + "/../sysroot").str();
    if (getVFS().exists(AndroidSysRootPath))
      return AndroidSysRootPath;
  }

  if (getTriple().isCSKY()) {
    // CSKY toolchains use different names for sysroot folder.
    if (!GCCInstallation.isValid())
      return std::string();
    // GCCInstallation.getInstallPath() =
    //   $GCCToolchainPath/lib/gcc/csky-linux-gnuabiv2/6.3.0
    // Path = $GCCToolchainPath/csky-linux-gnuabiv2/libc
    std::string Path = (GCCInstallation.getInstallPath() + "/../../../../" +
                        GCCInstallation.getTriple().str() + "/libc")
                           .str();
    if (getVFS().exists(Path))
      return Path;
    return std::string();
  }

  if (!GCCInstallation.isValid() || !getTriple().isMIPS())
    return std::string();

  // Standalone MIPS toolchains use different names for sysroot folder
  // and put it into different places. Here we try to check some known
  // variants.

  const StringRef InstallDir = GCCInstallation.getInstallPath();
  const StringRef TripleStr = GCCInstallation.getTriple().str();
  const Multilib &Multilib = GCCInstallation.getMultilib();

  std::string Path =
      (InstallDir + "/../../../../" + TripleStr + "/libc" + Multilib.osSuffix())
          .str();

  if (getVFS().exists(Path))
    return Path;

  Path = (InstallDir + "/../../../../sysroot" + Multilib.osSuffix()).str();

  if (getVFS().exists(Path))
    return Path;

  return std::string();
}

std::string Linux::getDynamicLinker(const ArgList &Args) const {
  const toolchain::Triple::ArchType Arch = getArch();
  const toolchain::Triple &Triple = getTriple();

  const Distro Distro(getDriver().getVFS(), Triple);

  if (Triple.isAndroid()) {
    if (getSanitizerArgs(Args).needsHwasanRt() &&
        !Triple.isAndroidVersionLT(34) && Triple.isArch64Bit()) {
      // On Android 14 and newer, there is a special linker_hwasan64 that
      // allows to run HWASan binaries on non-HWASan system images. This
      // is also available on HWASan system images, so we can just always
      // use that instead.
      return "/system/bin/linker_hwasan64";
    }
    return Triple.isArch64Bit() ? "/system/bin/linker64" : "/system/bin/linker";
  }
  if (Triple.isMusl()) {
    std::string ArchName;
    bool IsArm = false;

    switch (Arch) {
    case toolchain::Triple::arm:
    case toolchain::Triple::thumb:
      ArchName = "arm";
      IsArm = true;
      break;
    case toolchain::Triple::armeb:
    case toolchain::Triple::thumbeb:
      ArchName = "armeb";
      IsArm = true;
      break;
    case toolchain::Triple::x86:
      ArchName = "i386";
      break;
    case toolchain::Triple::x86_64:
      ArchName = Triple.isX32() ? "x32" : Triple.getArchName().str();
      break;
    default:
      ArchName = Triple.getArchName().str();
    }
    if (IsArm &&
        (Triple.getEnvironment() == toolchain::Triple::MuslEABIHF ||
         tools::arm::getARMFloatABI(*this, Args) == tools::arm::FloatABI::Hard))
      ArchName += "hf";
    if (Arch == toolchain::Triple::ppc &&
        Triple.getSubArch() == toolchain::Triple::PPCSubArch_spe)
      ArchName = "powerpc-sf";

    return "/lib/ld-musl-" + ArchName + ".so.1";
  }

  std::string LibDir;
  std::string Loader;

  switch (Arch) {
  default:
    toolchain_unreachable("unsupported architecture");

  case toolchain::Triple::aarch64:
    LibDir = "lib";
    Loader = "ld-linux-aarch64.so.1";
    break;
  case toolchain::Triple::aarch64_be:
    LibDir = "lib";
    Loader = "ld-linux-aarch64_be.so.1";
    break;
  case toolchain::Triple::arm:
  case toolchain::Triple::thumb:
  case toolchain::Triple::armeb:
  case toolchain::Triple::thumbeb: {
    const bool HF =
        Triple.getEnvironment() == toolchain::Triple::GNUEABIHF ||
        Triple.getEnvironment() == toolchain::Triple::GNUEABIHFT64 ||
        tools::arm::getARMFloatABI(*this, Args) == tools::arm::FloatABI::Hard;

    LibDir = "lib";
    Loader = HF ? "ld-linux-armhf.so.3" : "ld-linux.so.3";
    break;
  }
  case toolchain::Triple::loongarch32: {
    LibDir = "lib32";
    Loader =
        ("ld-linux-loongarch-" +
         tools::loongarch::getLoongArchABI(getDriver(), Args, Triple) + ".so.1")
            .str();
    break;
  }
  case toolchain::Triple::loongarch64: {
    LibDir = "lib64";
    Loader =
        ("ld-linux-loongarch-" +
         tools::loongarch::getLoongArchABI(getDriver(), Args, Triple) + ".so.1")
            .str();
    break;
  }
  case toolchain::Triple::m68k:
    LibDir = "lib";
    Loader = "ld.so.1";
    break;
  case toolchain::Triple::mips:
  case toolchain::Triple::mipsel:
  case toolchain::Triple::mips64:
  case toolchain::Triple::mips64el: {
    bool IsNaN2008 = tools::mips::isNaN2008(getDriver(), Args, Triple);

    LibDir = "lib" + tools::mips::getMipsABILibSuffix(Args, Triple);

    if (tools::mips::isUCLibc(Args))
      Loader = IsNaN2008 ? "ld-uClibc-mipsn8.so.0" : "ld-uClibc.so.0";
    else if (!Triple.hasEnvironment() &&
             Triple.getVendor() == toolchain::Triple::VendorType::MipsTechnologies)
      Loader =
          Triple.isLittleEndian() ? "ld-musl-mipsel.so.1" : "ld-musl-mips.so.1";
    else
      Loader = IsNaN2008 ? "ld-linux-mipsn8.so.1" : "ld.so.1";

    break;
  }
  case toolchain::Triple::ppc:
    LibDir = "lib";
    Loader = "ld.so.1";
    break;
  case toolchain::Triple::ppcle:
    LibDir = "lib";
    Loader = "ld.so.1";
    break;
  case toolchain::Triple::ppc64:
    LibDir = "lib64";
    Loader =
        (tools::ppc::hasPPCAbiArg(Args, "elfv2")) ? "ld64.so.2" : "ld64.so.1";
    break;
  case toolchain::Triple::ppc64le:
    LibDir = "lib64";
    Loader =
        (tools::ppc::hasPPCAbiArg(Args, "elfv1")) ? "ld64.so.1" : "ld64.so.2";
    break;
  case toolchain::Triple::riscv32:
  case toolchain::Triple::riscv64: {
    StringRef ArchName = toolchain::Triple::getArchTypeName(Arch);
    StringRef ABIName = tools::riscv::getRISCVABI(Args, Triple);
    LibDir = "lib";
    Loader = ("ld-linux-" + ArchName + "-" + ABIName + ".so.1").str();
    break;
  }
  case toolchain::Triple::sparc:
  case toolchain::Triple::sparcel:
    LibDir = "lib";
    Loader = "ld-linux.so.2";
    break;
  case toolchain::Triple::sparcv9:
    LibDir = "lib64";
    Loader = "ld-linux.so.2";
    break;
  case toolchain::Triple::systemz:
    LibDir = "lib";
    Loader = "ld64.so.1";
    break;
  case toolchain::Triple::x86:
    LibDir = "lib";
    Loader = "ld-linux.so.2";
    break;
  case toolchain::Triple::x86_64: {
    bool X32 = Triple.isX32();

    LibDir = X32 ? "libx32" : "lib64";
    Loader = X32 ? "ld-linux-x32.so.2" : "ld-linux-x86-64.so.2";
    break;
  }
  case toolchain::Triple::ve:
    return "/opt/nec/ve/lib/ld-linux-ve.so.1";
  case toolchain::Triple::csky: {
    LibDir = "lib";
    Loader = "ld.so.1";
    break;
  }
  }

  if (Distro == Distro::Exherbo &&
      (Triple.getVendor() == toolchain::Triple::UnknownVendor ||
       Triple.getVendor() == toolchain::Triple::PC))
    return "/usr/" + Triple.str() + "/lib/" + Loader;
  return "/" + LibDir + "/" + Loader;
}

void Linux::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                      ArgStringList &CC1Args) const {
  const Driver &D = getDriver();
  std::string SysRoot = computeSysRoot();

  if (DriverArgs.hasArg(language::Core::driver::options::OPT_nostdinc))
    return;

  // Add 'include' in the resource directory, which is similar to
  // GCC_INCLUDE_DIR (private headers) in GCC. Note: the include directory
  // contains some files conflicting with system /usr/include. musl systems
  // prefer the /usr/include copies which are more relevant.
  SmallString<128> ResourceDirInclude(D.ResourceDir);
  toolchain::sys::path::append(ResourceDirInclude, "include");
  if (!DriverArgs.hasArg(options::OPT_nobuiltininc) &&
      (!getTriple().isMusl() || DriverArgs.hasArg(options::OPT_nostdlibinc)))
    addSystemInclude(DriverArgs, CC1Args, ResourceDirInclude);

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // LOCAL_INCLUDE_DIR
  addSystemInclude(DriverArgs, CC1Args, concat(SysRoot, "/usr/local/include"));
  // TOOL_INCLUDE_DIR
  AddMultilibIncludeArgs(DriverArgs, CC1Args);

  // Check for configure-time C include directories.
  StringRef CIncludeDirs(C_INCLUDE_DIRS);
  if (CIncludeDirs != "") {
    SmallVector<StringRef, 5> dirs;
    CIncludeDirs.split(dirs, ":");
    for (StringRef dir : dirs) {
      StringRef Prefix =
          toolchain::sys::path::is_absolute(dir) ? "" : StringRef(SysRoot);
      addExternCSystemInclude(DriverArgs, CC1Args, Prefix + dir);
    }
    return;
  }

  // On systems using multiarch and Android, add /usr/include/$triple before
  // /usr/include.
  std::string MultiarchIncludeDir = getMultiarchTriple(D, getTriple(), SysRoot);
  if (!MultiarchIncludeDir.empty() &&
      D.getVFS().exists(concat(SysRoot, "/usr/include", MultiarchIncludeDir)))
    addExternCSystemInclude(
        DriverArgs, CC1Args,
        concat(SysRoot, "/usr/include", MultiarchIncludeDir));

  if (getTriple().getOS() == toolchain::Triple::RTEMS)
    return;

  // Add an include of '/include' directly. This isn't provided by default by
  // system GCCs, but is often used with cross-compiling GCCs, and harmless to
  // add even when Clang is acting as-if it were a system compiler.
  addExternCSystemInclude(DriverArgs, CC1Args, concat(SysRoot, "/include"));

  addExternCSystemInclude(DriverArgs, CC1Args, concat(SysRoot, "/usr/include"));

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc) && getTriple().isMusl())
    addSystemInclude(DriverArgs, CC1Args, ResourceDirInclude);
}

void Linux::addLibStdCxxIncludePaths(const toolchain::opt::ArgList &DriverArgs,
                                     toolchain::opt::ArgStringList &CC1Args) const {
  // We need a detected GCC installation on Linux to provide libstdc++'s
  // headers in odd Linuxish places.
  if (!GCCInstallation.isValid())
    return;

  // Detect Debian g++-multiarch-incdir.diff.
  StringRef TripleStr = GCCInstallation.getTriple().str();
  StringRef DebianMultiarch =
      GCCInstallation.getTriple().getArch() == toolchain::Triple::x86
          ? "i386-linux-gnu"
          : TripleStr;

  // Try generic GCC detection first.
  if (Generic_GCC::addGCCLibStdCxxIncludePaths(DriverArgs, CC1Args,
                                               DebianMultiarch))
    return;

  StringRef LibDir = GCCInstallation.getParentLibPath();
  const Multilib &Multilib = GCCInstallation.getMultilib();
  const GCCVersion &Version = GCCInstallation.getVersion();

  const std::string LibStdCXXIncludePathCandidates[] = {
      // Android standalone toolchain has C++ headers in yet another place.
      LibDir.str() + "/../" + TripleStr.str() + "/include/c++/" + Version.Text,
      // Freescale SDK C++ headers are directly in <sysroot>/usr/include/c++,
      // without a subdirectory corresponding to the gcc version.
      LibDir.str() + "/../include/c++",
      // Cray's gcc installation puts headers under "g++" without a
      // version suffix.
      LibDir.str() + "/../include/g++",
  };

  for (const auto &IncludePath : LibStdCXXIncludePathCandidates) {
    if (addLibStdCXXIncludePaths(IncludePath, TripleStr,
                                 Multilib.includeSuffix(), DriverArgs, CC1Args))
      break;
  }
}

void Linux::AddCudaIncludeArgs(const ArgList &DriverArgs,
                               ArgStringList &CC1Args) const {
  CudaInstallation->AddCudaIncludeArgs(DriverArgs, CC1Args);
}

void Linux::AddHIPIncludeArgs(const ArgList &DriverArgs,
                              ArgStringList &CC1Args) const {
  RocmInstallation->AddHIPIncludeArgs(DriverArgs, CC1Args);
}

void Linux::AddHIPRuntimeLibArgs(const ArgList &Args,
                                 ArgStringList &CmdArgs) const {
  CmdArgs.push_back(
      Args.MakeArgString(StringRef("-L") + RocmInstallation->getLibPath()));

  if (Args.hasFlag(options::OPT_frtlib_add_rpath,
                   options::OPT_fno_rtlib_add_rpath, false)) {
    SmallString<0> p = RocmInstallation->getLibPath();
    toolchain::sys::path::remove_dots(p, true);
    CmdArgs.append({"-rpath", Args.MakeArgString(p)});
  }

  CmdArgs.push_back("-lamdhip64");
}

void Linux::AddIAMCUIncludeArgs(const ArgList &DriverArgs,
                                ArgStringList &CC1Args) const {
  if (GCCInstallation.isValid()) {
    CC1Args.push_back("-isystem");
    CC1Args.push_back(DriverArgs.MakeArgString(
        GCCInstallation.getParentLibPath() + "/../" +
        GCCInstallation.getTriple().str() + "/include"));
  }
}

void Linux::addSYCLIncludeArgs(const ArgList &DriverArgs,
                               ArgStringList &CC1Args) const {
  SYCLInstallation->addSYCLIncludeArgs(DriverArgs, CC1Args);
}

bool Linux::isPIEDefault(const toolchain::opt::ArgList &Args) const {
  return CLANG_DEFAULT_PIE_ON_LINUX || getTriple().isAndroid() ||
         getTriple().isMusl() || getSanitizerArgs(Args).requiresPIE();
}

bool Linux::IsAArch64OutlineAtomicsDefault(const ArgList &Args) const {
  // Outline atomics for AArch64 are supported by compiler-rt
  // and libgcc since 9.3.1
  assert(getTriple().isAArch64() && "expected AArch64 target!");
  ToolChain::RuntimeLibType RtLib = GetRuntimeLibType(Args);
  if (RtLib == ToolChain::RLT_CompilerRT)
    return true;
  assert(RtLib == ToolChain::RLT_Libgcc && "unexpected runtime library type!");
  if (GCCInstallation.getVersion().isOlderThan(9, 3, 1))
    return false;
  return true;
}

bool Linux::IsMathErrnoDefault() const {
  if (getTriple().isAndroid() || getTriple().isMusl())
    return false;
  return Generic_ELF::IsMathErrnoDefault();
}

SanitizerMask Linux::getSupportedSanitizers() const {
  const bool IsX86 = getTriple().getArch() == toolchain::Triple::x86;
  const bool IsX86_64 = getTriple().getArch() == toolchain::Triple::x86_64;
  const bool IsMIPS = getTriple().isMIPS32();
  const bool IsMIPS64 = getTriple().isMIPS64();
  const bool IsPowerPC64 = getTriple().getArch() == toolchain::Triple::ppc64 ||
                           getTriple().getArch() == toolchain::Triple::ppc64le;
  const bool IsAArch64 = getTriple().getArch() == toolchain::Triple::aarch64 ||
                         getTriple().getArch() == toolchain::Triple::aarch64_be;
  const bool IsArmArch = getTriple().getArch() == toolchain::Triple::arm ||
                         getTriple().getArch() == toolchain::Triple::thumb ||
                         getTriple().getArch() == toolchain::Triple::armeb ||
                         getTriple().getArch() == toolchain::Triple::thumbeb;
  const bool IsLoongArch64 = getTriple().getArch() == toolchain::Triple::loongarch64;
  const bool IsRISCV64 = getTriple().getArch() == toolchain::Triple::riscv64;
  const bool IsSystemZ = getTriple().getArch() == toolchain::Triple::systemz;
  const bool IsHexagon = getTriple().getArch() == toolchain::Triple::hexagon;
  const bool IsAndroid = getTriple().isAndroid();
  SanitizerMask Res = ToolChain::getSupportedSanitizers();
  Res |= SanitizerKind::Address;
  Res |= SanitizerKind::PointerCompare;
  Res |= SanitizerKind::PointerSubtract;
  Res |= SanitizerKind::Realtime;
  Res |= SanitizerKind::Fuzzer;
  Res |= SanitizerKind::FuzzerNoLink;
  Res |= SanitizerKind::KernelAddress;
  Res |= SanitizerKind::Vptr;
  Res |= SanitizerKind::SafeStack;
  if (IsX86_64 || IsMIPS64 || IsAArch64 || IsLoongArch64)
    Res |= SanitizerKind::DataFlow;
  if (IsX86_64 || IsMIPS64 || IsAArch64 || IsX86 || IsArmArch || IsPowerPC64 ||
      IsRISCV64 || IsSystemZ || IsHexagon || IsLoongArch64)
    Res |= SanitizerKind::Leak;
  if (IsX86_64 || IsMIPS64 || IsAArch64 || IsPowerPC64 || IsSystemZ ||
      IsLoongArch64 || IsRISCV64)
    Res |= SanitizerKind::Thread;
  if (IsX86_64 || IsAArch64)
    Res |= SanitizerKind::Type;
  if (IsX86_64 || IsSystemZ || IsPowerPC64)
    Res |= SanitizerKind::KernelMemory;
  if (IsX86_64 || IsMIPS64 || IsAArch64 || IsX86 || IsMIPS || IsArmArch ||
      IsPowerPC64 || IsHexagon || IsLoongArch64 || IsRISCV64)
    Res |= SanitizerKind::Scudo;
  if (IsX86_64 || IsAArch64 || IsRISCV64) {
    Res |= SanitizerKind::HWAddress;
  }
  if (IsX86_64 || IsAArch64) {
    Res |= SanitizerKind::KernelHWAddress;
  }
  if (IsX86_64)
    Res |= SanitizerKind::NumericalStability;
  if (!IsAndroid)
    Res |= SanitizerKind::Memory;

  // Work around "Cannot represent a difference across sections".
  if (getTriple().getArch() == toolchain::Triple::ppc64)
    Res &= ~SanitizerKind::Function;
  return Res;
}

void Linux::addProfileRTLibs(const toolchain::opt::ArgList &Args,
                             toolchain::opt::ArgStringList &CmdArgs) const {
  // Add linker option -u__toolchain_profile_runtime to cause runtime
  // initialization module to be linked in.
  if (needsProfileRT(Args))
    CmdArgs.push_back(Args.MakeArgString(
        Twine("-u", toolchain::getInstrProfRuntimeHookVarName())));
  ToolChain::addProfileRTLibs(Args, CmdArgs);
}

void Linux::addExtraOpts(toolchain::opt::ArgStringList &CmdArgs) const {
  for (const auto &Opt : ExtraOpts)
    CmdArgs.push_back(Opt.c_str());
}

const char *Linux::getDefaultLinker() const {
  if (getTriple().isAndroid())
    return "ld.lld";
  return Generic_ELF::getDefaultLinker();
}
