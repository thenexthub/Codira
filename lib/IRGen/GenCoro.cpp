//===---- GenCoro.cpp - Code generation related to coroutines -------------===//
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

#include "language/ABI/MetadataValues.h"
#include "language/Basic/Assertions.h"
#include "language/IRGen/Linking.h"
#include "toolchain/IR/Function.h"
#include "toolchain/IR/Instructions.h"
#include "toolchain/IR/Module.h"

#include "Explosion.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"

using namespace language;
using namespace irgen;

// Add a flag which when set to false forces execution of the open-coded version
// in order to execution test it.
static toolchain::cl::opt<bool> EnableRuntimeTaskDeallocThrough(
    "enable-runtime-task-dealloc-through", toolchain::cl::init(true),
    toolchain::cl::Hidden,
    toolchain::cl::desc(
        "Use the language_task_dealloc_through symbol if it's available"));

namespace {

class GetDeallocThroughFn {
  IRGenModule &IGM;
  class Builder {
    IRGenModule &IGM;
    IRGenFunction &IGF;
    bool isPointer8Bytes;

  public:
    Builder(IRGenFunction &IGF)
        : IGM(IGF.IGM), IGF(IGF),
          isPointer8Bytes(IGM.getPointerSize() == Size(8)) {
      assert(!IGM.getAvailabilityRange().isContainedIn(
                 IGM.Context.getCoroutineAccessorsAvailability()) ||
             !EnableRuntimeTaskDeallocThrough);
    }

    /// Emit the function.
    void build() {
      auto parameters = IGF.collectParameters();
      auto *ptr = parameters.claimNext();

      if (EnableRuntimeTaskDeallocThrough) {
        emitForwardToWeakRuntimeFunction(ptr);
      }

      auto allocationPtrAddr = emitOffsetIntoTask();

      auto *loop = IGF.createBasicBlock("loop");
      IGF.Builder.CreateBr(loop);

      auto *exit = IGF.createBasicBlock("exit");
      emitLoopBlock(loop, allocationPtrAddr, ptr, exit);
      emitExitBlock(exit);
    }

  private:
    /// Check whether the language_task_dealloc_through is available and call it if
    /// so.
    void emitForwardToWeakRuntimeFunction(toolchain::Value *ptr) {
      auto runtimeFnValue = cast<toolchain::Function>(IGM.getTaskDeallocThroughFn());
      runtimeFnValue->setLinkage(toolchain::GlobalValue::ExternalWeakLinkage);
      auto *symbolExists = IGF.Builder.CreateICmpNE(
          runtimeFnValue,
          toolchain::ConstantPointerNull::get(IGF.IGM.Int8Ty->getPointerTo()),
          "runtime_has_dealloc_through");

      auto *noSymbolBlock = IGF.createBasicBlock("no_runtime_symbol");
      auto *hasSymbolBlock = IGF.createBasicBlock("runtime_symbol");

      IGF.Builder.CreateCondBr(symbolExists, hasSymbolBlock, noSymbolBlock);

      IGF.Builder.emitBlock(hasSymbolBlock);
      IGF.Builder.CreateCall(
          FunctionPointer::forDirect(
              FunctionPointer::Kind::Function, runtimeFnValue, nullptr,
              IGM.getTaskDeallocThroughFunctionPointer().getSignature()),
          {ptr});
      IGF.Builder.CreateRetVoid();

      IGF.Builder.emitBlock(noSymbolBlock);
      // The rest of the function will be emitted starting with this block.
    }

