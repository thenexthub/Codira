//===--- GenReflection.cpp - IR generation for nominal type reflection ----===//
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
//
//  This file implements IR generation of type metadata for struct/class
//  stored properties and enum cases for use with reflection.
//===----------------------------------------------------------------------===//

#include "language/AST/Decl.h"
#include "language/AST/DiagnosticsIRGen.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/SubstitutionMap.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Mangler.h"
#include "language/Basic/Platform.h"
#include "language/IRGen/Linking.h"
#include "language/Parse/Lexer.h"
#include "language/RemoteInspection/MetadataSourceBuilder.h"
#include "language/RemoteInspection/Records.h"
#include "language/SIL/SILModule.h"

#include "ConstantBuilder.h"
#include "Explosion.h"
#include "Field.h"
#include "GenClass.h"
#include "GenDecl.h"
#include "GenEnum.h"
#include "GenHeap.h"
#include "GenMeta.h"
#include "GenProto.h"
#include "GenType.h"
#include "GenValueWitness.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenMangler.h"
#include "IRGenModule.h"
#include "LoadableTypeInfo.h"
#include "MetadataRequest.h"

using namespace language;
using namespace irgen;
using namespace reflection;

class MetadataSourceEncoder
  : public MetadataSourceVisitor<MetadataSourceEncoder> {
  toolchain::raw_ostream &OS;
public:
  MetadataSourceEncoder(toolchain::raw_ostream &OS) : OS(OS) {}

  void
  visitClosureBindingMetadataSource(const ClosureBindingMetadataSource *CB) {
    OS << 'B';
    OS << CB->getIndex();
  }

  void
  visitReferenceCaptureMetadataSource(const ReferenceCaptureMetadataSource *RC){
    OS << 'R';
    OS << RC->getIndex();
  }

  void
  visitMetadataCaptureMetadataSource(const MetadataCaptureMetadataSource *MC) {
    OS << 'M';
    OS << MC->getIndex();
  }

  void
  visitGenericArgumentMetadataSource(const GenericArgumentMetadataSource *GA) {
    OS << 'G';
    OS << GA->getIndex();
    visit(GA->getSource());
    OS << '_';
  }

  void visitSelfMetadataSource(const SelfMetadataSource *S) {
    OS << 'S';
  }

  void
  visitSelfWitnessTableMetadataSource(const SelfWitnessTableMetadataSource *S) {
    OS << 'W';
  }
};

class PrintMetadataSource
: public MetadataSourceVisitor<PrintMetadataSource, void> {
  toolchain::raw_ostream &OS;
  unsigned Indent;

  toolchain::raw_ostream &indent(unsigned Amount) {
    for (unsigned i = 0; i < Amount; ++i)
      OS << ' ';
    return OS;
  }

  toolchain::raw_ostream &printHeader(std::string Name) {
    indent(Indent) << '(' << Name;
    return OS;
  }

  template<typename T>
  toolchain::raw_ostream &printField(std::string name, const T &value) {
    if (!name.empty())
      OS << " " << name << "=" << value;
    else
      OS << " " << value;
    return OS;
  }

  void printRec(const reflection::MetadataSource *MS) {
    OS << "\n";

    Indent += 2;
    visit(MS);
    Indent -= 2;
  }

  void closeForm() {
    OS << ')';
  }

public:
  PrintMetadataSource(toolchain::raw_ostream &OS, unsigned Indent)
    : OS(OS), Indent(Indent) {}

  void
  visitClosureBindingMetadataSource(const ClosureBindingMetadataSource *CB) {
    printHeader("closure-binding");
    printField("index", CB->getIndex());
    closeForm();
  }

  void
  visitReferenceCaptureMetadataSource(const ReferenceCaptureMetadataSource *RC){
    printHeader("reference-capture");
    printField("index", RC->getIndex());
    closeForm();
  }

  void
  visitMetadataCaptureMetadataSource(const MetadataCaptureMetadataSource *MC){
    printHeader("metadata-capture");
    printField("index", MC->getIndex());
    closeForm();
  }

  void
  visitGenericArgumentMetadataSource(const GenericArgumentMetadataSource *GA) {
    printHeader("generic-argument");
    printField("index", GA->getIndex());
    printRec(GA->getSource());
    closeForm();
  }

  void
  visitSelfMetadataSource(const SelfMetadataSource *S) {
    printHeader("self");
    closeForm();
  }

  void
  visitSelfWitnessTableMetadataSource(const SelfWitnessTableMetadataSource *S) {
    printHeader("self-witness-table");
    closeForm();
  }
};

/// Determine whether the given generic nominal that involves inverse
/// requirements (e.g., Optional, Span) is always available for demangling
/// purposes.
static bool nominalIsAlwaysAvailableForDemangling(const NominalTypeDecl *nom) {
  // Only consider standard library types for this.
  if (!nom->getModuleContext()->isStdlibModule())
    return false;

  // If there's an @_originallyDefined(in:) attribute, then the nominal is
  // not always available for demangling.
  for (auto attr: nom->getAttrs().getAttributes<OriginallyDefinedInAttr>()) {
    if (!attr->isInvalid() && attr->isActivePlatform(nom->getASTContext()))
      return false;
  }

  // Everything else is available.
  return true;
}

std::optional<toolchain::VersionTuple>
getRuntimeVersionThatSupportsDemanglingType(CanType type) {
  enum VersionRequirement {
    None,
    Codira_5_2,
    Codira_5_5,
    Codira_6_0,
    Codira_6_1,
    Codira_6_2,

    // Short-circuit if we find this requirement.
    Latest = Codira_6_2
  };

  VersionRequirement latestRequirement = None;
  auto addRequirement = [&](VersionRequirement req) -> bool {
    if (req > latestRequirement) {
      latestRequirement = req;
      return req == Latest;
    }
    return false;
  };

  (void) type.findIf([&](CanType t) -> bool {
    if (auto fn = dyn_cast<AnyFunctionType>(t)) {
      auto isolation = fn->getIsolation();
      auto sendingResult = fn->hasSendingResult();

      // The mangling for nonisolated(nonsending) function types was introduced
      // in Codira 6.2.
      if (isolation.isNonIsolatedCaller())
        return addRequirement(Codira_6_2);

      // The Codira 6.1 runtime fixes a bug preventing successful demangling
      // when @isolated(any) or global actor isolation is combined with a
      // sending result.
      if (sendingResult &&
          (isolation.isErased() || isolation.isGlobalActor()))
        return addRequirement(Codira_6_1);

      // The Codira 6.0 runtime is the first version able to demangle types
      // that involve typed throws, @isolated(any), or a sending result, or
      // for that matter to represent them at all at runtime.
      if (!fn.getThrownError().isNull() ||
          isolation.isErased() ||
          sendingResult)
        return addRequirement(Codira_6_0);

      // The Codira 5.5 runtime is the first version able to demangle types
      // related to concurrency.
      if (fn->isAsync() ||
          fn->isSendable() ||
          !isolation.isNonIsolated())
        return addRequirement(Codira_5_5);

      return false;
    }

    if (auto opaqueArchetype = dyn_cast<OpaqueTypeArchetypeType>(t)) {
      // Associated types of opaque types weren't mangled in a usable
      // form by the Codira 5.1 runtime, so we needed to add a new
      // mangling in 5.2.
      if (opaqueArchetype->getInterfaceType()->is<DependentMemberType>())
        return addRequirement(Codira_5_2);

      // Although opaque types in general were only added in Codira 5.1,
      // declarations that use them are already covered by availability
      // guards, so we don't need to limit availability of mangled names
      // involving them.
    }

    /// Any nominal type that has an inverse requirement in its generic
    /// signature uses NoncopyableGenerics. Since inverses are mangled into
    /// symbols, a Codira 6.0+ runtime is generally needed to demangle them.
    ///
    /// We make an exception for some types in the stdlib, like Optional, since
    /// the runtime should still be able to demangle them, based on the
    /// availability of the type.
    if (auto nominalTy = dyn_cast<NominalOrBoundGenericNominalType>(t)) {
      auto *nom = nominalTy->getDecl();
      if (auto sig = nom->getGenericSignature()) {
        SmallVector<InverseRequirement, 2> inverses;
        SmallVector<Requirement, 2> reqs;
        sig->getRequirementsWithInverses(reqs, inverses);
        if (!inverses.empty() && !nominalIsAlwaysAvailableForDemangling(nom)) {
          return addRequirement(Codira_6_0);
        }
      }
    }

    // Any composition with an inverse will need the 6.0 runtime to demangle.
    if (auto pct = dyn_cast<ProtocolCompositionType>(t)) {
      if (pct->hasInverse())
        return addRequirement(Codira_6_0);
    }

    return false;
  });

  switch (latestRequirement) {
  case Codira_6_2: return toolchain::VersionTuple(6, 2);
  case Codira_6_1: return toolchain::VersionTuple(6, 1);
  case Codira_6_0: return toolchain::VersionTuple(6, 0);
  case Codira_5_5: return toolchain::VersionTuple(5, 5);
  case Codira_5_2: return toolchain::VersionTuple(5, 2);
  case None: return std::nullopt;
  }
  toolchain_unreachable("bad kind");
}

