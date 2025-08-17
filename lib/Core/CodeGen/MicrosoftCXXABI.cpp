//===--- MicrosoftCXXABI.cpp - Emit LLVM Code from ASTs for a Module ------===//
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
//
// This provides C++ code generation targeting the Microsoft Visual C++ ABI.
// The class in this file generates structures that follow the Microsoft
// Visual C++ ABI, which is actually not very well documented at all outside
// of Microsoft.
//
//===----------------------------------------------------------------------===//

#include "ABIInfo.h"
#include "CGCXXABI.h"
#include "CGCleanup.h"
#include "CGDebugInfo.h"
#include "CGVTables.h"
#include "CodeGenModule.h"
#include "CodeGenTypes.h"
#include "TargetInfo.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/CXXInheritance.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/StmtCXX.h"
#include "language/Core/AST/VTableBuilder.h"
#include "language/Core/CodeGen/ConstantInitBuilder.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/IR/Intrinsics.h"

using namespace language::Core;
using namespace CodeGen;

namespace {

/// Holds all the vbtable globals for a given class.
struct VBTableGlobals {
  const VPtrInfoVector *VBTables;
  SmallVector<toolchain::GlobalVariable *, 2> Globals;
};

class MicrosoftCXXABI : public CGCXXABI {
public:
  MicrosoftCXXABI(CodeGenModule &CGM)
      : CGCXXABI(CGM), BaseClassDescriptorType(nullptr),
        ClassHierarchyDescriptorType(nullptr),
        CompleteObjectLocatorType(nullptr), CatchableTypeType(nullptr),
        ThrowInfoType(nullptr) {
    assert(!(CGM.getLangOpts().isExplicitDefaultVisibilityExportMapping() ||
             CGM.getLangOpts().isAllDefaultVisibilityExportMapping()) &&
           "visibility export mapping option unimplemented in this ABI");
  }

  bool HasThisReturn(GlobalDecl GD) const override;
  bool hasMostDerivedReturn(GlobalDecl GD) const override;

  bool classifyReturnType(CGFunctionInfo &FI) const override;

  RecordArgABI getRecordArgABI(const CXXRecordDecl *RD) const override;

  bool isSRetParameterAfterThis() const override { return true; }

  bool isThisCompleteObject(GlobalDecl GD) const override {
    // The Microsoft ABI doesn't use separate complete-object vs.
    // base-object variants of constructors, but it does of destructors.
    if (isa<CXXDestructorDecl>(GD.getDecl())) {
      switch (GD.getDtorType()) {
      case Dtor_Complete:
      case Dtor_Deleting:
        return true;

      case Dtor_Base:
        return false;

      case Dtor_Comdat: toolchain_unreachable("emitting dtor comdat as function?");
      }
      toolchain_unreachable("bad dtor kind");
    }

    // No other kinds.
    return false;
  }

  size_t getSrcArgforCopyCtor(const CXXConstructorDecl *CD,
                              FunctionArgList &Args) const override {
    assert(Args.size() >= 2 &&
           "expected the arglist to have at least two args!");
    // The 'most_derived' parameter goes second if the ctor is variadic and
    // has v-bases.
    if (CD->getParent()->getNumVBases() > 0 &&
        CD->getType()->castAs<FunctionProtoType>()->isVariadic())
      return 2;
    return 1;
  }

  std::vector<CharUnits> getVBPtrOffsets(const CXXRecordDecl *RD) override {
    std::vector<CharUnits> VBPtrOffsets;
    const ASTContext &Context = getContext();
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);

    const VBTableGlobals &VBGlobals = enumerateVBTables(RD);
    for (const std::unique_ptr<VPtrInfo> &VBT : *VBGlobals.VBTables) {
      const ASTRecordLayout &SubobjectLayout =
          Context.getASTRecordLayout(VBT->IntroducingObject);
      CharUnits Offs = VBT->NonVirtualOffset;
      Offs += SubobjectLayout.getVBPtrOffset();
      if (VBT->getVBaseWithVPtr())
        Offs += Layout.getVBaseClassOffset(VBT->getVBaseWithVPtr());
      VBPtrOffsets.push_back(Offs);
    }
    toolchain::array_pod_sort(VBPtrOffsets.begin(), VBPtrOffsets.end());
    return VBPtrOffsets;
  }

  StringRef GetPureVirtualCallName() override { return "_purecall"; }
  StringRef GetDeletedVirtualCallName() override { return "_purecall"; }

  void emitVirtualObjectDelete(CodeGenFunction &CGF, const CXXDeleteExpr *DE,
                               Address Ptr, QualType ElementType,
                               const CXXDestructorDecl *Dtor) override;

  void emitRethrow(CodeGenFunction &CGF, bool isNoReturn) override;
  void emitThrow(CodeGenFunction &CGF, const CXXThrowExpr *E) override;

  void emitBeginCatch(CodeGenFunction &CGF, const CXXCatchStmt *C) override;

  toolchain::GlobalVariable *getMSCompleteObjectLocator(const CXXRecordDecl *RD,
                                                   const VPtrInfo &Info);

  toolchain::Constant *getAddrOfRTTIDescriptor(QualType Ty) override;
  CatchTypeInfo
  getAddrOfCXXCatchHandlerType(QualType Ty, QualType CatchHandlerType) override;

  /// MSVC needs an extra flag to indicate a catchall.
  CatchTypeInfo getCatchAllTypeInfo() override {
    // For -EHa catch(...) must handle HW exception
    // Adjective = HT_IsStdDotDot (0x40), only catch C++ exceptions
    if (getContext().getLangOpts().EHAsynch)
      return CatchTypeInfo{nullptr, 0};
    else
      return CatchTypeInfo{nullptr, 0x40};
  }

  bool shouldTypeidBeNullChecked(QualType SrcRecordTy) override;
  void EmitBadTypeidCall(CodeGenFunction &CGF) override;
  toolchain::Value *EmitTypeid(CodeGenFunction &CGF, QualType SrcRecordTy,
                          Address ThisPtr,
                          toolchain::Type *StdTypeInfoPtrTy) override;

  bool shouldDynamicCastCallBeNullChecked(bool SrcIsPtr,
                                          QualType SrcRecordTy) override;

  bool shouldEmitExactDynamicCast(QualType DestRecordTy) override {
    // TODO: Add support for exact dynamic_casts.
    return false;
  }
  std::optional<ExactDynamicCastInfo>
  getExactDynamicCastInfo(QualType SrcRecordTy, QualType DestTy,
                          QualType DestRecordTy) override {
    toolchain_unreachable("unsupported");
  }
  toolchain::Value *emitExactDynamicCast(CodeGenFunction &CGF, Address Value,
                                    QualType SrcRecordTy, QualType DestTy,
                                    QualType DestRecordTy,
                                    const ExactDynamicCastInfo &CastInfo,
                                    toolchain::BasicBlock *CastSuccess,
                                    toolchain::BasicBlock *CastFail) override {
    toolchain_unreachable("unsupported");
  }

  toolchain::Value *emitDynamicCastCall(CodeGenFunction &CGF, Address Value,
                                   QualType SrcRecordTy, QualType DestTy,
                                   QualType DestRecordTy,
                                   toolchain::BasicBlock *CastEnd) override;

  toolchain::Value *emitDynamicCastToVoid(CodeGenFunction &CGF, Address Value,
                                     QualType SrcRecordTy) override;

  bool EmitBadCastCall(CodeGenFunction &CGF) override;
  bool canSpeculativelyEmitVTable(const CXXRecordDecl *RD) const override {
    return false;
  }

  toolchain::Value *
  GetVirtualBaseClassOffset(CodeGenFunction &CGF, Address This,
                            const CXXRecordDecl *ClassDecl,
                            const CXXRecordDecl *BaseClassDecl) override;

  toolchain::BasicBlock *
  EmitCtorCompleteObjectHandler(CodeGenFunction &CGF,
                                const CXXRecordDecl *RD) override;

  toolchain::BasicBlock *
  EmitDtorCompleteObjectHandler(CodeGenFunction &CGF);

  void initializeHiddenVirtualInheritanceMembers(CodeGenFunction &CGF,
                                              const CXXRecordDecl *RD) override;

  void EmitCXXConstructors(const CXXConstructorDecl *D) override;

  // Background on MSVC destructors
  // ==============================
  //
  // Both Itanium and MSVC ABIs have destructor variants.  The variant names
  // roughly correspond in the following way:
  //   Itanium       Microsoft
  //   Base       -> no name, just ~Class
  //   Complete   -> vbase destructor
  //   Deleting   -> scalar deleting destructor
  //                 vector deleting destructor
  //
  // The base and complete destructors are the same as in Itanium, although the
  // complete destructor does not accept a VTT parameter when there are virtual
  // bases.  A separate mechanism involving vtordisps is used to ensure that
  // virtual methods of destroyed subobjects are not called.
  //
  // The deleting destructors accept an i32 bitfield as a second parameter.  Bit
  // 1 indicates if the memory should be deleted.  Bit 2 indicates if the this
  // pointer points to an array.  The scalar deleting destructor assumes that
  // bit 2 is zero, and therefore does not contain a loop.
  //
  // For virtual destructors, only one entry is reserved in the vftable, and it
  // always points to the vector deleting destructor.  The vector deleting
  // destructor is the most general, so it can be used to destroy objects in
  // place, delete single heap objects, or delete arrays.
  //
  // A TU defining a non-inline destructor is only guaranteed to emit a base
  // destructor, and all of the other variants are emitted on an as-needed basis
  // in COMDATs.  Because a non-base destructor can be emitted in a TU that
  // lacks a definition for the destructor, non-base destructors must always
  // delegate to or alias the base destructor.

  AddedStructorArgCounts
  buildStructorSignature(GlobalDecl GD,
                         SmallVectorImpl<CanQualType> &ArgTys) override;

  /// Non-base dtors should be emitted as delegating thunks in this ABI.
  bool useThunkForDtorVariant(const CXXDestructorDecl *Dtor,
                              CXXDtorType DT) const override {
    return DT != Dtor_Base;
  }

  void setCXXDestructorDLLStorage(toolchain::GlobalValue *GV,
                                  const CXXDestructorDecl *Dtor,
                                  CXXDtorType DT) const override;

  toolchain::GlobalValue::LinkageTypes
  getCXXDestructorLinkage(GVALinkage Linkage, const CXXDestructorDecl *Dtor,
                          CXXDtorType DT) const override;

  void EmitCXXDestructors(const CXXDestructorDecl *D) override;

  const CXXRecordDecl *getThisArgumentTypeForMethod(GlobalDecl GD) override {
    auto *MD = cast<CXXMethodDecl>(GD.getDecl());

    if (MD->isVirtual()) {
      GlobalDecl LookupGD = GD;
      if (const auto *DD = dyn_cast<CXXDestructorDecl>(MD)) {
        // Complete dtors take a pointer to the complete object,
        // thus don't need adjustment.
        if (GD.getDtorType() == Dtor_Complete)
          return MD->getParent();

        // There's only Dtor_Deleting in vftable but it shares the this
        // adjustment with the base one, so look up the deleting one instead.
        LookupGD = GlobalDecl(DD, Dtor_Deleting);
      }
      MethodVFTableLocation ML =
          CGM.getMicrosoftVTableContext().getMethodVFTableLocation(LookupGD);

      // The vbases might be ordered differently in the final overrider object
      // and the complete object, so the "this" argument may sometimes point to
      // memory that has no particular type (e.g. past the complete object).
      // In this case, we just use a generic pointer type.
      // FIXME: might want to have a more precise type in the non-virtual
      // multiple inheritance case.
      if (ML.VBase || !ML.VFPtrOffset.isZero())
        return nullptr;
    }
    return MD->getParent();
  }

  Address
  adjustThisArgumentForVirtualFunctionCall(CodeGenFunction &CGF, GlobalDecl GD,
                                           Address This,
                                           bool VirtualCall) override;

  void addImplicitStructorParams(CodeGenFunction &CGF, QualType &ResTy,
                                 FunctionArgList &Params) override;

  void EmitInstanceFunctionProlog(CodeGenFunction &CGF) override;

  AddedStructorArgs getImplicitConstructorArgs(CodeGenFunction &CGF,
                                               const CXXConstructorDecl *D,
                                               CXXCtorType Type,
                                               bool ForVirtualBase,
                                               bool Delegating) override;

  toolchain::Value *getCXXDestructorImplicitParam(CodeGenFunction &CGF,
                                             const CXXDestructorDecl *DD,
                                             CXXDtorType Type,
                                             bool ForVirtualBase,
                                             bool Delegating) override;

  void EmitDestructorCall(CodeGenFunction &CGF, const CXXDestructorDecl *DD,
                          CXXDtorType Type, bool ForVirtualBase,
                          bool Delegating, Address This,
                          QualType ThisTy) override;

  void emitVTableTypeMetadata(const VPtrInfo &Info, const CXXRecordDecl *RD,
                              toolchain::GlobalVariable *VTable);

  void emitVTableDefinitions(CodeGenVTables &CGVT,
                             const CXXRecordDecl *RD) override;

  bool isVirtualOffsetNeededForVTableField(CodeGenFunction &CGF,
                                           CodeGenFunction::VPtr Vptr) override;

  /// Don't initialize vptrs if dynamic class
  /// is marked with the 'novtable' attribute.
  bool doStructorsInitializeVPtrs(const CXXRecordDecl *VTableClass) override {
    return !VTableClass->hasAttr<MSNoVTableAttr>();
  }

  toolchain::Constant *
  getVTableAddressPoint(BaseSubobject Base,
                        const CXXRecordDecl *VTableClass) override;

  toolchain::Value *getVTableAddressPointInStructor(
      CodeGenFunction &CGF, const CXXRecordDecl *VTableClass,
      BaseSubobject Base, const CXXRecordDecl *NearestVBase) override;

  toolchain::GlobalVariable *getAddrOfVTable(const CXXRecordDecl *RD,
                                        CharUnits VPtrOffset) override;

  CGCallee getVirtualFunctionPointer(CodeGenFunction &CGF, GlobalDecl GD,
                                     Address This, toolchain::Type *Ty,
                                     SourceLocation Loc) override;

  toolchain::Value *
  EmitVirtualDestructorCall(CodeGenFunction &CGF, const CXXDestructorDecl *Dtor,
                            CXXDtorType DtorType, Address This,
                            DeleteOrMemberCallExpr E,
                            toolchain::CallBase **CallOrInvoke) override;

  void adjustCallArgsForDestructorThunk(CodeGenFunction &CGF, GlobalDecl GD,
                                        CallArgList &CallArgs) override {
    assert(GD.getDtorType() == Dtor_Deleting &&
           "Only deleting destructor thunks are available in this ABI");
    CallArgs.add(RValue::get(getStructorImplicitParamValue(CGF)),
                 getContext().IntTy);
  }

  void emitVirtualInheritanceTables(const CXXRecordDecl *RD) override;

  toolchain::GlobalVariable *
  getAddrOfVBTable(const VPtrInfo &VBT, const CXXRecordDecl *RD,
                   toolchain::GlobalVariable::LinkageTypes Linkage);

  toolchain::GlobalVariable *
  getAddrOfVirtualDisplacementMap(const CXXRecordDecl *SrcRD,
                                  const CXXRecordDecl *DstRD) {
    SmallString<256> OutName;
    toolchain::raw_svector_ostream Out(OutName);
    getMangleContext().mangleCXXVirtualDisplacementMap(SrcRD, DstRD, Out);
    StringRef MangledName = OutName.str();

    if (auto *VDispMap = CGM.getModule().getNamedGlobal(MangledName))
      return VDispMap;

    MicrosoftVTableContext &VTContext = CGM.getMicrosoftVTableContext();
    unsigned NumEntries = 1 + SrcRD->getNumVBases();
    SmallVector<toolchain::Constant *, 4> Map(NumEntries,
                                         toolchain::PoisonValue::get(CGM.IntTy));
    Map[0] = toolchain::ConstantInt::get(CGM.IntTy, 0);
    bool AnyDifferent = false;
    for (const auto &I : SrcRD->vbases()) {
      const CXXRecordDecl *VBase = I.getType()->getAsCXXRecordDecl();
      if (!DstRD->isVirtuallyDerivedFrom(VBase))
        continue;

      unsigned SrcVBIndex = VTContext.getVBTableIndex(SrcRD, VBase);
      unsigned DstVBIndex = VTContext.getVBTableIndex(DstRD, VBase);
      Map[SrcVBIndex] = toolchain::ConstantInt::get(CGM.IntTy, DstVBIndex * 4);
      AnyDifferent |= SrcVBIndex != DstVBIndex;
    }
    // This map would be useless, don't use it.
    if (!AnyDifferent)
      return nullptr;

    toolchain::ArrayType *VDispMapTy = toolchain::ArrayType::get(CGM.IntTy, Map.size());
    toolchain::Constant *Init = toolchain::ConstantArray::get(VDispMapTy, Map);
    toolchain::GlobalValue::LinkageTypes Linkage =
        SrcRD->isExternallyVisible() && DstRD->isExternallyVisible()
            ? toolchain::GlobalValue::LinkOnceODRLinkage
            : toolchain::GlobalValue::InternalLinkage;
    auto *VDispMap = new toolchain::GlobalVariable(
        CGM.getModule(), VDispMapTy, /*isConstant=*/true, Linkage,
        /*Initializer=*/Init, MangledName);
    return VDispMap;
  }

  void emitVBTableDefinition(const VPtrInfo &VBT, const CXXRecordDecl *RD,
                             toolchain::GlobalVariable *GV) const;

  void setThunkLinkage(toolchain::Function *Thunk, bool ForVTable,
                       GlobalDecl GD, bool ReturnAdjustment) override {
    GVALinkage Linkage =
        getContext().GetGVALinkageForFunction(cast<FunctionDecl>(GD.getDecl()));

    if (Linkage == GVA_Internal)
      Thunk->setLinkage(toolchain::GlobalValue::InternalLinkage);
    else if (ReturnAdjustment)
      Thunk->setLinkage(toolchain::GlobalValue::WeakODRLinkage);
    else
      Thunk->setLinkage(toolchain::GlobalValue::LinkOnceODRLinkage);
  }

  bool exportThunk() override { return false; }

  toolchain::Value *performThisAdjustment(CodeGenFunction &CGF, Address This,
                                     const CXXRecordDecl * /*UnadjustedClass*/,
                                     const ThunkInfo &TI) override;

  toolchain::Value *performReturnAdjustment(CodeGenFunction &CGF, Address Ret,
                                       const CXXRecordDecl * /*UnadjustedClass*/,
                                       const ReturnAdjustment &RA) override;

  void EmitThreadLocalInitFuncs(
      CodeGenModule &CGM, ArrayRef<const VarDecl *> CXXThreadLocals,
      ArrayRef<toolchain::Function *> CXXThreadLocalInits,
      ArrayRef<const VarDecl *> CXXThreadLocalInitVars) override;

  bool usesThreadWrapperFunction(const VarDecl *VD) const override {
    return getContext().getLangOpts().isCompatibleWithMSVC(
               LangOptions::MSVC2019_5) &&
           CGM.getCodeGenOpts().TlsGuards &&
           (!isEmittedWithConstantInitializer(VD) || mayNeedDestruction(VD));
  }
  LValue EmitThreadLocalVarDeclLValue(CodeGenFunction &CGF, const VarDecl *VD,
                                      QualType LValType) override;

  void EmitGuardedInit(CodeGenFunction &CGF, const VarDecl &D,
                       toolchain::GlobalVariable *DeclPtr,
                       bool PerformInit) override;
  void registerGlobalDtor(CodeGenFunction &CGF, const VarDecl &D,
                          toolchain::FunctionCallee Dtor,
                          toolchain::Constant *Addr) override;

  // ==== Notes on array cookies =========
  //
  // MSVC seems to only use cookies when the class has a destructor; a
  // two-argument usual array deallocation function isn't sufficient.
  //
  // For example, this code prints "100" and "1":
  //   struct A {
  //     char x;
  //     void *operator new[](size_t sz) {
  //       printf("%u\n", sz);
  //       return malloc(sz);
  //     }
  //     void operator delete[](void *p, size_t sz) {
  //       printf("%u\n", sz);
  //       free(p);
  //     }
  //   };
  //   int main() {
  //     A *p = new A[100];
  //     delete[] p;
  //   }
  // Whereas it prints "104" and "104" if you give A a destructor.

  bool requiresArrayCookie(const CXXDeleteExpr *expr,
                           QualType elementType) override;
  bool requiresArrayCookie(const CXXNewExpr *expr) override;
  CharUnits getArrayCookieSizeImpl(QualType type) override;
  Address InitializeArrayCookie(CodeGenFunction &CGF,
                                Address NewPtr,
                                toolchain::Value *NumElements,
                                const CXXNewExpr *expr,
                                QualType ElementType) override;
  toolchain::Value *readArrayCookieImpl(CodeGenFunction &CGF,
                                   Address allocPtr,
                                   CharUnits cookieSize) override;

  friend struct MSRTTIBuilder;

  bool isImageRelative() const {
    return CGM.getTarget().getPointerWidth(LangAS::Default) == 64;
  }

  // 5 routines for constructing the toolchain types for MS RTTI structs.
  toolchain::StructType *getTypeDescriptorType(StringRef TypeInfoString) {
    toolchain::SmallString<32> TDTypeName("rtti.TypeDescriptor");
    TDTypeName += toolchain::utostr(TypeInfoString.size());
    toolchain::StructType *&TypeDescriptorType =
        TypeDescriptorTypeMap[TypeInfoString.size()];
    if (TypeDescriptorType)
      return TypeDescriptorType;
    toolchain::Type *FieldTypes[] = {
        CGM.Int8PtrPtrTy,
        CGM.Int8PtrTy,
        toolchain::ArrayType::get(CGM.Int8Ty, TypeInfoString.size() + 1)};
    TypeDescriptorType =
        toolchain::StructType::create(CGM.getLLVMContext(), FieldTypes, TDTypeName);
    return TypeDescriptorType;
  }

  toolchain::Type *getImageRelativeType(toolchain::Type *PtrType) {
    if (!isImageRelative())
      return PtrType;
    return CGM.IntTy;
  }

  toolchain::StructType *getBaseClassDescriptorType() {
    if (BaseClassDescriptorType)
      return BaseClassDescriptorType;
    toolchain::Type *FieldTypes[] = {
        getImageRelativeType(CGM.Int8PtrTy),
        CGM.IntTy,
        CGM.IntTy,
        CGM.IntTy,
        CGM.IntTy,
        CGM.IntTy,
        getImageRelativeType(CGM.UnqualPtrTy),
    };
    BaseClassDescriptorType = toolchain::StructType::create(
        CGM.getLLVMContext(), FieldTypes, "rtti.BaseClassDescriptor");
    return BaseClassDescriptorType;
  }

  toolchain::StructType *getClassHierarchyDescriptorType() {
    if (ClassHierarchyDescriptorType)
      return ClassHierarchyDescriptorType;
    // Forward-declare RTTIClassHierarchyDescriptor to break a cycle.
    toolchain::Type *FieldTypes[] = {CGM.IntTy, CGM.IntTy, CGM.IntTy,
                                getImageRelativeType(CGM.UnqualPtrTy)};
    ClassHierarchyDescriptorType =
        toolchain::StructType::create(FieldTypes, "rtti.ClassHierarchyDescriptor");
    return ClassHierarchyDescriptorType;
  }

  toolchain::StructType *getCompleteObjectLocatorType() {
    if (CompleteObjectLocatorType)
      return CompleteObjectLocatorType;
    toolchain::Type *FieldTypes[] = {
        CGM.IntTy,
        CGM.IntTy,
        CGM.IntTy,
        getImageRelativeType(CGM.Int8PtrTy),
        getImageRelativeType(CGM.UnqualPtrTy),
        getImageRelativeType(CGM.VoidTy),
    };
    toolchain::ArrayRef<toolchain::Type *> FieldTypesRef(FieldTypes);
    if (!isImageRelative())
      FieldTypesRef = FieldTypesRef.drop_back();
    CompleteObjectLocatorType =
        toolchain::StructType::create(FieldTypesRef, "rtti.CompleteObjectLocator");
    return CompleteObjectLocatorType;
  }

  toolchain::GlobalVariable *getImageBase() {
    StringRef Name = "__ImageBase";
    if (toolchain::GlobalVariable *GV = CGM.getModule().getNamedGlobal(Name))
      return GV;

    auto *GV = new toolchain::GlobalVariable(CGM.getModule(), CGM.Int8Ty,
                                        /*isConstant=*/true,
                                        toolchain::GlobalValue::ExternalLinkage,
                                        /*Initializer=*/nullptr, Name);
    CGM.setDSOLocal(GV);
    return GV;
  }

  toolchain::Constant *getImageRelativeConstant(toolchain::Constant *PtrVal) {
    if (!isImageRelative())
      return PtrVal;

    if (PtrVal->isNullValue())
      return toolchain::Constant::getNullValue(CGM.IntTy);

    toolchain::Constant *ImageBaseAsInt =
        toolchain::ConstantExpr::getPtrToInt(getImageBase(), CGM.IntPtrTy);
    toolchain::Constant *PtrValAsInt =
        toolchain::ConstantExpr::getPtrToInt(PtrVal, CGM.IntPtrTy);
    toolchain::Constant *Diff =
        toolchain::ConstantExpr::getSub(PtrValAsInt, ImageBaseAsInt,
                                   /*HasNUW=*/true, /*HasNSW=*/true);
    return toolchain::ConstantExpr::getTrunc(Diff, CGM.IntTy);
  }

private:
  MicrosoftMangleContext &getMangleContext() {
    return cast<MicrosoftMangleContext>(CodeGen::CGCXXABI::getMangleContext());
  }

  toolchain::Constant *getZeroInt() {
    return toolchain::ConstantInt::get(CGM.IntTy, 0);
  }

  toolchain::Constant *getAllOnesInt() {
    return  toolchain::Constant::getAllOnesValue(CGM.IntTy);
  }

  CharUnits getVirtualFunctionPrologueThisAdjustment(GlobalDecl GD) override;

  void
  GetNullMemberPointerFields(const MemberPointerType *MPT,
                             toolchain::SmallVectorImpl<toolchain::Constant *> &fields);

