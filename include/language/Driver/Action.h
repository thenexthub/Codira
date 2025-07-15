//===--- Action.h - Abstract compilation steps ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_DRIVER_ACTION_H
#define LANGUAGE_DRIVER_ACTION_H

#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "language/Driver/Util.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include "toolchain/Support/Chrono.h"

namespace toolchain {
namespace opt {
  class Arg;
}
}

namespace language {
namespace driver {
  class Action;

class Action {
public:
  using size_type = ArrayRef<const Action *>::size_type;
  using iterator = ArrayRef<const Action *>::iterator;
  using const_iterator = ArrayRef<const Action *>::const_iterator;

  enum class Kind : unsigned {
    Input = 0,
    CompileJob,
    InterpretJob,
    BackendJob,
    MergeModuleJob,
    ModuleWrapJob,
    AutolinkExtractJob,
    REPLJob,
    DynamicLinkJob,
    StaticLinkJob,
    GenerateDSYMJob,
    VerifyDebugInfoJob,
    GeneratePCHJob,
    VerifyModuleInterfaceJob,

    JobFirst = CompileJob,
    JobLast = VerifyModuleInterfaceJob
  };

  static const char *getClassName(Kind AC);

private:
  unsigned RawKind : 4;
  unsigned Type : 28;

  friend class Compilation;
  /// Actions must be created through Compilation::createAction.
  void *operator new(size_t size) { return ::operator new(size); };

protected:
  Action(Kind K, file_types::ID Type)
      : RawKind(unsigned(K)), Type(Type) {
    assert(K == getKind() && "not enough bits");
    assert(Type == getType() && "not enough bits");
  }

public:
  virtual ~Action() = default;

  const char *getClassName() const { return Action::getClassName(getKind()); }

  Kind getKind() const { return static_cast<Kind>(RawKind); }
  file_types::ID getType() const { return static_cast<file_types::ID>(Type); }
};

class InputAction : public Action {
  virtual void anchor();
  const toolchain::opt::Arg &Input;

public:
  InputAction(const toolchain::opt::Arg &Input, file_types::ID Type)
      : Action(Action::Kind::Input, Type), Input(Input) {}
  const toolchain::opt::Arg &getInputArg() const { return Input; }

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::Input;
  }
};

class JobAction : public Action {
  TinyPtrVector<const Action *> Inputs;
  virtual void anchor();
protected:
  JobAction(Kind Kind, ArrayRef<const Action *> Inputs,
            file_types::ID Type)
      : Action(Kind, Type), Inputs(Inputs) {}

public:
  ArrayRef<const Action *> getInputs() const { return Inputs; }
  void addInput(const Action *Input) { Inputs.push_back(Input); }

  size_type size() const { return Inputs.size(); }

  iterator begin() { return Inputs.begin(); }
  iterator end() { return Inputs.end(); }
  const_iterator begin() const { return Inputs.begin(); }
  const_iterator end() const { return Inputs.end(); }

  // Returns the index of the Input action's output file which is used as
  // (single) input to this action. Most actions produce only a single output
  // file, so we return 0 by default.
  virtual size_t getInputIndex() const { return 0; }

  static bool classof(const Action *A) {
    return (A->getKind() >= Kind::JobFirst &&
            A->getKind() <= Kind::JobLast);
  }
};


class CompileJobAction : public JobAction {
private:
  virtual void anchor() override;

public:
  CompileJobAction(file_types::ID OutputType)
      : JobAction(Action::Kind::CompileJob, std::nullopt, OutputType) {}
  CompileJobAction(Action *Input, file_types::ID OutputType)
      : JobAction(Action::Kind::CompileJob, Input, OutputType) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::CompileJob;
  }

  /// Return a _single_ TY_Codira InputAction, if one exists;
  /// if 0 or >1 such inputs exist, return nullptr.
  const InputAction *findSingleCodiraInput() const {
    auto Inputs = getInputs();
    const InputAction *IA = nullptr;
    for (auto const *I : Inputs) {
      if (auto const *S = dyn_cast<InputAction>(I)) {
        if (S->getType() == file_types::TY_Codira) {
          if (IA == nullptr) {
            IA = S;
          } else {
            // Already found one, two is too many.
            return nullptr;
          }
        }
      }
    }
    return IA;
  }
};

class InterpretJobAction : public JobAction {
private:
  virtual void anchor() override;

public:
  explicit InterpretJobAction()
      : JobAction(Action::Kind::InterpretJob, std::nullopt,
                  file_types::TY_Nothing) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::InterpretJob;
  }
};

class BackendJobAction : public JobAction {
private:
  virtual void anchor() override;
  
  // In case of multi-threaded compilation, the compile-action produces multiple
  // output bitcode-files. For each bitcode-file a BackendJobAction is created.
  // This index specifies which of the files to select for the input.
  size_t InputIndex;
public:
  BackendJobAction(const Action *Input, file_types::ID OutputType,
                   size_t InputIndex)
      : JobAction(Action::Kind::BackendJob, Input, OutputType),
        InputIndex(InputIndex) {}
  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::BackendJob;
  }
  
  virtual size_t getInputIndex() const override { return InputIndex; }
};

