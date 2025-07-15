//===--- APIGen.h - Public interface to APIGen ------------------*- C++ -*-===//
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
#ifndef LANGUAGE_APIGEN_APIGEN_H
#define LANGUAGE_APIGEN_APIGEN_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/BitmaskEnum.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/Error.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace toolchain {
class raw_ostream;
}

namespace language {
namespace apigen {
TOOLCHAIN_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();
enum class APIAccess : uint8_t {
  Unknown = 0, // No information about access.
  Project = 1, // APIs available within the project.
  Private = 2, // Private unstable APIs.
  Public  = 3, // Public stable APIs.
};

enum class APILinkage : uint8_t {
  Unknown    = 0, // Unknown.
  Internal   = 1, // API is internal.
  External   = 2, // External interface used.
  Reexported = 3, // API is re-exported.
  Exported   = 4, // API is exported.
};

enum class APIFlags : uint8_t {
  None             = 0,
  ThreadLocalValue = 1U << 0,
  WeakDefined      = 1U << 1,
  WeakReferenced   = 1U << 2,
  TOOLCHAIN_MARK_AS_BITMASK_ENUM(/*LargestValue=*/WeakReferenced)
};

class APILoc {
public:
  APILoc() = default;
  APILoc(std::string file, unsigned line, unsigned col)
      : file(file), line(line), col(col) {}

  StringRef getFilename() const { return file; }
  unsigned getLine() const { return line; }
  unsigned getColumn() const { return col; }

private:
  std::string file;
  unsigned line;
  unsigned col;
};

struct APIAvailability {
  std::string introduced;
  std::string obsoleted;
  bool unavailable = false;

  bool empty() {
    return introduced.empty() && obsoleted.empty() && !unavailable;
  }
};

struct APIRecord {
  std::string name;
  APILoc loc;
  APILinkage linkage;
  APIFlags flags;
  APIAccess access;
  APIAvailability availability;

  APIRecord(StringRef name, APILoc loc, APILinkage linkage, APIFlags flags,
            APIAccess access, APIAvailability availability)
      : name(name.data(), name.size()), loc(loc), linkage(linkage),
        flags(flags), access(access), availability(availability) {}

  bool isWeakDefined() const {
    return (flags & APIFlags::WeakDefined) == APIFlags::WeakDefined;
  }

  bool isWeakReferenced() const {
    return (flags & APIFlags::WeakReferenced) == APIFlags::WeakReferenced;
  }

  bool isThreadLocalValue() const {
    return (flags & APIFlags::ThreadLocalValue) == APIFlags::ThreadLocalValue;
  }

  bool isExternal() const { return linkage == APILinkage::External; }
  bool isExported() const { return linkage >= APILinkage::Reexported; }
  bool isReexported() const { return linkage == APILinkage::Reexported; }
};

enum class GVKind : uint8_t {
  Unknown = 0,
  Variable = 1,
  Function = 2,
};

struct GlobalRecord : APIRecord {
  GVKind kind;

  GlobalRecord(StringRef name, APILoc loc, APILinkage linkage, APIFlags flags,
               APIAccess access, GVKind kind, APIAvailability availability)
      : APIRecord(name, loc, linkage, flags, access, availability), kind(kind) {
  }
};

struct ObjCMethodRecord : APIRecord {
  bool isInstanceMethod;
  bool isOptional;

  ObjCMethodRecord(StringRef name, APILoc loc, APIAccess access,
                   bool isInstanceMethod, bool isOptional,
                   APIAvailability availability)
      : APIRecord(name, loc, APILinkage::Unknown, APIFlags::None, access,
                  availability),
        isInstanceMethod(isInstanceMethod), isOptional(isOptional) {}
};

struct ObjCContainerRecord : APIRecord {
  std::vector<ObjCMethodRecord*> methods;

  ObjCContainerRecord(StringRef name, APILinkage linkage, APILoc loc,
                      APIAccess access, const APIAvailability &availability)
      : APIRecord(name, loc, linkage, APIFlags::None, access, availability) {}
};

struct ObjCInterfaceRecord : ObjCContainerRecord {
  std::string superClassName;
  ObjCInterfaceRecord(StringRef name, APILinkage linkage, APILoc loc,
                      APIAccess access, APIAvailability availability,
                      StringRef superClassName)
      : ObjCContainerRecord(name, linkage, loc, access, availability),
        superClassName(superClassName.data(), superClassName.size()) {}
};

struct ObjCCategoryRecord : ObjCContainerRecord {
  std::string interface;

  ObjCCategoryRecord(StringRef name, APILinkage linkage, APILoc loc,
                     APIAccess access, APIAvailability availability,
                     StringRef interface)
      : ObjCContainerRecord(name, linkage, loc, access, availability),
        interface(interface.data(), interface.size()) {}
};

class API {
public:
  API(const toolchain::Triple &triple) : target(triple) {}

  const toolchain::Triple &getTarget() const { return target; }

  void addSymbol(StringRef symbol, APILoc loc, APILinkage linkage,
                 APIFlags flags, APIAccess access,
                 APIAvailability availability);

  ObjCInterfaceRecord *addObjCClass(StringRef name, APILinkage linkage,
                                    APILoc loc, APIAccess access,
                                    APIAvailability availability,
                                    StringRef superClassName);

  ObjCCategoryRecord *addObjCCategory(StringRef name, APILinkage linkage,
                                      APILoc loc, APIAccess access,
                                      APIAvailability availability,
                                      StringRef interface);

  void addObjCMethod(ObjCContainerRecord *record, StringRef name, APILoc loc,
                     APIAccess access, bool isInstanceMethod, bool isOptional,
                     APIAvailability availability);

  void writeAPIJSONFile(raw_ostream &os, bool PrettyPrint = false);

private:
  const toolchain::Triple target;

  toolchain::BumpPtrAllocator allocator;
  std::vector<GlobalRecord*> globals;
  std::vector<ObjCInterfaceRecord*> interfaces;
  std::vector<ObjCCategoryRecord *> categories;
};

} // end namespace apigen
} // end namespace language

#endif