  /// Shared code for virtual base adjustment.  Returns the offset from
  /// the vbptr to the virtual base.  Optionally returns the address of the
  /// vbptr itself.
  toolchain::Value *GetVBaseOffsetFromVBPtr(CodeGenFunction &CGF,
                                       Address Base,
                                       toolchain::Value *VBPtrOffset,
                                       toolchain::Value *VBTableOffset,
                                       toolchain::Value **VBPtr = nullptr);

  toolchain::Value *GetVBaseOffsetFromVBPtr(CodeGenFunction &CGF,
                                       Address Base,
                                       int32_t VBPtrOffset,
                                       int32_t VBTableOffset,
                                       toolchain::Value **VBPtr = nullptr) {
    assert(VBTableOffset % 4 == 0 && "should be byte offset into table of i32s");
    toolchain::Value *VBPOffset = toolchain::ConstantInt::get(CGM.IntTy, VBPtrOffset),
                *VBTOffset = toolchain::ConstantInt::get(CGM.IntTy, VBTableOffset);
    return GetVBaseOffsetFromVBPtr(CGF, Base, VBPOffset, VBTOffset, VBPtr);
  }

  std::tuple<Address, toolchain::Value *, const CXXRecordDecl *>
  performBaseAdjustment(CodeGenFunction &CGF, Address Value,
                        QualType SrcRecordTy);

  /// Performs a full virtual base adjustment.  Used to dereference
  /// pointers to members of virtual bases.
  toolchain::Value *AdjustVirtualBase(CodeGenFunction &CGF, const Expr *E,
                                 const CXXRecordDecl *RD, Address Base,
                                 toolchain::Value *VirtualBaseAdjustmentOffset,
                                 toolchain::Value *VBPtrOffset /* optional */);

  /// Emits a full member pointer with the fields common to data and
  /// function member pointers.
  toolchain::Constant *EmitFullMemberPointer(toolchain::Constant *FirstField,
                                        bool IsMemberFunction,
                                        const CXXRecordDecl *RD,
                                        CharUnits NonVirtualBaseAdjustment,
                                        unsigned VBTableIndex);

  bool MemberPointerConstantIsNull(const MemberPointerType *MPT,
                                   toolchain::Constant *MP);

  /// - Initialize all vbptrs of 'this' with RD as the complete type.
  void EmitVBPtrStores(CodeGenFunction &CGF, const CXXRecordDecl *RD);

  /// Caching wrapper around VBTableBuilder::enumerateVBTables().
  const VBTableGlobals &enumerateVBTables(const CXXRecordDecl *RD);

  /// Generate a thunk for calling a virtual member function MD.
  toolchain::Function *EmitVirtualMemPtrThunk(const CXXMethodDecl *MD,
                                         const MethodVFTableLocation &ML);

  toolchain::Constant *EmitMemberDataPointer(const CXXRecordDecl *RD,
                                        CharUnits offset);

public:
  toolchain::Type *ConvertMemberPointerType(const MemberPointerType *MPT) override;

  bool isZeroInitializable(const MemberPointerType *MPT) override;

  bool isMemberPointerConvertible(const MemberPointerType *MPT) const override {
    const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
    return RD->hasAttr<MSInheritanceAttr>();
  }

  toolchain::Constant *EmitNullMemberPointer(const MemberPointerType *MPT) override;

  toolchain::Constant *EmitMemberDataPointer(const MemberPointerType *MPT,
                                        CharUnits offset) override;
  toolchain::Constant *EmitMemberFunctionPointer(const CXXMethodDecl *MD) override;
  toolchain::Constant *EmitMemberPointer(const APValue &MP, QualType MPT) override;

  toolchain::Value *EmitMemberPointerComparison(CodeGenFunction &CGF,
                                           toolchain::Value *L,
                                           toolchain::Value *R,
                                           const MemberPointerType *MPT,
                                           bool Inequality) override;

  toolchain::Value *EmitMemberPointerIsNotNull(CodeGenFunction &CGF,
                                          toolchain::Value *MemPtr,
                                          const MemberPointerType *MPT) override;

  toolchain::Value *EmitMemberDataPointerAddress(CodeGenFunction &CGF, const Expr *E,
                                            Address Base, toolchain::Value *MemPtr,
                                            const MemberPointerType *MPT,
                                            bool IsInBounds) override;

  toolchain::Value *EmitNonNullMemberPointerConversion(
      const MemberPointerType *SrcTy, const MemberPointerType *DstTy,
      CastKind CK, CastExpr::path_const_iterator PathBegin,
      CastExpr::path_const_iterator PathEnd, toolchain::Value *Src,
      CGBuilderTy &Builder);

  toolchain::Value *EmitMemberPointerConversion(CodeGenFunction &CGF,
                                           const CastExpr *E,
                                           toolchain::Value *Src) override;

  toolchain::Constant *EmitMemberPointerConversion(const CastExpr *E,
                                              toolchain::Constant *Src) override;

  toolchain::Constant *EmitMemberPointerConversion(
      const MemberPointerType *SrcTy, const MemberPointerType *DstTy,
      CastKind CK, CastExpr::path_const_iterator PathBegin,
      CastExpr::path_const_iterator PathEnd, toolchain::Constant *Src);

  CGCallee
  EmitLoadOfMemberFunctionPointer(CodeGenFunction &CGF, const Expr *E,
                                  Address This, toolchain::Value *&ThisPtrForCall,
                                  toolchain::Value *MemPtr,
                                  const MemberPointerType *MPT) override;

  void emitCXXStructor(GlobalDecl GD) override;

  toolchain::StructType *getCatchableTypeType() {
    if (CatchableTypeType)
      return CatchableTypeType;
    toolchain::Type *FieldTypes[] = {
        CGM.IntTy,                           // Flags
        getImageRelativeType(CGM.Int8PtrTy), // TypeDescriptor
        CGM.IntTy,                           // NonVirtualAdjustment
        CGM.IntTy,                           // OffsetToVBPtr
        CGM.IntTy,                           // VBTableIndex
        CGM.IntTy,                           // Size
        getImageRelativeType(CGM.Int8PtrTy)  // CopyCtor
    };
    CatchableTypeType = toolchain::StructType::create(
        CGM.getLLVMContext(), FieldTypes, "eh.CatchableType");
    return CatchableTypeType;
  }

  toolchain::StructType *getCatchableTypeArrayType(uint32_t NumEntries) {
    toolchain::StructType *&CatchableTypeArrayType =
        CatchableTypeArrayTypeMap[NumEntries];
    if (CatchableTypeArrayType)
      return CatchableTypeArrayType;

    toolchain::SmallString<23> CTATypeName("eh.CatchableTypeArray.");
    CTATypeName += toolchain::utostr(NumEntries);
    toolchain::Type *CTType = getImageRelativeType(CGM.UnqualPtrTy);
    toolchain::Type *FieldTypes[] = {
        CGM.IntTy,                               // NumEntries
        toolchain::ArrayType::get(CTType, NumEntries) // CatchableTypes
    };
    CatchableTypeArrayType =
        toolchain::StructType::create(CGM.getLLVMContext(), FieldTypes, CTATypeName);
    return CatchableTypeArrayType;
  }

  toolchain::StructType *getThrowInfoType() {
    if (ThrowInfoType)
      return ThrowInfoType;
    toolchain::Type *FieldTypes[] = {
        CGM.IntTy,                           // Flags
        getImageRelativeType(CGM.Int8PtrTy), // CleanupFn
        getImageRelativeType(CGM.Int8PtrTy), // ForwardCompat
        getImageRelativeType(CGM.Int8PtrTy)  // CatchableTypeArray
    };
    ThrowInfoType = toolchain::StructType::create(CGM.getLLVMContext(), FieldTypes,
                                             "eh.ThrowInfo");
    return ThrowInfoType;
  }

  toolchain::FunctionCallee getThrowFn() {
    // _CxxThrowException is passed an exception object and a ThrowInfo object
    // which describes the exception.
    toolchain::Type *Args[] = {CGM.Int8PtrTy, CGM.UnqualPtrTy};
    toolchain::FunctionType *FTy =
        toolchain::FunctionType::get(CGM.VoidTy, Args, /*isVarArg=*/false);
    toolchain::FunctionCallee Throw =
        CGM.CreateRuntimeFunction(FTy, "_CxxThrowException");
    // _CxxThrowException is stdcall on 32-bit x86 platforms.
    if (CGM.getTarget().getTriple().getArch() == toolchain::Triple::x86) {
      if (auto *Fn = dyn_cast<toolchain::Function>(Throw.getCallee()))
        Fn->setCallingConv(toolchain::CallingConv::X86_StdCall);
    }
    return Throw;
  }

  toolchain::Function *getAddrOfCXXCtorClosure(const CXXConstructorDecl *CD,
                                          CXXCtorType CT);

  toolchain::Constant *getCatchableType(QualType T,
                                   uint32_t NVOffset = 0,
                                   int32_t VBPtrOffset = -1,
                                   uint32_t VBIndex = 0);

  toolchain::GlobalVariable *getCatchableTypeArray(QualType T);

  toolchain::GlobalVariable *getThrowInfo(QualType T) override;

  std::pair<toolchain::Value *, const CXXRecordDecl *>
  LoadVTablePtr(CodeGenFunction &CGF, Address This,
                const CXXRecordDecl *RD) override;

  bool
  isPermittedToBeHomogeneousAggregate(const CXXRecordDecl *RD) const override;

private:
  typedef std::pair<const CXXRecordDecl *, CharUnits> VFTableIdTy;
  typedef toolchain::DenseMap<VFTableIdTy, toolchain::GlobalVariable *> VTablesMapTy;
  typedef toolchain::DenseMap<VFTableIdTy, toolchain::GlobalValue *> VFTablesMapTy;
  /// All the vftables that have been referenced.
  VFTablesMapTy VFTablesMap;
  VTablesMapTy VTablesMap;

  /// This set holds the record decls we've deferred vtable emission for.
  toolchain::SmallPtrSet<const CXXRecordDecl *, 4> DeferredVFTables;


  /// All the vbtables which have been referenced.
  toolchain::DenseMap<const CXXRecordDecl *, VBTableGlobals> VBTablesMap;

  /// Info on the global variable used to guard initialization of static locals.
  /// The BitIndex field is only used for externally invisible declarations.
  struct GuardInfo {
    GuardInfo() = default;
    toolchain::GlobalVariable *Guard = nullptr;
    unsigned BitIndex = 0;
  };

  /// Map from DeclContext to the current guard variable.  We assume that the
  /// AST is visited in source code order.
  toolchain::DenseMap<const DeclContext *, GuardInfo> GuardVariableMap;
  toolchain::DenseMap<const DeclContext *, GuardInfo> ThreadLocalGuardVariableMap;
  toolchain::DenseMap<const DeclContext *, unsigned> ThreadSafeGuardNumMap;

  toolchain::DenseMap<size_t, toolchain::StructType *> TypeDescriptorTypeMap;
  toolchain::StructType *BaseClassDescriptorType;
  toolchain::StructType *ClassHierarchyDescriptorType;
  toolchain::StructType *CompleteObjectLocatorType;

  toolchain::DenseMap<QualType, toolchain::GlobalVariable *> CatchableTypeArrays;

  toolchain::StructType *CatchableTypeType;
  toolchain::DenseMap<uint32_t, toolchain::StructType *> CatchableTypeArrayTypeMap;
  toolchain::StructType *ThrowInfoType;
};

}

CGCXXABI::RecordArgABI
MicrosoftCXXABI::getRecordArgABI(const CXXRecordDecl *RD) const {
  // Use the default C calling convention rules for things that can be passed in
  // registers, i.e. non-trivially copyable records or records marked with
  // [[trivial_abi]].
  if (RD->canPassInRegisters())
    return RAA_Default;

  switch (CGM.getTarget().getTriple().getArch()) {
  default:
    // FIXME: Implement for other architectures.
    return RAA_Indirect;

  case toolchain::Triple::thumb:
    // Pass things indirectly for now because it is simple.
    // FIXME: This is incompatible with MSVC for arguments with a dtor and no
    // copy ctor.
    return RAA_Indirect;

  case toolchain::Triple::x86: {
    // If the argument has *required* alignment greater than four bytes, pass
    // it indirectly. Prior to MSVC version 19.14, passing overaligned
    // arguments was not supported and resulted in a compiler error. In 19.14
    // and later versions, such arguments are now passed indirectly.
    TypeInfo Info =
        getContext().getTypeInfo(getContext().getCanonicalTagType(RD));
    if (Info.isAlignRequired() && Info.Align > 4)
      return RAA_Indirect;

    // If C++ prohibits us from making a copy, construct the arguments directly
    // into argument memory.
    return RAA_DirectInMemory;
  }

  case toolchain::Triple::x86_64:
  case toolchain::Triple::aarch64:
    return RAA_Indirect;
  }

  toolchain_unreachable("invalid enum");
}

void MicrosoftCXXABI::emitVirtualObjectDelete(CodeGenFunction &CGF,
                                              const CXXDeleteExpr *DE,
                                              Address Ptr,
                                              QualType ElementType,
                                              const CXXDestructorDecl *Dtor) {
  // FIXME: Provide a source location here even though there's no
  // CXXMemberCallExpr for dtor call.
  bool UseGlobalDelete = DE->isGlobalDelete();
  CXXDtorType DtorType = UseGlobalDelete ? Dtor_Complete : Dtor_Deleting;
  toolchain::Value *MDThis = EmitVirtualDestructorCall(CGF, Dtor, DtorType, Ptr, DE,
                                                  /*CallOrInvoke=*/nullptr);
  if (UseGlobalDelete)
    CGF.EmitDeleteCall(DE->getOperatorDelete(), MDThis, ElementType);
}

void MicrosoftCXXABI::emitRethrow(CodeGenFunction &CGF, bool isNoReturn) {
  toolchain::Value *Args[] = {toolchain::ConstantPointerNull::get(CGM.Int8PtrTy),
                         toolchain::ConstantPointerNull::get(CGM.UnqualPtrTy)};
  toolchain::FunctionCallee Fn = getThrowFn();
  if (isNoReturn)
    CGF.EmitNoreturnRuntimeCallOrInvoke(Fn, Args);
  else
    CGF.EmitRuntimeCallOrInvoke(Fn, Args);
}

void MicrosoftCXXABI::emitBeginCatch(CodeGenFunction &CGF,
                                     const CXXCatchStmt *S) {
  // In the MS ABI, the runtime handles the copy, and the catch handler is
  // responsible for destruction.
  VarDecl *CatchParam = S->getExceptionDecl();
  toolchain::BasicBlock *CatchPadBB = CGF.Builder.GetInsertBlock();
  toolchain::CatchPadInst *CPI =
      cast<toolchain::CatchPadInst>(CatchPadBB->getFirstNonPHIIt());
  CGF.CurrentFuncletPad = CPI;

  // If this is a catch-all or the catch parameter is unnamed, we don't need to
  // emit an alloca to the object.
  if (!CatchParam || !CatchParam->getDeclName()) {
    CGF.EHStack.pushCleanup<CatchRetScope>(NormalCleanup, CPI);
    return;
  }

  CodeGenFunction::AutoVarEmission var = CGF.EmitAutoVarAlloca(*CatchParam);
  CPI->setArgOperand(2, var.getObjectAddress(CGF).emitRawPointer(CGF));
  CGF.EHStack.pushCleanup<CatchRetScope>(NormalCleanup, CPI);
  CGF.EmitAutoVarCleanups(var);
}

/// We need to perform a generic polymorphic operation (like a typeid
/// or a cast), which requires an object with a vfptr.  Adjust the
/// address to point to an object with a vfptr.
std::tuple<Address, toolchain::Value *, const CXXRecordDecl *>
MicrosoftCXXABI::performBaseAdjustment(CodeGenFunction &CGF, Address Value,
                                       QualType SrcRecordTy) {
  Value = Value.withElementType(CGF.Int8Ty);
  const CXXRecordDecl *SrcDecl = SrcRecordTy->getAsCXXRecordDecl();
  const ASTContext &Context = getContext();

  // If the class itself has a vfptr, great.  This check implicitly
  // covers non-virtual base subobjects: a class with its own virtual
  // functions would be a candidate to be a primary base.
  if (Context.getASTRecordLayout(SrcDecl).hasExtendableVFPtr())
    return std::make_tuple(Value, toolchain::ConstantInt::get(CGF.Int32Ty, 0),
                           SrcDecl);

  // Okay, one of the vbases must have a vfptr, or else this isn't
  // actually a polymorphic class.
  const CXXRecordDecl *PolymorphicBase = nullptr;
  for (auto &Base : SrcDecl->vbases()) {
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();
    if (Context.getASTRecordLayout(BaseDecl).hasExtendableVFPtr()) {
      PolymorphicBase = BaseDecl;
      break;
    }
  }
  assert(PolymorphicBase && "polymorphic class has no apparent vfptr?");

  toolchain::Value *Offset =
    GetVirtualBaseClassOffset(CGF, Value, SrcDecl, PolymorphicBase);
  toolchain::Value *Ptr = CGF.Builder.CreateInBoundsGEP(
      Value.getElementType(), Value.emitRawPointer(CGF), Offset);
  CharUnits VBaseAlign =
    CGF.CGM.getVBaseAlignment(Value.getAlignment(), SrcDecl, PolymorphicBase);
  return std::make_tuple(Address(Ptr, CGF.Int8Ty, VBaseAlign), Offset,
                         PolymorphicBase);
}

bool MicrosoftCXXABI::shouldTypeidBeNullChecked(QualType SrcRecordTy) {
  const CXXRecordDecl *SrcDecl = SrcRecordTy->getAsCXXRecordDecl();
  return !getContext().getASTRecordLayout(SrcDecl).hasExtendableVFPtr();
}

static toolchain::CallBase *emitRTtypeidCall(CodeGenFunction &CGF,
                                        toolchain::Value *Argument) {
  toolchain::Type *ArgTypes[] = {CGF.Int8PtrTy};
  toolchain::FunctionType *FTy =
      toolchain::FunctionType::get(CGF.Int8PtrTy, ArgTypes, false);
  toolchain::Value *Args[] = {Argument};
  toolchain::FunctionCallee Fn = CGF.CGM.CreateRuntimeFunction(FTy, "__RTtypeid");
  return CGF.EmitRuntimeCallOrInvoke(Fn, Args);
}

void MicrosoftCXXABI::EmitBadTypeidCall(CodeGenFunction &CGF) {
  toolchain::CallBase *Call =
      emitRTtypeidCall(CGF, toolchain::Constant::getNullValue(CGM.VoidPtrTy));
  Call->setDoesNotReturn();
  CGF.Builder.CreateUnreachable();
}

toolchain::Value *MicrosoftCXXABI::EmitTypeid(CodeGenFunction &CGF,
                                         QualType SrcRecordTy,
                                         Address ThisPtr,
                                         toolchain::Type *StdTypeInfoPtrTy) {
  std::tie(ThisPtr, std::ignore, std::ignore) =
      performBaseAdjustment(CGF, ThisPtr, SrcRecordTy);
  toolchain::CallBase *Typeid = emitRTtypeidCall(CGF, ThisPtr.emitRawPointer(CGF));
  return CGF.Builder.CreateBitCast(Typeid, StdTypeInfoPtrTy);
}

bool MicrosoftCXXABI::shouldDynamicCastCallBeNullChecked(bool SrcIsPtr,
                                                         QualType SrcRecordTy) {
  const CXXRecordDecl *SrcDecl = SrcRecordTy->getAsCXXRecordDecl();
  return SrcIsPtr &&
         !getContext().getASTRecordLayout(SrcDecl).hasExtendableVFPtr();
}

toolchain::Value *MicrosoftCXXABI::emitDynamicCastCall(
    CodeGenFunction &CGF, Address This, QualType SrcRecordTy, QualType DestTy,
    QualType DestRecordTy, toolchain::BasicBlock *CastEnd) {
  toolchain::Value *SrcRTTI =
      CGF.CGM.GetAddrOfRTTIDescriptor(SrcRecordTy.getUnqualifiedType());
  toolchain::Value *DestRTTI =
      CGF.CGM.GetAddrOfRTTIDescriptor(DestRecordTy.getUnqualifiedType());

  toolchain::Value *Offset;
  std::tie(This, Offset, std::ignore) =
      performBaseAdjustment(CGF, This, SrcRecordTy);
  toolchain::Value *ThisPtr = This.emitRawPointer(CGF);
  Offset = CGF.Builder.CreateTrunc(Offset, CGF.Int32Ty);

  // PVOID __RTDynamicCast(
  //   PVOID inptr,
  //   LONG VfDelta,
  //   PVOID SrcType,
  //   PVOID TargetType,
  //   BOOL isReference)
  toolchain::Type *ArgTypes[] = {CGF.Int8PtrTy, CGF.Int32Ty, CGF.Int8PtrTy,
                            CGF.Int8PtrTy, CGF.Int32Ty};
  toolchain::FunctionCallee Function = CGF.CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(CGF.Int8PtrTy, ArgTypes, false),
      "__RTDynamicCast");
  toolchain::Value *Args[] = {
      ThisPtr, Offset, SrcRTTI, DestRTTI,
      toolchain::ConstantInt::get(CGF.Int32Ty, DestTy->isReferenceType())};
  return CGF.EmitRuntimeCallOrInvoke(Function, Args);
}

toolchain::Value *MicrosoftCXXABI::emitDynamicCastToVoid(CodeGenFunction &CGF,
                                                    Address Value,
                                                    QualType SrcRecordTy) {
  std::tie(Value, std::ignore, std::ignore) =
      performBaseAdjustment(CGF, Value, SrcRecordTy);

  // PVOID __RTCastToVoid(
  //   PVOID inptr)
  toolchain::Type *ArgTypes[] = {CGF.Int8PtrTy};
  toolchain::FunctionCallee Function = CGF.CGM.CreateRuntimeFunction(
      toolchain::FunctionType::get(CGF.Int8PtrTy, ArgTypes, false),
      "__RTCastToVoid");
  toolchain::Value *Args[] = {Value.emitRawPointer(CGF)};
  return CGF.EmitRuntimeCall(Function, Args);
}

bool MicrosoftCXXABI::EmitBadCastCall(CodeGenFunction &CGF) {
  return false;
}

toolchain::Value *MicrosoftCXXABI::GetVirtualBaseClassOffset(
    CodeGenFunction &CGF, Address This, const CXXRecordDecl *ClassDecl,
    const CXXRecordDecl *BaseClassDecl) {
  const ASTContext &Context = getContext();
  int64_t VBPtrChars =
      Context.getASTRecordLayout(ClassDecl).getVBPtrOffset().getQuantity();
  toolchain::Value *VBPtrOffset = toolchain::ConstantInt::get(CGM.PtrDiffTy, VBPtrChars);
  CharUnits IntSize = Context.getTypeSizeInChars(Context.IntTy);
  CharUnits VBTableChars =
      IntSize *
      CGM.getMicrosoftVTableContext().getVBTableIndex(ClassDecl, BaseClassDecl);
  toolchain::Value *VBTableOffset =
      toolchain::ConstantInt::get(CGM.IntTy, VBTableChars.getQuantity());

  toolchain::Value *VBPtrToNewBase =
      GetVBaseOffsetFromVBPtr(CGF, This, VBPtrOffset, VBTableOffset);
  VBPtrToNewBase =
      CGF.Builder.CreateSExtOrBitCast(VBPtrToNewBase, CGM.PtrDiffTy);
  return CGF.Builder.CreateNSWAdd(VBPtrOffset, VBPtrToNewBase);
}

bool MicrosoftCXXABI::HasThisReturn(GlobalDecl GD) const {
  return isa<CXXConstructorDecl>(GD.getDecl());
}

static bool isDeletingDtor(GlobalDecl GD) {
  return isa<CXXDestructorDecl>(GD.getDecl()) &&
         GD.getDtorType() == Dtor_Deleting;
}

bool MicrosoftCXXABI::hasMostDerivedReturn(GlobalDecl GD) const {
  return isDeletingDtor(GD);
}

static bool isTrivialForMSVC(const CXXRecordDecl *RD, QualType Ty,
                             CodeGenModule &CGM) {
  // On AArch64, HVAs that can be passed in registers can also be returned
  // in registers. (Note this is using the MSVC definition of an HVA; see
  // isPermittedToBeHomogeneousAggregate().)
  const Type *Base = nullptr;
  uint64_t NumElts = 0;
  if (CGM.getTarget().getTriple().isAArch64() &&
      CGM.getABIInfo().isHomogeneousAggregate(Ty, Base, NumElts) &&
      isa<VectorType>(Base)) {
    return true;
  }

  // We use the C++14 definition of an aggregate, so we also
  // check for:
  //   No private or protected non static data members.
  //   No base classes
  //   No virtual functions
  // Additionally, we need to ensure that there is a trivial copy assignment
  // operator, a trivial destructor, no user-provided constructors and no
  // deleted copy assignment operator.

  // We need to cover two cases when checking for a deleted copy assignment
  // operator.
  //
  // struct S { int& r; };
  // The above will have an implicit copy assignment operator that is deleted
  // and there will not be a `CXXMethodDecl` for the copy assignment operator.
  // This is handled by the `needsImplicitCopyAssignment()` check below.
  //
  // struct S { S& operator=(const S&) = delete; int i; };
  // The above will not have an implicit copy assignment operator that is
  // deleted but there is a deleted `CXXMethodDecl` for the declared copy
  // assignment operator. This is handled by the `isDeleted()` check below.

  if (RD->hasProtectedFields() || RD->hasPrivateFields())
    return false;
  if (RD->getNumBases() > 0)
    return false;
  if (RD->isPolymorphic())
    return false;
  if (RD->hasNonTrivialCopyAssignment())
    return false;
  if (RD->needsImplicitCopyAssignment() && !RD->hasSimpleCopyAssignment())
    return false;
  for (const Decl *D : RD->decls()) {
    if (auto *Ctor = dyn_cast<CXXConstructorDecl>(D)) {
      if (Ctor->isUserProvided())
        return false;
    } else if (auto *Template = dyn_cast<FunctionTemplateDecl>(D)) {
      if (isa<CXXConstructorDecl>(Template->getTemplatedDecl()))
        return false;
    } else if (auto *MethodDecl = dyn_cast<CXXMethodDecl>(D)) {
      if (MethodDecl->isCopyAssignmentOperator() && MethodDecl->isDeleted())
        return false;
    }
  }
  if (RD->hasNonTrivialDestructor())
    return false;
  return true;
}