    /// Calculate the address (an offset into the current task) of the pointer
    /// to the latest allocation.
    ///
    /// This calculation depends on the deployment target.  If it's new enough
    /// (aligned with at least Codira 5.7), the "new" layouts can be used
    /// unconditionally.  Otherwise, the old layout must be used when the OS is
    /// aligned with a release before Codira 5.7.
    Address emitOffsetIntoTask() {
      auto *task = IGF.getAsyncTask();
      if (!IGM.Context.getCodira57Availability().hasMinimumVersion()) {
        return emitNewLayoutOffsetIntoTask(task);
      }
      auto deploymentRange =
          IGM.Context.getTargetAvailabilityDomain().getDeploymentRange(
              IGM.Context);
      if (!deploymentRange || deploymentRange->isContainedIn(
                                  IGM.Context.getCodira57Availability())) {
        return emitNewLayoutOffsetIntoTask(task);
      }

      auto *oldBlock = IGF.createBasicBlock("old_layout");
      auto *newBlock = IGF.createBasicBlock("new_layout");
      auto *mergeBlock = IGF.createBasicBlock("merge_layout");

      auto *isAtLeast57 = emitCodira57VersionCheck();
      IGF.Builder.CreateCondBr(isAtLeast57, newBlock, oldBlock);

      IGF.Builder.emitBlock(oldBlock);
      auto oldOffset = emitOldLayoutOffsetIntoTask(task);
      IGF.Builder.CreateBr(mergeBlock);

      IGF.Builder.emitBlock(newBlock);
      auto newOffset = emitNewLayoutOffsetIntoTask(task);
      IGF.Builder.CreateBr(mergeBlock);
      auto *finalNewBlock = IGF.Builder.GetInsertBlock();

      IGF.Builder.emitBlock(mergeBlock);
      auto offsetPhi = IGF.Builder.CreatePHI(newOffset.getType(), 2);
      offsetPhi->addIncoming(oldOffset.getAddress(), oldBlock);
      offsetPhi->addIncoming(newOffset.getAddress(), finalNewBlock);
      auto addressPhi =
          Address(offsetPhi, oldOffset.getType(), oldOffset.getAlignment());
      return addressPhi;
    }

    /// Given that the OS is aligned with a release before Codira 5.7, calculate
    /// the address (an offset from \p task) of the pointer to the latest
    /// allocation.
    Address emitOldLayoutOffsetIntoTask(toolchain::Value *task) {
      if (!isPointer8Bytes) {
        return computeOffsetIntoTask(
            task, getTaskLayoutInfo(PrivateLayout::Old32Bit));
      }
      return computeOffsetIntoTask(task,
                                   getTaskLayoutInfo(PrivateLayout::Old64Bit));
    }

    /// Given that the OS is aligned with at least Codira 5.7, calculate the
    /// address (an offset from \p task) of the pointer to the latest
    /// allocation.
    Address emitNewLayoutOffsetIntoTask(toolchain::Value *task) {
      if (!isPointer8Bytes) {
        return emitNew32BitOffsetIntoTask(task);
      }
      return computeOffsetIntoTask(task,
                                   getTaskLayoutInfo(PrivateLayout::New64Bit));
    }

    /// Given that the target is 32-bit and the OS is Codira 5.7 aligned or
    /// greater, calculate the address (an offset from \p task) of the pointer
    /// to the latest allocation.
    ///
    /// This is complicated by the fact that the layout changes depending on
    /// whether LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION is defined.  To
    /// determine this, the existence of
    /// _language_concurrency_debug_supportsPriorityEscalation is checked.
    Address emitNew32BitOffsetIntoTask(toolchain::Value *task) {
      auto *noEscalationBlock = IGF.createBasicBlock("escalation_no");
      auto *yesEscalationBlock = IGF.createBasicBlock("escalation_yes");
      auto *hasEscalationSymbolBlock =
          IGF.createBasicBlock("escalation_has_symbol");
      auto *mergeEscalationBlock = IGF.createBasicBlock("escalation_merge");

      auto *escalationSymbol = IGF.IGM.Module.getOrInsertGlobal(
          "_language_concurrency_debug_supportsPriorityEscalation",
          IGF.IGM.Int8Ty);
      ApplyIRLinkage(IRLinkage::ExternalWeakImport)
          .to(cast<toolchain::GlobalVariable>(escalationSymbol));
      auto *symbolExists = IGF.Builder.CreateICmpNE(
          escalationSymbol,
          toolchain::ConstantPointerNull::get(IGF.IGM.Int8Ty->getPointerTo()),
          "escalation_is_defined");

      IGF.Builder.CreateCondBr(symbolExists, hasEscalationSymbolBlock,
                               noEscalationBlock);
      IGF.Builder.emitBlock(hasEscalationSymbolBlock);
      auto escalationAddr =
          Address(escalationSymbol, IGF.IGM.Int8Ty, Alignment(1));
      auto *escalation = IGF.Builder.CreateLoad(escalationAddr, "escalation");
      auto *escalationIsTrue = IGF.Builder.CreateICmpNE(
          escalation, toolchain::ConstantInt::get(IGF.IGM.Int8Ty, false),
          "escalation_is_true");
      IGF.Builder.CreateCondBr(escalationIsTrue, yesEscalationBlock,
                               noEscalationBlock);

      IGF.Builder.emitBlock(yesEscalationBlock);
      auto escalationOffset = computeOffsetIntoTask(
          task, getTaskLayoutInfo(PrivateLayout::New32BitWithEscalation));
      IGF.Builder.CreateBr(mergeEscalationBlock);

      IGF.Builder.emitBlock(noEscalationBlock);
      auto noEscalationOffset = computeOffsetIntoTask(
          task, getTaskLayoutInfo(PrivateLayout::New32BitSansEscalation));
      IGF.Builder.CreateBr(mergeEscalationBlock);

      IGF.Builder.emitBlock(mergeEscalationBlock);
      auto offsetPhi = IGF.Builder.CreatePHI(noEscalationOffset.getType(), 2);
      offsetPhi->addIncoming(noEscalationOffset.getAddress(),
                             noEscalationBlock);
      offsetPhi->addIncoming(escalationOffset.getAddress(), yesEscalationBlock);
      auto addressPhi = Address(offsetPhi, noEscalationOffset.getType(),
                                noEscalationOffset.getAlignment());
      return addressPhi;
    }

