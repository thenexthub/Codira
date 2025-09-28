/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#ifndef _CUDAX___LIBRARY_LIBRARY_CUH
#define _CUDAX___LIBRARY_LIBRARY_CUH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__driver/driver_api.h>
#include <uscl/std/__cstddef/types.h>
#include <uscl/std/__exception/cuda_error.h>
#include <uscl/std/__memory/addressof.h>
#include <uscl/std/__utility/exchange.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/__utility/swap.h>

#include <uscl/experimental/__detail/utility.cuh>
#include <uscl/experimental/__library/library_ref.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{

//! @brief An owning wrapper for a CUDA library handle
struct library : public library_ref
{
  //! @brief Construct an `library` object from a native `CUlibrary`/`cudaLibrary_t` handle
  //!
  //! @param __handle The native handle
  //!
  //! @return The constructed `library` object
  //!
  //! @note The constructed `library` object takes ownership of the native handle
  [[nodiscard]] static library from_native_handle(value_type __handle) noexcept
  {
    return library{__handle};
  }

  //! @brief Disallow construction from a null pointer
  static library from_native_handle(::cuda::std::nullptr_t) = delete;

  //! @brief Construct a new `library` object into the moved-from state
  //!
  //! @post `get()` will return an invalid `CUlibrary` handle
  explicit constexpr library(no_init_t) noexcept
      : library{value_type{}}
  {}

  library(const library&) = delete;

  //! @brief Move-construct a new 'library' object
  //!
  //! @param __other The `library` to move from
  //!
  //! @post `__other` is in the moved-from state
  library(library&& __other) noexcept
      : library{__other.release()}
  {}

  //! @brief Destroy the `library` object
  //!
  //! @note If the library fails to unload, the error is silently ignored
  ~library()
  {
    if (__library_ != value_type{})
    {
      [[maybe_unused]] const auto __status = _CUDA_DRIVER::__libraryUnloadNoThrow(__library_);
    }
  }

  library& operator=(const library&) = delete;

  //! @brief Move-assign a new `library` object
  //!
  //! @param __other The `library` to move from
  //!
  //! @post `__other` is in the moved-from state
  library& operator=(library&& __other) noexcept
  {
    if (this != ::cuda::std::addressof(__other))
    {
      library __tmp{::cuda::std::move(__other)};
      ::cuda::std::swap(__library_, __tmp.__library_);
    }
    return *this;
  }

  //! @brief Retrieve the native `CUlibrary`/`cudaLibrary_t` handle and give up ownership
  //!
  //! @return The native handle being held by the `library` object
  //!
  //! @post The library object is in a moved-from state
  [[nodiscard]] constexpr value_type release() noexcept
  {
    return ::cuda::std::exchange(__library_, value_type{});
  }

private:
  constexpr explicit library(value_type __handle) noexcept
      : library_ref{__handle}
  {}
};

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX___LIBRARY_LIBRARY_CUH
