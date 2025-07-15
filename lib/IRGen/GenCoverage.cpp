//===--- GenCoverage.cpp - IR Generation for coverage ---------------------===//
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
//  This file implements IR generation for the initialization of
//  coverage related variables.
//
//===----------------------------------------------------------------------===//

#include "IRGenModule.h"
#include "CodiraTargetInfo.h"

#include "language/AST/IRGenOptions.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILModule.h"
#include "toolchain/IR/Constants.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IR/Type.h"
#include "toolchain/ProfileData/Coverage/CoverageMappingWriter.h"
#include "toolchain/ProfileData/InstrProf.h"
#include "toolchain/Support/FileSystem.h"

using namespace language;
using namespace irgen;

using toolchain::coverage::CounterMappingRegion;
using toolchain::coverage::CovMapVersion;

// This affects the coverage mapping format defined when `InstrProfData.inc`
// is textually included. Note that it means 'version >= 3', not 'version == 3'.
#define COVMAP_V3

/// This assert is here to make sure we make all the necessary code generation
/// changes that are needed to support the new coverage mapping format. Note we
/// cannot pin our version, as it must remain in sync with the version Clang is
/// using.
/// Do not bump without at least filing a bug and pinging a coverage maintainer.
static_assert(CovMapVersion::CurrentVersion == CovMapVersion::Version7,
              "Coverage mapping emission needs updating");

static std::string getInstrProfSection(IRGenModule &IGM,
                                       toolchain::InstrProfSectKind SK) {
  return toolchain::getInstrProfSectionName(SK, IGM.Triple.getObjectFormat());
}