    /// Calculate a bit suitable for a CondBr indicating that the OS is aligned
    /// with Codira 5.7 or greater.
    ///
    /// This is relevant because the layout of Task changed from pre-5.7 to 5.7.
    toolchain::Value *emitCodira57VersionCheck() {
      auto availability = IGM.Context.getCodira57Availability();
      auto deploymentRange =
          IGM.Context.getTargetAvailabilityDomain().getDeploymentRange(
              IGM.Context);
      assert(deploymentRange);
      assert(!deploymentRange->isContainedIn(
          IGM.Context.getCodira57Availability()));
      (void)deploymentRange;
      assert(availability.hasMinimumVersion());
      auto version = availability.getRawMinimumVersion();
      auto *major = getInt32Constant(version.getMajor());
      auto *minor = getInt32Constant(version.getMinor());
      auto *patch = getInt32Constant(version.getSubminor());

      auto *isAtLeastValue =
          IGF.emitTargetOSVersionAtLeastCall(major, minor, patch);

      auto *isAtLeast = IGF.Builder.CreateICmpNE(
          isAtLeastValue, toolchain::Constant::getNullValue(IGM.Int32Ty));

      return isAtLeast;
    }

    toolchain::ConstantInt *getInt32Constant(std::optional<unsigned> value) {
      return toolchain::ConstantInt::get(IGM.Int32Ty, value.value_or(0));
    };