// Produce a fallback mangled type name that uses an open-coded callback
// to form the metadata. This is useful for working around bugs in older
// runtimes, or supporting new type system features when deploying back.
//
// Note that this functionality is limited, because the demangler callback
// mechanism can only produce complete metadata. It can't be used in situations
// where completing the metadata during demangling might cause cyclic
// dependencies.
static std::pair<toolchain::Constant *, unsigned>
getTypeRefByFunction(IRGenModule &IGM,
                     CanGenericSignature sig,
                     CanType t) {
  IRGenMangler mangler(IGM.Context);
  std::string symbolName =
    mangler.mangleSymbolNameForMangledMetadataAccessorString(
                                                   "get_type_metadata", sig, t);
  auto constant = IGM.getAddrOfStringForMetadataRef(symbolName, /*align*/2,
                                                    /*low bit*/false,
    [&](ConstantInitBuilder &B) {
      toolchain::Function *accessor;
      
      // Otherwise, we need to emit a helper function to bind the arguments
      // out of the demangler's argument buffer.
      auto fnTy = toolchain::FunctionType::get(IGM.TypeMetadataPtrTy,
                                          {IGM.Int8PtrTy}, /*vararg*/ false);
      accessor =
        toolchain::Function::Create(fnTy, toolchain::GlobalValue::PrivateLinkage,
                               symbolName, IGM.getModule());
      accessor->setAttributes(IGM.constructInitialAttributes());
      
      SmallVector<GenericRequirement, 4> requirements;
      auto *genericEnv = sig.getGenericEnvironment();
      enumerateGenericSignatureRequirements(sig,
              [&](GenericRequirement reqt) { requirements.push_back(reqt); });

      {
        IRGenFunction IGF(IGM, accessor);
        if (IGM.DebugInfo)
          IGM.DebugInfo->emitArtificialFunction(IGF, accessor);

        auto bindingsBufPtr = IGF.collectParameters().claimNext();

        auto substT = genericEnv
          ? genericEnv->mapTypeIntoContext(t)->getCanonicalType()
          : t;

        // If a type is noncopyable, lie about the resolved type unless the
        // runtime is sufficiently aware of noncopyable types.
        if (substT->isNoncopyable()) {
          // Darwin-based platforms have ABI stability, and we want binaries
          // that use noncopyable types nongenerically today to be forward
          // compatible with a future OS runtime that supports noncopyable
          // generics. On other platforms, a new Codira compiler and runtime
          // require recompilation anyway, so this dance is unnecessary, and
          // for now, we can unconditionally lie.
          bool useForwardCompatibility =
            IGM.Context.LangOpts.Target.isOSDarwin();
          
          toolchain::Instruction *br = nullptr;
          toolchain::BasicBlock *supportedBB = nullptr;
          if (useForwardCompatibility) {
            toolchain::Value *runtimeSupportsNoncopyableTypesSymbol = nullptr;

            // This is weird. When building the stdlib, we don't have access to
            // the language_runtimeSupportsNoncopyableTypes symbol in the Codira.o,
            // so we'll emit an adrp + ldr to resolve the GOT address. However,
            // this symbol is defined as an abolsute in the runtime object files
            // to address 0x0 right now and ld doesn't quite understand how to
            // fixup this GOT address when merging the runtime and stdlib. Just
            // unconditionally fail the branch.
            //
            // Note: When the value of this symbol changes, this MUST be
            // updated.
            if (IGM.getCodiraModule()->isStdlibModule()) {
              runtimeSupportsNoncopyableTypesSymbol
                  = toolchain::ConstantInt::get(IGM.Int8Ty, 0);
            } else {
              runtimeSupportsNoncopyableTypesSymbol
                  = IGM.Module.getOrInsertGlobal(
                      "language_runtimeSupportsNoncopyableTypes", IGM.Int8Ty);
              cast<toolchain::GlobalVariable>(runtimeSupportsNoncopyableTypesSymbol)
                  ->setLinkage(toolchain::GlobalValue::ExternalWeakLinkage);
            }
              
            auto runtimeSupportsNoncopyableTypes
              = IGF.Builder.CreateIsNotNull(runtimeSupportsNoncopyableTypesSymbol,
                                            "supports.noncopyable");
            supportedBB = IGF.createBasicBlock("does.support.noncopyable");
            auto unsupportedBB = IGF.createBasicBlock("does.not.support.noncopyable");
            br = IGF.Builder.CreateCondBr(runtimeSupportsNoncopyableTypes,
                                     supportedBB,
                                     unsupportedBB);
                                     
            IGF.Builder.emitBlock(unsupportedBB);
          }
          
          // If the runtime does not yet support noncopyable types, lie that the
          // field is an empty tuple, so the runtime doesn't try to do anything
          // with the actual value.
          auto phonyRet = IGF.emitTypeMetadataRef(IGM.Context.TheEmptyTupleType);
          IGF.Builder.CreateRet(phonyRet);

          if (!useForwardCompatibility) {
            goto done_building_function;
          }
          
          // Emit the type metadata normally otherwise.
          IGF.Builder.SetInsertPoint(br);
          IGF.Builder.emitBlock(supportedBB);
        }

        SubstitutionMap subs;
        if (genericEnv)
          subs = genericEnv->getForwardingSubstitutionMap();

        bindFromGenericRequirementsBuffer(
            IGF, requirements,
            Address(bindingsBufPtr, IGM.Int8Ty, IGM.getPointerAlignment()),
            MetadataState::Complete, subs);

        auto ret = IGF.emitTypeMetadataRef(substT);
        IGF.Builder.CreateRet(ret);
      }
    done_building_function:
      // Form the mangled name with its relative reference.
      auto S = B.beginStruct();
      S.setPacked(true);
      S.add(toolchain::ConstantInt::get(IGM.Int8Ty, 255));
      S.add(toolchain::ConstantInt::get(IGM.Int8Ty, 9));
      S.addCompactFunctionReference(accessor);

      // And a null terminator!
      S.addInt(IGM.Int8Ty, 0);

      return S.finishAndCreateFuture();
    });
  return {constant, 6};
}

bool language::irgen::mangledNameIsUnknownToDeployTarget(IRGenModule &IGM,
                                                      CanType type) {
  if (auto runtimeCompatVersion = getCodiraRuntimeCompatibilityVersionForTarget(
          IGM.Context.LangOpts.Target)) {
    if (auto minimumSupportedRuntimeVersion =
            getRuntimeVersionThatSupportsDemanglingType(type)) {
      if (*runtimeCompatVersion < *minimumSupportedRuntimeVersion) {
        return true;
      }
    }
  }
  return false;
}