bool MicrosoftCXXABI::classifyReturnType(CGFunctionInfo &FI) const {
  const CXXRecordDecl *RD = FI.getReturnType()->getAsCXXRecordDecl();
  if (!RD)
    return false;

  bool isTrivialForABI = RD->canPassInRegisters() &&
                         isTrivialForMSVC(RD, FI.getReturnType(), CGM);

  // MSVC always returns structs indirectly from C++ instance methods.
  bool isIndirectReturn = !isTrivialForABI || FI.isInstanceMethod();

  if (isIndirectReturn) {
    CharUnits Align = CGM.getContext().getTypeAlignInChars(FI.getReturnType());
    FI.getReturnInfo() = ABIArgInfo::getIndirect(
        Align, /*AddrSpace=*/CGM.getDataLayout().getAllocaAddrSpace(),
        /*ByVal=*/false);

    // MSVC always passes `this` before the `sret` parameter.
    FI.getReturnInfo().setSRetAfterThis(FI.isInstanceMethod());

    // On AArch64, use the `inreg` attribute if the object is considered to not
    // be trivially copyable, or if this is an instance method struct return.
    FI.getReturnInfo().setInReg(CGM.getTarget().getTriple().isAArch64());

    return true;
  }

  // Otherwise, use the C ABI rules.
  return false;
}

toolchain::BasicBlock *
MicrosoftCXXABI::EmitCtorCompleteObjectHandler(CodeGenFunction &CGF,
                                               const CXXRecordDecl *RD) {
  toolchain::Value *IsMostDerivedClass = getStructorImplicitParamValue(CGF);
  assert(IsMostDerivedClass &&
         "ctor for a class with virtual bases must have an implicit parameter");
  toolchain::Value *IsCompleteObject =
    CGF.Builder.CreateIsNotNull(IsMostDerivedClass, "is_complete_object");

  toolchain::BasicBlock *CallVbaseCtorsBB = CGF.createBasicBlock("ctor.init_vbases");
  toolchain::BasicBlock *SkipVbaseCtorsBB = CGF.createBasicBlock("ctor.skip_vbases");
  CGF.Builder.CreateCondBr(IsCompleteObject,
                           CallVbaseCtorsBB, SkipVbaseCtorsBB);

  CGF.EmitBlock(CallVbaseCtorsBB);

  // Fill in the vbtable pointers here.
  EmitVBPtrStores(CGF, RD);

  // CGF will put the base ctor calls in this basic block for us later.

  return SkipVbaseCtorsBB;
}

toolchain::BasicBlock *
MicrosoftCXXABI::EmitDtorCompleteObjectHandler(CodeGenFunction &CGF) {
  toolchain::Value *IsMostDerivedClass = getStructorImplicitParamValue(CGF);
  assert(IsMostDerivedClass &&
         "ctor for a class with virtual bases must have an implicit parameter");
  toolchain::Value *IsCompleteObject =
      CGF.Builder.CreateIsNotNull(IsMostDerivedClass, "is_complete_object");

  toolchain::BasicBlock *CallVbaseDtorsBB = CGF.createBasicBlock("Dtor.dtor_vbases");
  toolchain::BasicBlock *SkipVbaseDtorsBB = CGF.createBasicBlock("Dtor.skip_vbases");
  CGF.Builder.CreateCondBr(IsCompleteObject,
                           CallVbaseDtorsBB, SkipVbaseDtorsBB);

  CGF.EmitBlock(CallVbaseDtorsBB);
  // CGF will put the base dtor calls in this basic block for us later.

  return SkipVbaseDtorsBB;
}

void MicrosoftCXXABI::initializeHiddenVirtualInheritanceMembers(
    CodeGenFunction &CGF, const CXXRecordDecl *RD) {
  // In most cases, an override for a vbase virtual method can adjust
  // the "this" parameter by applying a constant offset.
  // However, this is not enough while a constructor or a destructor of some
  // class X is being executed if all the following conditions are met:
  //  - X has virtual bases, (1)
  //  - X overrides a virtual method M of a vbase Y, (2)
  //  - X itself is a vbase of the most derived class.
  //
  // If (1) and (2) are true, the vtorDisp for vbase Y is a hidden member of X
  // which holds the extra amount of "this" adjustment we must do when we use
  // the X vftables (i.e. during X ctor or dtor).
  // Outside the ctors and dtors, the values of vtorDisps are zero.

  const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
  typedef ASTRecordLayout::VBaseOffsetsMapTy VBOffsets;
  const VBOffsets &VBaseMap = Layout.getVBaseOffsetsMap();
  CGBuilderTy &Builder = CGF.Builder;

  toolchain::Value *Int8This = nullptr;  // Initialize lazily.

  for (const CXXBaseSpecifier &S : RD->vbases()) {
    const CXXRecordDecl *VBase = S.getType()->getAsCXXRecordDecl();
    auto I = VBaseMap.find(VBase);
    assert(I != VBaseMap.end());
    if (!I->second.hasVtorDisp())
      continue;

    toolchain::Value *VBaseOffset =
        GetVirtualBaseClassOffset(CGF, getThisAddress(CGF), RD, VBase);
    uint64_t ConstantVBaseOffset = I->second.VBaseOffset.getQuantity();

    // vtorDisp_for_vbase = vbptr[vbase_idx] - offsetof(RD, vbase).
    toolchain::Value *VtorDispValue = Builder.CreateSub(
        VBaseOffset, toolchain::ConstantInt::get(CGM.PtrDiffTy, ConstantVBaseOffset),
        "vtordisp.value");
    VtorDispValue = Builder.CreateTruncOrBitCast(VtorDispValue, CGF.Int32Ty);

    if (!Int8This)
      Int8This = getThisValue(CGF);

    toolchain::Value *VtorDispPtr =
        Builder.CreateInBoundsGEP(CGF.Int8Ty, Int8This, VBaseOffset);
    // vtorDisp is always the 32-bits before the vbase in the class layout.
    VtorDispPtr = Builder.CreateConstGEP1_32(CGF.Int8Ty, VtorDispPtr, -4);

    Builder.CreateAlignedStore(VtorDispValue, VtorDispPtr,
                               CharUnits::fromQuantity(4));
  }
}

static bool hasDefaultCXXMethodCC(ASTContext &Context,
                                  const CXXMethodDecl *MD) {
  CallingConv ExpectedCallingConv = Context.getDefaultCallingConvention(
      /*IsVariadic=*/false, /*IsCXXMethod=*/true);
  CallingConv ActualCallingConv =
      MD->getType()->castAs<FunctionProtoType>()->getCallConv();
  return ExpectedCallingConv == ActualCallingConv;
}

void MicrosoftCXXABI::EmitCXXConstructors(const CXXConstructorDecl *D) {
  // There's only one constructor type in this ABI.
  CGM.EmitGlobal(GlobalDecl(D, Ctor_Complete));

  // Exported default constructors either have a simple call-site where they use
  // the typical calling convention and have a single 'this' pointer for an
  // argument -or- they get a wrapper function which appropriately thunks to the
  // real default constructor.  This thunk is the default constructor closure.
  if (D->hasAttr<DLLExportAttr>() && D->isDefaultConstructor() &&
      D->isDefined()) {
    if (!hasDefaultCXXMethodCC(getContext(), D) || D->getNumParams() != 0) {
      toolchain::Function *Fn = getAddrOfCXXCtorClosure(D, Ctor_DefaultClosure);
      Fn->setLinkage(toolchain::GlobalValue::WeakODRLinkage);
      CGM.setGVProperties(Fn, D);
    }
  }
}

void MicrosoftCXXABI::EmitVBPtrStores(CodeGenFunction &CGF,
                                      const CXXRecordDecl *RD) {
  Address This = getThisAddress(CGF);
  This = This.withElementType(CGM.Int8Ty);
  const ASTContext &Context = getContext();
  const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);

  const VBTableGlobals &VBGlobals = enumerateVBTables(RD);
  for (unsigned I = 0, E = VBGlobals.VBTables->size(); I != E; ++I) {
    const std::unique_ptr<VPtrInfo> &VBT = (*VBGlobals.VBTables)[I];
    toolchain::GlobalVariable *GV = VBGlobals.Globals[I];
    const ASTRecordLayout &SubobjectLayout =
        Context.getASTRecordLayout(VBT->IntroducingObject);
    CharUnits Offs = VBT->NonVirtualOffset;
    Offs += SubobjectLayout.getVBPtrOffset();
    if (VBT->getVBaseWithVPtr())
      Offs += Layout.getVBaseClassOffset(VBT->getVBaseWithVPtr());
    Address VBPtr = CGF.Builder.CreateConstInBoundsByteGEP(This, Offs);
    toolchain::Value *GVPtr =
        CGF.Builder.CreateConstInBoundsGEP2_32(GV->getValueType(), GV, 0, 0);
    VBPtr = VBPtr.withElementType(GVPtr->getType());
    CGF.Builder.CreateStore(GVPtr, VBPtr);
  }
}

CGCXXABI::AddedStructorArgCounts
MicrosoftCXXABI::buildStructorSignature(GlobalDecl GD,
                                        SmallVectorImpl<CanQualType> &ArgTys) {
  AddedStructorArgCounts Added;
  // TODO: 'for base' flag
  if (isa<CXXDestructorDecl>(GD.getDecl()) &&
      GD.getDtorType() == Dtor_Deleting) {
    // The scalar deleting destructor takes an implicit int parameter.
    ArgTys.push_back(getContext().IntTy);
    ++Added.Suffix;
  }
  auto *CD = dyn_cast<CXXConstructorDecl>(GD.getDecl());
  if (!CD)
    return Added;

  // All parameters are already in place except is_most_derived, which goes
  // after 'this' if it's variadic and last if it's not.

  const CXXRecordDecl *Class = CD->getParent();
  const FunctionProtoType *FPT = CD->getType()->castAs<FunctionProtoType>();
  if (Class->getNumVBases()) {
    if (FPT->isVariadic()) {
      ArgTys.insert(ArgTys.begin() + 1, getContext().IntTy);
      ++Added.Prefix;
    } else {
      ArgTys.push_back(getContext().IntTy);
      ++Added.Suffix;
    }
  }

  return Added;
}

void MicrosoftCXXABI::setCXXDestructorDLLStorage(toolchain::GlobalValue *GV,
                                                 const CXXDestructorDecl *Dtor,
                                                 CXXDtorType DT) const {
  // Deleting destructor variants are never imported or exported. Give them the
  // default storage class.
  if (DT == Dtor_Deleting) {
    GV->setDLLStorageClass(toolchain::GlobalValue::DefaultStorageClass);
  } else {
    const NamedDecl *ND = Dtor;
    CGM.setDLLImportDLLExport(GV, ND);
  }
}

toolchain::GlobalValue::LinkageTypes MicrosoftCXXABI::getCXXDestructorLinkage(
    GVALinkage Linkage, const CXXDestructorDecl *Dtor, CXXDtorType DT) const {
  // Internal things are always internal, regardless of attributes. After this,
  // we know the thunk is externally visible.
  if (Linkage == GVA_Internal)
    return toolchain::GlobalValue::InternalLinkage;

  switch (DT) {
  case Dtor_Base:
    // The base destructor most closely tracks the user-declared constructor, so
    // we delegate back to the normal declarator case.
    return CGM.getLLVMLinkageForDeclarator(Dtor, Linkage);
  case Dtor_Complete:
    // The complete destructor is like an inline function, but it may be
    // imported and therefore must be exported as well. This requires changing
    // the linkage if a DLL attribute is present.
    if (Dtor->hasAttr<DLLExportAttr>())
      return toolchain::GlobalValue::WeakODRLinkage;
    if (Dtor->hasAttr<DLLImportAttr>())
      return toolchain::GlobalValue::AvailableExternallyLinkage;
    return toolchain::GlobalValue::LinkOnceODRLinkage;
  case Dtor_Deleting:
    // Deleting destructors are like inline functions. They have vague linkage
    // and are emitted everywhere they are used. They are internal if the class
    // is internal.
    return toolchain::GlobalValue::LinkOnceODRLinkage;
  case Dtor_Comdat:
    toolchain_unreachable("MS C++ ABI does not support comdat dtors");
  }
  toolchain_unreachable("invalid dtor type");
}

void MicrosoftCXXABI::EmitCXXDestructors(const CXXDestructorDecl *D) {
  // The TU defining a dtor is only guaranteed to emit a base destructor.  All
  // other destructor variants are delegating thunks.
  CGM.EmitGlobal(GlobalDecl(D, Dtor_Base));

  // If the class is dllexported, emit the complete (vbase) destructor wherever
  // the base dtor is emitted.
  // FIXME: To match MSVC, this should only be done when the class is exported
  // with -fdllexport-inlines enabled.
  if (D->getParent()->getNumVBases() > 0 && D->hasAttr<DLLExportAttr>())
    CGM.EmitGlobal(GlobalDecl(D, Dtor_Complete));
}

CharUnits
MicrosoftCXXABI::getVirtualFunctionPrologueThisAdjustment(GlobalDecl GD) {
  const CXXMethodDecl *MD = cast<CXXMethodDecl>(GD.getDecl());

  if (const CXXDestructorDecl *DD = dyn_cast<CXXDestructorDecl>(MD)) {
    // Complete destructors take a pointer to the complete object as a
    // parameter, thus don't need this adjustment.
    if (GD.getDtorType() == Dtor_Complete)
      return CharUnits();

    // There's no Dtor_Base in vftable but it shares the this adjustment with
    // the deleting one, so look it up instead.
    GD = GlobalDecl(DD, Dtor_Deleting);
  }

  MethodVFTableLocation ML =
      CGM.getMicrosoftVTableContext().getMethodVFTableLocation(GD);
  CharUnits Adjustment = ML.VFPtrOffset;

  // Normal virtual instance methods need to adjust from the vfptr that first
  // defined the virtual method to the virtual base subobject, but destructors
  // do not.  The vector deleting destructor thunk applies this adjustment for
  // us if necessary.
  if (isa<CXXDestructorDecl>(MD))
    Adjustment = CharUnits::Zero();

  if (ML.VBase) {
    const ASTRecordLayout &DerivedLayout =
        getContext().getASTRecordLayout(MD->getParent());
    Adjustment += DerivedLayout.getVBaseClassOffset(ML.VBase);
  }

  return Adjustment;
}

Address MicrosoftCXXABI::adjustThisArgumentForVirtualFunctionCall(
    CodeGenFunction &CGF, GlobalDecl GD, Address This,
    bool VirtualCall) {
  if (!VirtualCall) {
    // If the call of a virtual function is not virtual, we just have to
    // compensate for the adjustment the virtual function does in its prologue.
    CharUnits Adjustment = getVirtualFunctionPrologueThisAdjustment(GD);
    if (Adjustment.isZero())
      return This;

    This = This.withElementType(CGF.Int8Ty);
    assert(Adjustment.isPositive());
    return CGF.Builder.CreateConstByteGEP(This, Adjustment);
  }

  const CXXMethodDecl *MD = cast<CXXMethodDecl>(GD.getDecl());

  GlobalDecl LookupGD = GD;
  if (const CXXDestructorDecl *DD = dyn_cast<CXXDestructorDecl>(MD)) {
    // Complete dtors take a pointer to the complete object,
    // thus don't need adjustment.
    if (GD.getDtorType() == Dtor_Complete)
      return This;

    // There's only Dtor_Deleting in vftable but it shares the this adjustment
    // with the base one, so look up the deleting one instead.
    LookupGD = GlobalDecl(DD, Dtor_Deleting);
  }
  MethodVFTableLocation ML =
      CGM.getMicrosoftVTableContext().getMethodVFTableLocation(LookupGD);

  CharUnits StaticOffset = ML.VFPtrOffset;

  // Base destructors expect 'this' to point to the beginning of the base
  // subobject, not the first vfptr that happens to contain the virtual dtor.
  // However, we still need to apply the virtual base adjustment.
  if (isa<CXXDestructorDecl>(MD) && GD.getDtorType() == Dtor_Base)
    StaticOffset = CharUnits::Zero();

  Address Result = This;
  if (ML.VBase) {
    Result = Result.withElementType(CGF.Int8Ty);

    const CXXRecordDecl *Derived = MD->getParent();
    const CXXRecordDecl *VBase = ML.VBase;
    toolchain::Value *VBaseOffset =
      GetVirtualBaseClassOffset(CGF, Result, Derived, VBase);
    toolchain::Value *VBasePtr = CGF.Builder.CreateInBoundsGEP(
        Result.getElementType(), Result.emitRawPointer(CGF), VBaseOffset);
    CharUnits VBaseAlign =
      CGF.CGM.getVBaseAlignment(Result.getAlignment(), Derived, VBase);
    Result = Address(VBasePtr, CGF.Int8Ty, VBaseAlign);
  }
  if (!StaticOffset.isZero()) {
    assert(StaticOffset.isPositive());
    Result = Result.withElementType(CGF.Int8Ty);
    if (ML.VBase) {
      // Non-virtual adjustment might result in a pointer outside the allocated
      // object, e.g. if the final overrider class is laid out after the virtual
      // base that declares a method in the most derived class.
      // FIXME: Update the code that emits this adjustment in thunks prologues.
      Result = CGF.Builder.CreateConstByteGEP(Result, StaticOffset);
    } else {
      Result = CGF.Builder.CreateConstInBoundsByteGEP(Result, StaticOffset);
    }
  }
  return Result;
}

void MicrosoftCXXABI::addImplicitStructorParams(CodeGenFunction &CGF,
                                                QualType &ResTy,
                                                FunctionArgList &Params) {
  ASTContext &Context = getContext();
  const CXXMethodDecl *MD = cast<CXXMethodDecl>(CGF.CurGD.getDecl());
  assert(isa<CXXConstructorDecl>(MD) || isa<CXXDestructorDecl>(MD));
  if (isa<CXXConstructorDecl>(MD) && MD->getParent()->getNumVBases()) {
    auto *IsMostDerived = ImplicitParamDecl::Create(
        Context, /*DC=*/nullptr, CGF.CurGD.getDecl()->getLocation(),
        &Context.Idents.get("is_most_derived"), Context.IntTy,
        ImplicitParamKind::Other);
    // The 'most_derived' parameter goes second if the ctor is variadic and last
    // if it's not.  Dtors can't be variadic.
    const FunctionProtoType *FPT = MD->getType()->castAs<FunctionProtoType>();
    if (FPT->isVariadic())
      Params.insert(Params.begin() + 1, IsMostDerived);
    else
      Params.push_back(IsMostDerived);
    getStructorImplicitParamDecl(CGF) = IsMostDerived;
  } else if (isDeletingDtor(CGF.CurGD)) {
    auto *ShouldDelete = ImplicitParamDecl::Create(
        Context, /*DC=*/nullptr, CGF.CurGD.getDecl()->getLocation(),
        &Context.Idents.get("should_call_delete"), Context.IntTy,
        ImplicitParamKind::Other);
    Params.push_back(ShouldDelete);
    getStructorImplicitParamDecl(CGF) = ShouldDelete;
  }
}

void MicrosoftCXXABI::EmitInstanceFunctionProlog(CodeGenFunction &CGF) {
  // Naked functions have no prolog.
  if (CGF.CurFuncDecl && CGF.CurFuncDecl->hasAttr<NakedAttr>())
    return;

  // Overridden virtual methods of non-primary bases need to adjust the incoming
  // 'this' pointer in the prologue. In this hierarchy, C::b will subtract
  // sizeof(void*) to adjust from B* to C*:
  //   struct A { virtual void a(); };
  //   struct B { virtual void b(); };
  //   struct C : A, B { virtual void b(); };
  //
  // Leave the value stored in the 'this' alloca unadjusted, so that the
  // debugger sees the unadjusted value. Microsoft debuggers require this, and
  // will apply the ThisAdjustment in the method type information.
  // FIXME: Do something better for DWARF debuggers, which won't expect this,
  // without making our codegen depend on debug info settings.
  toolchain::Value *This = loadIncomingCXXThis(CGF);
  const CXXMethodDecl *MD = cast<CXXMethodDecl>(CGF.CurGD.getDecl());
  if (!CGF.CurFuncIsThunk && MD->isVirtual()) {
    CharUnits Adjustment = getVirtualFunctionPrologueThisAdjustment(CGF.CurGD);
    if (!Adjustment.isZero()) {
      assert(Adjustment.isPositive());
      This = CGF.Builder.CreateConstInBoundsGEP1_32(CGF.Int8Ty, This,
                                                    -Adjustment.getQuantity());
    }
  }
  setCXXABIThisValue(CGF, This);

  // If this is a function that the ABI specifies returns 'this', initialize
  // the return slot to 'this' at the start of the function.
  //
  // Unlike the setting of return types, this is done within the ABI
  // implementation instead of by clients of CGCXXABI because:
  // 1) getThisValue is currently protected
  // 2) in theory, an ABI could implement 'this' returns some other way;
  //    HasThisReturn only specifies a contract, not the implementation
  if (HasThisReturn(CGF.CurGD) || hasMostDerivedReturn(CGF.CurGD))
    CGF.Builder.CreateStore(getThisValue(CGF), CGF.ReturnValue);

  if (isa<CXXConstructorDecl>(MD) && MD->getParent()->getNumVBases()) {
    assert(getStructorImplicitParamDecl(CGF) &&
           "no implicit parameter for a constructor with virtual bases?");
    getStructorImplicitParamValue(CGF)
      = CGF.Builder.CreateLoad(
          CGF.GetAddrOfLocalVar(getStructorImplicitParamDecl(CGF)),
          "is_most_derived");
  }

  if (isDeletingDtor(CGF.CurGD)) {
    assert(getStructorImplicitParamDecl(CGF) &&
           "no implicit parameter for a deleting destructor?");
    getStructorImplicitParamValue(CGF)
      = CGF.Builder.CreateLoad(
          CGF.GetAddrOfLocalVar(getStructorImplicitParamDecl(CGF)),
          "should_call_delete");
  }
}

CGCXXABI::AddedStructorArgs MicrosoftCXXABI::getImplicitConstructorArgs(
    CodeGenFunction &CGF, const CXXConstructorDecl *D, CXXCtorType Type,
    bool ForVirtualBase, bool Delegating) {
  assert(Type == Ctor_Complete || Type == Ctor_Base);

  // Check if we need a 'most_derived' parameter.
  if (!D->getParent()->getNumVBases())
    return AddedStructorArgs{};

  // Add the 'most_derived' argument second if we are variadic or last if not.
  const FunctionProtoType *FPT = D->getType()->castAs<FunctionProtoType>();
  toolchain::Value *MostDerivedArg;
  if (Delegating) {
    MostDerivedArg = getStructorImplicitParamValue(CGF);
  } else {
    MostDerivedArg = toolchain::ConstantInt::get(CGM.Int32Ty, Type == Ctor_Complete);
  }
  if (FPT->isVariadic()) {
    return AddedStructorArgs::prefix({{MostDerivedArg, getContext().IntTy}});
  }
  return AddedStructorArgs::suffix({{MostDerivedArg, getContext().IntTy}});
}

toolchain::Value *MicrosoftCXXABI::getCXXDestructorImplicitParam(
    CodeGenFunction &CGF, const CXXDestructorDecl *DD, CXXDtorType Type,
    bool ForVirtualBase, bool Delegating) {
  return nullptr;
}

void MicrosoftCXXABI::EmitDestructorCall(CodeGenFunction &CGF,
                                         const CXXDestructorDecl *DD,
                                         CXXDtorType Type, bool ForVirtualBase,
                                         bool Delegating, Address This,
                                         QualType ThisTy) {
  // Use the base destructor variant in place of the complete destructor variant
  // if the class has no virtual bases. This effectively implements some of the
  // -mconstructor-aliases optimization, but as part of the MS C++ ABI.
  if (Type == Dtor_Complete && DD->getParent()->getNumVBases() == 0)
    Type = Dtor_Base;

  GlobalDecl GD(DD, Type);
  CGCallee Callee = CGCallee::forDirect(CGM.getAddrOfCXXStructor(GD), GD);

  if (DD->isVirtual()) {
    assert(Type != CXXDtorType::Dtor_Deleting &&
           "The deleting destructor should only be called via a virtual call");
    This = adjustThisArgumentForVirtualFunctionCall(CGF, GlobalDecl(DD, Type),
                                                    This, false);
  }

  toolchain::BasicBlock *BaseDtorEndBB = nullptr;
  if (ForVirtualBase && isa<CXXConstructorDecl>(CGF.CurCodeDecl)) {
    BaseDtorEndBB = EmitDtorCompleteObjectHandler(CGF);
  }

  toolchain::Value *Implicit =
      getCXXDestructorImplicitParam(CGF, DD, Type, ForVirtualBase,
                                    Delegating); // = nullptr
  CGF.EmitCXXDestructorCall(GD, Callee, CGF.getAsNaturalPointerTo(This, ThisTy),
                            ThisTy,
                            /*ImplicitParam=*/Implicit,
                            /*ImplicitParamTy=*/QualType(), /*E=*/nullptr);
  if (BaseDtorEndBB) {
    // Complete object handler should continue to be the remaining
    CGF.Builder.CreateBr(BaseDtorEndBB);
    CGF.EmitBlock(BaseDtorEndBB);
  }
}

