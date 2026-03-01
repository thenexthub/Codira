//===-- MCODEIT.h - Class definition for the MCODEIT ----------------*- C++ -*-===//
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
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_EXECUTIONENGINE_MCODEIT_MCODEIT_H
#define LLVM_LIB_EXECUTIONENGINE_MCODEIT_MCODEIT_H

#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ExecutionEngine/ExecutionEngine.h"
#include "vm/core/ExecutionEngine/RTDyldMemoryManager.h"
#include "vm/core/ExecutionEngine/RuntimeDyld.h"

namespace vm::core {
class MCODEIT;
class Module;
class ObjectCache;

// This is a helper class that the MCODEIT execution engine uses for linking
// functions across modules that it owns.  It aggregates the memory manager
// that is passed in to the MCODEIT constructor and defers most functionality
// to that object.
class LinkingSymbolResolver : public LegacyJITSymbolResolver {
public:
  LinkingSymbolResolver(MCODEIT &Parent,
                        std::shared_ptr<LegacyJITSymbolResolver> Resolver)
      : ParentEngine(Parent), ClientResolver(std::move(Resolver)) {}

  JITSymbol findSymbol(const std::string &Name) override;

  // MCODEIT doesn't support logical dylibs.
  JITSymbol findSymbolInLogicalDylib(const std::string &Name) override {
    return nullptr;
  }

private:
  MCODEIT &ParentEngine;
  std::shared_ptr<LegacyJITSymbolResolver> ClientResolver;
  void anchor() override;
};

// About Module states: added->loaded->finalized.
//
// The purpose of the "added" state is having modules in standby. (added=known
// but not compiled). The idea is that you can add a module to provide function
// definitions but if nothing in that module is referenced by a module in which
// a function is executed (note the wording here because it's not exactly the
// ideal case) then the module never gets compiled. This is sort of lazy
// compilation.
//
// The purpose of the "loaded" state (loaded=compiled and required sections
// copied into local memory but not yet ready for execution) is to have an
// intermediate state wherein clients can remap the addresses of sections, using
// MCODEIT::mapSectionAddress, (in preparation for later copying to a new location
// or an external process) before relocations and page permissions are applied.
//
// It might not be obvious at first glance, but the "remote-mcodeit" case in the
// lli tool does this.  In that case, the intermediate action is taken by the
// RemoteMemoryManager in response to the notifyObjectLoaded function being
// called.

class MCODEIT : public ExecutionEngine {
  MCODEIT(std::unique_ptr<Module> M, std::unique_ptr<TargetMachine> tm,
        std::shared_ptr<MCODEITMemoryManager> MemMgr,
        std::shared_ptr<LegacyJITSymbolResolver> Resolver);

  typedef toolchain::SmallPtrSet<Module *, 4> ModulePtrSet;

  class OwningModuleContainer {
  public:
    OwningModuleContainer() = default;
    ~OwningModuleContainer() {
      freeModulePtrSet(AddedModules);
      freeModulePtrSet(LoadedModules);
      freeModulePtrSet(FinalizedModules);
    }

    ModulePtrSet::iterator begin_added() { return AddedModules.begin(); }
    ModulePtrSet::iterator end_added() { return AddedModules.end(); }
    iterator_range<ModulePtrSet::iterator> added() {
      return make_range(begin_added(), end_added());
    }

    ModulePtrSet::iterator begin_loaded() { return LoadedModules.begin(); }
    ModulePtrSet::iterator end_loaded() { return LoadedModules.end(); }

    ModulePtrSet::iterator begin_finalized() { return FinalizedModules.begin(); }
    ModulePtrSet::iterator end_finalized() { return FinalizedModules.end(); }

    void addModule(std::unique_ptr<Module> M) {
      AddedModules.insert(M.release());
    }

    bool removeModule(Module *M) {
      return AddedModules.erase(M) || LoadedModules.erase(M) ||
             FinalizedModules.erase(M);
    }

    bool hasModuleBeenAddedButNotLoaded(Module *M) {
      return AddedModules.contains(M);
    }

    bool hasModuleBeenLoaded(Module *M) {
      // If the module is in either the "loaded" or "finalized" sections it
      // has been loaded.
      return LoadedModules.contains(M) || FinalizedModules.contains(M);
    }