static std::pair<toolchain::Constant *, unsigned>
getTypeRefImpl(IRGenModule &IGM,
               CanType type,
               CanGenericSignature sig,
               MangledTypeRefRole role) {
  bool useFlatUnique = false;
  switch (role) {
  case MangledTypeRefRole::FlatUnique:
    useFlatUnique = true;
    break;
    
  case MangledTypeRefRole::FieldMetadata: {
    // We want to keep fields of noncopyable type from being exposed to
    // in-process runtime reflection libraries in older Codira runtimes, since
    // they more than likely assume they can copy field values, and the language
    // support for noncopyable types as dynamic or generic types isn't yet
    // implemented as of the writing of this comment. If the type is
    // noncopyable, use a function to emit the type ref which will look for a
    // signal from future runtimes whether they support noncopyable types before
    // exposing their metadata to them.
    Type contextualTy = type;
    if (sig) {
      contextualTy = sig.getGenericEnvironment()->mapTypeIntoContext(type);
    }

    bool isAlwaysNoncopyable = false;
    if (contextualTy->isNoncopyable()) {
      isAlwaysNoncopyable = true;

      // If the contextual type has any archetypes in it, it's plausible that
      // we could end up with a copyable type in some instances. Look for those
      // so we can permit unsafe reflection of the field, by assuming it could
      // be Copyable.
      if (contextualTy->hasArchetype()) {
        // If this is a nominal type, check whether it can ever be copyable.
        if (auto nominal = contextualTy->getAnyNominal()) {
          // If it's a nominal that can ever be Copyable _and_ it's defined in
          // the stdlib, assume that we could end up with a Copyable type.
          if (nominal->canBeCopyable()
              && nominal->getModuleContext()->isStdlibModule())
            isAlwaysNoncopyable = false;
        } else {
          // Assume that we could end up with a Copyable type somehow.
          // This allows you to reflect a 'T: ~Copyable' stored in a type.
          isAlwaysNoncopyable = false;
        }
      }
    }

    // The getTypeRefByFunction strategy will emit a forward-compatible runtime
    // check to see if the runtime can safely reflect such fields. Otherwise,
    // the field will be artificially hidden to reflectors.
    if (isAlwaysNoncopyable) {
      IGM.IRGen.noteUseOfTypeMetadata(type);
      return getTypeRefByFunction(IGM, sig, type);
    }
  }
  TOOLCHAIN_FALLTHROUGH;

  case MangledTypeRefRole::DefaultAssociatedTypeWitness:
  case MangledTypeRefRole::Metadata:
    // Note that we're using all of the nominal types referenced by this type,
    // ensuring that we can always reconstruct type metadata from a mangled name
    // in-process.
    IGM.IRGen.noteUseOfTypeMetadata(type);
    
    // If the minimum deployment target's runtime demangler wouldn't understand
    // this mangled name, then fall back to generating a "mangled name" with a
    // symbolic reference with a callback function.
    if (mangledNameIsUnknownToDeployTarget(IGM, type)) {
      return getTypeRefByFunction(IGM, sig, type);
    }

    break;

  case MangledTypeRefRole::Reflection:
    // For reflection records only used for out-of-process reflection, we do not
    // need to force emission of runtime type metadata.
    IGM.IRGen.noteUseOfFieldDescriptors(type);
    break;
  }

  IRGenMangler Mangler(IGM.Context);
  auto SymbolicName =
    useFlatUnique ? Mangler.mangleTypeForFlatUniqueTypeRef(sig, type)
                  : Mangler.mangleTypeForReflection(IGM, sig, type);
  return {IGM.getAddrOfStringForTypeRef(SymbolicName, role),
          SymbolicName.runtimeSizeInBytes()};
}

std::pair<toolchain::Constant *, unsigned>
IRGenModule::getTypeRef(CanType type, CanGenericSignature sig,
                        MangledTypeRefRole role) {
  type = substOpaqueTypesWithUnderlyingTypes(type);
  return getTypeRefImpl(*this, type, sig, role);
}

std::pair<toolchain::Constant *, unsigned>
IRGenModule::getTypeRef(Type type, GenericSignature genericSig,
                        MangledTypeRefRole role) {
  return getTypeRef(type->getReducedType(genericSig),
                    genericSig.getCanonicalSignature(), role);
}

std::pair<toolchain::Constant *, unsigned>
IRGenModule::getLoweredTypeRef(SILType loweredType,
                               CanGenericSignature genericSig,
                               MangledTypeRefRole role) {
  auto substTy =
    substOpaqueTypesWithUnderlyingTypes(loweredType, genericSig);
  auto type = substTy.getASTType();
  return getTypeRefImpl(*this, type, genericSig, role);
}

/// Emit a mangled string referencing a specific protocol conformance, so that
/// the runtime can fetch its witness table.
///
/// TODO: Currently this uses a stub mangling that just refers to an accessor
/// function. We need to fully develop the mangling with the ability to refer
/// to dependent conformances to be able to use mangled strings.
toolchain::Constant *
IRGenModule::emitWitnessTableRefString(CanType type,
                                      ProtocolConformanceRef conformance,
                                      GenericSignature origGenericSig,
                                      bool shouldSetLowBit) {
  std::tie(type, conformance)
    = substOpaqueTypesWithUnderlyingTypes(type, conformance);
  
  auto origType = type;
  auto genericSig = origGenericSig.getCanonicalSignature();

  SmallVector<GenericRequirement, 4> requirements;
  enumerateGenericSignatureRequirements(genericSig,
              [&](GenericRequirement reqt) { requirements.push_back(reqt); });
  auto *genericEnv = genericSig.getGenericEnvironment();

  IRGenMangler mangler(Context);
  std::string symbolName =
    mangler.mangleSymbolNameForMangledConformanceAccessorString(
      "get_witness_table", genericSig, type, conformance);

  return getAddrOfStringForMetadataRef(symbolName, /*alignment=*/2,
      shouldSetLowBit,
      [&](ConstantInitBuilder &B) {
        // Build a stub that loads the necessary bindings from the key path's
        // argument buffer then fetches the metadata.
        auto fnTy = toolchain::FunctionType::get(WitnessTablePtrTy,
                                            {Int8PtrTy}, /*vararg*/ false);
        auto accessorThunk =
          toolchain::Function::Create(fnTy, toolchain::GlobalValue::PrivateLinkage,
                                 symbolName, getModule());
        accessorThunk->setAttributes(constructInitialAttributes());
        
        {
          IRGenFunction IGF(*this, accessorThunk);
          if (DebugInfo)
            DebugInfo->emitArtificialFunction(IGF, accessorThunk);

          if (type->hasTypeParameter()) {
            auto bindingsBufPtr = IGF.collectParameters().claimNext();

            bindFromGenericRequirementsBuffer(
                IGF, requirements,
                Address(bindingsBufPtr, Int8Ty, getPointerAlignment()),
                MetadataState::Complete, genericEnv->getForwardingSubstitutionMap());

            type = genericEnv->mapTypeIntoContext(type)->getCanonicalType();
          }
          if (origType->hasTypeParameter()) {
            conformance = conformance.subst(
                genericEnv->getForwardingSubstitutionMap());
          }
          auto ret = emitWitnessTableRef(IGF, type, conformance);
          IGF.Builder.CreateRet(ret);
        }

        // Form the mangled name with its relative reference.
        auto S = B.beginStruct();
        S.setPacked(true);
        S.add(toolchain::ConstantInt::get(Int8Ty, 255));
        S.add(toolchain::ConstantInt::get(Int8Ty, 9));
        S.addCompactFunctionReference(accessorThunk);

        // And a null terminator!
        S.addInt(Int8Ty, 0);

        return S.finishAndCreateFuture();
      });
}


toolchain::Constant *IRGenModule::getMangledAssociatedConformance(
                                  const NormalProtocolConformance *conformance,
                                  const AssociatedConformance &requirement) {
  // Figure out the name of the symbol to be used for the conformance.
  IRGenMangler mangler(Context);
  auto symbolName =
    mangler.mangleSymbolNameForAssociatedConformanceWitness(
      conformance, requirement.getAssociation(),
      requirement.getAssociatedRequirement());

  // See if we emitted the constant already.
  auto &entry = StringsForTypeRef[symbolName];
  if (entry.second) {
    return entry.second;
  }

  // Get the accessor for this associated conformance.
  toolchain::Function *accessor;
  unsigned char kind;
  if (conformance) {
    kind = 7;
    accessor = getAddrOfAssociatedTypeWitnessTableAccessFunction(conformance,
                                                                requirement);
  } else {
    kind = 8;
    accessor = getAddrOfDefaultAssociatedConformanceAccessor(requirement);
  }

  // Form the mangled name with its relative reference.
  ConstantInitBuilder B(*this);
  auto S = B.beginStruct();
  S.setPacked(true);
  S.add(toolchain::ConstantInt::get(Int8Ty, 255));
  S.add(toolchain::ConstantInt::get(Int8Ty, kind));
  S.addCompactFunctionReference(accessor);

  // And a null terminator!
  S.addInt(Int8Ty, 0);

  auto finished = S.finishAndCreateFuture();
  auto var = new toolchain::GlobalVariable(Module, finished.getType(),
                                      /*constant*/ true,
                                      toolchain::GlobalValue::LinkOnceODRLinkage,
                                      nullptr,
                                      symbolName);
  ApplyIRLinkage(IRLinkage::InternalLinkOnceODR).to(var);
  var->setAlignment(toolchain::MaybeAlign(2));
  setTrueConstGlobal(var);
  var->setSection(getReflectionTypeRefSectionName());

  finished.installInGlobal(var);

  // Drill down to the i8* at the beginning of the constant.
  auto addr = toolchain::ConstantExpr::getBitCast(var, Int8PtrTy);

  // Set the low bit.
  unsigned bit = ProtocolRequirementFlags::AssociatedTypeMangledNameBit;
  auto bitConstant = toolchain::ConstantInt::get(IntPtrTy, bit);
  addr = toolchain::ConstantExpr::getGetElementPtr(Int8Ty, addr, bitConstant);

  // Update the entry.
  entry = {var, addr};

  return addr;
}

class ReflectionMetadataBuilder {
protected:
  IRGenModule &IGM;
  ConstantInitBuilder InitBuilder;
  ConstantStructBuilder B;