void MicrosoftCXXABI::emitVTableTypeMetadata(const VPtrInfo &Info,
                                             const CXXRecordDecl *RD,
                                             toolchain::GlobalVariable *VTable) {
  // Emit type metadata on vtables with LTO or IR instrumentation.
  // In IR instrumentation, the type metadata could be used to find out vtable
  // definitions (for type profiling) among all global variables.
  if (!CGM.getCodeGenOpts().LTOUnit &&
      !CGM.getCodeGenOpts().hasProfileIRInstr())
    return;

  // TODO: Should VirtualFunctionElimination also be supported here?
  // See similar handling in CodeGenModule::EmitVTableTypeMetadata.
  if (CGM.getCodeGenOpts().WholeProgramVTables) {
    toolchain::DenseSet<const CXXRecordDecl *> Visited;
    toolchain::GlobalObject::VCallVisibility TypeVis =
        CGM.GetVCallVisibilityLevel(RD, Visited);
    if (TypeVis != toolchain::GlobalObject::VCallVisibilityPublic)
      VTable->setVCallVisibilityMetadata(TypeVis);
  }

  // The location of the first virtual function pointer in the virtual table,
  // aka the "address point" on Itanium. This is at offset 0 if RTTI is
  // disabled, or sizeof(void*) if RTTI is enabled.
  CharUnits AddressPoint =
      getContext().getLangOpts().RTTIData
          ? getContext().toCharUnitsFromBits(
                getContext().getTargetInfo().getPointerWidth(LangAS::Default))
          : CharUnits::Zero();

  if (Info.PathToIntroducingObject.empty()) {
    CGM.AddVTableTypeMetadata(VTable, AddressPoint, RD);
    return;
  }

  // Add a bitset entry for the least derived base belonging to this vftable.
  CGM.AddVTableTypeMetadata(VTable, AddressPoint,
                            Info.PathToIntroducingObject.back());

  // Add a bitset entry for each derived class that is laid out at the same
  // offset as the least derived base.
  for (unsigned I = Info.PathToIntroducingObject.size() - 1; I != 0; --I) {
    const CXXRecordDecl *DerivedRD = Info.PathToIntroducingObject[I - 1];
    const CXXRecordDecl *BaseRD = Info.PathToIntroducingObject[I];

    const ASTRecordLayout &Layout =
        getContext().getASTRecordLayout(DerivedRD);
    CharUnits Offset;
    auto VBI = Layout.getVBaseOffsetsMap().find(BaseRD);
    if (VBI == Layout.getVBaseOffsetsMap().end())
      Offset = Layout.getBaseClassOffset(BaseRD);
    else
      Offset = VBI->second.VBaseOffset;
    if (!Offset.isZero())
      return;
    CGM.AddVTableTypeMetadata(VTable, AddressPoint, DerivedRD);
  }

  // Finally do the same for the most derived class.
  if (Info.FullOffsetInMDC.isZero())
    CGM.AddVTableTypeMetadata(VTable, AddressPoint, RD);
}

void MicrosoftCXXABI::emitVTableDefinitions(CodeGenVTables &CGVT,
                                            const CXXRecordDecl *RD) {
  MicrosoftVTableContext &VFTContext = CGM.getMicrosoftVTableContext();
  const VPtrInfoVector &VFPtrs = VFTContext.getVFPtrOffsets(RD);

  for (const std::unique_ptr<VPtrInfo>& Info : VFPtrs) {
    toolchain::GlobalVariable *VTable = getAddrOfVTable(RD, Info->FullOffsetInMDC);
    if (VTable->hasInitializer())
      continue;

    const VTableLayout &VTLayout =
      VFTContext.getVFTableLayout(RD, Info->FullOffsetInMDC);

    toolchain::Constant *RTTI = nullptr;
    if (any_of(VTLayout.vtable_components(),
               [](const VTableComponent &VTC) { return VTC.isRTTIKind(); }))
      RTTI = getMSCompleteObjectLocator(RD, *Info);

    ConstantInitBuilder builder(CGM);
    auto components = builder.beginStruct();
    CGVT.createVTableInitializer(components, VTLayout, RTTI,
                                 VTable->hasLocalLinkage());
    components.finishAndSetAsInitializer(VTable);

    emitVTableTypeMetadata(*Info, RD, VTable);
  }
}

bool MicrosoftCXXABI::isVirtualOffsetNeededForVTableField(
    CodeGenFunction &CGF, CodeGenFunction::VPtr Vptr) {
  return Vptr.NearestVBase != nullptr;
}

toolchain::Value *MicrosoftCXXABI::getVTableAddressPointInStructor(
    CodeGenFunction &CGF, const CXXRecordDecl *VTableClass, BaseSubobject Base,
    const CXXRecordDecl *NearestVBase) {
  toolchain::Constant *VTableAddressPoint = getVTableAddressPoint(Base, VTableClass);
  if (!VTableAddressPoint) {
    assert(Base.getBase()->getNumVBases() &&
           !getContext().getASTRecordLayout(Base.getBase()).hasOwnVFPtr());
  }
  return VTableAddressPoint;
}

static void mangleVFTableName(MicrosoftMangleContext &MangleContext,
                              const CXXRecordDecl *RD, const VPtrInfo &VFPtr,
                              SmallString<256> &Name) {
  toolchain::raw_svector_ostream Out(Name);
  MangleContext.mangleCXXVFTable(RD, VFPtr.MangledPath, Out);
}

toolchain::Constant *
MicrosoftCXXABI::getVTableAddressPoint(BaseSubobject Base,
                                       const CXXRecordDecl *VTableClass) {
  (void)getAddrOfVTable(VTableClass, Base.getBaseOffset());
  VFTableIdTy ID(VTableClass, Base.getBaseOffset());
  return VFTablesMap[ID];
}

toolchain::GlobalVariable *MicrosoftCXXABI::getAddrOfVTable(const CXXRecordDecl *RD,
                                                       CharUnits VPtrOffset) {
  // getAddrOfVTable may return 0 if asked to get an address of a vtable which
  // shouldn't be used in the given record type. We want to cache this result in
  // VFTablesMap, thus a simple zero check is not sufficient.

  VFTableIdTy ID(RD, VPtrOffset);
  auto [I, Inserted] = VTablesMap.try_emplace(ID);
  if (!Inserted)
    return I->second;

  toolchain::GlobalVariable *&VTable = I->second;

  MicrosoftVTableContext &VTContext = CGM.getMicrosoftVTableContext();
  const VPtrInfoVector &VFPtrs = VTContext.getVFPtrOffsets(RD);

  if (DeferredVFTables.insert(RD).second) {
    // We haven't processed this record type before.
    // Queue up this vtable for possible deferred emission.
    CGM.addDeferredVTable(RD);

#ifndef NDEBUG
    // Create all the vftables at once in order to make sure each vftable has
    // a unique mangled name.
    toolchain::StringSet<> ObservedMangledNames;
    for (const auto &VFPtr : VFPtrs) {
      SmallString<256> Name;
      mangleVFTableName(getMangleContext(), RD, *VFPtr, Name);
      if (!ObservedMangledNames.insert(Name.str()).second)
        toolchain_unreachable("Already saw this mangling before?");
    }
#endif
  }

  const std::unique_ptr<VPtrInfo> *VFPtrI =
      toolchain::find_if(VFPtrs, [&](const std::unique_ptr<VPtrInfo> &VPI) {
        return VPI->FullOffsetInMDC == VPtrOffset;
      });
  if (VFPtrI == VFPtrs.end()) {
    VFTablesMap[ID] = nullptr;
    return nullptr;
  }
  const std::unique_ptr<VPtrInfo> &VFPtr = *VFPtrI;

  SmallString<256> VFTableName;
  mangleVFTableName(getMangleContext(), RD, *VFPtr, VFTableName);

  // Classes marked __declspec(dllimport) need vftables generated on the
  // import-side in order to support features like constexpr.  No other
  // translation unit relies on the emission of the local vftable, translation
  // units are expected to generate them as needed.
  //
  // Because of this unique behavior, we maintain this logic here instead of
  // getVTableLinkage.
  toolchain::GlobalValue::LinkageTypes VFTableLinkage =
      RD->hasAttr<DLLImportAttr>() ? toolchain::GlobalValue::LinkOnceODRLinkage
                                   : CGM.getVTableLinkage(RD);
  bool VFTableComesFromAnotherTU =
      toolchain::GlobalValue::isAvailableExternallyLinkage(VFTableLinkage) ||
      toolchain::GlobalValue::isExternalLinkage(VFTableLinkage);
  bool VTableAliasIsRequred =
      !VFTableComesFromAnotherTU && getContext().getLangOpts().RTTIData;

  if (toolchain::GlobalValue *VFTable =
          CGM.getModule().getNamedGlobal(VFTableName)) {
    VFTablesMap[ID] = VFTable;
    VTable = VTableAliasIsRequred
                 ? cast<toolchain::GlobalVariable>(
                       cast<toolchain::GlobalAlias>(VFTable)->getAliaseeObject())
                 : cast<toolchain::GlobalVariable>(VFTable);
    return VTable;
  }

  const VTableLayout &VTLayout =
      VTContext.getVFTableLayout(RD, VFPtr->FullOffsetInMDC);
  toolchain::GlobalValue::LinkageTypes VTableLinkage =
      VTableAliasIsRequred ? toolchain::GlobalValue::PrivateLinkage : VFTableLinkage;

  StringRef VTableName = VTableAliasIsRequred ? StringRef() : VFTableName.str();

  toolchain::Type *VTableType = CGM.getVTables().getVTableType(VTLayout);

  // Create a backing variable for the contents of VTable.  The VTable may
  // or may not include space for a pointer to RTTI data.
  toolchain::GlobalValue *VFTable;
  VTable = new toolchain::GlobalVariable(CGM.getModule(), VTableType,
                                    /*isConstant=*/true, VTableLinkage,
                                    /*Initializer=*/nullptr, VTableName);
  VTable->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);

  toolchain::Comdat *C = nullptr;
  if (!VFTableComesFromAnotherTU &&
      toolchain::GlobalValue::isWeakForLinker(VFTableLinkage))
    C = CGM.getModule().getOrInsertComdat(VFTableName.str());

  // Only insert a pointer into the VFTable for RTTI data if we are not
  // importing it.  We never reference the RTTI data directly so there is no
  // need to make room for it.
  if (VTableAliasIsRequred) {
    toolchain::Value *GEPIndices[] = {toolchain::ConstantInt::get(CGM.Int32Ty, 0),
                                 toolchain::ConstantInt::get(CGM.Int32Ty, 0),
                                 toolchain::ConstantInt::get(CGM.Int32Ty, 1)};
    // Create a GEP which points just after the first entry in the VFTable,
    // this should be the location of the first virtual method.
    toolchain::Constant *VTableGEP = toolchain::ConstantExpr::getInBoundsGetElementPtr(
        VTable->getValueType(), VTable, GEPIndices);
    if (toolchain::GlobalValue::isWeakForLinker(VFTableLinkage)) {
      VFTableLinkage = toolchain::GlobalValue::ExternalLinkage;
      if (C)
        C->setSelectionKind(toolchain::Comdat::Largest);
    }
    VFTable = toolchain::GlobalAlias::create(CGM.Int8PtrTy,
                                        /*AddressSpace=*/0, VFTableLinkage,
                                        VFTableName.str(), VTableGEP,
                                        &CGM.getModule());
    VFTable->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);
  } else {
    // We don't need a GlobalAlias to be a symbol for the VTable if we won't
    // be referencing any RTTI data.
    // The GlobalVariable will end up being an appropriate definition of the
    // VFTable.
    VFTable = VTable;
  }
  if (C)
    VTable->setComdat(C);

  if (RD->hasAttr<DLLExportAttr>())
    VFTable->setDLLStorageClass(toolchain::GlobalValue::DLLExportStorageClass);

  VFTablesMap[ID] = VFTable;
  return VTable;
}

CGCallee MicrosoftCXXABI::getVirtualFunctionPointer(CodeGenFunction &CGF,
                                                    GlobalDecl GD,
                                                    Address This,
                                                    toolchain::Type *Ty,
                                                    SourceLocation Loc) {
  CGBuilderTy &Builder = CGF.Builder;

  Ty = CGF.UnqualPtrTy;
  Address VPtr =
      adjustThisArgumentForVirtualFunctionCall(CGF, GD, This, true);

  auto *MethodDecl = cast<CXXMethodDecl>(GD.getDecl());
  toolchain::Value *VTable =
      CGF.GetVTablePtr(VPtr, CGF.UnqualPtrTy, MethodDecl->getParent());

  MicrosoftVTableContext &VFTContext = CGM.getMicrosoftVTableContext();
  MethodVFTableLocation ML = VFTContext.getMethodVFTableLocation(GD);

  // Compute the identity of the most derived class whose virtual table is
  // located at the MethodVFTableLocation ML.
  auto getObjectWithVPtr = [&] {
    return toolchain::find_if(VFTContext.getVFPtrOffsets(
                             ML.VBase ? ML.VBase : MethodDecl->getParent()),
                         [&](const std::unique_ptr<VPtrInfo> &Info) {
                           return Info->FullOffsetInMDC == ML.VFPtrOffset;
                         })
        ->get()
        ->ObjectWithVPtr;
  };

  toolchain::Value *VFunc;
  if (CGF.ShouldEmitVTableTypeCheckedLoad(MethodDecl->getParent())) {
    VFunc = CGF.EmitVTableTypeCheckedLoad(
        getObjectWithVPtr(), VTable, Ty,
        ML.Index *
            CGM.getContext().getTargetInfo().getPointerWidth(LangAS::Default) /
            8);
  } else {
    if (CGM.getCodeGenOpts().PrepareForLTO)
      CGF.EmitTypeMetadataCodeForVCall(getObjectWithVPtr(), VTable, Loc);

    toolchain::Value *VFuncPtr =
        Builder.CreateConstInBoundsGEP1_64(Ty, VTable, ML.Index, "vfn");
    VFunc = Builder.CreateAlignedLoad(Ty, VFuncPtr, CGF.getPointerAlign());
  }

  CGCallee Callee(GD, VFunc);
  return Callee;
}

toolchain::Value *MicrosoftCXXABI::EmitVirtualDestructorCall(
    CodeGenFunction &CGF, const CXXDestructorDecl *Dtor, CXXDtorType DtorType,
    Address This, DeleteOrMemberCallExpr E, toolchain::CallBase **CallOrInvoke) {
  auto *CE = dyn_cast<const CXXMemberCallExpr *>(E);
  auto *D = dyn_cast<const CXXDeleteExpr *>(E);
  assert((CE != nullptr) ^ (D != nullptr));
  assert(CE == nullptr || CE->arguments().empty());
  assert(DtorType == Dtor_Deleting || DtorType == Dtor_Complete);

  // We have only one destructor in the vftable but can get both behaviors
  // by passing an implicit int parameter.
  GlobalDecl GD(Dtor, Dtor_Deleting);
  const CGFunctionInfo *FInfo =
      &CGM.getTypes().arrangeCXXStructorDeclaration(GD);
  toolchain::FunctionType *Ty = CGF.CGM.getTypes().GetFunctionType(*FInfo);
  CGCallee Callee = CGCallee::forVirtual(CE, GD, This, Ty);

  ASTContext &Context = getContext();
  toolchain::Value *ImplicitParam = toolchain::ConstantInt::get(
      toolchain::IntegerType::getInt32Ty(CGF.getLLVMContext()),
      DtorType == Dtor_Deleting);

  QualType ThisTy;
  if (CE) {
    ThisTy = CE->getObjectType();
  } else {
    ThisTy = D->getDestroyedType();
  }

  This = adjustThisArgumentForVirtualFunctionCall(CGF, GD, This, true);
  RValue RV =
      CGF.EmitCXXDestructorCall(GD, Callee, This.emitRawPointer(CGF), ThisTy,
                                ImplicitParam, Context.IntTy, CE, CallOrInvoke);
  return RV.getScalarVal();
}

const VBTableGlobals &
MicrosoftCXXABI::enumerateVBTables(const CXXRecordDecl *RD) {
  // At this layer, we can key the cache off of a single class, which is much
  // easier than caching each vbtable individually.
  auto [Entry, Added] = VBTablesMap.try_emplace(RD);
  VBTableGlobals &VBGlobals = Entry->second;
  if (!Added)
    return VBGlobals;

  MicrosoftVTableContext &Context = CGM.getMicrosoftVTableContext();
  VBGlobals.VBTables = &Context.enumerateVBTables(RD);

  // Cache the globals for all vbtables so we don't have to recompute the
  // mangled names.
  toolchain::GlobalVariable::LinkageTypes Linkage = CGM.getVTableLinkage(RD);
  for (VPtrInfoVector::const_iterator I = VBGlobals.VBTables->begin(),
                                      E = VBGlobals.VBTables->end();
       I != E; ++I) {
    VBGlobals.Globals.push_back(getAddrOfVBTable(**I, RD, Linkage));
  }

  return VBGlobals;
}

toolchain::Function *
MicrosoftCXXABI::EmitVirtualMemPtrThunk(const CXXMethodDecl *MD,
                                        const MethodVFTableLocation &ML) {
  assert(!isa<CXXConstructorDecl>(MD) && !isa<CXXDestructorDecl>(MD) &&
         "can't form pointers to ctors or virtual dtors");

  // Calculate the mangled name.
  SmallString<256> ThunkName;
  toolchain::raw_svector_ostream Out(ThunkName);
  getMangleContext().mangleVirtualMemPtrThunk(MD, ML, Out);

  // If the thunk has been generated previously, just return it.
  if (toolchain::GlobalValue *GV = CGM.getModule().getNamedValue(ThunkName))
    return cast<toolchain::Function>(GV);

  // Create the toolchain::Function.
  const CGFunctionInfo &FnInfo =
      CGM.getTypes().arrangeUnprototypedMustTailThunk(MD);
  toolchain::FunctionType *ThunkTy = CGM.getTypes().GetFunctionType(FnInfo);
  toolchain::Function *ThunkFn =
      toolchain::Function::Create(ThunkTy, toolchain::Function::ExternalLinkage,
                             ThunkName.str(), &CGM.getModule());
  assert(ThunkFn->getName() == ThunkName && "name was uniqued!");

  ThunkFn->setLinkage(MD->isExternallyVisible()
                          ? toolchain::GlobalValue::LinkOnceODRLinkage
                          : toolchain::GlobalValue::InternalLinkage);
  if (MD->isExternallyVisible())
    ThunkFn->setComdat(CGM.getModule().getOrInsertComdat(ThunkFn->getName()));

  CGM.SetLLVMFunctionAttributes(MD, FnInfo, ThunkFn, /*IsThunk=*/false);
  CGM.SetLLVMFunctionAttributesForDefinition(MD, ThunkFn);

  // Add the "thunk" attribute so that LLVM knows that the return type is
  // meaningless. These thunks can be used to call functions with differing
  // return types, and the caller is required to cast the prototype
  // appropriately to extract the correct value.
  ThunkFn->addFnAttr("thunk");

  // These thunks can be compared, so they are not unnamed.
  ThunkFn->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::None);

  // Start codegen.
  CodeGenFunction CGF(CGM);
  CGF.CurGD = GlobalDecl(MD);
  CGF.CurFuncIsThunk = true;

  // Build FunctionArgs, but only include the implicit 'this' parameter
  // declaration.
  FunctionArgList FunctionArgs;
  buildThisParam(CGF, FunctionArgs);

  // Start defining the function.
  CGF.StartFunction(GlobalDecl(), FnInfo.getReturnType(), ThunkFn, FnInfo,
                    FunctionArgs, MD->getLocation(), SourceLocation());

  ApplyDebugLocation AL(CGF, MD->getLocation());
  setCXXABIThisValue(CGF, loadIncomingCXXThis(CGF));

  // Load the vfptr and then callee from the vftable.  The callee should have
  // adjusted 'this' so that the vfptr is at offset zero.
  toolchain::Type *ThunkPtrTy = CGF.UnqualPtrTy;
  toolchain::Value *VTable =
      CGF.GetVTablePtr(getThisAddress(CGF), CGF.UnqualPtrTy, MD->getParent());

  toolchain::Value *VFuncPtr = CGF.Builder.CreateConstInBoundsGEP1_64(
      ThunkPtrTy, VTable, ML.Index, "vfn");
  toolchain::Value *Callee =
    CGF.Builder.CreateAlignedLoad(ThunkPtrTy, VFuncPtr, CGF.getPointerAlign());

  CGF.EmitMustTailThunk(MD, getThisValue(CGF), {ThunkTy, Callee});

  return ThunkFn;
}

void MicrosoftCXXABI::emitVirtualInheritanceTables(const CXXRecordDecl *RD) {
  const VBTableGlobals &VBGlobals = enumerateVBTables(RD);
  for (unsigned I = 0, E = VBGlobals.VBTables->size(); I != E; ++I) {
    const std::unique_ptr<VPtrInfo>& VBT = (*VBGlobals.VBTables)[I];
    toolchain::GlobalVariable *GV = VBGlobals.Globals[I];
    if (GV->isDeclaration())
      emitVBTableDefinition(*VBT, RD, GV);
  }
}

toolchain::GlobalVariable *
MicrosoftCXXABI::getAddrOfVBTable(const VPtrInfo &VBT, const CXXRecordDecl *RD,
                                  toolchain::GlobalVariable::LinkageTypes Linkage) {
  SmallString<256> OutName;
  toolchain::raw_svector_ostream Out(OutName);
  getMangleContext().mangleCXXVBTable(RD, VBT.MangledPath, Out);
  StringRef Name = OutName.str();

  toolchain::ArrayType *VBTableType =
      toolchain::ArrayType::get(CGM.IntTy, 1 + VBT.ObjectWithVPtr->getNumVBases());

  assert(!CGM.getModule().getNamedGlobal(Name) &&
         "vbtable with this name already exists: mangling bug?");
  CharUnits Alignment =
      CGM.getContext().getTypeAlignInChars(CGM.getContext().IntTy);
  toolchain::GlobalVariable *GV = CGM.CreateOrReplaceCXXRuntimeVariable(
      Name, VBTableType, Linkage, Alignment.getAsAlign());
  GV->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);

  if (RD->hasAttr<DLLImportAttr>())
    GV->setDLLStorageClass(toolchain::GlobalValue::DLLImportStorageClass);
  else if (RD->hasAttr<DLLExportAttr>())
    GV->setDLLStorageClass(toolchain::GlobalValue::DLLExportStorageClass);

  if (!GV->hasExternalLinkage())
    emitVBTableDefinition(VBT, RD, GV);

  return GV;
}

void MicrosoftCXXABI::emitVBTableDefinition(const VPtrInfo &VBT,
                                            const CXXRecordDecl *RD,
                                            toolchain::GlobalVariable *GV) const {
  const CXXRecordDecl *ObjectWithVPtr = VBT.ObjectWithVPtr;

  assert(RD->getNumVBases() && ObjectWithVPtr->getNumVBases() &&
         "should only emit vbtables for classes with vbtables");

  const ASTRecordLayout &BaseLayout =
      getContext().getASTRecordLayout(VBT.IntroducingObject);
  const ASTRecordLayout &DerivedLayout = getContext().getASTRecordLayout(RD);

  SmallVector<toolchain::Constant *, 4> Offsets(1 + ObjectWithVPtr->getNumVBases(),
                                           nullptr);

  // The offset from ObjectWithVPtr's vbptr to itself always leads.
  CharUnits VBPtrOffset = BaseLayout.getVBPtrOffset();
  Offsets[0] = toolchain::ConstantInt::get(CGM.IntTy, -VBPtrOffset.getQuantity());

  MicrosoftVTableContext &Context = CGM.getMicrosoftVTableContext();
  for (const auto &I : ObjectWithVPtr->vbases()) {
    const CXXRecordDecl *VBase = I.getType()->getAsCXXRecordDecl();
    CharUnits Offset = DerivedLayout.getVBaseClassOffset(VBase);
    assert(!Offset.isNegative());

    // Make it relative to the subobject vbptr.
    CharUnits CompleteVBPtrOffset = VBT.NonVirtualOffset + VBPtrOffset;
    if (VBT.getVBaseWithVPtr())
      CompleteVBPtrOffset +=
          DerivedLayout.getVBaseClassOffset(VBT.getVBaseWithVPtr());
    Offset -= CompleteVBPtrOffset;

    unsigned VBIndex = Context.getVBTableIndex(ObjectWithVPtr, VBase);
    assert(Offsets[VBIndex] == nullptr && "The same vbindex seen twice?");
    Offsets[VBIndex] = toolchain::ConstantInt::get(CGM.IntTy, Offset.getQuantity());
  }

  assert(Offsets.size() ==
         cast<toolchain::ArrayType>(GV->getValueType())->getNumElements());
  toolchain::ArrayType *VBTableType =
    toolchain::ArrayType::get(CGM.IntTy, Offsets.size());
  toolchain::Constant *Init = toolchain::ConstantArray::get(VBTableType, Offsets);
  GV->setInitializer(Init);

  if (RD->hasAttr<DLLImportAttr>())
    GV->setLinkage(toolchain::GlobalVariable::AvailableExternallyLinkage);
}

toolchain::Value *MicrosoftCXXABI::performThisAdjustment(
    CodeGenFunction &CGF, Address This,
    const CXXRecordDecl * /*UnadjustedClass*/, const ThunkInfo &TI) {
  const ThisAdjustment &TA = TI.This;
  if (TA.isEmpty())
    return This.emitRawPointer(CGF);

  This = This.withElementType(CGF.Int8Ty);

  toolchain::Value *V;
  if (TA.Virtual.isEmpty()) {
    V = This.emitRawPointer(CGF);
  } else {
    assert(TA.Virtual.Microsoft.VtordispOffset < 0);
    // Adjust the this argument based on the vtordisp value.
    Address VtorDispPtr =
        CGF.Builder.CreateConstInBoundsByteGEP(This,
                 CharUnits::fromQuantity(TA.Virtual.Microsoft.VtordispOffset));
    VtorDispPtr = VtorDispPtr.withElementType(CGF.Int32Ty);
    toolchain::Value *VtorDisp = CGF.Builder.CreateLoad(VtorDispPtr, "vtordisp");
    V = CGF.Builder.CreateGEP(This.getElementType(), This.emitRawPointer(CGF),
                              CGF.Builder.CreateNeg(VtorDisp));

    // Unfortunately, having applied the vtordisp means that we no
    // longer really have a known alignment for the vbptr step.
    // We'll assume the vbptr is pointer-aligned.

    if (TA.Virtual.Microsoft.VBPtrOffset) {
      // If the final overrider is defined in a virtual base other than the one
      // that holds the vfptr, we have to use a vtordispex thunk which looks up
      // the vbtable of the derived class.
      assert(TA.Virtual.Microsoft.VBPtrOffset > 0);
      assert(TA.Virtual.Microsoft.VBOffsetOffset >= 0);
      toolchain::Value *VBPtr;
      toolchain::Value *VBaseOffset = GetVBaseOffsetFromVBPtr(
          CGF, Address(V, CGF.Int8Ty, CGF.getPointerAlign()),
          -TA.Virtual.Microsoft.VBPtrOffset,
          TA.Virtual.Microsoft.VBOffsetOffset, &VBPtr);
      V = CGF.Builder.CreateInBoundsGEP(CGF.Int8Ty, VBPtr, VBaseOffset);
    }
  }

  if (TA.NonVirtual) {
    // Non-virtual adjustment might result in a pointer outside the allocated
    // object, e.g. if the final overrider class is laid out after the virtual
    // base that declares a method in the most derived class.
    V = CGF.Builder.CreateConstGEP1_32(CGF.Int8Ty, V, TA.NonVirtual);
  }

  // Don't need to bitcast back, the call CodeGen will handle this.
  return V;
}

