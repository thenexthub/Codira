//===---------------- ObjectFileContext.h - Codira Compiler ---------------===//
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

#ifndef LANGUAGE_OBJECT_FILE_CONTEXT_H
#define LANGUAGE_OBJECT_FILE_CONTEXT_H

#include "language/RemoteInspection/ReflectionContext.h"

namespace toolchain {
namespace object {
template <typename Type>
class ELFObjectFile;
class ELFObjectFileBase;
class MachOObjectFile;
} // namespace object
} // namespace toolchain

namespace language {
namespace static_mirror {

using ReadBytesResult = language::remote::MemoryReader::ReadBytesResult;

class Image {
private:
  struct Segment {
    uint64_t Addr;
    StringRef Contents;
  };
  const toolchain::object::ObjectFile *O;
  uint64_t HeaderAddress;
  std::vector<Segment> Segments;
  struct DynamicRelocation {
    /// The symbol name that the pointer refers to. Empty if only an absolute
    /// address is available.
    StringRef Symbol;
    // The offset (if the symbol is available), or the resolved remote address
    // if the symbol is empty.
    uint64_t OffsetOrAddress;
  };
  toolchain::DenseMap<uint64_t, DynamicRelocation> DynamicRelocations;

  void scanMachO(const toolchain::object::MachOObjectFile *O);

  template <typename ELFT>
  void scanELFType(const toolchain::object::ELFObjectFile<ELFT> *O);

  void scanELF(const toolchain::object::ELFObjectFileBase *O);

  void scanCOFF(const toolchain::object::COFFObjectFile *O);

  bool isMachOWithPtrAuth() const;

public:
  explicit Image(const toolchain::object::ObjectFile *O);

  const toolchain::object::ObjectFile *getObjectFile() const { return O; }

  unsigned getBytesInAddress() const { return O->getBytesInAddress(); }

  uint64_t getStartAddress() const { return HeaderAddress; }

  uint64_t getEndAddress() const;

  StringRef getContentsAtAddress(uint64_t Addr, uint64_t Size) const;

  remote::RemoteAbsolutePointer resolvePointer(uint64_t Addr,
                                               uint64_t pointerValue) const;

  remote::RemoteAbsolutePointer getDynamicSymbol(uint64_t Addr) const;
};

/// MemoryReader that reads from the on-disk representation of an executable
/// or dynamic library image.
///
/// This reader uses a remote addressing scheme where the most significant
/// 16 bits of the address value serve as an index into the array of loaded
/// images, and the low 48 bits correspond to the preferred virtual address
/// mapping of the image.
class ObjectMemoryReader : public reflection::MemoryReader {
  struct ImageEntry {
    Image TheImage;
    uint64_t Slide;
  };
  std::vector<ImageEntry> Images;

  uint64_t PtrauthMask = 0;

  std::pair<const Image *, uint64_t>
  decodeImageIndexAndAddress(uint64_t Addr) const;

  remote::RemoteAddress
  encodeImageIndexAndAddress(const Image *image,
                             remote::RemoteAddress imageAddr) const;

  StringRef getContentsAtAddress(uint64_t Addr, uint64_t Size);

  uint64_t getPtrauthMask();

public:
  explicit ObjectMemoryReader(
      const std::vector<const toolchain::object::ObjectFile *> &ObjectFiles);

  ArrayRef<ImageEntry> getImages() const { return Images; }

  bool queryDataLayout(DataLayoutQueryType type, void *inBuffer,
                       void *outBuffer) override;

  reflection::RemoteAddress getImageStartAddress(unsigned i) const;

  // TODO: We could consult the dynamic symbol tables of the images to
  // implement this.
  reflection::RemoteAddress getSymbolAddress(const std::string &name) override {
    return reflection::RemoteAddress();
  }

  ReadBytesResult readBytes(reflection::RemoteAddress Addr,
                            uint64_t Size) override;

  bool readString(reflection::RemoteAddress Addr, std::string &Dest) override;

  remote::RemoteAbsolutePointer resolvePointer(reflection::RemoteAddress Addr,
                                               uint64_t pointerValue) override;

  remote::RemoteAbsolutePointer
  getDynamicSymbol(reflection::RemoteAddress Addr) override;
};

using ReflectionContextOwner = std::unique_ptr<void, void (*)(void *)>;

struct ReflectionContextHolder {
  ReflectionContextOwner Owner;
  reflection::TypeRefBuilder &Builder;
  ObjectMemoryReader &Reader;
  uint8_t PointerSize;
};

template <typename T>
T unwrap(toolchain::Expected<T> value) {
  if (value)
    return std::move(value.get());
  toolchain::errs() << "language-reflection-test error: " << toString(value.takeError())
               << "\n";
  exit(EXIT_FAILURE);
}

std::unique_ptr<ReflectionContextHolder> makeReflectionContextForObjectFiles(
    const std::vector<const toolchain::object::ObjectFile *> &objectFiles,
    bool objcInterOp);

std::unique_ptr<ReflectionContextHolder> makeReflectionContextForMetadataReader(
    std::shared_ptr<ObjectMemoryReader> reader,
    bool objcInterOp);

} // end namespace static_mirror
} // end namespace language

#endif // LANGUAGE_OBJECT_FILE_CONTEXT_H
