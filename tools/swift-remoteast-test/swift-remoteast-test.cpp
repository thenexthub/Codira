//===--- language-remoteast-test.cpp - RemoteAST testing application ---------===//
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
// This file supports performing target-specific remote reflection tests
// on live language executables.
//===----------------------------------------------------------------------===//

#include "language/RemoteAST/RemoteAST.h"
#include "language/Remote/InProcessMemoryReader.h"
#include "language/Remote/MetadataReader.h"
#include "language/Runtime/Metadata.h"
#include "language/Frontend/Frontend.h"
#include "language/FrontendTool/FrontendTool.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/Basic/InitializeCodiraModules.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Format.h"
#include "toolchain/Support/raw_ostream.h"
#include <cassert>

#if defined(__ELF__)
#define LANGUAGE_REMOTEAST_TEST_ABI __attribute__((__visibility__("default")))
#elif defined(__MACH__)
#define LANGUAGE_REMOTEAST_TEST_ABI __attribute__((__visibility__("default")))
#else
#define LANGUAGE_REMOTEAST_TEST_ABI __declspec(dllexport)
#endif

using namespace language;
using namespace language::remote;
using namespace language::remoteAST;

#if defined(__APPLE__) && defined(__MACH__)
#include <dlfcn.h>
static unsigned long long computeClassIsCodiraMask(void) {
  uintptr_t *objc_debug_language_stable_abi_bit_ptr =
    (uintptr_t *)dlsym(RTLD_DEFAULT, "objc_debug_language_stable_abi_bit");
  return objc_debug_language_stable_abi_bit_ptr ?
           *objc_debug_language_stable_abi_bit_ptr : 1;
}
#else
static unsigned long long computeClassIsCodiraMask(void) {
  return 1;
}
#endif

extern "C" unsigned long long _language_classIsCodiraMask =
  computeClassIsCodiraMask();

/// The context for the code we're running.  Set by the observer.
static ASTContext *context = nullptr;

/// The RemoteAST for the code we're running.
std::unique_ptr<RemoteASTContext> remoteContext;

static RemoteASTContext &getRemoteASTContext() {
  if (remoteContext)
    return *remoteContext;

  std::shared_ptr<MemoryReader> reader(new InProcessMemoryReader());
  remoteContext.reset(new RemoteASTContext(*context, std::move(reader)));
  return *remoteContext;
}

// FIXME: languagecall
/// fn printType(forMetadata: Any.Type)
extern "C" void LANGUAGE_REMOTEAST_TEST_ABI TOOLCHAIN_ATTRIBUTE_USED
printMetadataType(const Metadata *typeMetadata) {
  auto &remoteAST = getRemoteASTContext();
  auto &out = toolchain::outs();

  auto result = remoteAST.getTypeForRemoteTypeMetadata(RemoteAddress(
      (uint64_t)typeMetadata, RemoteAddress::DefaultAddressSpace));
  if (result) {
    out << "found type: ";
    result.getValue().print(out);
    out << '\n';
  } else {
    out << result.getFailure().render() << '\n';
  }
}

// FIXME: languagecall
/// fn printDynamicType(_: AnyObject)
extern "C" void LANGUAGE_REMOTEAST_TEST_ABI TOOLCHAIN_ATTRIBUTE_USED
printHeapMetadataType(void *object) {
  auto &remoteAST = getRemoteASTContext();
  auto &out = toolchain::outs();

  auto metadataResult = remoteAST.getHeapMetadataForObject(
      RemoteAddress((uint64_t)object, RemoteAddress::DefaultAddressSpace));
  if (!metadataResult) {
    out << metadataResult.getFailure().render() << '\n';
    return;
  }
  auto metadata = metadataResult.getValue();

  auto result =
    remoteAST.getTypeForRemoteTypeMetadata(metadata, /*skipArtificial*/ true);
  if (result) {
    out << "found type: ";
    result.getValue().print(out);
    out << '\n';
  } else {
    out << result.getFailure().render() << '\n';
  }
}