  ReflectionMetadataBuilder(IRGenModule &IGM)
    : IGM(IGM), InitBuilder(IGM), B(InitBuilder.beginStruct()) {}

  virtual ~ReflectionMetadataBuilder() {}
  
  // Collect any builtin types referenced from this type.
  void addBuiltinTypeRefs(CanType type) {
    if (IGM.getCodiraModule()->isStdlibModule()) {
      type.visit([&](CanType t) {
        if (isa<BuiltinType>(t))
          IGM.BuiltinTypes.insert(t);
      });
    }
  }

  /// Add a 32-bit relative offset to a mangled typeref string
  /// in the typeref reflection section.
  ///
  /// By default, we use MangledTypeRefRole::Reflection, which does not
  /// force emission of any type metadata referenced from the typeref.
  ///
  /// For reflection records which are demangled to produce type metadata
  /// in-process, pass MangledTypeRefRole::Metadata instead.
  void addTypeRef(Type type, GenericSignature genericSig,
                  MangledTypeRefRole role =
                      MangledTypeRefRole::Reflection) {
    addTypeRef(type->getReducedType(genericSig),
               genericSig.getCanonicalSignature(), role);
  }

  /// Add a 32-bit relative offset to a mangled typeref string
  /// in the typeref reflection section.
  ///
  /// By default, we use MangledTypeRefRole::Reflection, which does not
  /// force emission of any type metadata referenced from the typeref.
  ///
  /// For reflection records which are demangled to produce type metadata
  /// in-process, pass MangledTypeRefRole::Metadata instead.
  void addTypeRef(CanType type,
                  CanGenericSignature sig,
                  MangledTypeRefRole role =
                      MangledTypeRefRole::Reflection) {
    B.addRelativeAddress(IGM.getTypeRef(type, sig, role).first);
    addBuiltinTypeRefs(type);
  }

  void
  addLoweredTypeRef(SILType loweredType,
                    CanGenericSignature genericSig,
                    MangledTypeRefRole role = MangledTypeRefRole::Reflection) {
    B.addRelativeAddress(
        IGM.getLoweredTypeRef(loweredType, genericSig, role).first);
    addBuiltinTypeRefs(loweredType.getASTType());
  }

  /// Add a 32-bit relative offset to a mangled nominal type string
  /// in the typeref reflection section.
  ///
  /// See above comment about 'role'.
  void addNominalRef(const NominalTypeDecl *nominal,
                     MangledTypeRefRole role =
                      MangledTypeRefRole::Reflection) {
    if (auto proto = dyn_cast<ProtocolDecl>(nominal)) {
      IRGenMangler mangler(nominal->getASTContext());
      SymbolicMangling mangledStr;
      mangledStr.String = mangler.mangleBareProtocol(proto);
      auto mangledName =
        IGM.getAddrOfStringForTypeRef(mangledStr, role);
      B.addRelativeAddress(mangledName);
    } else {
      addTypeRef(nominal->getDeclaredType(), GenericSignature(), role);
    }
  }

  // A function signature for a lambda wrapping an IRGenModule::getAddrOf*
  // method.
  using GetAddrOfEntityFn = toolchain::Constant* (IRGenModule &, ConstantInit);

  toolchain::GlobalVariable *
  emit(std::optional<toolchain::function_ref<GetAddrOfEntityFn>> getAddr,
       const char *section) {
    layout();

    toolchain::GlobalVariable *var;

    // Some reflection records have a mangled symbol name, for uniquing
    // imported type metadata.
    if (getAddr) {
      auto init = B.finishAndCreateFuture();

      var = cast<toolchain::GlobalVariable>((*getAddr)(IGM, init));
      var->setConstant(true);
    // Others, such as capture descriptors, do not have a name.
    } else {
      var = B.finishAndCreateGlobal("\x01l__language5_reflection_descriptor",
                                    Alignment(4), /*isConstant*/ true,
                                    toolchain::GlobalValue::PrivateLinkage);
    }

    var->setSection(section);

    // Only mark the reflection record as used when emitting for the runtime.
    // In ReflectionMetadataMode::DebuggerOnly mode we want to allow the linker
    // to remove/dead-strip these.
    if (IGM.IRGen.Opts.ReflectionMetadata == ReflectionMetadataMode::Runtime) {
      IGM.addUsedGlobal(var);
    }

    disableAddressSanitizer(IGM, var);

    return var;
  }

  toolchain::GlobalVariable *emit(std::nullopt_t none, const char *section) {
    return emit(std::optional<toolchain::function_ref<GetAddrOfEntityFn>>(),
                section);
  }

  virtual void layout() = 0;
};

class AssociatedTypeMetadataBuilder : public ReflectionMetadataBuilder {
  static const uint32_t AssociatedTypeRecordSize = 8;

  const ProtocolConformance *Conformance;
  ArrayRef<std::pair<StringRef, CanType>> AssociatedTypes;

  void layout() override {
    PrettyStackTraceConformance DebugStack("emitting associated type metadata",
                                           Conformance);

    auto *DC = Conformance->getDeclContext();
    addNominalRef(DC->getSelfNominalTypeDecl());
    addNominalRef(Conformance->getProtocol());

    B.addInt32(AssociatedTypes.size());
    B.addInt32(AssociatedTypeRecordSize);

    auto genericSig = DC->getGenericSignatureOfContext().getCanonicalSignature();
    for (auto AssocTy : AssociatedTypes) {
      auto NameGlobal = IGM.getAddrOfFieldName(AssocTy.first);
      B.addRelativeAddress(NameGlobal);
      addTypeRef(AssocTy.second, genericSig);
    }
  }

public:
  AssociatedTypeMetadataBuilder(IRGenModule &IGM,
                        const ProtocolConformance *Conformance,
                        ArrayRef<std::pair<StringRef, CanType>> AssociatedTypes)
    : ReflectionMetadataBuilder(IGM), Conformance(Conformance),
      AssociatedTypes(AssociatedTypes) {}

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getAssociatedTypeMetadataSectionName();
    toolchain::GlobalVariable *var = ReflectionMetadataBuilder::emit(
        [&](IRGenModule &IGM, ConstantInit init) -> toolchain::Constant * {
          return IGM.getAddrOfReflectionAssociatedTypeDescriptor(Conformance,
                                                                 init);
        },
        section);

    if (IGM.IRGen.Opts.ConditionalRuntimeRecords) {
      // Allow dead-stripping `var` (the reflection record) when the protocol
      // or type (from the conformance) is not referenced.
      IGM.appendLLVMUsedConditionalEntry(var, Conformance);
    }

    return var;
  }
};

class FieldTypeMetadataBuilder : public ReflectionMetadataBuilder {
public:
  static const uint32_t FieldRecordSize = 12;
  
private:
  const NominalTypeDecl *NTD;

  void addField(reflection::FieldRecordFlags flags,
                Type type, StringRef name) {
    B.addInt32(flags.getRawValue());

    if (!type) {
      B.addInt32(0);
    } else {
      auto genericSig = NTD->getGenericSignature();

      // Special case, UFOs are opaque pointers for now.
      if (type->isForeignReferenceType()) {
        auto opaqueType = type->getASTContext().getOpaquePointerType();
        // The standard library's Mirror demangles metadata from field
        // descriptors, so use MangledTypeRefRole::FieldMetadata to ensure
        // runtime metadata is available.
        addTypeRef(opaqueType, genericSig, MangledTypeRefRole::FieldMetadata);
      } else {
        // The standard library's Mirror demangles metadata from field
        // descriptors, so use MangledTypeRefRole::FieldMetadata to ensure
        // runtime metadata is available.
        addTypeRef(type, genericSig, MangledTypeRefRole::FieldMetadata);
      }
    }

    if (IGM.IRGen.Opts.EnableReflectionNames) {
      auto fieldName = IGM.getAddrOfFieldName(name);
      B.addRelativeAddress(fieldName);
    } else {
      B.addInt32(0);
    }
  }

  void addField(Field field) {
    reflection::FieldRecordFlags flags;
    bool isLet = false;

    switch (field.getKind()) {
    case Field::Var: {
      auto var = field.getVarDecl();
      isLet = var->isLet();
      break;
    }
    case Field::MissingMember:
      toolchain_unreachable("emitting reflection for type with missing member");
    case Field::DefaultActorStorage:
      flags.setIsArtificial();
      break;
    case Field::NonDefaultDistributedActorStorage:
      flags.setIsArtificial();
      break;
    }
    flags.setIsVar(!isLet);

    addField(flags, field.getInterfaceType(IGM), field.getName());
  }