toolchain::Value *MicrosoftCXXABI::performReturnAdjustment(
    CodeGenFunction &CGF, Address Ret,
    const CXXRecordDecl * /*UnadjustedClass*/, const ReturnAdjustment &RA) {

  if (RA.isEmpty())
    return Ret.emitRawPointer(CGF);

  Ret = Ret.withElementType(CGF.Int8Ty);

  toolchain::Value *V = Ret.emitRawPointer(CGF);
  if (RA.Virtual.Microsoft.VBIndex) {
    assert(RA.Virtual.Microsoft.VBIndex > 0);
    int32_t IntSize = CGF.getIntSize().getQuantity();
    toolchain::Value *VBPtr;
    toolchain::Value *VBaseOffset =
        GetVBaseOffsetFromVBPtr(CGF, Ret, RA.Virtual.Microsoft.VBPtrOffset,
                                IntSize * RA.Virtual.Microsoft.VBIndex, &VBPtr);
    V = CGF.Builder.CreateInBoundsGEP(CGF.Int8Ty, VBPtr, VBaseOffset);
  }

  if (RA.NonVirtual)
    V = CGF.Builder.CreateConstInBoundsGEP1_32(CGF.Int8Ty, V, RA.NonVirtual);

  return V;
}

bool MicrosoftCXXABI::requiresArrayCookie(const CXXDeleteExpr *expr,
                                   QualType elementType) {
  // Microsoft seems to completely ignore the possibility of a
  // two-argument usual deallocation function.
  return elementType.isDestructedType();
}

bool MicrosoftCXXABI::requiresArrayCookie(const CXXNewExpr *expr) {
  // Microsoft seems to completely ignore the possibility of a
  // two-argument usual deallocation function.
  return expr->getAllocatedType().isDestructedType();
}

CharUnits MicrosoftCXXABI::getArrayCookieSizeImpl(QualType type) {
  // The array cookie is always a size_t; we then pad that out to the
  // alignment of the element type.
  ASTContext &Ctx = getContext();
  return std::max(Ctx.getTypeSizeInChars(Ctx.getSizeType()),
                  Ctx.getTypeAlignInChars(type));
}

toolchain::Value *MicrosoftCXXABI::readArrayCookieImpl(CodeGenFunction &CGF,
                                                  Address allocPtr,
                                                  CharUnits cookieSize) {
  Address numElementsPtr = allocPtr.withElementType(CGF.SizeTy);
  return CGF.Builder.CreateLoad(numElementsPtr);
}

Address MicrosoftCXXABI::InitializeArrayCookie(CodeGenFunction &CGF,
                                               Address newPtr,
                                               toolchain::Value *numElements,
                                               const CXXNewExpr *expr,
                                               QualType elementType) {
  assert(requiresArrayCookie(expr));

  // The size of the cookie.
  CharUnits cookieSize = getArrayCookieSizeImpl(elementType);

  // Compute an offset to the cookie.
  Address cookiePtr = newPtr;

  // Write the number of elements into the appropriate slot.
  Address numElementsPtr = cookiePtr.withElementType(CGF.SizeTy);
  CGF.Builder.CreateStore(numElements, numElementsPtr);

  // Finally, compute a pointer to the actual data buffer by skipping
  // over the cookie completely.
  return CGF.Builder.CreateConstInBoundsByteGEP(newPtr, cookieSize);
}

static void emitGlobalDtorWithTLRegDtor(CodeGenFunction &CGF, const VarDecl &VD,
                                        toolchain::FunctionCallee Dtor,
                                        toolchain::Constant *Addr) {
  // Create a function which calls the destructor.
  toolchain::Constant *DtorStub = CGF.createAtExitStub(VD, Dtor, Addr);

  // extern "C" int __tlregdtor(void (*f)(void));
  toolchain::FunctionType *TLRegDtorTy = toolchain::FunctionType::get(
      CGF.IntTy, DtorStub->getType(), /*isVarArg=*/false);

  toolchain::FunctionCallee TLRegDtor = CGF.CGM.CreateRuntimeFunction(
      TLRegDtorTy, "__tlregdtor", toolchain::AttributeList(), /*Local=*/true);
  if (toolchain::Function *TLRegDtorFn =
          dyn_cast<toolchain::Function>(TLRegDtor.getCallee()))
    TLRegDtorFn->setDoesNotThrow();

  CGF.EmitNounwindRuntimeCall(TLRegDtor, DtorStub);
}

void MicrosoftCXXABI::registerGlobalDtor(CodeGenFunction &CGF, const VarDecl &D,
                                         toolchain::FunctionCallee Dtor,
                                         toolchain::Constant *Addr) {
  if (D.isNoDestroy(CGM.getContext()))
    return;

  if (D.getTLSKind())
    return emitGlobalDtorWithTLRegDtor(CGF, D, Dtor, Addr);

  // HLSL doesn't support atexit.
  if (CGM.getLangOpts().HLSL)
    return CGM.AddCXXDtorEntry(Dtor, Addr);

  // The default behavior is to use atexit.
  CGF.registerGlobalDtorWithAtExit(D, Dtor, Addr);
}

void MicrosoftCXXABI::EmitThreadLocalInitFuncs(
    CodeGenModule &CGM, ArrayRef<const VarDecl *> CXXThreadLocals,
    ArrayRef<toolchain::Function *> CXXThreadLocalInits,
    ArrayRef<const VarDecl *> CXXThreadLocalInitVars) {
  if (CXXThreadLocalInits.empty())
    return;

  CGM.AppendLinkerOptions(CGM.getTarget().getTriple().getArch() ==
                                  toolchain::Triple::x86
                              ? "/include:___dyn_tls_init@12"
                              : "/include:__dyn_tls_init");

  // This will create a GV in the .CRT$XDU section.  It will point to our
  // initialization function.  The CRT will call all of these function
  // pointers at start-up time and, eventually, at thread-creation time.
  auto AddToXDU = [&CGM](toolchain::Function *InitFunc) {
    toolchain::GlobalVariable *InitFuncPtr = new toolchain::GlobalVariable(
        CGM.getModule(), InitFunc->getType(), /*isConstant=*/true,
        toolchain::GlobalVariable::InternalLinkage, InitFunc,
        Twine(InitFunc->getName(), "$initializer$"));
    InitFuncPtr->setSection(".CRT$XDU");
    // This variable has discardable linkage, we have to add it to @toolchain.used to
    // ensure it won't get discarded.
    CGM.addUsedGlobal(InitFuncPtr);
    return InitFuncPtr;
  };

  std::vector<toolchain::Function *> NonComdatInits;
  for (size_t I = 0, E = CXXThreadLocalInitVars.size(); I != E; ++I) {
    toolchain::GlobalVariable *GV = cast<toolchain::GlobalVariable>(
        CGM.GetGlobalValue(CGM.getMangledName(CXXThreadLocalInitVars[I])));
    toolchain::Function *F = CXXThreadLocalInits[I];

    // If the GV is already in a comdat group, then we have to join it.
    if (toolchain::Comdat *C = GV->getComdat())
      AddToXDU(F)->setComdat(C);
    else
      NonComdatInits.push_back(F);
  }

  if (!NonComdatInits.empty()) {
    toolchain::FunctionType *FTy =
        toolchain::FunctionType::get(CGM.VoidTy, /*isVarArg=*/false);
    toolchain::Function *InitFunc = CGM.CreateGlobalInitOrCleanUpFunction(
        FTy, "__tls_init", CGM.getTypes().arrangeNullaryFunction(),
        SourceLocation(), /*TLS=*/true);
    CodeGenFunction(CGM).GenerateCXXGlobalInitFunc(InitFunc, NonComdatInits);

    AddToXDU(InitFunc);
  }
}

static toolchain::GlobalValue *getTlsGuardVar(CodeGenModule &CGM) {
  // __tls_guard comes from the MSVC runtime and reflects
  // whether TLS has been initialized for a particular thread.
  // It is set from within __dyn_tls_init by the runtime.
  // Every library and executable has its own variable.
  toolchain::Type *VTy = toolchain::Type::getInt8Ty(CGM.getLLVMContext());
  toolchain::Constant *TlsGuardConstant =
      CGM.CreateRuntimeVariable(VTy, "__tls_guard");
  toolchain::GlobalValue *TlsGuard = cast<toolchain::GlobalValue>(TlsGuardConstant);

  TlsGuard->setThreadLocal(true);

  return TlsGuard;
}

static toolchain::FunctionCallee getDynTlsOnDemandInitFn(CodeGenModule &CGM) {
  // __dyn_tls_on_demand_init comes from the MSVC runtime and triggers
  // dynamic TLS initialization by calling __dyn_tls_init internally.
  toolchain::FunctionType *FTy =
      toolchain::FunctionType::get(toolchain::Type::getVoidTy(CGM.getLLVMContext()), {},
                              /*isVarArg=*/false);
  return CGM.CreateRuntimeFunction(
      FTy, "__dyn_tls_on_demand_init",
      toolchain::AttributeList::get(CGM.getLLVMContext(),
                               toolchain::AttributeList::FunctionIndex,
                               toolchain::Attribute::NoUnwind),
      /*Local=*/true);
}

static void emitTlsGuardCheck(CodeGenFunction &CGF, toolchain::GlobalValue *TlsGuard,
                              toolchain::BasicBlock *DynInitBB,
                              toolchain::BasicBlock *ContinueBB) {
  toolchain::LoadInst *TlsGuardValue =
      CGF.Builder.CreateLoad(Address(TlsGuard, CGF.Int8Ty, CharUnits::One()));
  toolchain::Value *CmpResult =
      CGF.Builder.CreateICmpEQ(TlsGuardValue, CGF.Builder.getInt8(0));
  CGF.Builder.CreateCondBr(CmpResult, DynInitBB, ContinueBB);
}

static void emitDynamicTlsInitializationCall(CodeGenFunction &CGF,
                                             toolchain::GlobalValue *TlsGuard,
                                             toolchain::BasicBlock *ContinueBB) {
  toolchain::FunctionCallee Initializer = getDynTlsOnDemandInitFn(CGF.CGM);
  toolchain::Function *InitializerFunction =
      cast<toolchain::Function>(Initializer.getCallee());
  toolchain::CallInst *CallVal = CGF.Builder.CreateCall(InitializerFunction);
  CallVal->setCallingConv(InitializerFunction->getCallingConv());

  CGF.Builder.CreateBr(ContinueBB);
}

static void emitDynamicTlsInitialization(CodeGenFunction &CGF) {
  toolchain::BasicBlock *DynInitBB =
      CGF.createBasicBlock("dyntls.dyn_init", CGF.CurFn);
  toolchain::BasicBlock *ContinueBB =
      CGF.createBasicBlock("dyntls.continue", CGF.CurFn);

  toolchain::GlobalValue *TlsGuard = getTlsGuardVar(CGF.CGM);

  emitTlsGuardCheck(CGF, TlsGuard, DynInitBB, ContinueBB);
  CGF.Builder.SetInsertPoint(DynInitBB);
  emitDynamicTlsInitializationCall(CGF, TlsGuard, ContinueBB);
  CGF.Builder.SetInsertPoint(ContinueBB);
}

LValue MicrosoftCXXABI::EmitThreadLocalVarDeclLValue(CodeGenFunction &CGF,
                                                     const VarDecl *VD,
                                                     QualType LValType) {
  // Dynamic TLS initialization works by checking the state of a
  // guard variable (__tls_guard) to see whether TLS initialization
  // for a thread has happend yet.
  // If not, the initialization is triggered on-demand
  // by calling __dyn_tls_on_demand_init.
  emitDynamicTlsInitialization(CGF);

  // Emit the variable just like any regular global variable.

  toolchain::Value *V = CGF.CGM.GetAddrOfGlobalVar(VD);
  toolchain::Type *RealVarTy = CGF.getTypes().ConvertTypeForMem(VD->getType());

  CharUnits Alignment = CGF.getContext().getDeclAlign(VD);
  Address Addr(V, RealVarTy, Alignment);

  LValue LV = VD->getType()->isReferenceType()
                  ? CGF.EmitLoadOfReferenceLValue(Addr, VD->getType(),
                                                  AlignmentSource::Decl)
                  : CGF.MakeAddrLValue(Addr, LValType, AlignmentSource::Decl);
  return LV;
}

static ConstantAddress getInitThreadEpochPtr(CodeGenModule &CGM) {
  StringRef VarName("_Init_thread_epoch");
  CharUnits Align = CGM.getIntAlign();
  if (auto *GV = CGM.getModule().getNamedGlobal(VarName))
    return ConstantAddress(GV, GV->getValueType(), Align);
  auto *GV = new toolchain::GlobalVariable(
      CGM.getModule(), CGM.IntTy,
      /*isConstant=*/false, toolchain::GlobalVariable::ExternalLinkage,
      /*Initializer=*/nullptr, VarName,
      /*InsertBefore=*/nullptr, toolchain::GlobalVariable::GeneralDynamicTLSModel);
  GV->setAlignment(Align.getAsAlign());
  return ConstantAddress(GV, GV->getValueType(), Align);
}

static toolchain::FunctionCallee getInitThreadHeaderFn(CodeGenModule &CGM) {
  toolchain::FunctionType *FTy =
      toolchain::FunctionType::get(toolchain::Type::getVoidTy(CGM.getLLVMContext()),
                              CGM.UnqualPtrTy, /*isVarArg=*/false);
  return CGM.CreateRuntimeFunction(
      FTy, "_Init_thread_header",
      toolchain::AttributeList::get(CGM.getLLVMContext(),
                               toolchain::AttributeList::FunctionIndex,
                               toolchain::Attribute::NoUnwind),
      /*Local=*/true);
}

static toolchain::FunctionCallee getInitThreadFooterFn(CodeGenModule &CGM) {
  toolchain::FunctionType *FTy =
      toolchain::FunctionType::get(toolchain::Type::getVoidTy(CGM.getLLVMContext()),
                              CGM.UnqualPtrTy, /*isVarArg=*/false);
  return CGM.CreateRuntimeFunction(
      FTy, "_Init_thread_footer",
      toolchain::AttributeList::get(CGM.getLLVMContext(),
                               toolchain::AttributeList::FunctionIndex,
                               toolchain::Attribute::NoUnwind),
      /*Local=*/true);
}

static toolchain::FunctionCallee getInitThreadAbortFn(CodeGenModule &CGM) {
  toolchain::FunctionType *FTy =
      toolchain::FunctionType::get(toolchain::Type::getVoidTy(CGM.getLLVMContext()),
                              CGM.UnqualPtrTy, /*isVarArg=*/false);
  return CGM.CreateRuntimeFunction(
      FTy, "_Init_thread_abort",
      toolchain::AttributeList::get(CGM.getLLVMContext(),
                               toolchain::AttributeList::FunctionIndex,
                               toolchain::Attribute::NoUnwind),
      /*Local=*/true);
}

namespace {
struct ResetGuardBit final : EHScopeStack::Cleanup {
  Address Guard;
  unsigned GuardNum;
  ResetGuardBit(Address Guard, unsigned GuardNum)
      : Guard(Guard), GuardNum(GuardNum) {}

  void Emit(CodeGenFunction &CGF, Flags flags) override {
    // Reset the bit in the mask so that the static variable may be
    // reinitialized.
    CGBuilderTy &Builder = CGF.Builder;
    toolchain::LoadInst *LI = Builder.CreateLoad(Guard);
    toolchain::ConstantInt *Mask =
        toolchain::ConstantInt::get(CGF.IntTy, ~(1ULL << GuardNum));
    Builder.CreateStore(Builder.CreateAnd(LI, Mask), Guard);
  }
};

struct CallInitThreadAbort final : EHScopeStack::Cleanup {
  toolchain::Value *Guard;
  CallInitThreadAbort(RawAddress Guard) : Guard(Guard.getPointer()) {}

  void Emit(CodeGenFunction &CGF, Flags flags) override {
    // Calling _Init_thread_abort will reset the guard's state.
    CGF.EmitNounwindRuntimeCall(getInitThreadAbortFn(CGF.CGM), Guard);
  }
};
}

void MicrosoftCXXABI::EmitGuardedInit(CodeGenFunction &CGF, const VarDecl &D,
                                      toolchain::GlobalVariable *GV,
                                      bool PerformInit) {
  // MSVC only uses guards for static locals.
  if (!D.isStaticLocal()) {
    assert(GV->hasWeakLinkage() || GV->hasLinkOnceLinkage());
    // GlobalOpt is allowed to discard the initializer, so use linkonce_odr.
    toolchain::Function *F = CGF.CurFn;
    F->setLinkage(toolchain::GlobalValue::LinkOnceODRLinkage);
    F->setComdat(CGM.getModule().getOrInsertComdat(F->getName()));
    CGF.EmitCXXGlobalVarDeclInit(D, GV, PerformInit);
    return;
  }

  bool ThreadlocalStatic = D.getTLSKind();
  bool ThreadsafeStatic = getContext().getLangOpts().ThreadsafeStatics;

  // Thread-safe static variables which aren't thread-specific have a
  // per-variable guard.
  bool HasPerVariableGuard = ThreadsafeStatic && !ThreadlocalStatic;

  CGBuilderTy &Builder = CGF.Builder;
  toolchain::IntegerType *GuardTy = CGF.Int32Ty;
  toolchain::ConstantInt *Zero = toolchain::ConstantInt::get(GuardTy, 0);
  CharUnits GuardAlign = CharUnits::fromQuantity(4);

  // Get the guard variable for this function if we have one already.
  GuardInfo *GI = nullptr;
  if (ThreadlocalStatic)
    GI = &ThreadLocalGuardVariableMap[D.getDeclContext()];
  else if (!ThreadsafeStatic)
    GI = &GuardVariableMap[D.getDeclContext()];

  toolchain::GlobalVariable *GuardVar = GI ? GI->Guard : nullptr;
  unsigned GuardNum;
  if (D.isExternallyVisible()) {
    // Externally visible variables have to be numbered in Sema to properly
    // handle unreachable VarDecls.
    GuardNum = getContext().getStaticLocalNumber(&D);
    assert(GuardNum > 0);
    GuardNum--;
  } else if (HasPerVariableGuard) {
    GuardNum = ThreadSafeGuardNumMap[D.getDeclContext()]++;
  } else {
    // Non-externally visible variables are numbered here in CodeGen.
    GuardNum = GI->BitIndex++;
  }

  if (!HasPerVariableGuard && GuardNum >= 32) {
    if (D.isExternallyVisible())
      ErrorUnsupportedABI(CGF, "more than 32 guarded initializations");
    GuardNum %= 32;
    GuardVar = nullptr;
  }

  if (!GuardVar) {
    // Mangle the name for the guard.
    SmallString<256> GuardName;
    {
      toolchain::raw_svector_ostream Out(GuardName);
      if (HasPerVariableGuard)
        getMangleContext().mangleThreadSafeStaticGuardVariable(&D, GuardNum,
                                                               Out);
      else
        getMangleContext().mangleStaticGuardVariable(&D, Out);
    }

    // Create the guard variable with a zero-initializer. Just absorb linkage,
    // visibility and dll storage class from the guarded variable.
    GuardVar =
        new toolchain::GlobalVariable(CGM.getModule(), GuardTy, /*isConstant=*/false,
                                 GV->getLinkage(), Zero, GuardName.str());
    GuardVar->setVisibility(GV->getVisibility());
    GuardVar->setDLLStorageClass(GV->getDLLStorageClass());
    GuardVar->setAlignment(GuardAlign.getAsAlign());
    if (GuardVar->isWeakForLinker())
      GuardVar->setComdat(
          CGM.getModule().getOrInsertComdat(GuardVar->getName()));
    if (D.getTLSKind())
      CGM.setTLSMode(GuardVar, D);
    if (GI && !HasPerVariableGuard)
      GI->Guard = GuardVar;
  }

  ConstantAddress GuardAddr(GuardVar, GuardTy, GuardAlign);

  assert(GuardVar->getLinkage() == GV->getLinkage() &&
         "static local from the same function had different linkage");

  if (!HasPerVariableGuard) {
    // Pseudo code for the test:
    // if (!(GuardVar & MyGuardBit)) {
    //   GuardVar |= MyGuardBit;
    //   ... initialize the object ...;
    // }

    // Test our bit from the guard variable.
    toolchain::ConstantInt *Bit = toolchain::ConstantInt::get(GuardTy, 1ULL << GuardNum);
    toolchain::LoadInst *LI = Builder.CreateLoad(GuardAddr);
    toolchain::Value *NeedsInit =
        Builder.CreateICmpEQ(Builder.CreateAnd(LI, Bit), Zero);
    toolchain::BasicBlock *InitBlock = CGF.createBasicBlock("init");
    toolchain::BasicBlock *EndBlock = CGF.createBasicBlock("init.end");
    CGF.EmitCXXGuardedInitBranch(NeedsInit, InitBlock, EndBlock,
                                 CodeGenFunction::GuardKind::VariableGuard, &D);

    // Set our bit in the guard variable and emit the initializer and add a global
    // destructor if appropriate.
    CGF.EmitBlock(InitBlock);
    Builder.CreateStore(Builder.CreateOr(LI, Bit), GuardAddr);
    CGF.EHStack.pushCleanup<ResetGuardBit>(EHCleanup, GuardAddr, GuardNum);
    CGF.EmitCXXGlobalVarDeclInit(D, GV, PerformInit);
    CGF.PopCleanupBlock();
    Builder.CreateBr(EndBlock);

    // Continue.
    CGF.EmitBlock(EndBlock);
  } else {
    // Pseudo code for the test:
    // if (TSS > _Init_thread_epoch) {
    //   _Init_thread_header(&TSS);
    //   if (TSS == -1) {
    //     ... initialize the object ...;
    //     _Init_thread_footer(&TSS);
    //   }
    // }
    //
    // The algorithm is almost identical to what can be found in the appendix
    // found in N2325.

    // This BasicBLock determines whether or not we have any work to do.
    toolchain::LoadInst *FirstGuardLoad = Builder.CreateLoad(GuardAddr);
    FirstGuardLoad->setOrdering(toolchain::AtomicOrdering::Unordered);
    toolchain::LoadInst *InitThreadEpoch =
        Builder.CreateLoad(getInitThreadEpochPtr(CGM));
    toolchain::Value *IsUninitialized =
        Builder.CreateICmpSGT(FirstGuardLoad, InitThreadEpoch);
    toolchain::BasicBlock *AttemptInitBlock = CGF.createBasicBlock("init.attempt");
    toolchain::BasicBlock *EndBlock = CGF.createBasicBlock("init.end");
    CGF.EmitCXXGuardedInitBranch(IsUninitialized, AttemptInitBlock, EndBlock,
                                 CodeGenFunction::GuardKind::VariableGuard, &D);

    // This BasicBlock attempts to determine whether or not this thread is
    // responsible for doing the initialization.
    CGF.EmitBlock(AttemptInitBlock);
    CGF.EmitNounwindRuntimeCall(getInitThreadHeaderFn(CGM),
                                GuardAddr.getPointer());
    toolchain::LoadInst *SecondGuardLoad = Builder.CreateLoad(GuardAddr);
    SecondGuardLoad->setOrdering(toolchain::AtomicOrdering::Unordered);
    toolchain::Value *ShouldDoInit =
        Builder.CreateICmpEQ(SecondGuardLoad, getAllOnesInt());
    toolchain::BasicBlock *InitBlock = CGF.createBasicBlock("init");
    Builder.CreateCondBr(ShouldDoInit, InitBlock, EndBlock);

    // Ok, we ended up getting selected as the initializing thread.
    CGF.EmitBlock(InitBlock);
    CGF.EHStack.pushCleanup<CallInitThreadAbort>(EHCleanup, GuardAddr);
    CGF.EmitCXXGlobalVarDeclInit(D, GV, PerformInit);
    CGF.PopCleanupBlock();
    CGF.EmitNounwindRuntimeCall(getInitThreadFooterFn(CGM),
                                GuardAddr.getPointer());
    Builder.CreateBr(EndBlock);

    CGF.EmitBlock(EndBlock);
  }
}

bool MicrosoftCXXABI::isZeroInitializable(const MemberPointerType *MPT) {
  // Null-ness for function memptrs only depends on the first field, which is
  // the function pointer.  The rest don't matter, so we can zero initialize.
  if (MPT->isMemberFunctionPointer())
    return true;

  // The virtual base adjustment field is always -1 for null, so if we have one
  // we can't zero initialize.  The field offset is sometimes also -1 if 0 is a
  // valid field offset.
  const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();
  return (!inheritanceModelHasVBTableOffsetField(Inheritance) &&
          RD->nullFieldOffsetIsZero());
}

toolchain::Type *
MicrosoftCXXABI::ConvertMemberPointerType(const MemberPointerType *MPT) {
  const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();
  toolchain::SmallVector<toolchain::Type *, 4> fields;
  if (MPT->isMemberFunctionPointer())
    fields.push_back(CGM.VoidPtrTy);  // FunctionPointerOrVirtualThunk
  else
    fields.push_back(CGM.IntTy);  // FieldOffset

  if (inheritanceModelHasNVOffsetField(MPT->isMemberFunctionPointer(),
                                       Inheritance))
    fields.push_back(CGM.IntTy);
  if (inheritanceModelHasVBPtrOffsetField(Inheritance))
    fields.push_back(CGM.IntTy);
  if (inheritanceModelHasVBTableOffsetField(Inheritance))
    fields.push_back(CGM.IntTy);  // VirtualBaseAdjustmentOffset

  if (fields.size() == 1)
    return fields[0];
  return toolchain::StructType::get(CGM.getLLVMContext(), fields);
}

void MicrosoftCXXABI::
GetNullMemberPointerFields(const MemberPointerType *MPT,
                           toolchain::SmallVectorImpl<toolchain::Constant *> &fields) {
  assert(fields.empty());
  const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();
  if (MPT->isMemberFunctionPointer()) {
    // FunctionPointerOrVirtualThunk
    fields.push_back(toolchain::Constant::getNullValue(CGM.VoidPtrTy));
  } else {
    if (RD->nullFieldOffsetIsZero())
      fields.push_back(getZeroInt());  // FieldOffset
    else
      fields.push_back(getAllOnesInt());  // FieldOffset
  }

  if (inheritanceModelHasNVOffsetField(MPT->isMemberFunctionPointer(),
                                       Inheritance))
    fields.push_back(getZeroInt());
  if (inheritanceModelHasVBPtrOffsetField(Inheritance))
    fields.push_back(getZeroInt());
  if (inheritanceModelHasVBTableOffsetField(Inheritance))
    fields.push_back(getAllOnesInt());
}

