//===--- CompactArray.h - ---------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKITD_COMPACTARRAY_H
#define TOOLCHAIN_SOURCEKITD_COMPACTARRAY_H

#include "sourcekitd/Internal.h"
#include "toolchain/ADT/SmallString.h"

namespace sourcekitd {

class CompactArrayBuilderImpl {
public:
  std::unique_ptr<toolchain::MemoryBuffer> createBuffer(CustomBufferKind Kind) const;
  void appendTo(toolchain::SmallVectorImpl<char> &Buf) const;
  unsigned copyInto(char *BufPtr) const;
  size_t sizeInBytes() const;
  bool empty() const;

protected:
  CompactArrayBuilderImpl();

  void addImpl(uint8_t Val);
  void addImpl(unsigned Val);
  void addImpl(toolchain::StringRef Val);
  void addImpl(SourceKit::UIdent Val);
  void addImpl(std::optional<toolchain::StringRef> Val);

private:
  unsigned getOffsetForString(toolchain::StringRef Str);
  void copyInto(char *BufPtr, size_t Length) const;

  toolchain::SmallVector<uint8_t, 256> EntriesBuffer;
  toolchain::SmallString<256> StringBuffer;
};


template <typename T>
struct CompactArrayField {
  static constexpr size_t getByteSize() {
    return sizeof(T);
  }
};

template <>
struct CompactArrayField<const char *> {
  static constexpr size_t getByteSize() {
    return sizeof(unsigned);
  }
};

template <typename T, typename ...Targs>
struct CompactArrayEntriesSizer {
  static constexpr size_t getByteSize() {
    return CompactArrayEntriesSizer<T>::getByteSize() +
      CompactArrayEntriesSizer<Targs...>::getByteSize();
  }
};
template <typename T>
struct CompactArrayEntriesSizer<T> {
  static constexpr size_t getByteSize() {
    return CompactArrayField<T>::getByteSize();
  }
};

template <typename ...EntryTypes>
class CompactArrayBuilder : public CompactArrayBuilderImpl {
public:
  void addEntry(EntryTypes... Args) {
    add(Args...);
    count += 1;
  }

  size_t size() const { return count; }

private:
  template <typename T, typename ...Targs>
  void add(T Val, Targs... Args) {
    add(Val);
    add(Args...);
  }

  template <typename T>
  void add(T Val) {
    addImpl(Val);
  }

  size_t count = 0;
};

class CompactArrayReaderImpl {
protected:
  CompactArrayReaderImpl(void *Buf) : Buf(Buf) {}

  uint64_t getEntriesBufSize() const {
    uint64_t result;
    std::memcpy(&result, Buf, sizeof result);
    return result;
  }
  const uint8_t *getEntriesBufStart() const {
    return (const uint8_t *)(((uint64_t*)Buf)+1);
  }
  const char *getStringBufStart() const {
    return (const char *)(getEntriesBufStart() + getEntriesBufSize());
  }

  void readImpl(size_t Offset, uint8_t &Val);
  void readImpl(size_t Offset, unsigned &Val);
  void readImpl(size_t Offset, const char * &Val);
  void readImpl(size_t Offset, sourcekitd_uid_t &Val);

private:
  void *Buf;
};

template <typename ...EntryTypes>
class CompactArrayReader : public CompactArrayReaderImpl {
public:
  CompactArrayReader(void *Buf) : CompactArrayReaderImpl(Buf) {}

  size_t getCount() const {
    assert(getEntriesBufSize() % getEntriesSize() == 0);
    return getEntriesBufSize() / getEntriesSize();
  }

  void readEntries(size_t Index, EntryTypes &... Fields) {
    assert(Index < getCount());
    read(getEntriesSize() * Index, Fields...);
  }

private:
  template <typename T, typename ...Targs>
  void read(size_t Offset, T &Field, Targs &... Fields) {
    read(Offset, Field);
    read(Offset + CompactArrayField<T>::getByteSize(), Fields...);
  }

  template <typename T>
  void read(size_t Offset, T &Field) {
    readImpl(Offset, Field);
  }