  void layoutRecord() {
    auto kind = FieldDescriptorKind::Struct;

    if (auto CD = dyn_cast<ClassDecl>(NTD)) {
      auto type = CD->getDeclaredType()->getCanonicalType();
      auto RC = type->getReferenceCounting();
      if (RC == ReferenceCounting::ObjC)
        kind = FieldDescriptorKind::ObjCClass;
      else
        kind = FieldDescriptorKind::Class;
    }

    B.addInt16(uint16_t(kind));
    B.addInt16(FieldRecordSize);

    // Emit exportable fields, prefixed with a count
    B.addInt32(countExportableFields(IGM, NTD));

    // Filter to select which fields we'll export FieldDescriptor for.
    forEachField(IGM, NTD, [&](Field field) {
      if (isExportableField(field))
        addField(field);
    });
  }

  void addField(const EnumDecl *enumDecl, const EnumElementDecl *decl,
                bool hasPayload) {
    reflection::FieldRecordFlags flags;
    if (hasPayload && (decl->isIndirect() || enumDecl->isIndirect()))
      flags.setIsIndirectCase();

    Type interfaceType = decl->isAvailableDuringLowering()
                             ? decl->getPayloadInterfaceType()
                             : nullptr;

    addField(flags, interfaceType, decl->getBaseIdentifier().str());
  }

  void layoutEnum() {
    auto enumDecl = cast<EnumDecl>(NTD);
    auto &strategy = irgen::getEnumImplStrategy(
        IGM, enumDecl->getDeclaredTypeInContext()
                     ->getCanonicalType());

    auto kind = FieldDescriptorKind::Enum;

    if (strategy.getElementsWithPayload().size() > 1 &&
        !strategy.needsPayloadSizeInMetadata()) {
      kind = FieldDescriptorKind::MultiPayloadEnum;
    }

    B.addInt16(uint16_t(kind));
    B.addInt16(FieldRecordSize);
    B.addInt32(strategy.getElementsWithPayload().size()
               + strategy.getElementsWithNoPayload().size());

    for (auto enumCase : strategy.getElementsWithPayload()) {
      addField(enumDecl, enumCase.decl, /*has payload*/ true);
    }

    for (auto enumCase : strategy.getElementsWithNoPayload()) {
      addField(enumDecl, enumCase.decl, /*has payload*/ false);
    }
  }

  void layoutProtocol() {
    auto PD = cast<ProtocolDecl>(NTD);
    FieldDescriptorKind Kind;
    if (PD->isObjC())
      Kind = FieldDescriptorKind::ObjCProtocol;
    else if (PD->requiresClass())
      Kind = FieldDescriptorKind::ClassProtocol;
    else
      Kind = FieldDescriptorKind::Protocol;
    B.addInt16(uint16_t(Kind));
    B.addInt16(FieldRecordSize);
    B.addInt32(0);
  }

  void layout() override {
    if (NTD->hasClangNode()) {
      auto *enumDecl = dyn_cast<EnumDecl>(NTD);
      // Structs and namespace-like enums are ok.
      assert(isa<StructDecl>(NTD) || (enumDecl && !enumDecl->hasCases()));
    }

    PrettyStackTraceDecl DebugStack("emitting field type metadata", NTD);
    addNominalRef(NTD);

    auto *CD = dyn_cast<ClassDecl>(NTD);
    auto *PD = dyn_cast<ProtocolDecl>(NTD);
    if (CD && CD->getSuperclass()) {
      addTypeRef(CD->getSuperclass(),
                 CD->getGenericSignature());
    } else if (PD && PD->getDeclaredInterfaceType()->getSuperclass()) {
      addTypeRef(PD->getDeclaredInterfaceType()->getSuperclass(),
                 PD->getGenericSignature());
    } else {
      B.addInt32(0);
    }

    switch (NTD->getKind()) {
    case DeclKind::Class:
    case DeclKind::Struct:
      layoutRecord();
      break;

    case DeclKind::Enum:
      layoutEnum();
      break;

    case DeclKind::Protocol:
      layoutProtocol();
      break;

    default:
      toolchain_unreachable("Not a nominal type");
      break;
    }
  }

public:
  FieldTypeMetadataBuilder(IRGenModule &IGM,
                           const NominalTypeDecl * NTD)
    : ReflectionMetadataBuilder(IGM), NTD(NTD) {}

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getFieldTypeMetadataSectionName();
    toolchain::GlobalVariable *var = ReflectionMetadataBuilder::emit(
        [&](IRGenModule &IGM, ConstantInit definition) -> toolchain::Constant * {
          return IGM.getAddrOfReflectionFieldDescriptor(
              NTD->getDeclaredType()->getCanonicalType(), definition);
        },
        section);

    if (IGM.IRGen.Opts.ConditionalRuntimeRecords) {
      // Allow dead-stripping `var` (the reflection record) when the type
      // (NTD) is not referenced.
      auto ref = IGM.getTypeEntityReference(const_cast<NominalTypeDecl *>(NTD));
      IGM.appendLLVMUsedConditionalEntry(var, ref.getValue());
    }

    return var;
  }
};

static bool
deploymentTargetHasRemoteMirrorZeroSizedTypeDescriptorBug(IRGenModule &IGM) {
  auto target = IGM.Context.LangOpts.Target;
  
  if (target.isMacOSX() && target.isMacOSXVersionLT(10, 15, 4)) {
    return true;
  }
  if (target.isiOS() && target.isOSVersionLT(13, 4)) { // includes tvOS
    return true;
  }
  if (target.isWatchOS() && target.isOSVersionLT(6, 2)) {
    return true;
  }
  
  return false;
}

/// Metadata builder that emits a fixed-layout empty type as an empty struct, as
/// a workaround for a RemoteMirror crash in older OSes.
class EmptyStructMetadataBuilder : public ReflectionMetadataBuilder {
  const NominalTypeDecl *NTD;
  
  void layout() override {
    addNominalRef(NTD);
    B.addInt32(0);
    B.addInt16(uint16_t(FieldDescriptorKind::Struct));
    B.addInt16(FieldTypeMetadataBuilder::FieldRecordSize);
    B.addInt32(0);
  }
  
public:
  EmptyStructMetadataBuilder(IRGenModule &IGM,
                             const NominalTypeDecl *NTD)
    : ReflectionMetadataBuilder(IGM), NTD(NTD) {
      assert(IGM.getTypeInfoForUnlowered(
                           NTD->getDeclaredTypeInContext()->getCanonicalType())
                .isKnownEmpty(ResilienceExpansion::Maximal)
             && "should only be used for known empty types");
  }

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getFieldTypeMetadataSectionName();
    return ReflectionMetadataBuilder::emit(
      [&](IRGenModule &IGM, ConstantInit definition) -> toolchain::Constant* {
        return IGM.getAddrOfReflectionFieldDescriptor(
          NTD->getDeclaredType()->getCanonicalType(), definition);
      },
      section);
  }
};

class FixedTypeMetadataBuilder : public ReflectionMetadataBuilder {
  ModuleDecl *module;
  CanType type;
  const FixedTypeInfo *ti;

public:
  FixedTypeMetadataBuilder(IRGenModule &IGM,
                           CanType builtinType)
    : ReflectionMetadataBuilder(IGM) {
    module = builtinType->getASTContext().TheBuiltinModule;
    type = builtinType;
    ti = &cast<FixedTypeInfo>(IGM.getTypeInfoForUnlowered(builtinType));
  }

  FixedTypeMetadataBuilder(IRGenModule &IGM,
                           const NominalTypeDecl *nominalDecl)
    : ReflectionMetadataBuilder(IGM) {
    module = nominalDecl->getParentModule();
    type = nominalDecl->getDeclaredType()->getCanonicalType();
    ti = &cast<FixedTypeInfo>(IGM.getTypeInfoForUnlowered(
        nominalDecl->getDeclaredTypeInContext()->getCanonicalType()));
  }
  
  void layout() override {
    if (type->isAnyObject()) {
      // AnyObject isn't actually a builtin type; we're emitting it as the old
      // Builtin.UnknownObject type for ABI compatibility.
      B.addRelativeAddress(
          IGM.getAddrOfStringForTypeRef("BO", MangledTypeRefRole::Reflection));
    } else {
      addTypeRef(type, CanGenericSignature());
    }

    B.addInt32(ti->getFixedSize().getValue());

    auto alignment = ti->getFixedAlignment().getValue();
    unsigned bitwiseTakable =
      (ti->getBitwiseTakable(ResilienceExpansion::Minimal) >= IsBitwiseTakableOnly
       ? 1 : 0);
    B.addInt32(alignment | (bitwiseTakable << 16));

    B.addInt32(ti->getFixedStride().getValue());
    B.addInt32(ti->getFixedExtraInhabitantCount(IGM));
  }

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getBuiltinTypeMetadataSectionName();
    return ReflectionMetadataBuilder::emit(
      [&](IRGenModule &IGM, ConstantInit definition) -> toolchain::Constant * {
        return IGM.getAddrOfReflectionBuiltinDescriptor(type, definition);
      },
      section);
  }
};