    /// Specifies a layout of the Task runtime struct, specifically just enough
    /// of its private fields in order to find the pointer to the latest
    /// allocation.
    ///
    /// There are two "flavors" of layouts:
    /// Codira 5.6 and before:
    /// class Task {
    ///   AsyncContext * __ptrauth_language_task_resume_context ResumeContext;
    ///
    ///   #if LANGUAGE_POINTER_IS_8_BYTES
    ///   void *Reserved64;
    ///   #endif
    ///
    ///   // May or may not have been visible in Task.h.
    ///   language::atomic<ActiveTaskStatus> Status;
    ///   // Never directly visible in Task.h.
    ///   TaskAllocator Allocator;
    /// }
    /// Codira 5.7 and after:
    /// class Task {
    ///   AsyncContext * __ptrauth_language_task_resume_context ResumeContext;
    ///
    ///   #if LANGUAGE_POINTER_IS_8_BYTES
    ///   void *Reserved64;
    ///   #endif
    ///
    ///   // Always hidden via OpaquePrivateStorage.
    ///   uintptr_t ExclusivityAccessSet[2] = {0, 0};
    ///   // size here was always ((32-bit && escalating) ? 4 : 2) * word_size.
    ///   alignas(ActiveTaskStatus) char StatusStorage[sizeof(ActiveTaskStatus)]
    ///
    ///   TaskAllocator Allocator;
    /// }
    struct PrivateLayout {
      enum Kind {
        Old32Bit,
        Old64Bit,
        New32BitSansEscalation,
        New32BitWithEscalation,
        New64Bit,
      };
      Kind kind;
      PrivateLayout(Kind kind) : kind(kind) {}
      bool isPointer8Bytes() {
        switch (kind) {
        case Old64Bit:
        case New64Bit:
          return true;
        case Old32Bit:
        case New32BitSansEscalation:
        case New32BitWithEscalation:
          return false;
        }
        toolchain_unreachable("covered switch");
      }
      bool isOld() {
        switch (kind) {
        case Old32Bit:
        case Old64Bit:
          return true;
        case New32BitSansEscalation:
        case New32BitWithEscalation:
        case New64Bit:
          return false;
        }
        toolchain_unreachable("covered switch");
      }
      unsigned activeTaskStatusSizeInWords() {
        switch (kind) {
        case Old32Bit:
        case Old64Bit:
          return 2;

        // #if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION &&
        // LANGUAGE_POINTER_IS_4_BYTES #define ACTIVE_TASK_STATUS_SIZE (4 *
        // (sizeof(uintptr_t))) #else #define ACTIVE_TASK_STATUS_SIZE (2 *
        // (sizeof(uintptr_t))) #endif
        case New32BitSansEscalation:
          return 2;
        case New32BitWithEscalation:
          return 4;
        case New64Bit:
          return 2;
        }
        toolchain_unreachable("covered switch");
      }
      struct Field {
        enum Kind {
          RefCountedStruct,
          Int8Ptr,
          Int32,
          FunctionPtr,
          CodiraContextPtr,
          Ptr,
          IntPtr,
          AllocationPtr,
        };
        Kind kind;
        Field(Kind kind) : kind(kind) {}
        Size getSizeInBytes(Size wordSize) {
          switch (kind) {
          case RefCountedStruct:
            return wordSize * 2;
          case Int8Ptr:
          case FunctionPtr:
          case CodiraContextPtr:
          case Ptr:
          case IntPtr:
            return wordSize;
          case Int32:
            return Size(4);
          case AllocationPtr:
            return wordSize;
          }
          toolchain_unreachable("covered switch");
        }
        toolchain::Type *getType(IRGenModule &IGM, toolchain::Type *allocationTy) {
          switch (kind) {
          case RefCountedStruct:
            return IGM.RefCountedStructTy;
          case Int8Ptr:
            return IGM.Int8PtrTy;
          case Int32:
            return IGM.Int32Ty;
          case FunctionPtr:
            return IGM.FunctionPtrTy;
          case CodiraContextPtr:
            return IGM.CodiraContextPtrTy;
          case Ptr:
            return IGM.PtrTy;
          case IntPtr:
            return IGM.IntPtrTy;
          case AllocationPtr:
            return allocationTy->getPointerTo();
          }
          toolchain_unreachable("covered switch");
        }
      };
      template <typename Impl>
      struct FieldVisitor {
        Impl &asImpl() { return *static_cast<Impl *>(this); }
        void visit(PrivateLayout layout) {
          asImpl().visitField(Field::RefCountedStruct); // object header
          asImpl().visitField(Field::Int8Ptr);
          asImpl().visitField(Field::Int8Ptr);     // Job.SchedulerPrivate
          asImpl().visitField(Field::Int32);       // Job.Flags
          asImpl().visitField(Field::Int32);       // Job.ID
          asImpl().visitField(Field::Int8Ptr);     // Job.Voucher
          asImpl().visitField(Field::Int8Ptr);     // Job.Reserved
          asImpl().visitField(Field::FunctionPtr); // Job.RunJob/Job.ResumeTask
          asImpl().visitField(Field::CodiraContextPtr); // Task.ResumeContext
          if (layout.isPointer8Bytes()) {
            // #if LANGUAGE_POINTER_IS_8_BYTES
            //   void *Reserved64;
            // #endif
            asImpl().visitField(Field::Ptr);
          }
          if (layout.isOld()) {
            //// May or may not have been visible in Task.h.
            // language::atomic<ActiveTaskStatus> Status;
            asImpl().visitArray(Field::Ptr,
                                layout.activeTaskStatusSizeInWords());
          } else {
            //// Always hidden via OpaquePrivateStorage.
            // uintptr_t ExclusivityAccessSet[2] = {0, 0};
            asImpl().visitArray(Field::IntPtr, 2);
            //// The size here is always ((32-bit && escalating) ? 4 : 2) *
            /// word_size.
            // alignas(ActiveTaskStatus) char
            // StatusStorage[sizeof(ActiveTaskStatus)];
            asImpl().visitArray(Field::Ptr,
                                layout.activeTaskStatusSizeInWords());
          }
          //// Never directly visible in Task.h.
          // TaskAllocator Allocator;
          //  The first field is always a pointer to the latest allocation.
          asImpl().visitField(Field::AllocationPtr);
        }
      };
      std::string getName() {
        switch (kind) {
        case Old32Bit:
          return "language.back_deploy.task.pre_57";
        case Old64Bit:
          return "language.back_deploy.task.pre_57";
        case New32BitSansEscalation:
          return "language.back_deploy.task.post_57.nonescalating";
        case New32BitWithEscalation:
          return "language.back_deploy.task.post_57.escalating";
        case New64Bit:
          return "language.back_deploy.task.post_57";
        }
        toolchain_unreachable("covered switch");
      };
      void getFields(IRGenModule &IGM, toolchain::Type *allocationTy,
                     SmallVectorImpl<toolchain::Type *> &fieldTys) {
        struct Visitor : PrivateLayout::FieldVisitor<Visitor> {
          SmallVectorImpl<toolchain::Type *> &fieldTys;
          IRGenModule &IGM;
          toolchain::Type *allocationTy;
          Visitor(SmallVectorImpl<toolchain::Type *> &fieldTys, IRGenModule &IGM,
                  toolchain::Type *allocationTy)
              : fieldTys(fieldTys), IGM(IGM), allocationTy(allocationTy) {}

          void visitField(PrivateLayout::Field field) {
            fieldTys.push_back(field.getType(IGM, allocationTy));
          };
          void visitArray(PrivateLayout::Field field, unsigned count) {
            fieldTys.push_back(
                toolchain::ArrayType::get(field.getType(IGM, allocationTy), count));
          }
        };

        Visitor visitor(fieldTys, IGM, allocationTy);
        visitor.visit(*this);
      }
      unsigned getAllocatorIndex() {
        struct Visitor : FieldVisitor<Visitor> {
          unsigned index = 0;
          void visitField(PrivateLayout::Field field) {
            if (field.kind == Field::AllocationPtr)
              return;
            index++;
          };
          void visitArray(PrivateLayout::Field field, unsigned count) {
            index++;
          }
        };
        Visitor visitor;
        visitor.visit(*this);
        return visitor.index;
      }
      Size getAllocatorOffset(IRGenModule &IGM) {
        struct Visitor : FieldVisitor<Visitor> {
          Size wordSize;
          Size offset;
          Visitor(Size wordSize) : wordSize(wordSize), offset(0) {}
          void visitField(PrivateLayout::Field field) {
            if (field.kind == Field::AllocationPtr)
              return;
            offset += field.getSizeInBytes(wordSize);
          };
          void visitArray(PrivateLayout::Field field, unsigned count) {
            offset += field.getSizeInBytes(wordSize) * count;
          }
        };
        Visitor visitor(IGM.getPointerSize());
        visitor.visit(*this);
        return Size(visitor.offset);
      }
    };