toolchain::Constant *
MicrosoftCXXABI::EmitNullMemberPointer(const MemberPointerType *MPT) {
  toolchain::SmallVector<toolchain::Constant *, 4> fields;
  GetNullMemberPointerFields(MPT, fields);
  if (fields.size() == 1)
    return fields[0];
  toolchain::Constant *Res = toolchain::ConstantStruct::getAnon(fields);
  assert(Res->getType() == ConvertMemberPointerType(MPT));
  return Res;
}

toolchain::Constant *
MicrosoftCXXABI::EmitFullMemberPointer(toolchain::Constant *FirstField,
                                       bool IsMemberFunction,
                                       const CXXRecordDecl *RD,
                                       CharUnits NonVirtualBaseAdjustment,
                                       unsigned VBTableIndex) {
  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();

  // Single inheritance class member pointer are represented as scalars instead
  // of aggregates.
  if (inheritanceModelHasOnlyOneField(IsMemberFunction, Inheritance))
    return FirstField;

  toolchain::SmallVector<toolchain::Constant *, 4> fields;
  fields.push_back(FirstField);

  if (inheritanceModelHasNVOffsetField(IsMemberFunction, Inheritance))
    fields.push_back(toolchain::ConstantInt::get(
      CGM.IntTy, NonVirtualBaseAdjustment.getQuantity()));

  if (inheritanceModelHasVBPtrOffsetField(Inheritance)) {
    CharUnits Offs = CharUnits::Zero();
    if (VBTableIndex)
      Offs = getContext().getASTRecordLayout(RD).getVBPtrOffset();
    fields.push_back(toolchain::ConstantInt::get(CGM.IntTy, Offs.getQuantity()));
  }

  // The rest of the fields are adjusted by conversions to a more derived class.
  if (inheritanceModelHasVBTableOffsetField(Inheritance))
    fields.push_back(toolchain::ConstantInt::get(CGM.IntTy, VBTableIndex));

  return toolchain::ConstantStruct::getAnon(fields);
}

toolchain::Constant *
MicrosoftCXXABI::EmitMemberDataPointer(const MemberPointerType *MPT,
                                       CharUnits offset) {
  return EmitMemberDataPointer(MPT->getMostRecentCXXRecordDecl(), offset);
}

toolchain::Constant *MicrosoftCXXABI::EmitMemberDataPointer(const CXXRecordDecl *RD,
                                                       CharUnits offset) {
  if (RD->getMSInheritanceModel() ==
      MSInheritanceModel::Virtual)
    offset -= getContext().getOffsetOfBaseWithVBPtr(RD);
  toolchain::Constant *FirstField =
    toolchain::ConstantInt::get(CGM.IntTy, offset.getQuantity());
  return EmitFullMemberPointer(FirstField, /*IsMemberFunction=*/false, RD,
                               CharUnits::Zero(), /*VBTableIndex=*/0);
}

toolchain::Constant *MicrosoftCXXABI::EmitMemberPointer(const APValue &MP,
                                                   QualType MPType) {
  const MemberPointerType *DstTy = MPType->castAs<MemberPointerType>();
  const ValueDecl *MPD = MP.getMemberPointerDecl();
  if (!MPD)
    return EmitNullMemberPointer(DstTy);

  ASTContext &Ctx = getContext();
  ArrayRef<const CXXRecordDecl *> MemberPointerPath = MP.getMemberPointerPath();

  toolchain::Constant *C;
  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(MPD)) {
    C = EmitMemberFunctionPointer(MD);
  } else {
    // For a pointer to data member, start off with the offset of the field in
    // the class in which it was declared, and convert from there if necessary.
    // For indirect field decls, get the outermost anonymous field and use the
    // parent class.
    CharUnits FieldOffset = Ctx.toCharUnitsFromBits(Ctx.getFieldOffset(MPD));
    const FieldDecl *FD = dyn_cast<FieldDecl>(MPD);
    if (!FD)
      FD = cast<FieldDecl>(*cast<IndirectFieldDecl>(MPD)->chain_begin());
    const CXXRecordDecl *RD = cast<CXXRecordDecl>(FD->getParent());
    RD = RD->getMostRecentDecl();
    C = EmitMemberDataPointer(RD, FieldOffset);
  }

  if (!MemberPointerPath.empty()) {
    const CXXRecordDecl *SrcRD = cast<CXXRecordDecl>(MPD->getDeclContext());
    const MemberPointerType *SrcTy =
        Ctx.getMemberPointerType(DstTy->getPointeeType(),
                                 /*Qualifier=*/std::nullopt, SrcRD)
            ->castAs<MemberPointerType>();

    bool DerivedMember = MP.isMemberPointerToDerivedMember();
    SmallVector<const CXXBaseSpecifier *, 4> DerivedToBasePath;
    const CXXRecordDecl *PrevRD = SrcRD;
    for (const CXXRecordDecl *PathElem : MemberPointerPath) {
      const CXXRecordDecl *Base = nullptr;
      const CXXRecordDecl *Derived = nullptr;
      if (DerivedMember) {
        Base = PathElem;
        Derived = PrevRD;
      } else {
        Base = PrevRD;
        Derived = PathElem;
      }
      for (const CXXBaseSpecifier &BS : Derived->bases())
        if (BS.getType()->getAsCXXRecordDecl()->getCanonicalDecl() ==
            Base->getCanonicalDecl())
          DerivedToBasePath.push_back(&BS);
      PrevRD = PathElem;
    }
    assert(DerivedToBasePath.size() == MemberPointerPath.size());

    CastKind CK = DerivedMember ? CK_DerivedToBaseMemberPointer
                                : CK_BaseToDerivedMemberPointer;
    C = EmitMemberPointerConversion(SrcTy, DstTy, CK, DerivedToBasePath.begin(),
                                    DerivedToBasePath.end(), C);
  }
  return C;
}

toolchain::Constant *
MicrosoftCXXABI::EmitMemberFunctionPointer(const CXXMethodDecl *MD) {
  assert(MD->isInstance() && "Member function must not be static!");

  CharUnits NonVirtualBaseAdjustment = CharUnits::Zero();
  const CXXRecordDecl *RD = MD->getParent()->getMostRecentDecl();
  CodeGenTypes &Types = CGM.getTypes();

  unsigned VBTableIndex = 0;
  toolchain::Constant *FirstField;
  const FunctionProtoType *FPT = MD->getType()->castAs<FunctionProtoType>();
  if (!MD->isVirtual()) {
    toolchain::Type *Ty;
    // Check whether the function has a computable LLVM signature.
    if (Types.isFuncTypeConvertible(FPT)) {
      // The function has a computable LLVM signature; use the correct type.
      Ty = Types.GetFunctionType(Types.arrangeCXXMethodDeclaration(MD));
    } else {
      // Use an arbitrary non-function type to tell GetAddrOfFunction that the
      // function type is incomplete.
      Ty = CGM.PtrDiffTy;
    }
    FirstField = CGM.GetAddrOfFunction(MD, Ty);
  } else {
    auto &VTableContext = CGM.getMicrosoftVTableContext();
    MethodVFTableLocation ML = VTableContext.getMethodVFTableLocation(MD);
    FirstField = EmitVirtualMemPtrThunk(MD, ML);
    // Include the vfptr adjustment if the method is in a non-primary vftable.
    NonVirtualBaseAdjustment += ML.VFPtrOffset;
    if (ML.VBase)
      VBTableIndex = VTableContext.getVBTableIndex(RD, ML.VBase) * 4;
  }

  if (VBTableIndex == 0 &&
      RD->getMSInheritanceModel() ==
          MSInheritanceModel::Virtual)
    NonVirtualBaseAdjustment -= getContext().getOffsetOfBaseWithVBPtr(RD);

  // The rest of the fields are common with data member pointers.
  return EmitFullMemberPointer(FirstField, /*IsMemberFunction=*/true, RD,
                               NonVirtualBaseAdjustment, VBTableIndex);
}

/// Member pointers are the same if they're either bitwise identical *or* both
/// null.  Null-ness for function members is determined by the first field,
/// while for data member pointers we must compare all fields.
toolchain::Value *
MicrosoftCXXABI::EmitMemberPointerComparison(CodeGenFunction &CGF,
                                             toolchain::Value *L,
                                             toolchain::Value *R,
                                             const MemberPointerType *MPT,
                                             bool Inequality) {
  CGBuilderTy &Builder = CGF.Builder;

  // Handle != comparisons by switching the sense of all boolean operations.
  toolchain::ICmpInst::Predicate Eq;
  toolchain::Instruction::BinaryOps And, Or;
  if (Inequality) {
    Eq = toolchain::ICmpInst::ICMP_NE;
    And = toolchain::Instruction::Or;
    Or = toolchain::Instruction::And;
  } else {
    Eq = toolchain::ICmpInst::ICMP_EQ;
    And = toolchain::Instruction::And;
    Or = toolchain::Instruction::Or;
  }

  // If this is a single field member pointer (single inheritance), this is a
  // single icmp.
  const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();
  if (inheritanceModelHasOnlyOneField(MPT->isMemberFunctionPointer(),
                                      Inheritance))
    return Builder.CreateICmp(Eq, L, R);

  // Compare the first field.
  toolchain::Value *L0 = Builder.CreateExtractValue(L, 0, "lhs.0");
  toolchain::Value *R0 = Builder.CreateExtractValue(R, 0, "rhs.0");
  toolchain::Value *Cmp0 = Builder.CreateICmp(Eq, L0, R0, "memptr.cmp.first");

  // Compare everything other than the first field.
  toolchain::Value *Res = nullptr;
  toolchain::StructType *LType = cast<toolchain::StructType>(L->getType());
  for (unsigned I = 1, E = LType->getNumElements(); I != E; ++I) {
    toolchain::Value *LF = Builder.CreateExtractValue(L, I);
    toolchain::Value *RF = Builder.CreateExtractValue(R, I);
    toolchain::Value *Cmp = Builder.CreateICmp(Eq, LF, RF, "memptr.cmp.rest");
    if (Res)
      Res = Builder.CreateBinOp(And, Res, Cmp);
    else
      Res = Cmp;
  }

  // Check if the first field is 0 if this is a function pointer.
  if (MPT->isMemberFunctionPointer()) {
    // (l1 == r1 && ...) || l0 == 0
    toolchain::Value *Zero = toolchain::Constant::getNullValue(L0->getType());
    toolchain::Value *IsZero = Builder.CreateICmp(Eq, L0, Zero, "memptr.cmp.iszero");
    Res = Builder.CreateBinOp(Or, Res, IsZero);
  }

  // Combine the comparison of the first field, which must always be true for
  // this comparison to succeeed.
  return Builder.CreateBinOp(And, Res, Cmp0, "memptr.cmp");
}

toolchain::Value *
MicrosoftCXXABI::EmitMemberPointerIsNotNull(CodeGenFunction &CGF,
                                            toolchain::Value *MemPtr,
                                            const MemberPointerType *MPT) {
  CGBuilderTy &Builder = CGF.Builder;
  toolchain::SmallVector<toolchain::Constant *, 4> fields;
  // We only need one field for member functions.
  if (MPT->isMemberFunctionPointer())
    fields.push_back(toolchain::Constant::getNullValue(CGM.VoidPtrTy));
  else
    GetNullMemberPointerFields(MPT, fields);
  assert(!fields.empty());
  toolchain::Value *FirstField = MemPtr;
  if (MemPtr->getType()->isStructTy())
    FirstField = Builder.CreateExtractValue(MemPtr, 0);
  toolchain::Value *Res = Builder.CreateICmpNE(FirstField, fields[0], "memptr.cmp0");

  // For function member pointers, we only need to test the function pointer
  // field.  The other fields if any can be garbage.
  if (MPT->isMemberFunctionPointer())
    return Res;

  // Otherwise, emit a series of compares and combine the results.
  for (int I = 1, E = fields.size(); I < E; ++I) {
    toolchain::Value *Field = Builder.CreateExtractValue(MemPtr, I);
    toolchain::Value *Next = Builder.CreateICmpNE(Field, fields[I], "memptr.cmp");
    Res = Builder.CreateOr(Res, Next, "memptr.tobool");
  }
  return Res;
}

bool MicrosoftCXXABI::MemberPointerConstantIsNull(const MemberPointerType *MPT,
                                                  toolchain::Constant *Val) {
  // Function pointers are null if the pointer in the first field is null.
  if (MPT->isMemberFunctionPointer()) {
    toolchain::Constant *FirstField = Val->getType()->isStructTy() ?
      Val->getAggregateElement(0U) : Val;
    return FirstField->isNullValue();
  }

  // If it's not a function pointer and it's zero initializable, we can easily
  // check zero.
  if (isZeroInitializable(MPT) && Val->isNullValue())
    return true;

  // Otherwise, break down all the fields for comparison.  Hopefully these
  // little Constants are reused, while a big null struct might not be.
  toolchain::SmallVector<toolchain::Constant *, 4> Fields;
  GetNullMemberPointerFields(MPT, Fields);
  if (Fields.size() == 1) {
    assert(Val->getType()->isIntegerTy());
    return Val == Fields[0];
  }

  unsigned I, E;
  for (I = 0, E = Fields.size(); I != E; ++I) {
    if (Val->getAggregateElement(I) != Fields[I])
      break;
  }
  return I == E;
}

toolchain::Value *
MicrosoftCXXABI::GetVBaseOffsetFromVBPtr(CodeGenFunction &CGF,
                                         Address This,
                                         toolchain::Value *VBPtrOffset,
                                         toolchain::Value *VBTableOffset,
                                         toolchain::Value **VBPtrOut) {
  CGBuilderTy &Builder = CGF.Builder;
  // Load the vbtable pointer from the vbptr in the instance.
  toolchain::Value *VBPtr = Builder.CreateInBoundsGEP(
      CGM.Int8Ty, This.emitRawPointer(CGF), VBPtrOffset, "vbptr");
  if (VBPtrOut)
    *VBPtrOut = VBPtr;

  CharUnits VBPtrAlign;
  if (auto CI = dyn_cast<toolchain::ConstantInt>(VBPtrOffset)) {
    VBPtrAlign = This.getAlignment().alignmentAtOffset(
                                   CharUnits::fromQuantity(CI->getSExtValue()));
  } else {
    VBPtrAlign = CGF.getPointerAlign();
  }

  toolchain::Value *VBTable =
      Builder.CreateAlignedLoad(CGM.UnqualPtrTy, VBPtr, VBPtrAlign, "vbtable");

  // Translate from byte offset to table index. It improves analyzability.
  toolchain::Value *VBTableIndex = Builder.CreateAShr(
      VBTableOffset, toolchain::ConstantInt::get(VBTableOffset->getType(), 2),
      "vbtindex", /*isExact=*/true);

  // Load an i32 offset from the vb-table.
  toolchain::Value *VBaseOffs =
      Builder.CreateInBoundsGEP(CGM.Int32Ty, VBTable, VBTableIndex);
  return Builder.CreateAlignedLoad(CGM.Int32Ty, VBaseOffs,
                                   CharUnits::fromQuantity(4), "vbase_offs");
}

// Returns an adjusted base cast to i8*, since we do more address arithmetic on
// it.
toolchain::Value *MicrosoftCXXABI::AdjustVirtualBase(
    CodeGenFunction &CGF, const Expr *E, const CXXRecordDecl *RD,
    Address Base, toolchain::Value *VBTableOffset, toolchain::Value *VBPtrOffset) {
  CGBuilderTy &Builder = CGF.Builder;
  Base = Base.withElementType(CGM.Int8Ty);
  toolchain::BasicBlock *OriginalBB = nullptr;
  toolchain::BasicBlock *SkipAdjustBB = nullptr;
  toolchain::BasicBlock *VBaseAdjustBB = nullptr;

  // In the unspecified inheritance model, there might not be a vbtable at all,
  // in which case we need to skip the virtual base lookup.  If there is a
  // vbtable, the first entry is a no-op entry that gives back the original
  // base, so look for a virtual base adjustment offset of zero.
  if (VBPtrOffset) {
    OriginalBB = Builder.GetInsertBlock();
    VBaseAdjustBB = CGF.createBasicBlock("memptr.vadjust");
    SkipAdjustBB = CGF.createBasicBlock("memptr.skip_vadjust");
    toolchain::Value *IsVirtual =
      Builder.CreateICmpNE(VBTableOffset, getZeroInt(),
                           "memptr.is_vbase");
    Builder.CreateCondBr(IsVirtual, VBaseAdjustBB, SkipAdjustBB);
    CGF.EmitBlock(VBaseAdjustBB);
  }

  // If we weren't given a dynamic vbptr offset, RD should be complete and we'll
  // know the vbptr offset.
  if (!VBPtrOffset) {
    CharUnits offs = CharUnits::Zero();
    if (!RD->hasDefinition()) {
      DiagnosticsEngine &Diags = CGF.CGM.getDiags();
      unsigned DiagID = Diags.getCustomDiagID(
          DiagnosticsEngine::Error,
          "member pointer representation requires a "
          "complete class type for %0 to perform this expression");
      Diags.Report(E->getExprLoc(), DiagID) << RD << E->getSourceRange();
    } else if (RD->getNumVBases())
      offs = getContext().getASTRecordLayout(RD).getVBPtrOffset();
    VBPtrOffset = toolchain::ConstantInt::get(CGM.IntTy, offs.getQuantity());
  }
  toolchain::Value *VBPtr = nullptr;
  toolchain::Value *VBaseOffs =
    GetVBaseOffsetFromVBPtr(CGF, Base, VBPtrOffset, VBTableOffset, &VBPtr);
  toolchain::Value *AdjustedBase =
    Builder.CreateInBoundsGEP(CGM.Int8Ty, VBPtr, VBaseOffs);

  // Merge control flow with the case where we didn't have to adjust.
  if (VBaseAdjustBB) {
    Builder.CreateBr(SkipAdjustBB);
    CGF.EmitBlock(SkipAdjustBB);
    toolchain::PHINode *Phi = Builder.CreatePHI(CGM.Int8PtrTy, 2, "memptr.base");
    Phi->addIncoming(Base.emitRawPointer(CGF), OriginalBB);
    Phi->addIncoming(AdjustedBase, VBaseAdjustBB);
    return Phi;
  }
  return AdjustedBase;
}

toolchain::Value *MicrosoftCXXABI::EmitMemberDataPointerAddress(
    CodeGenFunction &CGF, const Expr *E, Address Base, toolchain::Value *MemPtr,
    const MemberPointerType *MPT, bool IsInBounds) {
  assert(MPT->isMemberDataPointer());
  CGBuilderTy &Builder = CGF.Builder;
  const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();

  // Extract the fields we need, regardless of model.  We'll apply them if we
  // have them.
  toolchain::Value *FieldOffset = MemPtr;
  toolchain::Value *VirtualBaseAdjustmentOffset = nullptr;
  toolchain::Value *VBPtrOffset = nullptr;
  if (MemPtr->getType()->isStructTy()) {
    // We need to extract values.
    unsigned I = 0;
    FieldOffset = Builder.CreateExtractValue(MemPtr, I++);
    if (inheritanceModelHasVBPtrOffsetField(Inheritance))
      VBPtrOffset = Builder.CreateExtractValue(MemPtr, I++);
    if (inheritanceModelHasVBTableOffsetField(Inheritance))
      VirtualBaseAdjustmentOffset = Builder.CreateExtractValue(MemPtr, I++);
  }

  toolchain::Value *Addr;
  if (VirtualBaseAdjustmentOffset) {
    Addr = AdjustVirtualBase(CGF, E, RD, Base, VirtualBaseAdjustmentOffset,
                             VBPtrOffset);
  } else {
    Addr = Base.emitRawPointer(CGF);
  }

  // Apply the offset.
  return Builder.CreateGEP(CGF.Int8Ty, Addr, FieldOffset, "memptr.offset",
                           IsInBounds ? toolchain::GEPNoWrapFlags::inBounds()
                                      : toolchain::GEPNoWrapFlags::none());
}

toolchain::Value *
MicrosoftCXXABI::EmitMemberPointerConversion(CodeGenFunction &CGF,
                                             const CastExpr *E,
                                             toolchain::Value *Src) {
  assert(E->getCastKind() == CK_DerivedToBaseMemberPointer ||
         E->getCastKind() == CK_BaseToDerivedMemberPointer ||
         E->getCastKind() == CK_ReinterpretMemberPointer);

  // Use constant emission if we can.
  if (isa<toolchain::Constant>(Src))
    return EmitMemberPointerConversion(E, cast<toolchain::Constant>(Src));

  // We may be adding or dropping fields from the member pointer, so we need
  // both types and the inheritance models of both records.
  const MemberPointerType *SrcTy =
    E->getSubExpr()->getType()->castAs<MemberPointerType>();
  const MemberPointerType *DstTy = E->getType()->castAs<MemberPointerType>();
  bool IsFunc = SrcTy->isMemberFunctionPointer();

  // If the classes use the same null representation, reinterpret_cast is a nop.
  bool IsReinterpret = E->getCastKind() == CK_ReinterpretMemberPointer;
  if (IsReinterpret && IsFunc)
    return Src;

  CXXRecordDecl *SrcRD = SrcTy->getMostRecentCXXRecordDecl();
  CXXRecordDecl *DstRD = DstTy->getMostRecentCXXRecordDecl();
  if (IsReinterpret &&
      SrcRD->nullFieldOffsetIsZero() == DstRD->nullFieldOffsetIsZero())
    return Src;

  CGBuilderTy &Builder = CGF.Builder;

  // Branch past the conversion if Src is null.
  toolchain::Value *IsNotNull = EmitMemberPointerIsNotNull(CGF, Src, SrcTy);
  toolchain::Constant *DstNull = EmitNullMemberPointer(DstTy);

  // C++ 5.2.10p9: The null member pointer value is converted to the null member
  //   pointer value of the destination type.
  if (IsReinterpret) {
    // For reinterpret casts, sema ensures that src and dst are both functions
    // or data and have the same size, which means the LLVM types should match.
    assert(Src->getType() == DstNull->getType());
    return Builder.CreateSelect(IsNotNull, Src, DstNull);
  }

  toolchain::BasicBlock *OriginalBB = Builder.GetInsertBlock();
  toolchain::BasicBlock *ConvertBB = CGF.createBasicBlock("memptr.convert");
  toolchain::BasicBlock *ContinueBB = CGF.createBasicBlock("memptr.converted");
  Builder.CreateCondBr(IsNotNull, ConvertBB, ContinueBB);
  CGF.EmitBlock(ConvertBB);

  toolchain::Value *Dst = EmitNonNullMemberPointerConversion(
      SrcTy, DstTy, E->getCastKind(), E->path_begin(), E->path_end(), Src,
      Builder);

  Builder.CreateBr(ContinueBB);

  // In the continuation, choose between DstNull and Dst.
  CGF.EmitBlock(ContinueBB);
  toolchain::PHINode *Phi = Builder.CreatePHI(DstNull->getType(), 2, "memptr.converted");
  Phi->addIncoming(DstNull, OriginalBB);
  Phi->addIncoming(Dst, ConvertBB);
  return Phi;
}