void IRGenModule::emitBuiltinTypeMetadataRecord(CanType builtinType) {
  // If this builtin is generic, don't emit anything.
  if (builtinType->hasTypeParameter()) {
    return;
  }

  FixedTypeMetadataBuilder builder(*this, builtinType);
  builder.emit();
}

class MultiPayloadEnumDescriptorBuilder : public ReflectionMetadataBuilder {
  CanType type;
  CanType typeInContext;
  const FixedTypeInfo *ti;

public:
  MultiPayloadEnumDescriptorBuilder(IRGenModule &IGM,
                                    const NominalTypeDecl *nominalDecl)
    : ReflectionMetadataBuilder(IGM) {
    type = nominalDecl->getDeclaredType()->getCanonicalType();
    typeInContext = nominalDecl->getDeclaredTypeInContext()->getCanonicalType();
    ti = &cast<FixedTypeInfo>(IGM.getTypeInfoForUnlowered(typeInContext));
  }

  void layout() override {
    auto &strategy = getEnumImplStrategy(IGM, typeInContext);
    bool isMPE = strategy.getElementsWithPayload().size() > 1;
    assert(isMPE && "Cannot emit Multi-Payload Enum data for an enum that "
                    "doesn't have multiple payloads");

    const TypeInfo &TI = strategy.getTypeInfo();
    auto fixedTI = dyn_cast<FixedTypeInfo>(&TI);
    assert(fixedTI != nullptr &&
           "MPE reflection records can only be emitted for fixed-layout enums");

    auto spareBitsMaskInfo = strategy.calculateSpareBitsMask();

    // Never write an MPE descriptor bigger than 16k
    // The runtime will fall back on its own internal
    // spare bits calculation for this (very rare) case.
    if (!spareBitsMaskInfo)
      return;

    auto bits = spareBitsMaskInfo->bits;

    addTypeRef(type, CanGenericSignature());

    bool usesPayloadSpareBits = spareBitsMaskInfo->bytesInMask > 0;

    // MPE record contents are a multiple of 32-bits
    uint32_t contentsSizeInWords = 1; /* Size + flags is mandatory */
    if (usesPayloadSpareBits) {
      contentsSizeInWords += 1 /* SpareBits byte count */
                             + spareBitsMaskInfo->wordsInMask();
    }

    uint32_t flags = usesPayloadSpareBits ? 1 : 0;

    B.addInt32((contentsSizeInWords << 16) | flags);

    if (usesPayloadSpareBits) {
      B.addInt32((spareBitsMaskInfo->byteOffset << 16) |
                 spareBitsMaskInfo->bytesInMask);
      // TODO: Endianness??
      for (unsigned i = 0; i < spareBitsMaskInfo->wordsInMask(); ++i) {
        uint32_t nextWord = bits.extractBitsAsZExtValue(32, 0);
        B.addInt32(nextWord);
        bits.lshrInPlace(32);
      }
    }
  }

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getMultiPayloadEnumDescriptorSectionName();
    return ReflectionMetadataBuilder::emit(std::nullopt, section);
  }
};

/// Builds a constant LLVM struct describing the layout of a fixed-size
/// SIL @box. These look like closure contexts, but without any necessary
/// bindings or metadata sources, and only a single captured value.
class BoxDescriptorBuilder : public ReflectionMetadataBuilder {
  SILType BoxedType;
  CanGenericSignature genericSig;
public:
  BoxDescriptorBuilder(IRGenModule &IGM, SILType BoxedType,
                       CanGenericSignature genericSig)
      : ReflectionMetadataBuilder(IGM), BoxedType(BoxedType),
        genericSig(genericSig) {}

  void layout() override {
    B.addInt32(1);
    B.addInt32(0); // Number of sources
    B.addInt32(0); // Number of generic bindings

    addLoweredTypeRef(BoxedType, genericSig);
  }

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getCaptureDescriptorMetadataSectionName();
    return ReflectionMetadataBuilder::emit(std::nullopt, section);
  }
};

/// Builds a constant LLVM struct describing the layout of a heap closure,
/// the types of its captures, and the sources of metadata if any of the
/// captures are generic.
///
/// For now capture descriptors are only used by out-of-process reflection.
///
/// If the standard library's Mirror type ever gains the ability to reflect
/// closure contexts, we should use MangledTypeRefRole::Metadata below.
class CaptureDescriptorBuilder : public ReflectionMetadataBuilder {
  language::reflection::MetadataSourceBuilder SourceBuilder;
  CanSILFunctionType OrigCalleeType;
  CanSILFunctionType SubstCalleeType;
  SubstitutionMap Subs;
  const HeapLayout &Layout;

public:
  CaptureDescriptorBuilder(IRGenModule &IGM,
                           CanSILFunctionType OrigCalleeType,
                           CanSILFunctionType SubstCalleeType,
                           SubstitutionMap Subs,
                           const HeapLayout &Layout)
    : ReflectionMetadataBuilder(IGM),
      // TODO: Preserve substitutions, since they may affect representation in
      // the box
      OrigCalleeType(OrigCalleeType->getUnsubstitutedType(IGM.getSILModule())),
      SubstCalleeType(SubstCalleeType->getUnsubstitutedType(IGM.getSILModule())),
      Subs(Subs),
      Layout(Layout) {}

  struct Entry {
    enum Kind {
      Metadata,
      Shape,
      Value
    };

    Kind kind;

    CanType type;
    const reflection::MetadataSource *source;

    Entry(Kind kind, CanType type, const reflection::MetadataSource *source)
      : kind(kind), type(type), source(source) {}
  };

  using MetadataSourceMap = std::vector<Entry>;

  void addMetadataSource(Entry::Kind Kind, const reflection::MetadataSource *Source) {
    if (Source == nullptr) {
      B.addInt32(0);
    } else {
      SmallString<16> EncodeBuffer;
      toolchain::raw_svector_ostream OS(EncodeBuffer);
      switch (Kind) {
      case Entry::Kind::Value:
        OS << "v";
        break;
      case Entry::Kind::Shape:
        OS << "s";
        break;
      case Entry::Kind::Metadata:
        break;
      }

      MetadataSourceEncoder Encoder(OS);
      Encoder.visit(Source);

      auto EncodedSource =
        IGM.getAddrOfStringForTypeRef(OS.str(), MangledTypeRefRole::Reflection);
      B.addRelativeAddress(EncodedSource);
    }
  }

  /// Give up if we captured an opened existential type. Eventually we
  /// should figure out how to represent this.
  static bool hasLocalArchetype(CanSILFunctionType OrigCalleeType,
                                const HeapLayout &Layout) {
    if (!OrigCalleeType->isPolymorphic() ||
        OrigCalleeType->isPseudogeneric())
      return false;

    auto &Bindings = Layout.getBindings();
    for (unsigned i = 0; i < Bindings.size(); ++i) {
      // Skip protocol requirements and counts.  It shouldn't be possible
      // to get an opened existential type in a conformance requirement
      // without having one in the generic arguments.
      if (!Bindings[i].isAnyMetadata())
        continue;

      if (Bindings[i].getTypeParameter().subst(Bindings.getSubstitutionMap())
            ->hasLocalArchetype())
        return true;
    }

    auto ElementTypes =
        Layout.getElementTypes().slice(Layout.getIndexAfterBindings());
    for (auto ElementType : ElementTypes) {
      auto CodiraType = ElementType.getASTType();
      if (CodiraType->hasLocalArchetype())
        return true;
    }

    return false;
  }

  /// Slice off the NecessaryBindings struct at the beginning, if it's there.
  /// We'll keep track of how many things are in the bindings struct with its
  /// own count in the capture descriptor.
  ArrayRef<SILType> getElementTypes() {
    return Layout.getElementTypes().slice(Layout.getIndexAfterBindings());
  }