    toolchain::DenseMap<PrivateLayout::Kind, toolchain::StructType *> extendedTaskTys;
    toolchain::StructType *getExtendedCodiraTaskTy(PrivateLayout layout) {
      auto iterator = extendedTaskTys.find(layout.kind);
      if (iterator != extendedTaskTys.end())
        return iterator->second;

      assert(layout.isPointer8Bytes() == isPointer8Bytes);

      SmallVector<toolchain::Type *, 16> fieldTys;
      layout.getFields(IGM, getAllocationTy(), fieldTys);

      auto name = layout.getName();
      return toolchain::StructType::create(IGM.getLLVMContext(), fieldTys, name,
                                      /*packed*/ false);
    }

    toolchain::StructType *_allocationTy = nullptr;
    /// The type of the allocation to which there's a pointer within the Task.
    toolchain::StructType *getAllocationTy() {
      if (!_allocationTy) {
        _allocationTy = IGM.createTransientStructType(
            "language.back_deploy.task.stack_allocator.allocation",
            {// Allocation *previous;
             IGM.PtrTy,
             // Slab *slab;
             IGM.PtrTy});
      }
      return _allocationTy;
    }

    /// The facts about the layout of Task in some PrivateLayout that will
    /// allow the offset to the address of the latest allocation to be
    /// calculated.
    struct TaskLayoutInfo {
      toolchain::StructType *ty;
      unsigned allocationIndex;
      Size allocationOffset;
    };
    TaskLayoutInfo getTaskLayoutInfo(PrivateLayout layout) {
      return {getExtendedCodiraTaskTy(layout), layout.getAllocatorIndex(),
              layout.getAllocatorOffset(IGM)};
    };