toolchain::Value *MicrosoftCXXABI::EmitNonNullMemberPointerConversion(
    const MemberPointerType *SrcTy, const MemberPointerType *DstTy, CastKind CK,
    CastExpr::path_const_iterator PathBegin,
    CastExpr::path_const_iterator PathEnd, toolchain::Value *Src,
    CGBuilderTy &Builder) {
  const CXXRecordDecl *SrcRD = SrcTy->getMostRecentCXXRecordDecl();
  const CXXRecordDecl *DstRD = DstTy->getMostRecentCXXRecordDecl();
  MSInheritanceModel SrcInheritance = SrcRD->getMSInheritanceModel();
  MSInheritanceModel DstInheritance = DstRD->getMSInheritanceModel();
  bool IsFunc = SrcTy->isMemberFunctionPointer();
  bool IsConstant = isa<toolchain::Constant>(Src);

  // Decompose src.
  toolchain::Value *FirstField = Src;
  toolchain::Value *NonVirtualBaseAdjustment = getZeroInt();
  toolchain::Value *VirtualBaseAdjustmentOffset = getZeroInt();
  toolchain::Value *VBPtrOffset = getZeroInt();
  if (!inheritanceModelHasOnlyOneField(IsFunc, SrcInheritance)) {
    // We need to extract values.
    unsigned I = 0;
    FirstField = Builder.CreateExtractValue(Src, I++);
    if (inheritanceModelHasNVOffsetField(IsFunc, SrcInheritance))
      NonVirtualBaseAdjustment = Builder.CreateExtractValue(Src, I++);
    if (inheritanceModelHasVBPtrOffsetField(SrcInheritance))
      VBPtrOffset = Builder.CreateExtractValue(Src, I++);
    if (inheritanceModelHasVBTableOffsetField(SrcInheritance))
      VirtualBaseAdjustmentOffset = Builder.CreateExtractValue(Src, I++);
  }

  bool IsDerivedToBase = (CK == CK_DerivedToBaseMemberPointer);
  const MemberPointerType *DerivedTy = IsDerivedToBase ? SrcTy : DstTy;
  const CXXRecordDecl *DerivedClass = DerivedTy->getMostRecentCXXRecordDecl();

  // For data pointers, we adjust the field offset directly.  For functions, we
  // have a separate field.
  toolchain::Value *&NVAdjustField = IsFunc ? NonVirtualBaseAdjustment : FirstField;

  // The virtual inheritance model has a quirk: the virtual base table is always
  // referenced when dereferencing a member pointer even if the member pointer
  // is non-virtual.  This is accounted for by adjusting the non-virtual offset
  // to point backwards to the top of the MDC from the first VBase.  Undo this
  // adjustment to normalize the member pointer.
  toolchain::Value *SrcVBIndexEqZero =
      Builder.CreateICmpEQ(VirtualBaseAdjustmentOffset, getZeroInt());
  if (SrcInheritance == MSInheritanceModel::Virtual) {
    if (int64_t SrcOffsetToFirstVBase =
            getContext().getOffsetOfBaseWithVBPtr(SrcRD).getQuantity()) {
      toolchain::Value *UndoSrcAdjustment = Builder.CreateSelect(
          SrcVBIndexEqZero,
          toolchain::ConstantInt::get(CGM.IntTy, SrcOffsetToFirstVBase),
          getZeroInt());
      NVAdjustField = Builder.CreateNSWAdd(NVAdjustField, UndoSrcAdjustment);
    }
  }

  // A non-zero vbindex implies that we are dealing with a source member in a
  // floating virtual base in addition to some non-virtual offset.  If the
  // vbindex is zero, we are dealing with a source that exists in a non-virtual,
  // fixed, base.  The difference between these two cases is that the vbindex +
  // nvoffset *always* point to the member regardless of what context they are
  // evaluated in so long as the vbindex is adjusted.  A member inside a fixed
  // base requires explicit nv adjustment.
  toolchain::Constant *BaseClassOffset = toolchain::ConstantInt::get(
      CGM.IntTy,
      CGM.computeNonVirtualBaseClassOffset(DerivedClass, PathBegin, PathEnd)
          .getQuantity());

  toolchain::Value *NVDisp;
  if (IsDerivedToBase)
    NVDisp = Builder.CreateNSWSub(NVAdjustField, BaseClassOffset, "adj");
  else
    NVDisp = Builder.CreateNSWAdd(NVAdjustField, BaseClassOffset, "adj");

  NVAdjustField = Builder.CreateSelect(SrcVBIndexEqZero, NVDisp, getZeroInt());

  // Update the vbindex to an appropriate value in the destination because
  // SrcRD's vbtable might not be a strict prefix of the one in DstRD.
  toolchain::Value *DstVBIndexEqZero = SrcVBIndexEqZero;
  if (inheritanceModelHasVBTableOffsetField(DstInheritance) &&
      inheritanceModelHasVBTableOffsetField(SrcInheritance)) {
    if (toolchain::GlobalVariable *VDispMap =
            getAddrOfVirtualDisplacementMap(SrcRD, DstRD)) {
      toolchain::Value *VBIndex = Builder.CreateExactUDiv(
          VirtualBaseAdjustmentOffset, toolchain::ConstantInt::get(CGM.IntTy, 4));
      if (IsConstant) {
        toolchain::Constant *Mapping = VDispMap->getInitializer();
        VirtualBaseAdjustmentOffset =
            Mapping->getAggregateElement(cast<toolchain::Constant>(VBIndex));
      } else {
        toolchain::Value *Idxs[] = {getZeroInt(), VBIndex};
        VirtualBaseAdjustmentOffset = Builder.CreateAlignedLoad(
            CGM.IntTy, Builder.CreateInBoundsGEP(VDispMap->getValueType(),
                                                 VDispMap, Idxs),
            CharUnits::fromQuantity(4));
      }

      DstVBIndexEqZero =
          Builder.CreateICmpEQ(VirtualBaseAdjustmentOffset, getZeroInt());
    }
  }

  // Set the VBPtrOffset to zero if the vbindex is zero.  Otherwise, initialize
  // it to the offset of the vbptr.
  if (inheritanceModelHasVBPtrOffsetField(DstInheritance)) {
    toolchain::Value *DstVBPtrOffset = toolchain::ConstantInt::get(
        CGM.IntTy,
        getContext().getASTRecordLayout(DstRD).getVBPtrOffset().getQuantity());
    VBPtrOffset =
        Builder.CreateSelect(DstVBIndexEqZero, getZeroInt(), DstVBPtrOffset);
  }

  // Likewise, apply a similar adjustment so that dereferencing the member
  // pointer correctly accounts for the distance between the start of the first
  // virtual base and the top of the MDC.
  if (DstInheritance == MSInheritanceModel::Virtual) {
    if (int64_t DstOffsetToFirstVBase =
            getContext().getOffsetOfBaseWithVBPtr(DstRD).getQuantity()) {
      toolchain::Value *DoDstAdjustment = Builder.CreateSelect(
          DstVBIndexEqZero,
          toolchain::ConstantInt::get(CGM.IntTy, DstOffsetToFirstVBase),
          getZeroInt());
      NVAdjustField = Builder.CreateNSWSub(NVAdjustField, DoDstAdjustment);
    }
  }

  // Recompose dst from the null struct and the adjusted fields from src.
  toolchain::Value *Dst;
  if (inheritanceModelHasOnlyOneField(IsFunc, DstInheritance)) {
    Dst = FirstField;
  } else {
    Dst = toolchain::PoisonValue::get(ConvertMemberPointerType(DstTy));
    unsigned Idx = 0;
    Dst = Builder.CreateInsertValue(Dst, FirstField, Idx++);
    if (inheritanceModelHasNVOffsetField(IsFunc, DstInheritance))
      Dst = Builder.CreateInsertValue(Dst, NonVirtualBaseAdjustment, Idx++);
    if (inheritanceModelHasVBPtrOffsetField(DstInheritance))
      Dst = Builder.CreateInsertValue(Dst, VBPtrOffset, Idx++);
    if (inheritanceModelHasVBTableOffsetField(DstInheritance))
      Dst = Builder.CreateInsertValue(Dst, VirtualBaseAdjustmentOffset, Idx++);
  }
  return Dst;
}

toolchain::Constant *
MicrosoftCXXABI::EmitMemberPointerConversion(const CastExpr *E,
                                             toolchain::Constant *Src) {
  const MemberPointerType *SrcTy =
      E->getSubExpr()->getType()->castAs<MemberPointerType>();
  const MemberPointerType *DstTy = E->getType()->castAs<MemberPointerType>();

  CastKind CK = E->getCastKind();

  return EmitMemberPointerConversion(SrcTy, DstTy, CK, E->path_begin(),
                                     E->path_end(), Src);
}

toolchain::Constant *MicrosoftCXXABI::EmitMemberPointerConversion(
    const MemberPointerType *SrcTy, const MemberPointerType *DstTy, CastKind CK,
    CastExpr::path_const_iterator PathBegin,
    CastExpr::path_const_iterator PathEnd, toolchain::Constant *Src) {
  assert(CK == CK_DerivedToBaseMemberPointer ||
         CK == CK_BaseToDerivedMemberPointer ||
         CK == CK_ReinterpretMemberPointer);
  // If src is null, emit a new null for dst.  We can't return src because dst
  // might have a new representation.
  if (MemberPointerConstantIsNull(SrcTy, Src))
    return EmitNullMemberPointer(DstTy);

  // We don't need to do anything for reinterpret_casts of non-null member
  // pointers.  We should only get here when the two type representations have
  // the same size.
  if (CK == CK_ReinterpretMemberPointer)
    return Src;

  CGBuilderTy Builder(CGM, CGM.getLLVMContext());
  auto *Dst = cast<toolchain::Constant>(EmitNonNullMemberPointerConversion(
      SrcTy, DstTy, CK, PathBegin, PathEnd, Src, Builder));

  return Dst;
}

CGCallee MicrosoftCXXABI::EmitLoadOfMemberFunctionPointer(
    CodeGenFunction &CGF, const Expr *E, Address This,
    toolchain::Value *&ThisPtrForCall, toolchain::Value *MemPtr,
    const MemberPointerType *MPT) {
  assert(MPT->isMemberFunctionPointer());
  const FunctionProtoType *FPT =
    MPT->getPointeeType()->castAs<FunctionProtoType>();
  const CXXRecordDecl *RD = MPT->getMostRecentCXXRecordDecl();
  CGBuilderTy &Builder = CGF.Builder;

  MSInheritanceModel Inheritance = RD->getMSInheritanceModel();

  // Extract the fields we need, regardless of model.  We'll apply them if we
  // have them.
  toolchain::Value *FunctionPointer = MemPtr;
  toolchain::Value *NonVirtualBaseAdjustment = nullptr;
  toolchain::Value *VirtualBaseAdjustmentOffset = nullptr;
  toolchain::Value *VBPtrOffset = nullptr;
  if (MemPtr->getType()->isStructTy()) {
    // We need to extract values.
    unsigned I = 0;
    FunctionPointer = Builder.CreateExtractValue(MemPtr, I++);
    if (inheritanceModelHasNVOffsetField(MPT, Inheritance))
      NonVirtualBaseAdjustment = Builder.CreateExtractValue(MemPtr, I++);
    if (inheritanceModelHasVBPtrOffsetField(Inheritance))
      VBPtrOffset = Builder.CreateExtractValue(MemPtr, I++);
    if (inheritanceModelHasVBTableOffsetField(Inheritance))
      VirtualBaseAdjustmentOffset = Builder.CreateExtractValue(MemPtr, I++);
  }

  if (VirtualBaseAdjustmentOffset) {
    ThisPtrForCall = AdjustVirtualBase(CGF, E, RD, This,
                                   VirtualBaseAdjustmentOffset, VBPtrOffset);
  } else {
    ThisPtrForCall = This.emitRawPointer(CGF);
  }

  if (NonVirtualBaseAdjustment)
    ThisPtrForCall = Builder.CreateInBoundsGEP(CGF.Int8Ty, ThisPtrForCall,
                                               NonVirtualBaseAdjustment);

  CGCallee Callee(FPT, FunctionPointer);
  return Callee;
}

CGCXXABI *language::Core::CodeGen::CreateMicrosoftCXXABI(CodeGenModule &CGM) {
  return new MicrosoftCXXABI(CGM);
}

// MS RTTI Overview:
// The run time type information emitted by cl.exe contains 5 distinct types of
// structures.  Many of them reference each other.
//
// TypeInfo:  Static classes that are returned by typeid.
//
// CompleteObjectLocator:  Referenced by vftables.  They contain information
//   required for dynamic casting, including OffsetFromTop.  They also contain
//   a reference to the TypeInfo for the type and a reference to the
//   CompleteHierarchyDescriptor for the type.
//
// ClassHierarchyDescriptor: Contains information about a class hierarchy.
//   Used during dynamic_cast to walk a class hierarchy.  References a base
//   class array and the size of said array.
//
// BaseClassArray: Contains a list of classes in a hierarchy.  BaseClassArray is
//   somewhat of a misnomer because the most derived class is also in the list
//   as well as multiple copies of virtual bases (if they occur multiple times
//   in the hierarchy.)  The BaseClassArray contains one BaseClassDescriptor for
//   every path in the hierarchy, in pre-order depth first order.  Note, we do
//   not declare a specific toolchain type for BaseClassArray, it's merely an array
//   of BaseClassDescriptor pointers.
//
// BaseClassDescriptor: Contains information about a class in a class hierarchy.
//   BaseClassDescriptor is also somewhat of a misnomer for the same reason that
//   BaseClassArray is.  It contains information about a class within a
//   hierarchy such as: is this base is ambiguous and what is its offset in the
//   vbtable.  The names of the BaseClassDescriptors have all of their fields
//   mangled into them so they can be aggressively deduplicated by the linker.

static toolchain::GlobalVariable *getTypeInfoVTable(CodeGenModule &CGM) {
  StringRef MangledName("??_7type_info@@6B@");
  if (auto VTable = CGM.getModule().getNamedGlobal(MangledName))
    return VTable;
  return new toolchain::GlobalVariable(CGM.getModule(), CGM.Int8PtrTy,
                                  /*isConstant=*/true,
                                  toolchain::GlobalVariable::ExternalLinkage,
                                  /*Initializer=*/nullptr, MangledName);
}

namespace {

/// A Helper struct that stores information about a class in a class
/// hierarchy.  The information stored in these structs struct is used during
/// the generation of ClassHierarchyDescriptors and BaseClassDescriptors.
// During RTTI creation, MSRTTIClasses are stored in a contiguous array with
// implicit depth first pre-order tree connectivity.  getFirstChild and
// getNextSibling allow us to walk the tree efficiently.
struct MSRTTIClass {
  enum {
    IsPrivateOnPath = 1 | 8,
    IsAmbiguous = 2,
    IsPrivate = 4,
    IsVirtual = 16,
    HasHierarchyDescriptor = 64
  };
  MSRTTIClass(const CXXRecordDecl *RD) : RD(RD) {}
  uint32_t initialize(const MSRTTIClass *Parent,
                      const CXXBaseSpecifier *Specifier);

  MSRTTIClass *getFirstChild() { return this + 1; }
  static MSRTTIClass *getNextChild(MSRTTIClass *Child) {
    return Child + 1 + Child->NumBases;
  }

  const CXXRecordDecl *RD, *VirtualRoot;
  uint32_t Flags, NumBases, OffsetInVBase;
};

/// Recursively initialize the base class array.
uint32_t MSRTTIClass::initialize(const MSRTTIClass *Parent,
                                 const CXXBaseSpecifier *Specifier) {
  Flags = HasHierarchyDescriptor;
  if (!Parent) {
    VirtualRoot = nullptr;
    OffsetInVBase = 0;
  } else {
    if (Specifier->getAccessSpecifier() != AS_public)
      Flags |= IsPrivate | IsPrivateOnPath;
    if (Specifier->isVirtual()) {
      Flags |= IsVirtual;
      VirtualRoot = RD;
      OffsetInVBase = 0;
    } else {
      if (Parent->Flags & IsPrivateOnPath)
        Flags |= IsPrivateOnPath;
      VirtualRoot = Parent->VirtualRoot;
      OffsetInVBase = Parent->OffsetInVBase + RD->getASTContext()
          .getASTRecordLayout(Parent->RD).getBaseClassOffset(RD).getQuantity();
    }
  }
  NumBases = 0;
  MSRTTIClass *Child = getFirstChild();
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    NumBases += Child->initialize(this, &Base) + 1;
    Child = getNextChild(Child);
  }
  return NumBases;
}

static toolchain::GlobalValue::LinkageTypes getLinkageForRTTI(QualType Ty) {
  switch (Ty->getLinkage()) {
  case Linkage::Invalid:
    toolchain_unreachable("Linkage hasn't been computed!");

  case Linkage::None:
  case Linkage::Internal:
  case Linkage::UniqueExternal:
    return toolchain::GlobalValue::InternalLinkage;

  case Linkage::VisibleNone:
  case Linkage::Module:
  case Linkage::External:
    return toolchain::GlobalValue::LinkOnceODRLinkage;
  }
  toolchain_unreachable("Invalid linkage!");
}

/// An ephemeral helper class for building MS RTTI types.  It caches some
/// calls to the module and information about the most derived class in a
/// hierarchy.
struct MSRTTIBuilder {
  enum {
    HasBranchingHierarchy = 1,
    HasVirtualBranchingHierarchy = 2,
    HasAmbiguousBases = 4
  };

  MSRTTIBuilder(MicrosoftCXXABI &ABI, const CXXRecordDecl *RD)
      : CGM(ABI.CGM), Context(CGM.getContext()),
        VMContext(CGM.getLLVMContext()), Module(CGM.getModule()), RD(RD),
        Linkage(getLinkageForRTTI(CGM.getContext().getCanonicalTagType(RD))),
        ABI(ABI) {}

  toolchain::GlobalVariable *getBaseClassDescriptor(const MSRTTIClass &Classes);
  toolchain::GlobalVariable *
  getBaseClassArray(SmallVectorImpl<MSRTTIClass> &Classes);
  toolchain::GlobalVariable *getClassHierarchyDescriptor();
  toolchain::GlobalVariable *getCompleteObjectLocator(const VPtrInfo &Info);

  CodeGenModule &CGM;
  ASTContext &Context;
  toolchain::LLVMContext &VMContext;
  toolchain::Module &Module;
  const CXXRecordDecl *RD;
  toolchain::GlobalVariable::LinkageTypes Linkage;
  MicrosoftCXXABI &ABI;
};

} // namespace

/// Recursively serializes a class hierarchy in pre-order depth first
/// order.
static void serializeClassHierarchy(SmallVectorImpl<MSRTTIClass> &Classes,
                                    const CXXRecordDecl *RD) {
  Classes.push_back(MSRTTIClass(RD));
  for (const CXXBaseSpecifier &Base : RD->bases())
    serializeClassHierarchy(Classes, Base.getType()->getAsCXXRecordDecl());
}

/// Find ambiguity among base classes.
static void
detectAmbiguousBases(SmallVectorImpl<MSRTTIClass> &Classes) {
  toolchain::SmallPtrSet<const CXXRecordDecl *, 8> VirtualBases;
  toolchain::SmallPtrSet<const CXXRecordDecl *, 8> UniqueBases;
  toolchain::SmallPtrSet<const CXXRecordDecl *, 8> AmbiguousBases;
  for (MSRTTIClass *Class = &Classes.front(); Class <= &Classes.back();) {
    if ((Class->Flags & MSRTTIClass::IsVirtual) &&
        !VirtualBases.insert(Class->RD).second) {
      Class = MSRTTIClass::getNextChild(Class);
      continue;
    }
    if (!UniqueBases.insert(Class->RD).second)
      AmbiguousBases.insert(Class->RD);
    Class++;
  }
  if (AmbiguousBases.empty())
    return;
  for (MSRTTIClass &Class : Classes)
    if (AmbiguousBases.count(Class.RD))
      Class.Flags |= MSRTTIClass::IsAmbiguous;
}

toolchain::GlobalVariable *MSRTTIBuilder::getClassHierarchyDescriptor() {
  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    ABI.getMangleContext().mangleCXXRTTIClassHierarchyDescriptor(RD, Out);
  }

  // Check to see if we've already declared this ClassHierarchyDescriptor.
  if (auto CHD = Module.getNamedGlobal(MangledName))
    return CHD;

  // Serialize the class hierarchy and initialize the CHD Fields.
  SmallVector<MSRTTIClass, 8> Classes;
  serializeClassHierarchy(Classes, RD);
  Classes.front().initialize(/*Parent=*/nullptr, /*Specifier=*/nullptr);
  detectAmbiguousBases(Classes);
  int Flags = 0;
  for (const MSRTTIClass &Class : Classes) {
    if (Class.RD->getNumBases() > 1)
      Flags |= HasBranchingHierarchy;
    // Note: cl.exe does not calculate "HasAmbiguousBases" correctly.  We
    // believe the field isn't actually used.
    if (Class.Flags & MSRTTIClass::IsAmbiguous)
      Flags |= HasAmbiguousBases;
  }
  if ((Flags & HasBranchingHierarchy) && RD->getNumVBases() != 0)
    Flags |= HasVirtualBranchingHierarchy;
  // These gep indices are used to get the address of the first element of the
  // base class array.
  toolchain::Value *GEPIndices[] = {toolchain::ConstantInt::get(CGM.IntTy, 0),
                               toolchain::ConstantInt::get(CGM.IntTy, 0)};

  // Forward-declare the class hierarchy descriptor
  auto Type = ABI.getClassHierarchyDescriptorType();
  auto CHD = new toolchain::GlobalVariable(Module, Type, /*isConstant=*/true, Linkage,
                                      /*Initializer=*/nullptr,
                                      MangledName);
  if (CHD->isWeakForLinker())
    CHD->setComdat(CGM.getModule().getOrInsertComdat(CHD->getName()));

  auto *Bases = getBaseClassArray(Classes);

  // Initialize the base class ClassHierarchyDescriptor.
  toolchain::Constant *Fields[] = {
      toolchain::ConstantInt::get(CGM.IntTy, 0), // reserved by the runtime
      toolchain::ConstantInt::get(CGM.IntTy, Flags),
      toolchain::ConstantInt::get(CGM.IntTy, Classes.size()),
      ABI.getImageRelativeConstant(toolchain::ConstantExpr::getInBoundsGetElementPtr(
          Bases->getValueType(), Bases,
          toolchain::ArrayRef<toolchain::Value *>(GEPIndices))),
  };
  CHD->setInitializer(toolchain::ConstantStruct::get(Type, Fields));
  return CHD;
}

toolchain::GlobalVariable *
MSRTTIBuilder::getBaseClassArray(SmallVectorImpl<MSRTTIClass> &Classes) {
  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    ABI.getMangleContext().mangleCXXRTTIBaseClassArray(RD, Out);
  }

  // Forward-declare the base class array.
  // cl.exe pads the base class array with 1 (in 32 bit mode) or 4 (in 64 bit
  // mode) bytes of padding.  We provide a pointer sized amount of padding by
  // adding +1 to Classes.size().  The sections have pointer alignment and are
  // marked pick-any so it shouldn't matter.
  toolchain::Type *PtrType = ABI.getImageRelativeType(CGM.UnqualPtrTy);
  auto *ArrType = toolchain::ArrayType::get(PtrType, Classes.size() + 1);
  auto *BCA =
      new toolchain::GlobalVariable(Module, ArrType,
                               /*isConstant=*/true, Linkage,
                               /*Initializer=*/nullptr, MangledName);
  if (BCA->isWeakForLinker())
    BCA->setComdat(CGM.getModule().getOrInsertComdat(BCA->getName()));

  // Initialize the BaseClassArray.
  SmallVector<toolchain::Constant *, 8> BaseClassArrayData;
  for (MSRTTIClass &Class : Classes)
    BaseClassArrayData.push_back(
        ABI.getImageRelativeConstant(getBaseClassDescriptor(Class)));
  BaseClassArrayData.push_back(toolchain::Constant::getNullValue(PtrType));
  BCA->setInitializer(toolchain::ConstantArray::get(ArrType, BaseClassArrayData));
  return BCA;
}

toolchain::GlobalVariable *
MSRTTIBuilder::getBaseClassDescriptor(const MSRTTIClass &Class) {
  // Compute the fields for the BaseClassDescriptor.  They are computed up front
  // because they are mangled into the name of the object.
  uint32_t OffsetInVBTable = 0;
  int32_t VBPtrOffset = -1;
  if (Class.VirtualRoot) {
    auto &VTableContext = CGM.getMicrosoftVTableContext();
    OffsetInVBTable = VTableContext.getVBTableIndex(RD, Class.VirtualRoot) * 4;
    VBPtrOffset = Context.getASTRecordLayout(RD).getVBPtrOffset().getQuantity();
  }

  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    ABI.getMangleContext().mangleCXXRTTIBaseClassDescriptor(
        Class.RD, Class.OffsetInVBase, VBPtrOffset, OffsetInVBTable,
        Class.Flags, Out);
  }

  // Check to see if we've already declared this object.
  if (auto BCD = Module.getNamedGlobal(MangledName))
    return BCD;

  // Forward-declare the base class descriptor.
  auto Type = ABI.getBaseClassDescriptorType();
  auto BCD =
      new toolchain::GlobalVariable(Module, Type, /*isConstant=*/true, Linkage,
                               /*Initializer=*/nullptr, MangledName);
  if (BCD->isWeakForLinker())
    BCD->setComdat(CGM.getModule().getOrInsertComdat(BCD->getName()));

  // Initialize the BaseClassDescriptor.
  toolchain::Constant *Fields[] = {
      ABI.getImageRelativeConstant(
          ABI.getAddrOfRTTIDescriptor(Context.getCanonicalTagType(Class.RD))),
      toolchain::ConstantInt::get(CGM.IntTy, Class.NumBases),
      toolchain::ConstantInt::get(CGM.IntTy, Class.OffsetInVBase),
      toolchain::ConstantInt::get(CGM.IntTy, VBPtrOffset),
      toolchain::ConstantInt::get(CGM.IntTy, OffsetInVBTable),
      toolchain::ConstantInt::get(CGM.IntTy, Class.Flags),
      ABI.getImageRelativeConstant(
          MSRTTIBuilder(ABI, Class.RD).getClassHierarchyDescriptor()),
  };
  BCD->setInitializer(toolchain::ConstantStruct::get(Type, Fields));
  return BCD;
}

toolchain::GlobalVariable *
MSRTTIBuilder::getCompleteObjectLocator(const VPtrInfo &Info) {
  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    ABI.getMangleContext().mangleCXXRTTICompleteObjectLocator(RD, Info.MangledPath, Out);
  }

  // Check to see if we've already computed this complete object locator.
  if (auto COL = Module.getNamedGlobal(MangledName))
    return COL;

  // Compute the fields of the complete object locator.
  int OffsetToTop = Info.FullOffsetInMDC.getQuantity();
  int VFPtrOffset = 0;
  // The offset includes the vtordisp if one exists.
  if (const CXXRecordDecl *VBase = Info.getVBaseWithVPtr())
    if (Context.getASTRecordLayout(RD)
      .getVBaseOffsetsMap()
      .find(VBase)
      ->second.hasVtorDisp())
      VFPtrOffset = Info.NonVirtualOffset.getQuantity() + 4;

  // Forward-declare the complete object locator.
  toolchain::StructType *Type = ABI.getCompleteObjectLocatorType();
  auto COL = new toolchain::GlobalVariable(Module, Type, /*isConstant=*/true, Linkage,
    /*Initializer=*/nullptr, MangledName);

  // Initialize the CompleteObjectLocator.
  toolchain::Constant *Fields[] = {
      toolchain::ConstantInt::get(CGM.IntTy, ABI.isImageRelative()),
      toolchain::ConstantInt::get(CGM.IntTy, OffsetToTop),
      toolchain::ConstantInt::get(CGM.IntTy, VFPtrOffset),
      ABI.getImageRelativeConstant(
          CGM.GetAddrOfRTTIDescriptor(Context.getCanonicalTagType(RD))),
      ABI.getImageRelativeConstant(getClassHierarchyDescriptor()),
      ABI.getImageRelativeConstant(COL),
  };
  toolchain::ArrayRef<toolchain::Constant *> FieldsRef(Fields);
  if (!ABI.isImageRelative())
    FieldsRef = FieldsRef.drop_back();
  COL->setInitializer(toolchain::ConstantStruct::get(Type, FieldsRef));
  if (COL->isWeakForLinker())
    COL->setComdat(CGM.getModule().getOrInsertComdat(COL->getName()));
  return COL;
}

static QualType decomposeTypeForEH(ASTContext &Context, QualType T,
                                   bool &IsConst, bool &IsVolatile,
                                   bool &IsUnaligned) {
  T = Context.getExceptionObjectType(T);

  // C++14 [except.handle]p3:
  //   A handler is a match for an exception object of type E if [...]
  //     - the handler is of type cv T or const T& where T is a pointer type and
  //       E is a pointer type that can be converted to T by [...]
  //         - a qualification conversion
  IsConst = false;
  IsVolatile = false;
  IsUnaligned = false;
  QualType PointeeType = T->getPointeeType();
  if (!PointeeType.isNull()) {
    IsConst = PointeeType.isConstQualified();
    IsVolatile = PointeeType.isVolatileQualified();
    IsUnaligned = PointeeType.getQualifiers().hasUnaligned();
  }

  // Member pointer types like "const int A::*" are represented by having RTTI
  // for "int A::*" and separately storing the const qualifier.
  if (const auto *MPTy = T->getAs<MemberPointerType>())
    T = Context.getMemberPointerType(PointeeType.getUnqualifiedType(),
                                     MPTy->getQualifier(),
                                     MPTy->getMostRecentCXXRecordDecl());

  // Pointer types like "const int * const *" are represented by having RTTI
  // for "const int **" and separately storing the const qualifier.
  if (T->isPointerType())
    T = Context.getPointerType(PointeeType.getUnqualifiedType());

  return T;
}

