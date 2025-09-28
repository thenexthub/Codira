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

// constexpr sentinel<false> end();
// constexpr iterator<false> end() requires common_range<V>;
// constexpr sentinel<true> end() const
//   requires range<const V> &&
//            regular_invocable<const F&, range_reference_t<const V>>;
// constexpr iterator<true> end() const
//   requires common_range<const V> &&
//            regular_invocable<const F&, range_reference_t<const V>>;

#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

template <class T>
_CCCL_CONCEPT HasConstQualifiedEnd = _CCCL_REQUIRES_EXPR((T), const T& t)((t.end()));

__host__ __device__ constexpr bool test()
{
  {
    using TransformView = cuda::std::ranges::transform_view<ForwardView, PlusOneMutable>;
    static_assert(cuda::std::ranges::common_range<TransformView>);
    TransformView tv{};
    auto it  = tv.end();
    using It = decltype(it);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&>(it).base()), const forward_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&&>(it).base()), forward_iterator<int*>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&>(it).base()), const forward_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&&>(it).base()), const forward_iterator<int*>&>);
    assert(base(it.base()) == globalBuff + 8);
    assert(base(cuda::std::move(it).base()) == globalBuff + 8);
    static_assert(!HasConstQualifiedEnd<TransformView>);
  }
  {
    using TransformView = cuda::std::ranges::transform_view<InputView, PlusOneMutable>;
    static_assert(!cuda::std::ranges::common_range<TransformView>);
    TransformView tv{};
    auto sent  = tv.end();
    using Sent = decltype(sent);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<Sent&>(sent).base()), sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<Sent&&>(sent).base()), sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const Sent&>(sent).base()),
                                       sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const Sent&&>(sent).base()),
                                       sentinel_wrapper<cpp20_input_iterator<int*>>>);
    assert(base(base(sent.base())) == globalBuff + 8);
    assert(base(base(cuda::std::move(sent).base())) == globalBuff + 8);
    static_assert(!HasConstQualifiedEnd<TransformView>);
  }
  {
    using TransformView = cuda::std::ranges::transform_view<InputView, PlusOne>;
    static_assert(!cuda::std::ranges::common_range<TransformView>);
    TransformView tv{};
    auto sent  = tv.end();
    using Sent = decltype(sent);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<Sent&>(sent).base()), sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<Sent&&>(sent).base()), sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const Sent&>(sent).base()),
                                       sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const Sent&&>(sent).base()),
                                       sentinel_wrapper<cpp20_input_iterator<int*>>>);
    assert(base(base(sent.base())) == globalBuff + 8);
    assert(base(base(cuda::std::move(sent).base())) == globalBuff + 8);

    auto csent  = cuda::std::as_const(tv).end();
    using CSent = decltype(csent);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<CSent&>(csent).base()), sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<CSent&&>(csent).base()), sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const CSent&>(csent).base()),
                                       sentinel_wrapper<cpp20_input_iterator<int*>>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const CSent&&>(csent).base()),
                                       sentinel_wrapper<cpp20_input_iterator<int*>>>);
    assert(base(base(csent.base())) == globalBuff + 8);
    assert(base(base(cuda::std::move(csent).base())) == globalBuff + 8);
  }
  {
    using TransformView = cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable>;
    static_assert(cuda::std::ranges::common_range<TransformView>);
    TransformView tv{};
    auto it  = tv.end();
    using It = decltype(it);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&>(it).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&&>(it).base()), int*>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&>(it).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&&>(it).base()), int* const&>);
    assert(base(it.base()) == globalBuff + 8);
    assert(base(cuda::std::move(it).base()) == globalBuff + 8);
    static_assert(!HasConstQualifiedEnd<TransformView>);
  }
  {
    using TransformView = cuda::std::ranges::transform_view<MoveOnlyView, PlusOne>;
    static_assert(cuda::std::ranges::common_range<TransformView>);
    TransformView tv{};
    auto it  = tv.end();
    using It = decltype(it);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&>(it).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&&>(it).base()), int*>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&>(it).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&&>(it).base()), int* const&>);
    assert(base(it.base()) == globalBuff + 8);
    assert(base(cuda::std::move(it).base()) == globalBuff + 8);

    auto csent  = cuda::std::as_const(tv).end();
    using CSent = decltype(csent);
    static_assert(cuda::std::is_same_v<decltype(static_cast<CSent&>(csent).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<CSent&&>(csent).base()), int*>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const CSent&>(csent).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const CSent&&>(csent).base()), int* const&>);
    assert(base(base(csent.base())) == globalBuff + 8);
    assert(base(base(cuda::std::move(csent).base())) == globalBuff + 8);
  }
  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