    bool hasModuleBeenFinalized(Module *M) {
      return FinalizedModules.contains(M);
    }

    bool ownsModule(Module* M) {
      return AddedModules.contains(M) || LoadedModules.contains(M) ||
             FinalizedModules.contains(M);
    }

    void markModuleAsLoaded(Module *M) {
      // This checks against logic errors in the MCODEIT implementation.
      // This function should never be called with either a Module that MCODEIT
      // does not own or a Module that has already been loaded and/or finalized.
      assert(AddedModules.count(M) &&
             "markModuleAsLoaded: Module not found in AddedModules");

      // Remove the module from the "Added" set.
      AddedModules.erase(M);

      // Add the Module to the "Loaded" set.
      LoadedModules.insert(M);
    }

    void markModuleAsFinalized(Module *M) {
      // This checks against logic errors in the MCODEIT implementation.
      // This function should never be called with either a Module that MCODEIT
      // does not own, a Module that has not been loaded or a Module that has
      // already been finalized.
      assert(LoadedModules.count(M) &&
             "markModuleAsFinalized: Module not found in LoadedModules");

      // Remove the module from the "Loaded" section of the list.
      LoadedModules.erase(M);

      // Add the Module to the "Finalized" section of the list by inserting it
      // before the 'end' iterator.
      FinalizedModules.insert(M);
    }

    void markAllLoadedModulesAsFinalized() {
      FinalizedModules.insert_range(LoadedModules);
      LoadedModules.clear();
    }

  private:
    ModulePtrSet AddedModules;
    ModulePtrSet LoadedModules;
    ModulePtrSet FinalizedModules;

    void freeModulePtrSet(ModulePtrSet& MPS) {
      // Go through the module set and delete everything.
      for (Module *M : MPS)
        delete M;
      MPS.clear();
    }
  };

  std::unique_ptr<TargetMachine> TM;
  MCContext *Ctx;
  std::shared_ptr<MCODEITMemoryManager> MemMgr;
  LinkingSymbolResolver Resolver;
  RuntimeDyld Dyld;
  std::vector<JITEventListener*> EventListeners;

  OwningModuleContainer OwnedModules;

  SmallVector<object::OwningBinary<object::Archive>, 2> Archives;
  SmallVector<std::unique_ptr<MemoryBuffer>, 2> Buffers;

  SmallVector<std::unique_ptr<object::ObjectFile>, 2> LoadedObjects;

  // An optional ObjectCache to be notified of compiled objects and used to
  // perform lookup of pre-compiled code to avoid re-compilation.
  ObjectCache *ObjCache;

  Function *FindFunctionNamedInModulePtrSet(StringRef FnName,
                                            ModulePtrSet::iterator I,
                                            ModulePtrSet::iterator E);

  GlobalVariable *FindGlobalVariableNamedInModulePtrSet(StringRef Name,
                                                        bool AllowInternal,
                                                        ModulePtrSet::iterator I,
                                                        ModulePtrSet::iterator E);

  void runStaticConstructorsDestructorsInModulePtrSet(bool isDtors,
                                                      ModulePtrSet::iterator I,
                                                      ModulePtrSet::iterator E);

public:
  ~MCODEIT() override;

  /// @name ExecutionEngine interface implementation
  /// @{
  void addModule(std::unique_ptr<Module> M) override;
  void addObjectFile(std::unique_ptr<object::ObjectFile> O) override;
  void addObjectFile(object::OwningBinary<object::ObjectFile> O) override;
  void addArchive(object::OwningBinary<object::Archive> O) override;
  bool removeModule(Module *M) override;

  /// FindFunctionNamed - Search all of the active modules to find the function that
  /// defines FnName.  This is very slow operation and shouldn't be used for
  /// general code.
  Function *FindFunctionNamed(StringRef FnName) override;

  /// FindGlobalVariableNamed - Search all of the active modules to find the
  /// global variable that defines Name.  This is very slow operation and
  /// shouldn't be used for general code.
  GlobalVariable *FindGlobalVariableNamed(StringRef Name,
                                          bool AllowInternal = false) override;

  /// Sets the object manager that MCODEIT should use to avoid compilation.
  void setObjectCache(ObjectCache *manager) override;

  void setProcessAllSections(bool ProcessAllSections) override {
    Dyld.setProcessAllSections(ProcessAllSections);
  }