CatchTypeInfo
MicrosoftCXXABI::getAddrOfCXXCatchHandlerType(QualType Type,
                                              QualType CatchHandlerType) {
  // TypeDescriptors for exceptions never have qualified pointer types,
  // qualifiers are stored separately in order to support qualification
  // conversions.
  bool IsConst, IsVolatile, IsUnaligned;
  Type =
      decomposeTypeForEH(getContext(), Type, IsConst, IsVolatile, IsUnaligned);

  bool IsReference = CatchHandlerType->isReferenceType();

  uint32_t Flags = 0;
  if (IsConst)
    Flags |= 1;
  if (IsVolatile)
    Flags |= 2;
  if (IsUnaligned)
    Flags |= 4;
  if (IsReference)
    Flags |= 8;

  return CatchTypeInfo{getAddrOfRTTIDescriptor(Type)->stripPointerCasts(),
                       Flags};
}

/// Gets a TypeDescriptor.  Returns a toolchain::Constant * rather than a
/// toolchain::GlobalVariable * because different type descriptors have different
/// types, and need to be abstracted.  They are abstracting by casting the
/// address to an Int8PtrTy.
toolchain::Constant *MicrosoftCXXABI::getAddrOfRTTIDescriptor(QualType Type) {
  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    getMangleContext().mangleCXXRTTI(Type, Out);
  }

  // Check to see if we've already declared this TypeDescriptor.
  if (toolchain::GlobalVariable *GV = CGM.getModule().getNamedGlobal(MangledName))
    return GV;

  // Note for the future: If we would ever like to do deferred emission of
  // RTTI, check if emitting vtables opportunistically need any adjustment.

  // Compute the fields for the TypeDescriptor.
  SmallString<256> TypeInfoString;
  {
    toolchain::raw_svector_ostream Out(TypeInfoString);
    getMangleContext().mangleCXXRTTIName(Type, Out);
  }

  // Declare and initialize the TypeDescriptor.
  toolchain::Constant *Fields[] = {
    getTypeInfoVTable(CGM),                        // VFPtr
    toolchain::ConstantPointerNull::get(CGM.Int8PtrTy), // Runtime data
    toolchain::ConstantDataArray::getString(CGM.getLLVMContext(), TypeInfoString)};
  toolchain::StructType *TypeDescriptorType =
      getTypeDescriptorType(TypeInfoString);
  auto *Var = new toolchain::GlobalVariable(
      CGM.getModule(), TypeDescriptorType, /*isConstant=*/false,
      getLinkageForRTTI(Type),
      toolchain::ConstantStruct::get(TypeDescriptorType, Fields),
      MangledName);
  if (Var->isWeakForLinker())
    Var->setComdat(CGM.getModule().getOrInsertComdat(Var->getName()));
  return Var;
}

/// Gets or a creates a Microsoft CompleteObjectLocator.
toolchain::GlobalVariable *
MicrosoftCXXABI::getMSCompleteObjectLocator(const CXXRecordDecl *RD,
                                            const VPtrInfo &Info) {
  return MSRTTIBuilder(*this, RD).getCompleteObjectLocator(Info);
}

void MicrosoftCXXABI::emitCXXStructor(GlobalDecl GD) {
  if (auto *ctor = dyn_cast<CXXConstructorDecl>(GD.getDecl())) {
    // There are no constructor variants, always emit the complete destructor.
    toolchain::Function *Fn =
        CGM.codegenCXXStructor(GD.getWithCtorType(Ctor_Complete));
    CGM.maybeSetTrivialComdat(*ctor, *Fn);
    return;
  }

  auto *dtor = cast<CXXDestructorDecl>(GD.getDecl());

  // Emit the base destructor if the base and complete (vbase) destructors are
  // equivalent. This effectively implements -mconstructor-aliases as part of
  // the ABI.
  if (GD.getDtorType() == Dtor_Complete &&
      dtor->getParent()->getNumVBases() == 0)
    GD = GD.getWithDtorType(Dtor_Base);

  // The base destructor is equivalent to the base destructor of its
  // base class if there is exactly one non-virtual base class with a
  // non-trivial destructor, there are no fields with a non-trivial
  // destructor, and the body of the destructor is trivial.
  if (GD.getDtorType() == Dtor_Base && !CGM.TryEmitBaseDestructorAsAlias(dtor))
    return;

  toolchain::Function *Fn = CGM.codegenCXXStructor(GD);
  if (Fn->isWeakForLinker())
    Fn->setComdat(CGM.getModule().getOrInsertComdat(Fn->getName()));
}

toolchain::Function *
MicrosoftCXXABI::getAddrOfCXXCtorClosure(const CXXConstructorDecl *CD,
                                         CXXCtorType CT) {
  assert(CT == Ctor_CopyingClosure || CT == Ctor_DefaultClosure);

  // Calculate the mangled name.
  SmallString<256> ThunkName;
  toolchain::raw_svector_ostream Out(ThunkName);
  getMangleContext().mangleName(GlobalDecl(CD, CT), Out);

  // If the thunk has been generated previously, just return it.
  if (toolchain::GlobalValue *GV = CGM.getModule().getNamedValue(ThunkName))
    return cast<toolchain::Function>(GV);

  // Create the toolchain::Function.
  const CGFunctionInfo &FnInfo = CGM.getTypes().arrangeMSCtorClosure(CD, CT);
  toolchain::FunctionType *ThunkTy = CGM.getTypes().GetFunctionType(FnInfo);
  const CXXRecordDecl *RD = CD->getParent();
  CanQualType RecordTy = getContext().getCanonicalTagType(RD);
  toolchain::Function *ThunkFn = toolchain::Function::Create(
      ThunkTy, getLinkageForRTTI(RecordTy), ThunkName.str(), &CGM.getModule());
  ThunkFn->setCallingConv(static_cast<toolchain::CallingConv::ID>(
      FnInfo.getEffectiveCallingConvention()));
  if (ThunkFn->isWeakForLinker())
    ThunkFn->setComdat(CGM.getModule().getOrInsertComdat(ThunkFn->getName()));
  bool IsCopy = CT == Ctor_CopyingClosure;

  // Start codegen.
  CodeGenFunction CGF(CGM);
  CGF.CurGD = GlobalDecl(CD, Ctor_Complete);

  // Build FunctionArgs.
  FunctionArgList FunctionArgs;

  // A constructor always starts with a 'this' pointer as its first argument.
  buildThisParam(CGF, FunctionArgs);

  // Following the 'this' pointer is a reference to the source object that we
  // are copying from.
  ImplicitParamDecl SrcParam(
      getContext(), /*DC=*/nullptr, SourceLocation(),
      &getContext().Idents.get("src"),
      getContext().getLValueReferenceType(RecordTy,
                                          /*SpelledAsLValue=*/true),
      ImplicitParamKind::Other);
  if (IsCopy)
    FunctionArgs.push_back(&SrcParam);

  // Constructors for classes which utilize virtual bases have an additional
  // parameter which indicates whether or not it is being delegated to by a more
  // derived constructor.
  ImplicitParamDecl IsMostDerived(getContext(), /*DC=*/nullptr,
                                  SourceLocation(),
                                  &getContext().Idents.get("is_most_derived"),
                                  getContext().IntTy, ImplicitParamKind::Other);
  // Only add the parameter to the list if the class has virtual bases.
  if (RD->getNumVBases() > 0)
    FunctionArgs.push_back(&IsMostDerived);

  // Start defining the function.
  auto NL = ApplyDebugLocation::CreateEmpty(CGF);
  CGF.StartFunction(GlobalDecl(), FnInfo.getReturnType(), ThunkFn, FnInfo,
                    FunctionArgs, CD->getLocation(), SourceLocation());
  // Create a scope with an artificial location for the body of this function.
  auto AL = ApplyDebugLocation::CreateArtificial(CGF);
  setCXXABIThisValue(CGF, loadIncomingCXXThis(CGF));
  toolchain::Value *This = getThisValue(CGF);

  toolchain::Value *SrcVal =
      IsCopy ? CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(&SrcParam), "src")
             : nullptr;

  CallArgList Args;

  // Push the this ptr.
  Args.add(RValue::get(This), CD->getThisType());

  // Push the src ptr.
  if (SrcVal)
    Args.add(RValue::get(SrcVal), SrcParam.getType());

  // Add the rest of the default arguments.
  SmallVector<const Stmt *, 4> ArgVec;
  ArrayRef<ParmVarDecl *> params = CD->parameters().drop_front(IsCopy ? 1 : 0);
  for (const ParmVarDecl *PD : params) {
    assert(PD->hasDefaultArg() && "ctor closure lacks default args");
    ArgVec.push_back(PD->getDefaultArg());
  }

  CodeGenFunction::RunCleanupsScope Cleanups(CGF);

  const auto *FPT = CD->getType()->castAs<FunctionProtoType>();
  CGF.EmitCallArgs(Args, FPT, toolchain::ArrayRef(ArgVec), CD, IsCopy ? 1 : 0);

  // Insert any ABI-specific implicit constructor arguments.
  AddedStructorArgCounts ExtraArgs =
      addImplicitConstructorArgs(CGF, CD, Ctor_Complete,
                                 /*ForVirtualBase=*/false,
                                 /*Delegating=*/false, Args);
  // Call the destructor with our arguments.
  toolchain::Constant *CalleePtr =
      CGM.getAddrOfCXXStructor(GlobalDecl(CD, Ctor_Complete));
  CGCallee Callee =
      CGCallee::forDirect(CalleePtr, GlobalDecl(CD, Ctor_Complete));
  const CGFunctionInfo &CalleeInfo = CGM.getTypes().arrangeCXXConstructorCall(
      Args, CD, Ctor_Complete, ExtraArgs.Prefix, ExtraArgs.Suffix);
  CGF.EmitCall(CalleeInfo, Callee, ReturnValueSlot(), Args);

  Cleanups.ForceCleanup();

  // Emit the ret instruction, remove any temporary instructions created for the
  // aid of CodeGen.
  CGF.FinishFunction(SourceLocation());

  return ThunkFn;
}

toolchain::Constant *MicrosoftCXXABI::getCatchableType(QualType T,
                                                  uint32_t NVOffset,
                                                  int32_t VBPtrOffset,
                                                  uint32_t VBIndex) {
  assert(!T->isReferenceType());

  CXXRecordDecl *RD = T->getAsCXXRecordDecl();
  const CXXConstructorDecl *CD =
      RD ? CGM.getContext().getCopyConstructorForExceptionObject(RD) : nullptr;
  CXXCtorType CT = Ctor_Complete;
  if (CD)
    if (!hasDefaultCXXMethodCC(getContext(), CD) || CD->getNumParams() != 1)
      CT = Ctor_CopyingClosure;

  uint32_t Size = getContext().getTypeSizeInChars(T).getQuantity();
  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    getMangleContext().mangleCXXCatchableType(T, CD, CT, Size, NVOffset,
                                              VBPtrOffset, VBIndex, Out);
  }
  if (toolchain::GlobalVariable *GV = CGM.getModule().getNamedGlobal(MangledName))
    return getImageRelativeConstant(GV);

  // The TypeDescriptor is used by the runtime to determine if a catch handler
  // is appropriate for the exception object.
  toolchain::Constant *TD = getImageRelativeConstant(getAddrOfRTTIDescriptor(T));

  // The runtime is responsible for calling the copy constructor if the
  // exception is caught by value.
  toolchain::Constant *CopyCtor;
  if (CD) {
    if (CT == Ctor_CopyingClosure)
      CopyCtor = getAddrOfCXXCtorClosure(CD, Ctor_CopyingClosure);
    else
      CopyCtor = CGM.getAddrOfCXXStructor(GlobalDecl(CD, Ctor_Complete));
  } else {
    CopyCtor = toolchain::Constant::getNullValue(CGM.Int8PtrTy);
  }
  CopyCtor = getImageRelativeConstant(CopyCtor);

  bool IsScalar = !RD;
  bool HasVirtualBases = false;
  bool IsStdBadAlloc = false; // std::bad_alloc is special for some reason.
  QualType PointeeType = T;
  if (T->isPointerType())
    PointeeType = T->getPointeeType();
  if (const CXXRecordDecl *RD = PointeeType->getAsCXXRecordDecl()) {
    HasVirtualBases = RD->getNumVBases() > 0;
    if (IdentifierInfo *II = RD->getIdentifier())
      IsStdBadAlloc = II->isStr("bad_alloc") && RD->isInStdNamespace();
  }

  // Encode the relevant CatchableType properties into the Flags bitfield.
  // FIXME: Figure out how bits 2 or 8 can get set.
  uint32_t Flags = 0;
  if (IsScalar)
    Flags |= 1;
  if (HasVirtualBases)
    Flags |= 4;
  if (IsStdBadAlloc)
    Flags |= 16;

  toolchain::Constant *Fields[] = {
      toolchain::ConstantInt::get(CGM.IntTy, Flags),       // Flags
      TD,                                             // TypeDescriptor
      toolchain::ConstantInt::get(CGM.IntTy, NVOffset),    // NonVirtualAdjustment
      toolchain::ConstantInt::get(CGM.IntTy, VBPtrOffset), // OffsetToVBPtr
      toolchain::ConstantInt::get(CGM.IntTy, VBIndex),     // VBTableIndex
      toolchain::ConstantInt::get(CGM.IntTy, Size),        // Size
      CopyCtor                                        // CopyCtor
  };
  toolchain::StructType *CTType = getCatchableTypeType();
  auto *GV = new toolchain::GlobalVariable(
      CGM.getModule(), CTType, /*isConstant=*/true, getLinkageForRTTI(T),
      toolchain::ConstantStruct::get(CTType, Fields), MangledName);
  GV->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);
  GV->setSection(".xdata");
  if (GV->isWeakForLinker())
    GV->setComdat(CGM.getModule().getOrInsertComdat(GV->getName()));
  return getImageRelativeConstant(GV);
}

toolchain::GlobalVariable *MicrosoftCXXABI::getCatchableTypeArray(QualType T) {
  assert(!T->isReferenceType());

  // See if we've already generated a CatchableTypeArray for this type before.
  toolchain::GlobalVariable *&CTA = CatchableTypeArrays[T];
  if (CTA)
    return CTA;

  // Ensure that we don't have duplicate entries in our CatchableTypeArray by
  // using a SmallSetVector.  Duplicates may arise due to virtual bases
  // occurring more than once in the hierarchy.
  toolchain::SmallSetVector<toolchain::Constant *, 2> CatchableTypes;

  // C++14 [except.handle]p3:
  //   A handler is a match for an exception object of type E if [...]
  //     - the handler is of type cv T or cv T& and T is an unambiguous public
  //       base class of E, or
  //     - the handler is of type cv T or const T& where T is a pointer type and
  //       E is a pointer type that can be converted to T by [...]
  //         - a standard pointer conversion (4.10) not involving conversions to
  //           pointers to private or protected or ambiguous classes
  const CXXRecordDecl *MostDerivedClass = nullptr;
  bool IsPointer = T->isPointerType();
  if (IsPointer)
    MostDerivedClass = T->getPointeeType()->getAsCXXRecordDecl();
  else
    MostDerivedClass = T->getAsCXXRecordDecl();

  // Collect all the unambiguous public bases of the MostDerivedClass.
  if (MostDerivedClass) {
    const ASTContext &Context = getContext();
    const ASTRecordLayout &MostDerivedLayout =
        Context.getASTRecordLayout(MostDerivedClass);
    MicrosoftVTableContext &VTableContext = CGM.getMicrosoftVTableContext();
    SmallVector<MSRTTIClass, 8> Classes;
    serializeClassHierarchy(Classes, MostDerivedClass);
    Classes.front().initialize(/*Parent=*/nullptr, /*Specifier=*/nullptr);
    detectAmbiguousBases(Classes);
    for (const MSRTTIClass &Class : Classes) {
      // Skip any ambiguous or private bases.
      if (Class.Flags &
          (MSRTTIClass::IsPrivateOnPath | MSRTTIClass::IsAmbiguous))
        continue;
      // Write down how to convert from a derived pointer to a base pointer.
      uint32_t OffsetInVBTable = 0;
      int32_t VBPtrOffset = -1;
      if (Class.VirtualRoot) {
        OffsetInVBTable =
          VTableContext.getVBTableIndex(MostDerivedClass, Class.VirtualRoot)*4;
        VBPtrOffset = MostDerivedLayout.getVBPtrOffset().getQuantity();
      }

      // Turn our record back into a pointer if the exception object is a
      // pointer.
      CanQualType RTTITy = Context.getCanonicalTagType(Class.RD);
      if (IsPointer)
        RTTITy = Context.getPointerType(RTTITy);
      CatchableTypes.insert(getCatchableType(RTTITy, Class.OffsetInVBase,
                                             VBPtrOffset, OffsetInVBTable));
    }
  }

  // C++14 [except.handle]p3:
  //   A handler is a match for an exception object of type E if
  //     - The handler is of type cv T or cv T& and E and T are the same type
  //       (ignoring the top-level cv-qualifiers)
  CatchableTypes.insert(getCatchableType(T));

  // C++14 [except.handle]p3:
  //   A handler is a match for an exception object of type E if
  //     - the handler is of type cv T or const T& where T is a pointer type and
  //       E is a pointer type that can be converted to T by [...]
  //         - a standard pointer conversion (4.10) not involving conversions to
  //           pointers to private or protected or ambiguous classes
  //
  // C++14 [conv.ptr]p2:
  //   A prvalue of type "pointer to cv T," where T is an object type, can be
  //   converted to a prvalue of type "pointer to cv void".
  if (IsPointer && T->getPointeeType()->isObjectType())
    CatchableTypes.insert(getCatchableType(getContext().VoidPtrTy));

  // C++14 [except.handle]p3:
  //   A handler is a match for an exception object of type E if [...]
  //     - the handler is of type cv T or const T& where T is a pointer or
  //       pointer to member type and E is std::nullptr_t.
  //
  // We cannot possibly list all possible pointer types here, making this
  // implementation incompatible with the standard.  However, MSVC includes an
  // entry for pointer-to-void in this case.  Let's do the same.
  if (T->isNullPtrType())
    CatchableTypes.insert(getCatchableType(getContext().VoidPtrTy));

  uint32_t NumEntries = CatchableTypes.size();
  toolchain::Type *CTType = getImageRelativeType(CGM.UnqualPtrTy);
  toolchain::ArrayType *AT = toolchain::ArrayType::get(CTType, NumEntries);
  toolchain::StructType *CTAType = getCatchableTypeArrayType(NumEntries);
  toolchain::Constant *Fields[] = {
      toolchain::ConstantInt::get(CGM.IntTy, NumEntries), // NumEntries
      toolchain::ConstantArray::get(
          AT, toolchain::ArrayRef(CatchableTypes.begin(),
                             CatchableTypes.end())) // CatchableTypes
  };
  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    getMangleContext().mangleCXXCatchableTypeArray(T, NumEntries, Out);
  }
  CTA = new toolchain::GlobalVariable(
      CGM.getModule(), CTAType, /*isConstant=*/true, getLinkageForRTTI(T),
      toolchain::ConstantStruct::get(CTAType, Fields), MangledName);
  CTA->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);
  CTA->setSection(".xdata");
  if (CTA->isWeakForLinker())
    CTA->setComdat(CGM.getModule().getOrInsertComdat(CTA->getName()));
  return CTA;
}

toolchain::GlobalVariable *MicrosoftCXXABI::getThrowInfo(QualType T) {
  bool IsConst, IsVolatile, IsUnaligned;
  T = decomposeTypeForEH(getContext(), T, IsConst, IsVolatile, IsUnaligned);

  // The CatchableTypeArray enumerates the various (CV-unqualified) types that
  // the exception object may be caught as.
  toolchain::GlobalVariable *CTA = getCatchableTypeArray(T);
  // The first field in a CatchableTypeArray is the number of CatchableTypes.
  // This is used as a component of the mangled name which means that we need to
  // know what it is in order to see if we have previously generated the
  // ThrowInfo.
  uint32_t NumEntries =
      cast<toolchain::ConstantInt>(CTA->getInitializer()->getAggregateElement(0U))
          ->getLimitedValue();

  SmallString<256> MangledName;
  {
    toolchain::raw_svector_ostream Out(MangledName);
    getMangleContext().mangleCXXThrowInfo(T, IsConst, IsVolatile, IsUnaligned,
                                          NumEntries, Out);
  }

  // Reuse a previously generated ThrowInfo if we have generated an appropriate
  // one before.
  if (toolchain::GlobalVariable *GV = CGM.getModule().getNamedGlobal(MangledName))
    return GV;

  // The RTTI TypeDescriptor uses an unqualified type but catch clauses must
  // be at least as CV qualified.  Encode this requirement into the Flags
  // bitfield.
  uint32_t Flags = 0;
  if (IsConst)
    Flags |= 1;
  if (IsVolatile)
    Flags |= 2;
  if (IsUnaligned)
    Flags |= 4;

  // The cleanup-function (a destructor) must be called when the exception
  // object's lifetime ends.
  toolchain::Constant *CleanupFn = toolchain::Constant::getNullValue(CGM.Int8PtrTy);
  if (const CXXRecordDecl *RD = T->getAsCXXRecordDecl())
    if (CXXDestructorDecl *DtorD = RD->getDestructor())
      if (!DtorD->isTrivial())
        CleanupFn = CGM.getAddrOfCXXStructor(GlobalDecl(DtorD, Dtor_Complete));
  // This is unused as far as we can tell, initialize it to null.
  toolchain::Constant *ForwardCompat =
      getImageRelativeConstant(toolchain::Constant::getNullValue(CGM.Int8PtrTy));
  toolchain::Constant *PointerToCatchableTypes = getImageRelativeConstant(CTA);
  toolchain::StructType *TIType = getThrowInfoType();
  toolchain::Constant *Fields[] = {
      toolchain::ConstantInt::get(CGM.IntTy, Flags), // Flags
      getImageRelativeConstant(CleanupFn),      // CleanupFn
      ForwardCompat,                            // ForwardCompat
      PointerToCatchableTypes                   // CatchableTypeArray
  };
  auto *GV = new toolchain::GlobalVariable(
      CGM.getModule(), TIType, /*isConstant=*/true, getLinkageForRTTI(T),
      toolchain::ConstantStruct::get(TIType, Fields), MangledName.str());
  GV->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);
  GV->setSection(".xdata");
  if (GV->isWeakForLinker())
    GV->setComdat(CGM.getModule().getOrInsertComdat(GV->getName()));
  return GV;
}

void MicrosoftCXXABI::emitThrow(CodeGenFunction &CGF, const CXXThrowExpr *E) {
  const Expr *SubExpr = E->getSubExpr();
  assert(SubExpr && "SubExpr cannot be null");
  QualType ThrowType = SubExpr->getType();
  // The exception object lives on the stack and it's address is passed to the
  // runtime function.
  Address AI = CGF.CreateMemTemp(ThrowType);
  CGF.EmitAnyExprToMem(SubExpr, AI, ThrowType.getQualifiers(),
                       /*IsInit=*/true);

  // The so-called ThrowInfo is used to describe how the exception object may be
  // caught.
  toolchain::GlobalVariable *TI = getThrowInfo(ThrowType);

  // Call into the runtime to throw the exception.
  toolchain::Value *Args[] = {AI.emitRawPointer(CGF), TI};
  CGF.EmitNoreturnRuntimeCallOrInvoke(getThrowFn(), Args);
}

std::pair<toolchain::Value *, const CXXRecordDecl *>
MicrosoftCXXABI::LoadVTablePtr(CodeGenFunction &CGF, Address This,
                               const CXXRecordDecl *RD) {
  CanQualType T = CGF.getContext().getCanonicalTagType(RD);
  std::tie(This, std::ignore, RD) = performBaseAdjustment(CGF, This, T);
  return {CGF.GetVTablePtr(This, CGM.Int8PtrTy, RD), RD};
}

bool MicrosoftCXXABI::isPermittedToBeHomogeneousAggregate(
    const CXXRecordDecl *RD) const {
  // All aggregates are permitted to be HFA on non-ARM platforms, which mostly
  // affects vectorcall on x64/x86.
  if (!CGM.getTarget().getTriple().isAArch64())
    return true;
  // MSVC Windows on Arm64 has its own rules for determining if a type is HFA
  // that are inconsistent with the AAPCS64 ABI. The following are our best
  // determination of those rules so far, based on observation of MSVC's
  // behavior.
  if (RD->isEmpty())
    return false;
  if (RD->isPolymorphic())
    return false;
  if (RD->hasNonTrivialCopyAssignment())
    return false;
  if (RD->hasNonTrivialDestructor())
    return false;
  if (RD->hasNonTrivialDefaultConstructor())
    return false;
  // These two are somewhat redundant given the caller
  // (ABIInfo::isHomogeneousAggregate) checks the bases and fields, but that
  // caller doesn't consider empty bases/fields to be non-homogenous, but it
  // looks like Microsoft's AArch64 ABI does care about these empty types &
  // anything containing/derived from one is non-homogeneous.
  // Instead we could add another CXXABI entry point to query this property and
  // have ABIInfo::isHomogeneousAggregate use that property.
  // I don't think any other of the features listed above could be true of a
  // base/field while not true of the outer struct. For example, if you have a
  // base/field that has an non-trivial copy assignment/dtor/default ctor, then
  // the outer struct's corresponding operation must be non-trivial.
  for (const CXXBaseSpecifier &B : RD->bases()) {
    if (const CXXRecordDecl *FRD = B.getType()->getAsCXXRecordDecl()) {
      if (!isPermittedToBeHomogeneousAggregate(FRD))
        return false;
    }
  }
  // empty fields seem to be caught by the ABIInfo::isHomogeneousAggregate
  // checking for padding - but maybe there are ways to end up with an empty
  // field without padding? Not that I know of, so don't check fields here &
  // rely on the padding check.
  return true;
}
