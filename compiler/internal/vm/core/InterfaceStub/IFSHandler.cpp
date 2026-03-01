//===- IFSHandler.cpp -----------------------------------------------------===//
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
//===-----------------------------------------------------------------------===/

#include "vm/core/InterfaceStub/IFSHandler.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/InterfaceStub/IFSStub.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/GlobPattern.h"
#include "vm/core/Support/LineIterator.h"
#include "vm/core/Support/YAMLTraits.h"
#include "vm/core/TargetParser/Triple.h"
#include <functional>
#include <optional>

using namespace vm::core;
using namespace vm::core::ifs;

LLVM_YAML_IS_SEQUENCE_VECTOR(IFSSymbol)

namespace vm::core {
namespace yaml {

/// YAML traits for ELFSymbolType.
template <> struct ScalarEnumerationTraits<IFSSymbolType> {
  static void enumeration(IO &IO, IFSSymbolType &SymbolType) {
    IO.enumCase(SymbolType, "NoType", IFSSymbolType::NoType);
    IO.enumCase(SymbolType, "Func", IFSSymbolType::Func);
    IO.enumCase(SymbolType, "Object", IFSSymbolType::Object);
    IO.enumCase(SymbolType, "TLS", IFSSymbolType::TLS);
    IO.enumCase(SymbolType, "Unknown", IFSSymbolType::Unknown);
    // Treat other symbol types as noise, and map to Unknown.
    if (!IO.outputting() && IO.matchEnumFallback())
      SymbolType = IFSSymbolType::Unknown;
  }
};

template <> struct ScalarTraits<IFSEndiannessType> {
  static void output(const IFSEndiannessType &Value, void *,
                     toolchain::raw_ostream &Out) {
    switch (Value) {
    case IFSEndiannessType::Big:
      Out << "big";
      break;
    case IFSEndiannessType::Little:
      Out << "little";
      break;
    default:
      llvm_unreachable("Unsupported endianness");
    }
  }

  static StringRef input(StringRef Scalar, void *, IFSEndiannessType &Value) {
    Value = StringSwitch<IFSEndiannessType>(Scalar)
                .Case("big", IFSEndiannessType::Big)
                .Case("little", IFSEndiannessType::Little)
                .Default(IFSEndiannessType::Unknown);
    if (Value == IFSEndiannessType::Unknown) {
      return "Unsupported endianness";
    }
    return StringRef();
  }

  static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

template <> struct ScalarTraits<IFSBitWidthType> {
  static void output(const IFSBitWidthType &Value, void *,
                     toolchain::raw_ostream &Out) {
    switch (Value) {
    case IFSBitWidthType::IFS32:
      Out << "32";
      break;
    case IFSBitWidthType::IFS64:
      Out << "64";
      break;
    default:
      llvm_unreachable("Unsupported bit width");
    }
  }

  static StringRef input(StringRef Scalar, void *, IFSBitWidthType &Value) {
    Value = StringSwitch<IFSBitWidthType>(Scalar)
                .Case("32", IFSBitWidthType::IFS32)
                .Case("64", IFSBitWidthType::IFS64)
                .Default(IFSBitWidthType::Unknown);
    if (Value == IFSBitWidthType::Unknown) {
      return "Unsupported bit width";
    }
    return StringRef();
  }

  static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

template <> struct MappingTraits<IFSTarget> {
  static void mapping(IO &IO, IFSTarget &Target) {
    IO.mapOptional("ObjectFormat", Target.ObjectFormat);
    IO.mapOptional("Arch", Target.ArchString);
    IO.mapOptional("Endianness", Target.Endianness);
    IO.mapOptional("BitWidth", Target.BitWidth);
  }

  // Compacts symbol information into a single line.
  static const bool flow = true; // NOLINT(readability-identifier-naming)
};

/// YAML traits for ELFSymbol.
template <> struct MappingTraits<IFSSymbol> {
  static void mapping(IO &IO, IFSSymbol &Symbol) {
    IO.mapRequired("Name", Symbol.Name);
    IO.mapRequired("Type", Symbol.Type);
    // The need for symbol size depends on the symbol type.
    if (Symbol.Type == IFSSymbolType::NoType) {
      // Size is None, so we are reading it in, or it is non 0 so we
      // should emit it.
      if (!Symbol.Size || *Symbol.Size)
        IO.mapOptional("Size", Symbol.Size);
    } else if (Symbol.Type != IFSSymbolType::Func) {
      IO.mapOptional("Size", Symbol.Size);
    }
    IO.mapOptional("Undefined", Symbol.Undefined, false);
    IO.mapOptional("Weak", Symbol.Weak, false);
    IO.mapOptional("Warning", Symbol.Warning);
  }

  // Compacts symbol information into a single line.
  static const bool flow = true; // NOLINT(readability-identifier-naming)
};

/// YAML traits for ELFStub objects.
template <> struct MappingTraits<IFSStub> {
  static void mapping(IO &IO, IFSStub &Stub) {
    if (!IO.mapTag("!ifs-v1", true))
      IO.setError("Not a .tbe YAML file.");
    IO.mapRequired("IfsVersion", Stub.IfsVersion);
    IO.mapOptional("SoName", Stub.SoName);
    IO.mapOptional("Target", Stub.Target);
    IO.mapOptional("NeededLibs", Stub.NeededLibs);
    IO.mapRequired("Symbols", Stub.Symbols);
  }
};

/// YAML traits for ELFStubTriple objects.
template <> struct MappingTraits<IFSStubTriple> {
  static void mapping(IO &IO, IFSStubTriple &Stub) {
    if (!IO.mapTag("!ifs-v1", true))
      IO.setError("Not a .tbe YAML file.");
    IO.mapRequired("IfsVersion", Stub.IfsVersion);
    IO.mapOptional("SoName", Stub.SoName);
    IO.mapOptional("Target", Stub.Target.Triple);
    IO.mapOptional("NeededLibs", Stub.NeededLibs);
    IO.mapRequired("Symbols", Stub.Symbols);
  }
};
} // end namespace yaml
} // end namespace vm::core

/// Attempt to determine if a Text stub uses target triple.
bool usesTriple(StringRef Buf) {
  for (line_iterator I(MemoryBufferRef(Buf, "ELFStub")); !I.is_at_eof(); ++I) {
    StringRef Line = (*I).trim();
    if (Line.starts_with("Target:")) {
      if (Line == "Target:" || Line.contains("{")) {
        return false;
      }
    }
  }
  return true;
}

Expected<std::unique_ptr<IFSStub>> ifs::readIFSFromBuffer(StringRef Buf) {
  yaml::Input YamlIn(Buf);
  std::unique_ptr<IFSStubTriple> Stub(new IFSStubTriple());
  if (usesTriple(Buf)) {
    YamlIn >> *Stub;
  } else {
    YamlIn >> *static_cast<IFSStub *>(Stub.get());
  }
  if (std::error_code Err = YamlIn.error()) {
    return createStringError(Err, "YAML failed reading as IFS");
  }

  if (Stub->IfsVersion > IFSVersionCurrent)
    return make_error<StringError>(
        "IFS version " + Stub->IfsVersion.getAsString() + " is unsupported.",
        std::make_error_code(std::errc::invalid_argument));
  if (Stub->Target.ArchString) {
    uint16_t eMachine =
        ELF::convertArchNameToEMachine(*Stub->Target.ArchString);
    if (eMachine == ELF::EM_NONE)
      return createStringError(
          std::make_error_code(std::errc::invalid_argument),
          "IFS arch '" + *Stub->Target.ArchString + "' is unsupported");
    Stub->Target.Arch = eMachine;
  }
  for (const auto &Item : Stub->Symbols) {
    if (Item.Type == IFSSymbolType::Unknown)
      return createStringError(
          std::make_error_code(std::errc::invalid_argument),
          "IFS symbol type for symbol '" + Item.Name + "' is unsupported");
  }
  return std::move(Stub);
}

Error ifs::writeIFSToOutputStream(raw_ostream &OS, const IFSStub &Stub) {
  yaml::Output YamlOut(OS, nullptr, /*WrapColumn =*/0);
  std::unique_ptr<IFSStubTriple> CopyStub(new IFSStubTriple(Stub));
  if (Stub.Target.Arch) {
    CopyStub->Target.ArchString =
        std::string(ELF::convertEMachineToArchName(*Stub.Target.Arch));
  }
  IFSTarget Target = Stub.Target;

  if (CopyStub->Target.Triple ||
      (!CopyStub->Target.ArchString && !CopyStub->Target.Endianness &&
       !CopyStub->Target.BitWidth))
    YamlOut << *CopyStub;
  else
    YamlOut << *static_cast<IFSStub *>(CopyStub.get());
  return Error::success();
}

Error ifs::overrideIFSTarget(
    IFSStub &Stub, std::optional<IFSArch> OverrideArch,
    std::optional<IFSEndiannessType> OverrideEndianness,
    std::optional<IFSBitWidthType> OverrideBitWidth,
    std::optional<std::string> OverrideTriple) {
  std::error_code OverrideEC(1, std::generic_category());
  if (OverrideArch) {
    if (Stub.Target.Arch && *Stub.Target.Arch != *OverrideArch) {
      return make_error<StringError>(
          "Supplied Arch conflicts with the text stub", OverrideEC);
    }
    Stub.Target.Arch = *OverrideArch;
  }
  if (OverrideEndianness) {
    if (Stub.Target.Endianness &&
        *Stub.Target.Endianness != *OverrideEndianness) {
      return make_error<StringError>(
          "Supplied Endianness conflicts with the text stub", OverrideEC);
    }
    Stub.Target.Endianness = *OverrideEndianness;
  }
  if (OverrideBitWidth) {
    if (Stub.Target.BitWidth && *Stub.Target.BitWidth != *OverrideBitWidth) {
      return make_error<StringError>(
          "Supplied BitWidth conflicts with the text stub", OverrideEC);
    }
    Stub.Target.BitWidth = *OverrideBitWidth;
  }
  if (OverrideTriple) {
    if (Stub.Target.Triple && *Stub.Target.Triple != *OverrideTriple) {
      return make_error<StringError>(
          "Supplied Triple conflicts with the text stub", OverrideEC);
    }
    Stub.Target.Triple = *OverrideTriple;
  }
  return Error::success();
}

Error ifs::validateIFSTarget(IFSStub &Stub, bool ParseTriple) {
  std::error_code ValidationEC(1, std::generic_category());
  if (Stub.Target.Triple) {
    if (Stub.Target.Arch || Stub.Target.BitWidth || Stub.Target.Endianness ||
        Stub.Target.ObjectFormat) {
      return make_error<StringError>(
          "Target triple cannot be used simultaneously with ELF target format",
          ValidationEC);
    }
    if (ParseTriple) {
      IFSTarget TargetFromTriple = parseTriple(*Stub.Target.Triple);
      Stub.Target.Arch = TargetFromTriple.Arch;
      Stub.Target.BitWidth = TargetFromTriple.BitWidth;
      Stub.Target.Endianness = TargetFromTriple.Endianness;
    }
    return Error::success();
  }
  if (!Stub.Target.Arch || !Stub.Target.BitWidth || !Stub.Target.Endianness) {
    // TODO: unify the error message.
    if (!Stub.Target.Arch) {
      return make_error<StringError>("Arch is not defined in the text stub",
                                     ValidationEC);
    }
    if (!Stub.Target.BitWidth) {
      return make_error<StringError>("BitWidth is not defined in the text stub",
                                     ValidationEC);
    }
    if (!Stub.Target.Endianness) {
      return make_error<StringError>(
          "Endianness is not defined in the text stub", ValidationEC);
    }
  }
  return Error::success();
}

IFSTarget ifs::parseTriple(StringRef TripleStr) {
  Triple IFSTriple(TripleStr);
  IFSTarget RetTarget;
  IFSArch TripleArch =
      ELF::convertTripleArchTypeToEMachine(IFSTriple.getArch());
  if (TripleArch != ELF::EM_NONE) {
    RetTarget.Arch = TripleArch;
  }
  RetTarget.Endianness = IFSTriple.isLittleEndian() ? IFSEndiannessType::Little
                                                    : IFSEndiannessType::Big;
  RetTarget.BitWidth =
      IFSTriple.isArch64Bit() ? IFSBitWidthType::IFS64 : IFSBitWidthType::IFS32;
  return RetTarget;
}

void ifs::stripIFSTarget(IFSStub &Stub, bool StripTriple, bool StripArch,
                         bool StripEndianness, bool StripBitWidth) {
  if (StripTriple || StripArch) {
    Stub.Target.Arch.reset();
    Stub.Target.ArchString.reset();
  }
  if (StripTriple || StripEndianness) {
    Stub.Target.Endianness.reset();
  }
  if (StripTriple || StripBitWidth) {
    Stub.Target.BitWidth.reset();
  }
  if (StripTriple) {
    Stub.Target.Triple.reset();
  }
  if (!Stub.Target.Arch && !Stub.Target.BitWidth && !Stub.Target.Endianness) {
    Stub.Target.ObjectFormat.reset();
  }
}

Error ifs::filterIFSSyms(IFSStub &Stub, bool StripUndefined,
                         const std::vector<std::string> &Exclude) {
  std::function<bool(const IFSSymbol &)> Filter = [](const IFSSymbol &) {
    return false;
  };

  if (StripUndefined) {
    Filter = [Filter](const IFSSymbol &Sym) {
      return Sym.Undefined || Filter(Sym);
    };
  }

  for (StringRef Glob : Exclude) {
    Expected<toolchain::GlobPattern> PatternOrErr = toolchain::GlobPattern::create(Glob);
    if (!PatternOrErr)
      return PatternOrErr.takeError();
    Filter = [Pattern = *PatternOrErr, Filter](const IFSSymbol &Sym) {
      return Pattern.match(Sym.Name) || Filter(Sym);
    };
  }

  toolchain::erase_if(Stub.Symbols, Filter);

  return Error::success();
}