static void printMemberOffset(const Metadata *typeMetadata,
                              StringRef memberName, bool passMetadata) {
  auto &remoteAST = getRemoteASTContext();
  auto &out = toolchain::outs();

  // The first thing we have to do is get the type.
  auto typeResult = remoteAST.getTypeForRemoteTypeMetadata(RemoteAddress(
      (uint64_t)typeMetadata, RemoteAddress::DefaultAddressSpace));
  if (!typeResult) {
    out << "failed to find type: " << typeResult.getFailure().render() << '\n';
    return;
  }

  Type type = typeResult.getValue();

  RemoteAddress address =
      (passMetadata ? RemoteAddress((uint64_t)typeMetadata,
                                    RemoteAddress::DefaultAddressSpace)
                    : RemoteAddress());

  auto offsetResult =
    remoteAST.getOffsetOfMember(type, address, memberName);
  if (!offsetResult) {
    out << "failed to find offset: "
        << offsetResult.getFailure().render() << '\n';
    return;
  }

  out << "found offset: " << offsetResult.getValue() << '\n';
}

// FIXME: languagecall
/// fn printTypeMemberOffset(forType: Any.Type, memberName: StaticString)
extern "C" void LANGUAGE_REMOTEAST_TEST_ABI TOOLCHAIN_ATTRIBUTE_USED
printTypeMemberOffset(const Metadata *typeMetadata,
                                      const char *memberName) {
  printMemberOffset(typeMetadata, memberName, /*pass metadata*/ false);
}

// FIXME: languagecall
/// fn printTypeMetadataMemberOffset(forType: Any.Type,
///                                    memberName: StaticString)
extern "C" void LANGUAGE_REMOTEAST_TEST_ABI TOOLCHAIN_ATTRIBUTE_USED
printTypeMetadataMemberOffset(const Metadata *typeMetadata,
                              const char *memberName) {
  printMemberOffset(typeMetadata, memberName, /*pass metadata*/ true);
}

// FIXME: languagecall
/// fn printDynamicTypeAndAddressForExistential<T>(_: T)
extern "C" void LANGUAGE_REMOTEAST_TEST_ABI TOOLCHAIN_ATTRIBUTE_USED
printDynamicTypeAndAddressForExistential(void *object,
                                         const Metadata *typeMetadata) {
  auto &remoteAST = getRemoteASTContext();
  auto &out = toolchain::outs();

  // First, retrieve the static type of the existential, so we can understand
  // which kind of existential this is.
  auto staticTypeResult = remoteAST.getTypeForRemoteTypeMetadata(RemoteAddress(
      (uint64_t)typeMetadata, RemoteAddress::DefaultAddressSpace));
  if (!staticTypeResult) {
    out << "failed to resolve static type: "
        << staticTypeResult.getFailure().render() << '\n';
    return;
  }

  // OK, we can reconstruct the dynamic type and the address now.
  auto result = remoteAST.getDynamicTypeAndAddressForExistential(
      RemoteAddress((uint64_t)object, RemoteAddress::DefaultAddressSpace),
      staticTypeResult.getValue());
  if (result) {
    out << "found type: ";
    result.getValue().InstanceType.print(out);
    out << "\n";
  } else {
    out << result.getFailure().render() << '\n';
  }
}

// FIXME: languagecall
/// fn stopRemoteAST(_: AnyObject)
extern "C" void LANGUAGE_REMOTEAST_TEST_ABI TOOLCHAIN_ATTRIBUTE_USED
stopRemoteAST() {
  if (remoteContext)
    remoteContext.reset();
}

namespace {

struct Observer : public FrontendObserver {
  void configuredCompiler(CompilerInstance &instance) override {
    context = &instance.getASTContext();
  }
};

} // end anonymous namespace

int main(int argc, const char *argv[]) {
  PROGRAM_START(argc, argv);

  unsigned numForwardedArgs = argc
      - 1  // we drop argv[0]
      + 1; // -interpret

  SmallVector<const char *, 8> forwardedArgs;
  forwardedArgs.reserve(numForwardedArgs);
  forwardedArgs.append(&argv[1], &argv[argc]);
  forwardedArgs.push_back("-interpret");
  assert(forwardedArgs.size() == numForwardedArgs);

  initializeCodiraModules();

  Observer observer;
  return performFrontend(forwardedArgs, argv[0], (void*) &printMetadataType,
                         &observer);
}