void IRGenModule::emitCoverageMaps(ArrayRef<const SILCoverageMap *> Mappings) {
  // If there aren't any coverage maps, there's nothing to emit.
  if (Mappings.empty())
    return;

  SmallVector<toolchain::Constant *, 4> UnusedFuncNames;
  for (const auto *Mapping : Mappings) {
    auto FuncName = Mapping->getPGOFuncName();
    auto VarLinkage = toolchain::GlobalValue::LinkOnceAnyLinkage;
    auto FuncNameVarName = toolchain::getPGOFuncNameVarName(FuncName, VarLinkage);

    // Check whether this coverage mapping can reference its name data within
    // the profile symbol table. If the name global isn't there, this function
    // has been optimized out. We need to tell LLVM about it by emitting the
    // name data separately.
    if (!Module.getNamedGlobal(FuncNameVarName)) {
      auto *Var = toolchain::createPGOFuncNameVar(Module, VarLinkage, FuncName);
      UnusedFuncNames.push_back(toolchain::ConstantExpr::getBitCast(Var, Int8PtrTy));
    }
  }

  // Emit the name data for any unused functions.
  if (!UnusedFuncNames.empty()) {
    auto NamePtrsTy = toolchain::ArrayType::get(Int8PtrTy, UnusedFuncNames.size());
    auto NamePtrs = toolchain::ConstantArray::get(NamePtrsTy, UnusedFuncNames);

    // Note we don't mark this variable as used, as it doesn't need to be
    // present in the object file, it gets picked up by the LLVM instrumentation
    // lowering pass.
    new toolchain::GlobalVariable(Module, NamePtrsTy, /*IsConstant*/ true,
                             toolchain::GlobalValue::InternalLinkage, NamePtrs,
                             toolchain::getCoverageUnusedNamesVarName());
  }

  toolchain::DenseMap<StringRef, unsigned> RawFileIndices;
  toolchain::SmallVector<StringRef, 8> RawFiles;
  for (const auto &M : Mappings) {
    auto Filename = M->getFilename();
    auto Inserted = RawFileIndices.insert({Filename, RawFiles.size()}).second;
    if (!Inserted)
      continue;
    RawFiles.push_back(Filename);
  }
  const auto &Remapper = getOptions().CoveragePrefixMap;

  toolchain::SmallVector<std::string, 8> FilenameStrs;
  FilenameStrs.reserve(RawFiles.size() + 1);

  // First element needs to be the current working directory. Note if this
  // scheme ever changes, the FileID computation below will need updating.
  SmallString<256> WorkingDirectory;
  toolchain::sys::fs::current_path(WorkingDirectory);
  FilenameStrs.emplace_back(Remapper.remapPath(WorkingDirectory));

  // Following elements are the filenames present. We use their relative path,
  // which toolchain-cov will turn back into absolute paths using the working
  // directory element.
  for (auto Name : RawFiles)
    FilenameStrs.emplace_back(Remapper.remapPath(Name));

  // Encode the filenames.
  std::string Filenames;
  toolchain::LLVMContext &Ctx = getLLVMContext();
  {
    toolchain::raw_string_ostream OS(Filenames);
    toolchain::coverage::CoverageFilenamesSectionWriter(FilenameStrs).write(OS);
  }
  auto *FilenamesVal =
      toolchain::ConstantDataArray::getString(Ctx, Filenames, false);
  const int64_t FilenamesRef = toolchain::IndexedInstrProf::ComputeHash(Filenames);
  const size_t FilenamesSize = Filenames.size();

  // Emit the function records.
  auto *Int32Ty = toolchain::Type::getInt32Ty(Ctx);
  for (const auto &M : Mappings) {
    StringRef NameValue = M->getPGOFuncName();
    assert(!NameValue.empty() && "Expected a named record");
    uint64_t FuncHash = M->getHash();

    const uint64_t NameHash = toolchain::IndexedInstrProf::ComputeHash(NameValue);
    std::string FuncRecordName = "__covrec_" + toolchain::utohexstr(NameHash);

    // The file ID needs to be bumped by 1 to account for the working directory
    // as the first element.
    unsigned FileID = [&]() {
      auto Result = RawFileIndices.find(M->getFilename());
      assert(Result != RawFileIndices.end());
      return Result->second + 1;
    }();
    assert(FileID < FilenameStrs.size());

    std::vector<CounterMappingRegion> Regions;
    for (const auto &MR : M->getMappedRegions()) {
      // The FileID here is 0, because it's an index into VirtualFileMapping,
      // and we only ever have a single file associated for a function.
      Regions.push_back(MR.getLLVMRegion(/*FileID*/ 0));
    }
    // Append each function's regions into the encoded buffer.
    ArrayRef<unsigned> VirtualFileMapping(FileID);
    toolchain::coverage::CoverageMappingWriter W(VirtualFileMapping,
                                            M->getExpressions(), Regions);
    std::string CoverageMapping;
    {
      toolchain::raw_string_ostream OS(CoverageMapping);
      W.write(OS);
    }

#define COVMAP_FUNC_RECORD(Type, LLVMType, Name, Init) LLVMType,
    toolchain::Type *FunctionRecordTypes[] = {
#include "toolchain/ProfileData/InstrProfData.inc"
    };
    auto *FunctionRecordTy =
        toolchain::StructType::get(Ctx, toolchain::ArrayRef(FunctionRecordTypes),
                              /*isPacked=*/true);

    // Create the function record constant.
#define COVMAP_FUNC_RECORD(Type, LLVMType, Name, Init) Init,
    toolchain::Constant *FunctionRecordVals[] = {
#include "toolchain/ProfileData/InstrProfData.inc"
    };
    auto *FuncRecordConstant = toolchain::ConstantStruct::get(
        FunctionRecordTy, toolchain::ArrayRef(FunctionRecordVals));

    // Create the function record global.
    auto *FuncRecord = new toolchain::GlobalVariable(
        *getModule(), FunctionRecordTy, /*isConstant=*/true,
        toolchain::GlobalValue::LinkOnceODRLinkage, FuncRecordConstant,
        FuncRecordName);
    FuncRecord->setVisibility(toolchain::GlobalValue::HiddenVisibility);
    FuncRecord->setSection(getInstrProfSection(*this, toolchain::IPSK_covfun));
    FuncRecord->setAlignment(toolchain::Align(8));
    if (Triple.supportsCOMDAT())
      FuncRecord->setComdat(getModule()->getOrInsertComdat(FuncRecordName));

    // Make sure the data doesn't get deleted.
    addUsedGlobal(FuncRecord);
  }

  // Create the coverage data header.
  const unsigned NRecords = 0;
  const unsigned CoverageMappingSize = 0;
  toolchain::Type *CovDataHeaderTypes[] = {
#define COVMAP_HEADER(Type, LLVMType, Name, Init) LLVMType,
#include "toolchain/ProfileData/InstrProfData.inc"
  };
  auto CovDataHeaderTy =
      toolchain::StructType::get(Ctx, toolchain::ArrayRef(CovDataHeaderTypes));
  toolchain::Constant *CovDataHeaderVals[] = {
#define COVMAP_HEADER(Type, LLVMType, Name, Init) Init,
#include "toolchain/ProfileData/InstrProfData.inc"
  };
  auto CovDataHeaderVal = toolchain::ConstantStruct::get(
      CovDataHeaderTy, toolchain::ArrayRef(CovDataHeaderVals));

  // Create the coverage data record
  toolchain::Type *CovDataTypes[] = {CovDataHeaderTy, FilenamesVal->getType()};
  auto CovDataTy = toolchain::StructType::get(Ctx, toolchain::ArrayRef(CovDataTypes));
  toolchain::Constant *TUDataVals[] = {CovDataHeaderVal, FilenamesVal};
  auto CovDataVal =
      toolchain::ConstantStruct::get(CovDataTy, toolchain::ArrayRef(TUDataVals));
  auto CovData = new toolchain::GlobalVariable(
      *getModule(), CovDataTy, true, toolchain::GlobalValue::PrivateLinkage,
      CovDataVal, toolchain::getCoverageMappingVarName());

  CovData->setSection(getInstrProfSection(*this, toolchain::IPSK_covmap));
  CovData->setAlignment(toolchain::Align(8));
  addUsedGlobal(CovData);
}

void IRGenerator::emitCoverageMapping() {
  if (SIL.getCoverageMaps().empty())
    return;

  // Shard the coverage maps across their designated IRGenModules. This is
  // necessary to ensure we don't output N copies of a coverage map when doing
  // parallel IRGen, where N is the number of output object files.
  //
  // Note we don't just dump all the coverage maps into the primary IGM as
  // that would require creating unecessary name data entries, since the name
  // data is likely to already be present in the IGM that contains the entity
  // being profiled (unless it has been optimized out). Matching the coverage
  // map to its originating SourceFile also matches the behavior of a debug
  // build where the files are compiled separately.
  toolchain::DenseMap<IRGenModule *, std::vector<const SILCoverageMap *>> MapsToEmit;
  for (const auto &M : SIL.getCoverageMaps()) {
    auto &Mapping = M.second;
    auto *SF = Mapping->getParentSourceFile();
    MapsToEmit[getGenModule(SF)].push_back(Mapping);
  }
  for (auto &IGMPair : *this) {
    auto *IGM = IGMPair.second;
    IGM->emitCoverageMaps(MapsToEmit[IGM]);
  }
}
