// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES
//
//===----------------------------------------------------------------------===//
#ifndef _CUDA___ITERATOR_ZIP_FUNCTION_H
#define _CUDA___ITERATOR_ZIP_FUNCTION_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__fwd/zip_iterator.h>
#include <uscl/std/__concepts/constructible.h>
#include <uscl/std/__functional/invoke.h>
#include <uscl/std/__type_traits/is_nothrow_copy_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_default_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_move_constructible.h>
#include <uscl/std/__utility/declval.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/__utility/pair.h>
#include <uscl/std/tuple>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief Adaptor that transforms a N-ary function \c _Fn into one accepting a \c tuple of size N
template <class _Fn>
class zip_function
{
private:
  _Fn __fun_;

public:
  //! @brief default construct a zip_function if \c _Fn is default_initializable
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Fn2 = _Fn)
  _CCCL_REQUIRES(::cuda::std::default_initializable<_Fn2>)
  _CCCL_API constexpr zip_function() noexcept(::cuda::std::is_nothrow_default_constructible_v<_Fn2>)
      : __fun_()
  {}

  //! @brief construct a zip_function from a functor \p __fun
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API constexpr zip_function(const _Fn& __fun) noexcept(::cuda::std::is_nothrow_copy_constructible_v<_Fn>)
      : __fun_(__fun)
  {}

  //! @brief construct a zip_function from a functor \p __fun
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API constexpr zip_function(_Fn&& __fun) noexcept(::cuda::std::is_nothrow_move_constructible_v<_Fn>)
      : __fun_(::cuda::std::move(__fun))
  {}

  template <class _Fn2, class _Tuple>
  static constexpr bool __is_nothrow_invocable =
    noexcept(::cuda::std::apply(::cuda::std::declval<_Fn2>(), ::cuda::std::declval<_Tuple>()));

  //! @brief Applies a tuple \p __tuple to the stored functor
  _CCCL_EXEC_CHECK_DISABLE
  template <class _Tuple>
  [[nodiscard]] _CCCL_API constexpr decltype(auto) operator()(_Tuple&& __tuple) const
    noexcept(__is_nothrow_invocable<const _Fn&, _Tuple>)
  {
    return ::cuda::std::apply(__fun_, ::cuda::std::forward<_Tuple>(__tuple));
  }

  //! @brief Applies a tuple \p __tuple to the stored functor
  _CCCL_EXEC_CHECK_DISABLE
  template <class _Tuple>
  [[nodiscard]] _CCCL_API constexpr decltype(auto)
  operator()(_Tuple&& __tuple) noexcept(__is_nothrow_invocable<_Fn&, _Tuple>)
  {
    return ::cuda::std::apply(__fun_, ::cuda::std::forward<_Tuple>(__tuple));
  }
};

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___ITERATOR_ZIP_FUNCTION_H