  /// Build a map from generic parameter -> source of its metadata at runtime.
  ///
  /// If the callee that we are partially applying to create a box/closure
  /// isn't generic, then the map is empty.
  MetadataSourceMap getMetadataSourceMap() {
    MetadataSourceMap SourceMap;

    // Generic parameters of pseudogeneric functions do not have
    // runtime metadata.
    if (!OrigCalleeType->isPolymorphic() ||
        OrigCalleeType->isPseudogeneric())
      return SourceMap;

    // Any generic parameters that are not fulfilled are passed in via the
    // bindings. Structural types are decomposed, so emit the contents of
    // the bindings structure directly.
    auto &Bindings = Layout.getBindings();
    for (unsigned i = 0; i < Bindings.size(); ++i) {
      switch (Bindings[i].getKind()) {
      case GenericRequirement::Kind::Shape:
      case GenericRequirement::Kind::Metadata:
      case GenericRequirement::Kind::MetadataPack:
      case GenericRequirement::Kind::Value: {
        auto Kind = Entry::Kind::Metadata;

        if (Bindings[i].getKind() == GenericRequirement::Kind::Shape) {
          Kind = Entry::Kind::Shape;
        }

        if (Bindings[i].getKind() == GenericRequirement::Kind::Value) {
          Kind = Entry::Kind::Value;
        }

        auto Source = SourceBuilder.createClosureBinding(i);
        auto BindingType = Bindings[i].getTypeParameter().subst(Subs);
        auto InterfaceType = BindingType->mapTypeOutOfContext();
        SourceMap.emplace_back(Kind, InterfaceType->getCanonicalType(), Source);
        break;
      }
      case GenericRequirement::Kind::WitnessTable:
      case GenericRequirement::Kind::WitnessTablePack:
        // Skip protocol requirements (FIXME: for now?)
        break;
      }
    }

    // Check if any requirements were fulfilled by metadata stored inside a
    // captured value.

    enumerateGenericParamFulfillments(IGM, OrigCalleeType,
        [&](GenericRequirement Req,
            const irgen::MetadataSource &Source,
            const MetadataPath &Path) {

      const reflection::MetadataSource *Root;
      switch (Source.getKind()) {
      case irgen::MetadataSource::Kind::SelfMetadata:
      case irgen::MetadataSource::Kind::SelfWitnessTable:
        // Handled as part of bindings
        return;

      case irgen::MetadataSource::Kind::GenericLValueMetadata:
        // FIXME?
        return;

      case irgen::MetadataSource::Kind::ClassPointer:
        Root = SourceBuilder.createReferenceCapture(Source.getParamIndex());
        break;

      case irgen::MetadataSource::Kind::Metadata:
        Root = SourceBuilder.createMetadataCapture(Source.getParamIndex());
        break;

      case irgen::MetadataSource::Kind::ErasedTypeMetadata:
        // Fixed in the function body
        break;
      }

      Entry::Kind Kind;
      switch (Req.getKind()) {
      case GenericRequirement::Kind::Shape:
        Kind = Entry::Kind::Shape;
        break;

      case GenericRequirement::Kind::Metadata:
      case GenericRequirement::Kind::MetadataPack:
        Kind = Entry::Kind::Metadata;
        break;

      case GenericRequirement::Kind::Value:
        Kind = Entry::Kind::Value;
        break;

      case GenericRequirement::Kind::WitnessTable:
      case GenericRequirement::Kind::WitnessTablePack:
        toolchain_unreachable("Bad kind");
      }

      // The metadata might be reached via a non-trivial path (eg,
      // dereferencing an isa pointer or a generic argument). Record
      // the path. We assume captured values map 1-1 with function
      // parameters.
      auto Src = Path.getMetadataSource(SourceBuilder, Root);

      auto SubstType = Req.getTypeParameter().subst(Subs);
      auto InterfaceType = SubstType->mapTypeOutOfContext();
      SourceMap.emplace_back(Kind, InterfaceType->getCanonicalType(), Src);
    });

    return SourceMap;
  }

  /// Get the interface types of all of the captured values, mapped out of the
  /// context of the callee we're partially applying.
  std::vector<SILType> getCaptureTypes() {
    std::vector<SILType> CaptureTypes;

    for (auto ElementType : getElementTypes()) {
      auto CodiraType = ElementType.getASTType();

      // Erase pseudogeneric captures down to AnyObject.
      if (OrigCalleeType->isPseudogeneric()) {
        CodiraType = CodiraType.transformRec([&](Type t) -> std::optional<Type> {
          if (auto *archetype = t->getAs<ArchetypeType>()) {
            assert(archetype->requiresClass() && "don't know what to do");
            return IGM.Context.getAnyObjectType();
          }
          return std::nullopt;
        })->getCanonicalType();
      }
      
      // TODO: We should preserve substitutions in SILFunctionType captures
      // once the runtime MetadataReader can understand them, since they can
      // affect representation.
      //
      // For now, eliminate substitutions from the capture representation.
      CodiraType =
        CodiraType->replaceSubstitutedSILFunctionTypesWithUnsubstituted(IGM.getSILModule())
                 ->getCanonicalType();

      CaptureTypes.push_back(SILType::getPrimitiveObjectType(CodiraType));
    }

    return CaptureTypes;
  }

  void layout() override {
    auto CaptureTypes = getCaptureTypes();
    auto MetadataSources = getMetadataSourceMap();

    B.addInt32(CaptureTypes.size());
    B.addInt32(MetadataSources.size());
    B.addInt32(Layout.getBindings().size());

    auto sig =
      OrigCalleeType->getInvocationGenericSignature().getCanonicalSignature();

    // Now add typerefs of all of the captures.
    for (auto CaptureType : CaptureTypes) {
      addLoweredTypeRef(CaptureType.mapTypeOutOfContext(), sig);
    }

    // Add the pairs that make up the generic param -> metadata source map
    // to the struct.
    for (auto entry : MetadataSources) {
      addTypeRef(entry.type, sig);
      addMetadataSource(entry.kind, entry.source);
    }
  }

  toolchain::GlobalVariable *emit() {
    auto section = IGM.getCaptureDescriptorMetadataSectionName();
    return ReflectionMetadataBuilder::emit(std::nullopt, section);
  }
};

static std::string getReflectionSectionName(IRGenModule &IGM,
                                            StringRef LongName,
                                            StringRef FourCC) {
  SmallString<50> SectionName;
  toolchain::raw_svector_ostream OS(SectionName);
  switch (IGM.TargetInfo.OutputObjectFormat) {
  case toolchain::Triple::DXContainer:
  case toolchain::Triple::GOFF:
  case toolchain::Triple::SPIRV:
  case toolchain::Triple::UnknownObjectFormat:
    toolchain_unreachable("unknown object format");
  case toolchain::Triple::XCOFF:
  case toolchain::Triple::COFF:
    assert(FourCC.size() <= 4 &&
           "COFF section name length must be <= 8 characters");
    OS << ".sw5" << FourCC << "$B";
    break;
  case toolchain::Triple::ELF:
  case toolchain::Triple::Wasm:
    OS << "language5_" << LongName;
    break;
  case toolchain::Triple::MachO:
    assert(LongName.size() <= 7 &&
           "Mach-O section name length must be <= 16 characters");
    OS << "__TEXT,__language5_" << LongName << ", regular";
    break;
  }
  return std::string(OS.str());
}

const char *IRGenModule::getFieldTypeMetadataSectionName() {
  if (FieldTypeSection.empty())
    FieldTypeSection = getReflectionSectionName(*this, "fieldmd", "flmd");
  return FieldTypeSection.c_str();
}

const char *IRGenModule::getBuiltinTypeMetadataSectionName() {
  if (BuiltinTypeSection.empty())
    BuiltinTypeSection = getReflectionSectionName(*this, "builtin", "bltn");
  return BuiltinTypeSection.c_str();
}

const char *IRGenModule::getAssociatedTypeMetadataSectionName() {
  if (AssociatedTypeSection.empty())
    AssociatedTypeSection = getReflectionSectionName(*this, "assocty", "asty");
  return AssociatedTypeSection.c_str();
}

const char *IRGenModule::getCaptureDescriptorMetadataSectionName() {
  if (CaptureDescriptorSection.empty())
    CaptureDescriptorSection = getReflectionSectionName(*this, "capture", "cptr");
  return CaptureDescriptorSection.c_str();
}

const char *IRGenModule::getReflectionStringsSectionName() {
  if (ReflectionStringsSection.empty())
    ReflectionStringsSection = getReflectionSectionName(*this, "reflstr", "rfst");
  return ReflectionStringsSection.c_str();
}

const char *IRGenModule::getReflectionTypeRefSectionName() {
  if (ReflectionTypeRefSection.empty())
    ReflectionTypeRefSection = getReflectionSectionName(*this, "typeref", "tyrf");
  return ReflectionTypeRefSection.c_str();
}

const char *IRGenModule::getMultiPayloadEnumDescriptorSectionName() {
  if (MultiPayloadEnumDescriptorSection.empty())
    MultiPayloadEnumDescriptorSection = getReflectionSectionName(*this, "mpenum", "mpen");
  return MultiPayloadEnumDescriptorSection.c_str();
}

toolchain::Constant *IRGenModule::getAddrOfFieldName(StringRef Name) {
  auto &entry = FieldNames[Name];
  if (entry.second)
    return entry.second;

  toolchain::SmallString<256> ReflName;
  if (Lexer::identifierMustAlwaysBeEscaped(Name)) {
    Mangle::Mangler::appendRawIdentifierForRuntime(Name.str(), ReflName);
  } else {
    ReflName = Name;
  }
  entry = createStringConstant(ReflName, /*willBeRelativelyAddressed*/ true,
                               getReflectionStringsSectionName());
  disableAddressSanitizer(*this, entry.first);
  return entry.second;
}