  static size_t getEntriesSize() {
    return CompactArrayEntriesSizer<EntryTypes...>::getByteSize();
  }
};

template <typename T>
struct CompactVariantFuncs {
  static sourcekitd_variant_type_t get_type(sourcekitd_variant_t var) {
    return SOURCEKITD_VARIANT_TYPE_DICTIONARY;
  }

  static bool
  dictionary_apply(sourcekitd_variant_t dict,
                   sourcekitd_variant_dictionary_applier_f_t applier,
                   void *context) {
    void *Buf = (void *)dict.data[1];
    size_t Index = dict.data[2];
    return T::dictionary_apply(Buf, Index, applier, context);
  }

  static VariantFunctions Funcs;
};

template <typename T>
VariantFunctions CompactVariantFuncs<T>::Funcs = {
    get_type,
    nullptr /*Annot_array_apply*/,
    nullptr /*Annot_array_get_bool*/,
    nullptr /*Annot_array_get_double*/,
    nullptr /*Annot_array_get_count*/,
    nullptr /*Annot_array_get_int64*/,
    nullptr /*Annot_array_get_string*/,
    nullptr /*Annot_array_get_uid*/,
    nullptr /*Annot_array_get_value*/,
    nullptr /*Annot_bool_get_value*/,
    nullptr /*Annot_double_get_value*/,
    dictionary_apply,
    nullptr /*Annot_dictionary_get_bool*/,
    nullptr /*Annot_dictionary_get_double*/,
    nullptr /*Annot_dictionary_get_int64*/,
    nullptr /*Annot_dictionary_get_string*/,
    nullptr /*Annot_dictionary_get_value*/,
    nullptr /*Annot_dictionary_get_uid*/,
    nullptr /*Annot_string_get_length*/,
    nullptr /*Annot_string_get_ptr*/,
    nullptr /*Annot_int64_get_value*/,
    nullptr /*Annot_uid_get_value*/,
    nullptr /*Annot_data_get_size*/,
    nullptr /*Annot_data_get_ptr*/,
};

template <typename T>
struct CompactArrayFuncs {
  static sourcekitd_variant_type_t get_type(sourcekitd_variant_t var) {
    return SOURCEKITD_VARIANT_TYPE_ARRAY;
  }

  static size_t array_get_count(sourcekitd_variant_t array) {
    void *Buf = (void*)array.data[1];
    return typename T::CompactArrayReaderTy(Buf).getCount();
  }

  static sourcekitd_variant_t
  array_get_value(sourcekitd_variant_t array, size_t index) {
    assert(index < array_get_count(array));
    return {{ (uintptr_t)&CompactVariantFuncs<T>::Funcs,
              (uintptr_t)array.data[1],
                         index }};
  }

  static VariantFunctions Funcs;
};

template <typename T>
VariantFunctions CompactArrayFuncs<T>::Funcs = {
    get_type,
    nullptr /*AnnotArray_array_apply*/,
    nullptr /*AnnotArray_array_get_bool*/,
    nullptr /*AnnotArray_array_get_double*/,
    array_get_count,
    nullptr /*AnnotArray_array_get_int64*/,
    nullptr /*AnnotArray_array_get_string*/,
    nullptr /*AnnotArray_array_get_uid*/,
    array_get_value,
    nullptr /*AnnotArray_bool_get_value*/,
    nullptr /*AnnotArray_double_get_value*/,
    nullptr /*AnnotArray_dictionary_apply*/,
    nullptr /*AnnotArray_dictionary_get_bool*/,
    nullptr /*AnnotArray_dictionary_get_double*/,
    nullptr /*AnnotArray_dictionary_get_int64*/,
    nullptr /*AnnotArray_dictionary_get_string*/,
    nullptr /*AnnotArray_dictionary_get_value*/,
    nullptr /*AnnotArray_dictionary_get_uid*/,
    nullptr /*AnnotArray_string_get_length*/,
    nullptr /*AnnotArray_string_get_ptr*/,
    nullptr /*AnnotArray_int64_get_value*/,
    nullptr /*AnnotArray_uid_get_value*/,
    nullptr /*Annot_data_get_size*/,
    nullptr /*Annot_data_get_ptr*/,
};
}

#endif