class REPLJobAction : public JobAction {
public:
  enum class Mode {
    Integrated,
    PreferLLDB,
    RequireLLDB
  };
private:
  virtual void anchor() override;
  Mode RequestedMode;
public:
  REPLJobAction(Mode mode)
      : JobAction(Action::Kind::REPLJob, std::nullopt, file_types::TY_Nothing),
        RequestedMode(mode) {}

  Mode getRequestedMode() const { return RequestedMode; }

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::REPLJob;
  }
};

class MergeModuleJobAction : public JobAction {
  virtual void anchor() override;
public:
  MergeModuleJobAction(ArrayRef<const Action *> Inputs)
      : JobAction(Action::Kind::MergeModuleJob, Inputs,
                  file_types::TY_CodiraModuleFile) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::MergeModuleJob;
  }
};

class ModuleWrapJobAction : public JobAction {
  virtual void anchor() override;
public:
  ModuleWrapJobAction(ArrayRef<const Action *> Inputs)
      : JobAction(Action::Kind::ModuleWrapJob, Inputs,
                  file_types::TY_Object) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::ModuleWrapJob;
  }
};

class AutolinkExtractJobAction : public JobAction {
  virtual void anchor() override;
public:
  AutolinkExtractJobAction(ArrayRef<const Action *> Inputs)
      : JobAction(Action::Kind::AutolinkExtractJob, Inputs,
                  file_types::TY_AutolinkFile) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::AutolinkExtractJob;
  }
};

class GenerateDSYMJobAction : public JobAction {
  virtual void anchor() override;
public:
  explicit GenerateDSYMJobAction(const Action *Input)
      : JobAction(Action::Kind::GenerateDSYMJob, Input,
                  file_types::TY_dSYM) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::GenerateDSYMJob;
  }
};

class VerifyDebugInfoJobAction : public JobAction {
  virtual void anchor() override;
public:
  explicit VerifyDebugInfoJobAction(const Action *Input)
      : JobAction(Action::Kind::VerifyDebugInfoJob, Input,
                  file_types::TY_Nothing) {}

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::VerifyDebugInfoJob;
  }
};
  
class GeneratePCHJobAction : public JobAction {
  std::string PersistentPCHDir;

  virtual void anchor() override;
public:
  GeneratePCHJobAction(const Action *Input, StringRef persistentPCHDir)
      : JobAction(Action::Kind::GeneratePCHJob, Input,
                  persistentPCHDir.empty() ? file_types::TY_PCH
                                           : file_types::TY_Nothing),
        PersistentPCHDir(persistentPCHDir) {}

  bool isPersistentPCH() const { return !PersistentPCHDir.empty(); }
  StringRef getPersistentPCHDir() const { return PersistentPCHDir; }

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::GeneratePCHJob;
  }
};

class DynamicLinkJobAction : public JobAction {
  virtual void anchor() override;
  LinkKind Kind;
  bool ShouldPerformLTO;

public:
  DynamicLinkJobAction(ArrayRef<const Action *> Inputs, LinkKind K,
                       bool ShouldPerformLTO)
      : JobAction(Action::Kind::DynamicLinkJob, Inputs, file_types::TY_Image),
        Kind(K), ShouldPerformLTO(ShouldPerformLTO) {
    assert(Kind != LinkKind::None && Kind != LinkKind::StaticLibrary);
  }

  LinkKind getKind() const { return Kind; }

  bool shouldPerformLTO() const { return ShouldPerformLTO; }

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::DynamicLinkJob;
  }
};

class StaticLinkJobAction : public JobAction {
  virtual void anchor() override;

public:
  StaticLinkJobAction(ArrayRef<const Action *> Inputs, LinkKind K)
      : JobAction(Action::Kind::StaticLinkJob, Inputs, file_types::TY_Image) {
    assert(K == LinkKind::StaticLibrary);
  }

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::StaticLinkJob;
  }
};

class VerifyModuleInterfaceJobAction : public JobAction {
  virtual void anchor() override;
  file_types::ID inputType;

public:
  VerifyModuleInterfaceJobAction(const Action * ModuleEmitter,
                                 file_types::ID inputType)
    : JobAction(Action::Kind::VerifyModuleInterfaceJob, { ModuleEmitter },
                file_types::TY_Nothing), inputType(inputType) {
    assert(inputType == file_types::TY_CodiraModuleInterfaceFile ||
           inputType == file_types::TY_PrivateCodiraModuleInterfaceFile ||
           inputType == file_types::TY_PackageCodiraModuleInterfaceFile);
  }

  file_types::ID getInputType() const { return inputType; }

  static bool classof(const Action *A) {
    return A->getKind() == Action::Kind::VerifyModuleInterfaceJob;
  }
};

} // end namespace driver
} // end namespace language

#endif