    /// Calculate the address of the pointer to the latest allocation.  It's an
    /// offset from \p task determined by \p info.
    Address computeOffsetIntoTask(toolchain::Value *task, TaskLayoutInfo info) {
      auto taskAddr = Address(task, info.ty, IGF.IGM.getPointerAlignment());
      auto allocationPtrAddr = IGF.Builder.CreateStructGEP(
          taskAddr, info.allocationIndex, info.allocationOffset);
      return allocationPtrAddr;
    }

    /// Load the \p allocationPtrAddr and offset from it to calculate the latest
    /// allocated memory.  Pass that allocated memory to language_task_dealloc.
    ///
    /// If this allocated memory is the memory we are deallocating "through"
    /// (i.e \p ptr), then we're done (i.e. br \p exit).  Otherwise, keep going
    /// (i.e. br \p loop).
    void emitLoopBlock(toolchain::BasicBlock *loop, Address allocationPtrAddr,
                       toolchain::Value *ptr, toolchain::BasicBlock *exit) {
      IGF.Builder.emitBlock(loop);
      auto allocationAddr =
          IGF.Builder.CreateLoad(allocationPtrAddr, "allocation");
      auto headerSize = IGF.alignUpToMaximumAlignment(
          IGM.SizeTy, toolchain::ConstantInt::get(
                          IGM.IntPtrTy, 2 * IGM.getPointerSize().getValue()));
      auto *current = IGF.Builder.CreateInBoundsGEP(IGM.Int8Ty, allocationAddr,
                                                    {headerSize});

      IGF.Builder.CreateCall(IGM.getTaskDeallocFunctionPointer(), {current});

      auto *currentIsntPtr =
          IGF.Builder.CreateICmpNE(current, ptr, "current_is_not_ptr");
      IGF.Builder.CreateCondBr(currentIsntPtr, loop, exit);
    }

    /// Return void.
    void emitExitBlock(toolchain::BasicBlock *exit) {
      // Emit the exit block.
      IGF.Builder.emitBlock(exit);
      IGF.Builder.CreateRetVoid();
    }
  };

public:
  GetDeallocThroughFn(IRGenModule &IGM) : IGM(IGM) {}
  toolchain::Constant *get() {
    if (EnableRuntimeTaskDeallocThrough &&
        IGM.getAvailabilityRange().isContainedIn(
            IGM.Context.getCoroutineAccessorsAvailability())) {
      // For high enough deployment targets, just use the runtime entry point.
      return IGM.getTaskDeallocThroughFn();
    }
    return IGM.getOrCreateHelperFunction(
        "_language_task_dealloc_through", IGM.VoidTy, {IGM.Int8PtrTy},
        [](auto &IGF) { Builder(IGF).build(); },
        /*setIsNoInline=*/true,
        /*forPrologue=*/false,
        /*isPerformanceConstraint=*/false,
        /*optionalLinkageOverride=*/nullptr, IGM.CodiraCC);
  }
};

} // end anonymous namespace

void IRGenFunction::emitTaskDeallocThrough(Address address) {
  auto getDeallocThroughFn = GetDeallocThroughFn(IGM);
  auto fnPtr = FunctionPointer::forDirect(
      FunctionPointer::Kind::Function, getDeallocThroughFn.get(), nullptr,
      IGM.getTaskDeallocThroughFunctionPointer().getSignature());
  auto *call = Builder.CreateCall(fnPtr, {address.getAddress()});
  call->setDoesNotThrow();
  call->setCallingConv(IGM.CodiraCC);
}
