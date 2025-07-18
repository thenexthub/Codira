//===--- ImmutableTextBuffer.h - --------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_SUPPORT_IMMUTABLETEXTBUFFER_H
#define TOOLCHAIN_SOURCEKIT_SUPPORT_IMMUTABLETEXTBUFFER_H

#include "SourceKit/Core/Toolchain.h"
#include "SourceKit/Support/ThreadSafeRefCntPtr.h"
#include "language/Basic/ThreadSafeRefCounted.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/Support/Mutex.h"
#include <functional>
#include <memory>

namespace toolchain {
  class MemoryBuffer;
  class SourceMgr;
}

namespace SourceKit {

class ImmutableTextUpdate;
typedef RefPtr<ImmutableTextUpdate> ImmutableTextUpdateRef;

class ImmutableTextBuffer;
typedef RefPtr<ImmutableTextBuffer> ImmutableTextBufferRef;

class ReplaceImmutableTextUpdate;
typedef RefPtr<ReplaceImmutableTextUpdate> ReplaceImmutableTextUpdateRef;

class ImmutableTextSnapshot;
typedef RefPtr<ImmutableTextSnapshot> ImmutableTextSnapshotRef;

class EditableTextBuffer;
typedef RefPtr<EditableTextBuffer> EditableTextBufferRef;

class ImmutableTextUpdate : public ThreadSafeRefCountedBaseVPTR {
public:
  enum class Kind {
    Buffer,
    Replace,
  };

  virtual ~ImmutableTextUpdate() = default;

  ImmutableTextUpdateRef getNext() const { return Next; }
  Kind getKind() const { return K; }
  uint64_t getStamp() const { return Stamp; }

protected:
  ImmutableTextUpdate(Kind K, uint64_t Stamp) : K(K), Stamp(Stamp) {}

private:
  Kind K;
  uint64_t Stamp;
  ThreadSafeRefCntPtr<ImmutableTextUpdate> Next;

  friend class EditableTextBuffer;
  virtual void anchor() override;
};

class ImmutableTextBuffer : public ImmutableTextUpdate {
  std::unique_ptr<toolchain::SourceMgr> SrcMgr;
  unsigned BufId;

public:
  explicit ImmutableTextBuffer(std::unique_ptr<toolchain::MemoryBuffer> MemBuf,
                               uint64_t Stamp);
  ImmutableTextBuffer(StringRef Filename, StringRef Text, uint64_t Stamp);

  StringRef getText() const;
  StringRef getFilename() const;

  /// Returns the internal memory buffer.
  /// The caller must make sure that the pointer does not outlive the
  /// ImmutableTextBuffer object that it came from.
  const toolchain::MemoryBuffer *getInternalBuffer() const;

  std::pair<unsigned, unsigned> getLineAndColumn(unsigned ByteOffset) const;

  static bool classof(const ImmutableTextUpdate *ITD) {
    return ITD->getKind() == Kind::Buffer;
  }
};

class ReplaceImmutableTextUpdate : public ImmutableTextUpdate {
  std::unique_ptr<toolchain::MemoryBuffer> Buf;
  unsigned ByteOffset;
  unsigned Length;

public:
  ReplaceImmutableTextUpdate(unsigned ByteOffset, unsigned Length,
                             StringRef Text, uint64_t Stamp);

  unsigned getByteOffset() const { return ByteOffset; }
  unsigned getLength() const { return Length; }
  StringRef getText() const;

  static bool classof(const ImmutableTextUpdate *ITD) {
    return ITD->getKind() == Kind::Replace;
  }
};

class ImmutableTextSnapshot :
    public ThreadSafeRefCountedBase<ImmutableTextSnapshot> {

  EditableTextBufferRef EditableBuf;
  ImmutableTextBufferRef BufferStart;
  ImmutableTextUpdateRef DiffEnd;

  ImmutableTextSnapshot(EditableTextBufferRef EditableBuf,
                        ImmutableTextBufferRef Buffer)
    : EditableBuf(std::move(EditableBuf)),
      BufferStart(std::move(Buffer)), DiffEnd(BufferStart) {}

  ImmutableTextSnapshot(EditableTextBufferRef EditableBuf,
                        ImmutableTextBufferRef BufferStart,
                        ImmutableTextUpdateRef DiffEnd)
    : EditableBuf(std::move(EditableBuf)),
      BufferStart(std::move(BufferStart)), DiffEnd(std::move(DiffEnd)) {}

friend class EditableTextBuffer;

public:
  StringRef getFilename() const;

  uint64_t getStamp() const;

  ImmutableTextBufferRef getBuffer() const;

  size_t getSize() const;

  bool isFromSameBuffer(ImmutableTextSnapshotRef Other) const {
    return Other->EditableBuf.get() == EditableBuf.get();
  }

  EditableTextBufferRef getEditableBuffer() const {
    return EditableBuf;
  }

  bool precedesOrSame(ImmutableTextSnapshotRef Other);

  bool foreachReplaceUntil(ImmutableTextSnapshotRef EndSnapshot,
                     std::function<bool(ReplaceImmutableTextUpdateRef Upd)> Fn);
};

class EditableTextBuffer : public ThreadSafeRefCountedBase<EditableTextBuffer> {
  mutable toolchain::sys::Mutex EditMtx;
  ImmutableTextBufferRef Root;
  ImmutableTextUpdateRef CurrUpd;
  std::string Filename;

public:
  explicit EditableTextBuffer(StringRef Filename, StringRef Text = StringRef());

  StringRef getFilename() const { return Filename; }

  ImmutableTextSnapshotRef getSnapshot() const;

  ImmutableTextBufferRef getBuffer() const {
    return getSnapshot()->getBuffer();
  }

  size_t getSize() const {
    return getSnapshot()->getSize();
  }

  ImmutableTextSnapshotRef insert(unsigned ByteOffset, StringRef Text);
  ImmutableTextSnapshotRef erase(unsigned ByteOffset, unsigned Length);
  ImmutableTextSnapshotRef replace(unsigned ByteOffset, unsigned Length,
                                   StringRef Text);

private:
  ImmutableTextSnapshotRef addAtomicUpdate(ImmutableTextUpdateRef NewUpd);
  ImmutableTextBufferRef getBufferForSnapshot(
      const ImmutableTextSnapshot &Snap);
  size_t getSizeForSnapshot(const ImmutableTextSnapshot &Snap) const;
  void refresh();
  friend class ImmutableTextSnapshot;
};

class EditableTextBufferManager {
  toolchain::sys::Mutex Mtx;
  toolchain::StringMap<EditableTextBufferRef> FileBufferMap;

public:
  EditableTextBufferRef getOrCreateBuffer(StringRef Filename,
                                          StringRef Text = StringRef());
  EditableTextBufferRef resetBuffer(StringRef Filename,
                                    StringRef Text = StringRef());
};

} // namespace SourceKit

#endif