  void generateCodeForModule(Module *M) override;

  /// finalizeObject - ensure the module is fully processed and is usable.
  ///
  /// It is the user-level function for completing the process of making the
  /// object usable for execution. It should be called after sections within an
  /// object have been relocated using mapSectionAddress.  When this method is
  /// called the MCODEIT execution engine will reapply relocations for a loaded
  /// object.
  /// Is it OK to finalize a set of modules, add modules and finalize again.
  // FIXME: Do we really need both of these?
  void finalizeObject() override;
  virtual void finalizeModule(Module *);
  void finalizeLoadedModules();

  /// runStaticConstructorsDestructors - This method is used to execute all of
  /// the static constructors or destructors for a program.
  ///
  /// \param isDtors - Run the destructors instead of constructors.
  void runStaticConstructorsDestructors(bool isDtors) override;

  void *getPointerToFunction(Function *F) override;

  GenericValue runFunction(Function *F,
                           ArrayRef<GenericValue> ArgValues) override;

  /// getPointerToNamedFunction - This method returns the address of the
  /// specified function by using the dlsym function call.  As such it is only
  /// useful for resolving library symbols, not code generated symbols.
  ///
  /// If AbortOnFailure is false and no function with the given name is
  /// found, this function silently returns a null pointer. Otherwise,
  /// it prints a message to stderr and aborts.
  ///
  void *getPointerToNamedFunction(StringRef Name,
                                  bool AbortOnFailure = true) override;

  /// mapSectionAddress - map a section to its target address space value.
  /// Map the address of a JIT section as returned from the memory manager
  /// to the address in the target process as the running code will see it.
  /// This is the address which will be used for relocation resolution.
  void mapSectionAddress(const void *LocalAddress,
                         uint64_t TargetAddress) override {
    Dyld.mapSectionAddress(LocalAddress, TargetAddress);
  }
  void RegisterJITEventListener(JITEventListener *L) override;
  void UnregisterJITEventListener(JITEventListener *L) override;

  // If successful, these function will implicitly finalize all loaded objects.
  // To get a function address within MCODEIT without causing a finalize, use
  // getSymbolAddress.
  uint64_t getGlobalValueAddress(const std::string &Name) override;
  uint64_t getFunctionAddress(const std::string &Name) override;

  TargetMachine *getTargetMachine() override { return TM.get(); }

  /// @}
  /// @name (Private) Registration Interfaces
  /// @{

  static void Register() {
    MCODEITCtor = createJIT;
  }

  static ExecutionEngine *
  createJIT(std::unique_ptr<Module> M, std::string *ErrorStr,
            std::shared_ptr<MCODEITMemoryManager> MemMgr,
            std::shared_ptr<LegacyJITSymbolResolver> Resolver,
            std::unique_ptr<TargetMachine> TM);

  // @}

  // Takes a mangled name and returns the corresponding JITSymbol (if a
  // definition of that mangled name has been added to the JIT).
  JITSymbol findSymbol(const std::string &Name, bool CheckFunctionsOnly);

  // DEPRECATED - Please use findSymbol instead.
  //
  // This is not directly exposed via the ExecutionEngine API, but it is
  // used by the LinkingMemoryManager.
  //
  // getSymbolAddress takes an unmangled name and returns the corresponding
  // JITSymbol if a definition of the name has been added to the JIT.
  uint64_t getSymbolAddress(const std::string &Name,
                            bool CheckFunctionsOnly);

protected:
  /// emitObject -- Generate a JITed object in memory from the specified module
  /// Currently, MCODEIT only supports a single module and the module passed to
  /// this function call is expected to be the contained module.  The module
  /// is passed as a parameter here to prepare for multiple module support in
  /// the future.
  std::unique_ptr<MemoryBuffer> emitObject(Module *M);

  void notifyObjectLoaded(const object::ObjectFile &Obj,
                          const RuntimeDyld::LoadedObjectInfo &L);
  void notifyFreeingObject(const object::ObjectFile &Obj);

  JITSymbol findExistingSymbol(const std::string &Name);
  Module *findModuleForSymbol(const std::string &Name, bool CheckFunctionsOnly);
};

} // end toolchain namespace

#endif // LLVM_LIB_EXECUTIONENGINE_MCODEIT_MCODEIT_H
