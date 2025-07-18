//===--- TaskLocal.h - ABI of task local values -----------------*- C++ -*-===//
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
// Codira ABI describing task locals.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_TASKLOCAL_BACKDEPLOY56_H
#define LANGUAGE_ABI_TASKLOCAL_BACKDEPLOY56_H

#include "language/ABI/HeapObject.h"
#include "language/ABI/Metadata.h"
#include "language/ABI/MetadataValues.h"

namespace language {
class AsyncTask;
struct OpaqueValue;
struct CodiraError;
class TaskStatusRecord;
class TaskGroup;

// ==== Task Locals Values ---------------------------------------------------

class TaskLocal {
public:
  /// Type of the pointed at `next` task local item.
  enum class NextLinkType : uintptr_t {
    /// The storage pointer points at the next TaskLocal::Item in this task.
    IsNext = 0b00,
    /// The storage pointer points at a item stored by another AsyncTask.
    ///
    /// Note that this may not necessarily be the same as the task's parent
    /// task -- we may point to a super-parent if we know / that the parent
    /// does not "contribute" any task local values. This is to speed up
    /// lookups by skipping empty parent tasks during get(), and explained
    /// in depth in `createParentLink`.
    IsParent = 0b01,
  };

  class Item {
  private:
    /// Mask used for the low status bits in a task local chain item.
    static const uintptr_t statusMask = 0x03;

    /// Pointer to one of the following:
    /// - next task local item as OpaqueValue* if it is task-local allocated
    /// - next task local item as HeapObject* if it is heap allocated "heavy"
    /// - the parent task's TaskLocal::Storage
    ///
    /// Low bits encode `NextLinkType`, based on which the type of the pointer
    /// is determined.
    uintptr_t next;

  public:
    /// The type of the key with which this value is associated.
    const HeapObject *key;
    /// The type of the value stored by this item.
    const Metadata *valueType;

    // Trailing storage for the value itself. The storage will be
    // uninitialized or contain an instance of \c valueType.

    /// Returns true if this item is a 'parent pointer'.
    ///
    /// A parent pointer is special kind of `Item` is created when pointing at
    /// the parent storage, forming a chain of task local items spanning multiple
    /// tasks.
    bool isParentPointer() const {
      return !valueType;
    }

  protected:
    explicit Item()
      : next(0),
        key(nullptr),
        valueType(nullptr) {}

    explicit Item(const HeapObject *key, const Metadata *valueType)
      : next(0),
        key(key),
        valueType(valueType) {}

  public:
    /// Item which does not by itself store any value, but only points
    /// to the nearest task-local-value containing parent's first task item.
    ///
    /// This item type is used to link to the appropriate parent task's item,
    /// when the current task itself does not have any task local values itself.
    ///
    /// When a task actually has its own task locals, it should rather point
    /// to the parent's *first* task-local item in its *last* item, extending
    /// the Item linked list into the appropriate parent.
    static Item *createParentLink(AsyncTask *task, AsyncTask *parent);

    static Item *createLink(AsyncTask *task,
                            const HeapObject *key,
                            const Metadata *valueType);

    void destroy(AsyncTask *task);

    Item *getNext() {
      return reinterpret_cast<Item *>(next & ~statusMask);
    }

    NextLinkType getNextLinkType() const {
      return static_cast<NextLinkType>(next & statusMask);
    }

    /// Item does not contain any actual value, and is only used to point at
    /// a specific parent item.
    bool isEmpty() const {
      return !valueType;
    }

    /// Retrieve a pointer to the storage of the value.
    OpaqueValue *getStoragePtr() {
      return reinterpret_cast<OpaqueValue *>(
        reinterpret_cast<char *>(this) + storageOffset(valueType));
    }

    void copyTo(AsyncTask *task);

    /// Compute the offset of the storage from the base of the item.
    static size_t storageOffset(const Metadata *valueType) {
      size_t offset = sizeof(Item);

      if (valueType) {
        size_t alignment = valueType->vw_alignment();
        return (offset + alignment - 1) & ~(alignment - 1);
      } else {
        return offset;
      }
    }

    /// Determine the size of the item given a particular value type.
    static size_t itemSize(const Metadata *valueType) {
      size_t offset = storageOffset(valueType);
      if (valueType) {
        offset += valueType->vw_size();
      }
      return offset;
    }
  };

  class Storage {
    friend class TaskLocal::Item;
  private:
    /// A stack (single-linked list) of task local values.
    ///
    /// Once task local values within this task are traversed, the list continues
    /// to the "next parent that contributes task local values," or if no such
    /// parent exists it terminates with null.
    ///
    /// If the TaskLocalValuesFragment was allocated, it is expected that this
    /// value should be NOT null; it either has own values, or at least one
    /// parent that has values. If this task does not have any values, the head
    /// pointer MAY immediately point at this task's parent task which has values.
    ///
    /// ### Concurrency
    /// Access to the head is only performed from the task itself, when it
    /// creates child tasks, the child during creation will inspect its parent's
    /// task local value stack head, and point to it. This is done on the calling
    /// task, and thus needs not to be synchronized. Subsequent traversal is
    /// performed by child tasks concurrently, however they use their own
    /// pointers/stack and can never mutate the parent's stack.
    ///
    /// The stack is only pushed/popped by the owning task, at the beginning and
    /// end a `body` block of `withLocal(_:boundTo:body:)` respectively.
    ///
    /// Correctness of the stack strongly relies on the guarantee that tasks
    /// never outline a scope in which they are created. Thanks to this, if
    /// tasks are created inside the `body` of `withLocal(_:,boundTo:body:)`
    /// all tasks created inside the `withLocal` body must complete before it
    /// returns, as such, any child tasks potentially accessing the value stack
    /// are guaranteed to be completed by the time we pop values off the stack
    /// (after the body has completed).
    TaskLocal::Item *head = nullptr;

  public:

    void initializeLinkParent(AsyncTask *task, AsyncTask *parent);

    void pushValue(AsyncTask *task,
                   const HeapObject *key,
                   /* +1 */ OpaqueValue *value, const Metadata *valueType);

    OpaqueValue* getValue(AsyncTask *task, const HeapObject *key);

    /// Returns `true` of more bindings remain in this storage,
    /// and `false` if the just popped value was the last one and the storage
    /// can be safely disposed of.
    bool popValue(AsyncTask *task);

    /// Copy all task-local bindings to the target task.
    ///
    /// The new bindings allocate their own items and can out-live the current task.
    ///
    /// ### Optimizations
    /// Only the most recent binding of a value is copied over, i.e. given
    /// a key bound to `A` and then `B`, only the `B` binding will be copied.
    /// This is safe and correct because the new task would never have a chance
    /// to observe the `A` value, because it semantically will never observe a
    /// "pop" of the `B` value - it was spawned from a scope where only B was observable.
    void copyTo(AsyncTask *target);

    /// Destroy and deallocate all items stored by this specific task.
    ///
    /// Items owned by a parent task are left untouched, since we do not own them.
    void destroy(AsyncTask *task);
  };
};

} // end namespace language

#endif // LANGUAGE_ABI_TASKLOCAL_BACKDEPLOY56_H
