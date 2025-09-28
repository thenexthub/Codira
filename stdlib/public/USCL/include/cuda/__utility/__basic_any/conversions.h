/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef _CUDA___UTILITY_BASIC_ANY_CONVERSIONS_H
#define _CUDA___UTILITY_BASIC_ANY_CONVERSIONS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/__utility/__basic_any/interfaces.h>
#include <uscl/std/__concepts/convertible_to.h>
#include <uscl/std/__type_traits/is_convertible.h>
#include <uscl/std/__type_traits/remove_reference.h>
#include <uscl/std/__type_traits/type_list.h>
#include <uscl/std/__utility/declval.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//!
//! conversions
//!
//! Can one __basic_any type convert to another? Implicitly? Explicitly?
//! Statically? Dynamically? We answer these questions by mapping two
//! cvref qualified __basic_any types to archetype types, and then using
//! the built-in language rules to determine if the conversion is valid.
//!
template <bool _Movable, bool _Copyable>
struct __archetype;

// Archetype for interfaces that extend neither __imovable nor __icopyable
template <>
struct __archetype<false, false> // immovable archetype
{
  __archetype()                   = default;
  __archetype(__archetype&&)      = delete;
  __archetype(const __archetype&) = delete;

  template <class _Value>
  _CCCL_API __archetype(_Value) noexcept;
  template <class _Value>
  _CCCL_API __archetype(_Value*) = delete;
};

// Archetype for interfaces that extend __imovable but not __icopyable
template <>
struct __archetype<true, false> : __archetype<false, false> // movable archetype
{
  __archetype() = default;
  _CCCL_API __archetype(__archetype&&) noexcept;
  __archetype(const __archetype&) = delete;
};

// Archetype for interfaces that extend __icopyable
template <>
struct __archetype<true, true> : __archetype<true, false>
{
  __archetype() = default;
  _CCCL_API __archetype(__archetype const&);
};

template <class _Interface>
using __archetype_t _CCCL_NODEBUG_ALIAS =
  __archetype<__extension_of<_Interface, __imovable<>>, __extension_of<_Interface, __icopyable<>>>;

// Strip top-level cv- and ref-qualifiers from pointer types:
template <class _Ty>
_CCCL_API auto __normalize_interface(_Ty&&) -> _Ty;

template <class _Ty>
_CCCL_API auto __normalize_interface(_Ty*) -> _Ty*;

template <class _Ty>
using __normalize_t _CCCL_NODEBUG_ALIAS = decltype(::cuda::__normalize_interface(::cuda::std::declval<_Ty>()));

// Used to map a __basic_any specialization to a normalized interface type:
template <class _Ty>
extern ::cuda::std::__undefined<_Ty> __interface_from;
template <class _Interface>
extern _Interface __interface_from<__basic_any<_Interface>>;
template <class _Interface>
extern _Interface __interface_from<__basic_any<__ireference<_Interface>>>;
template <class _Interface>
extern _Interface& __interface_from<__basic_any<_Interface>&>;
template <class _Interface>
extern _Interface const& __interface_from<__basic_any<_Interface> const&>;
template <class _Interface>
extern _Interface* __interface_from<__basic_any<_Interface>*>;
template <class _Interface>
extern _Interface const* __interface_from<__basic_any<_Interface> const*>;
template <class _Interface>
extern _Interface* __interface_from<__basic_any<__ireference<_Interface>>*>;
template <class _Interface>
extern _Interface* __interface_from<__basic_any<__ireference<_Interface>> const*>;

// Used to map a normalized interface type to an archetype for conversion testing:
template <class _Interface>
extern __archetype_t<_Interface> __as_archetype;
template <class _Interface>
extern __archetype_t<_Interface>& __as_archetype<_Interface&>;
template <class _Interface>
extern __archetype_t<_Interface> const& __as_archetype<_Interface const&>;
template <class _Interface>
extern __archetype_t<_Interface>* __as_archetype<_Interface*>;
template <class _Interface>
extern __archetype_t<_Interface> const* __as_archetype<_Interface const*>;
template <class _Interface>
extern __archetype_t<_Interface>& __as_archetype<__ireference<_Interface>>;
template <class _Interface>
extern __archetype_t<_Interface> const& __as_archetype<__ireference<_Interface const>>;

// Used to map an archetype to an immovable archetype
template <class _Archetype>
extern __archetype<false, false> __as_immovable;
template <class _Archetype>
extern __archetype<false, false>& __as_immovable<_Archetype&>;
template <class _Archetype>
extern __archetype<false, false> const& __as_immovable<_Archetype const&>;
template <class _Archetype>
extern __archetype<false, false>* __as_immovable<_Archetype*>;
template <class _Archetype>
extern __archetype<false, false> const* __as_immovable<_Archetype const*>;

template <class _CvAny>
using __normalized_interface_of _CCCL_NODEBUG_ALIAS = __normalize_t<decltype(__interface_from<__normalize_t<_CvAny>>)>;

template <class _CvAny>
using __src_archetype_of _CCCL_NODEBUG_ALIAS = decltype(__as_archetype<__normalized_interface_of<_CvAny>>);

template <class _CvAny>
using __dst_archetype_of _CCCL_NODEBUG_ALIAS = decltype(__as_immovable<__src_archetype_of<_CvAny>>);

// If the archetypes are implicitly convertible, then it is possible to
// dynamically cast from the source to the destination. The cast may fail,
// but at least it is possible.
template <class _SrcCvAny, class _DstCvAny>
_CCCL_CONCEPT __any_castable_to =
  ::cuda::std::convertible_to<__src_archetype_of<_SrcCvAny>, __dst_archetype_of<_DstCvAny>>;

// If the archetypes are implicitly convertible **and** the source interface
// is an extension of the destination one, then it is possible to implicitly
// convert from the source to the destination.
template <class _SrcCvAny, class _DstCvAny>
_CCCL_CONCEPT __any_convertible_to =
  __any_castable_to<_SrcCvAny, _DstCvAny> && //
  __extension_of<typename ::cuda::std::remove_reference_t<_SrcCvAny>::interface_type,
                 typename ::cuda::std::remove_reference_t<_DstCvAny>::interface_type>;

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_CONVERSIONS_H