toolchain::Constant *
IRGenModule::getAddrOfBoxDescriptor(SILType BoxedType,
                                    CanGenericSignature genericSig) {
  if (IRGen.Opts.ReflectionMetadata != ReflectionMetadataMode::Runtime)
    return toolchain::Constant::getNullValue(CaptureDescriptorPtrTy);

  BoxDescriptorBuilder builder(*this, BoxedType, genericSig);
  auto var = builder.emit();

  return toolchain::ConstantExpr::getBitCast(var, CaptureDescriptorPtrTy);
}

toolchain::Constant *
IRGenModule::getAddrOfCaptureDescriptor(SILFunction &Caller,
                                        CanSILFunctionType OrigCalleeType,
                                        CanSILFunctionType SubstCalleeType,
                                        SubstitutionMap Subs,
                                        const HeapLayout &Layout) {
  if (IRGen.Opts.ReflectionMetadata != ReflectionMetadataMode::Runtime)
    return toolchain::Constant::getNullValue(CaptureDescriptorPtrTy);

  if (CaptureDescriptorBuilder::hasLocalArchetype(OrigCalleeType, Layout))
    return toolchain::Constant::getNullValue(CaptureDescriptorPtrTy);

  CaptureDescriptorBuilder builder(*this,
                                   OrigCalleeType, SubstCalleeType, Subs,
                                   Layout);
  auto var = builder.emit();
  return toolchain::ConstantExpr::getBitCast(var, CaptureDescriptorPtrTy);
}

void IRGenModule::
emitAssociatedTypeMetadataRecord(const RootProtocolConformance *conformance) {
  auto normalConf = dyn_cast<NormalProtocolConformance>(conformance);
  if (!normalConf)
    return;

  if (IRGen.Opts.ReflectionMetadata != ReflectionMetadataMode::Runtime)
    return;

  SmallVector<std::pair<StringRef, CanType>, 2> AssociatedTypes;

  auto collectTypeWitness = [&](const AssociatedTypeDecl *AssocTy,
                                Type Replacement,
                                const TypeDecl *TD) -> bool {
    AssociatedTypes.push_back({
      AssocTy->getNameStr(),
      Replacement->getCanonicalType()
    });
    return false;
  };

  normalConf->forEachTypeWitness(collectTypeWitness);

  // If there are no associated types, don't bother emitting any
  // metadata.
  if (AssociatedTypes.empty())
    return;

  AssociatedTypeMetadataBuilder builder(*this, normalConf, AssociatedTypes);
  builder.emit();
}

toolchain::ArrayRef<CanType> IRGenModule::getOrCreateSpecialStlibBuiltinTypes() {
  if (SpecialStdlibBuiltinTypes.empty()) {
    SpecialStdlibBuiltinTypes.push_back(Context.TheNativeObjectType);
    SpecialStdlibBuiltinTypes.push_back(Context.getAnyObjectType());
    SpecialStdlibBuiltinTypes.push_back(Context.TheBridgeObjectType);
    SpecialStdlibBuiltinTypes.push_back(Context.TheRawPointerType);
    SpecialStdlibBuiltinTypes.push_back(Context.TheUnsafeValueBufferType);

    // This would not be necessary if RawPointer had the same set of
    // extra inhabitants as these. But maybe it's best not to codify
    // that in the ABI anyway.
    CanType thinFunction =
        CanFunctionType::get({}, Context.TheEmptyTupleType,
                             AnyFunctionType::ExtInfo().withRepresentation(
                                 FunctionTypeRepresentation::Thin));
    SpecialStdlibBuiltinTypes.push_back(thinFunction);

    CanType anyMetatype = CanExistentialMetatypeType::get(Context.TheAnyType);
    SpecialStdlibBuiltinTypes.push_back(anyMetatype);
  }
  return SpecialStdlibBuiltinTypes;
}

void IRGenModule::emitBuiltinReflectionMetadata() {
  if (getSILModule().getOptions().StopOptimizationAfterSerialization) {
    // We're asked to emit an empty IR module
    return;
  }

  if (getCodiraModule()->isStdlibModule()) {
    auto SpecialBuiltins = getOrCreateSpecialStlibBuiltinTypes();
    BuiltinTypes.insert(SpecialBuiltins.begin(), SpecialBuiltins.end());
  }

  for (auto builtinType : BuiltinTypes)
    emitBuiltinTypeMetadataRecord(builtinType);
}

void IRGenerator::emitBuiltinReflectionMetadata() {
  for (auto &m : *this) {
    m.second->emitBuiltinReflectionMetadata();
  }
}

void IRGenModule::emitFieldDescriptor(const NominalTypeDecl *D) {
  if (IRGen.Opts.ReflectionMetadata == ReflectionMetadataMode::None)
    return;

  auto T = D->getDeclaredTypeInContext()->getCanonicalType();

  bool needsOpaqueDescriptor = false;
  bool needsMPEDescriptor = false;
  bool needsFieldDescriptor = true;

  if (isa<EnumDecl>(D)) {
    auto &strategy = getEnumImplStrategy(*this, T);

    // @objc enums never have generic parameters or payloads,
    // and lower as their raw type.
    if (!strategy.isReflectable()) {
      needsOpaqueDescriptor = true;
      needsFieldDescriptor = false;
    }

    // If this is a fixed-size multi-payload enum, we have to emit a descriptor
    // with the size and alignment of the type and another with the spare bit
    // mask data, because the reflection library cannot consistently derive this
    // information at runtime.
    if (strategy.getElementsWithPayload().size() > 1 &&
        !strategy.needsPayloadSizeInMetadata()) {
      needsOpaqueDescriptor = true;
      needsMPEDescriptor = true;
    }
  }

  if (auto *SD = dyn_cast<StructDecl>(D)) {
    if (SD->hasClangNode())
      needsOpaqueDescriptor = true;
  }

  if (auto *CD = dyn_cast<ClassDecl>(D)) {
    if (CD->getObjCImplementationDecl())
      needsFieldDescriptor = false;
  }

  // If the type has custom @_alignment, @_rawLayout, or other manual layout
  // attributes, emit a fixed record with the size and alignment since the
  // remote mirrors will need to treat the type as opaque.
  //
  // Note that we go on to also emit a field descriptor in this case,
  // since in-process reflection only cares about the types of the fields
  // and does not independently re-derive the layout.
  if (D->getAttrs().hasAttribute<AlignmentAttr>()
      || D->getAttrs().hasAttribute<RawLayoutAttr>()) {
    auto &TI = getTypeInfoForUnlowered(T);
    if (isa<FixedTypeInfo>(TI)) {
      needsOpaqueDescriptor = true;
    }
  }

  if (needsOpaqueDescriptor) {
    // Work around an issue in the RemoteMirror library that ships in
    // macOS 10.15/iOS 13 and earlier that causes it to crash on a
    // BuiltinTypeDescriptor with zero size. If the type has zero size, emit it
    // as an empty struct instead, which will have the same impact on the
    // encoded type layout.
    auto &TI = getTypeInfoForUnlowered(T);
    if (deploymentTargetHasRemoteMirrorZeroSizedTypeDescriptorBug(*this)
        && TI.isKnownEmpty(ResilienceExpansion::Maximal)) {
      EmptyStructMetadataBuilder builder(*this, D);
      builder.emit();
      return;
    }
    
    FixedTypeMetadataBuilder builder(*this, D);
    builder.emit();
  }

  if (needsMPEDescriptor) {
    MultiPayloadEnumDescriptorBuilder builder(*this, D);
    builder.emit();
  }

  if (needsFieldDescriptor) {
    FieldTypeMetadataBuilder builder(*this, D);
    builder.emit();
  }
}

void IRGenModule::emitReflectionMetadataVersion() {
  if (IRGen.Opts.ReflectionMetadata == ReflectionMetadataMode::None)
    return;

  auto Init =
    toolchain::ConstantInt::get(Int16Ty, LANGUAGE_REFLECTION_METADATA_VERSION);
  auto Version = new toolchain::GlobalVariable(Module, Int16Ty, /*constant*/ true,
                                          toolchain::GlobalValue::LinkOnceODRLinkage,
                                          Init,
                                          "__language_reflection_version");
  ApplyIRLinkage(IRLinkage::InternalLinkOnceODR).to(Version);
  addUsedGlobal(Version);
}

void IRGenerator::emitReflectionMetadataVersion() {
  for (auto &m : *this) {
    m.second->emitReflectionMetadataVersion();
  }
}
