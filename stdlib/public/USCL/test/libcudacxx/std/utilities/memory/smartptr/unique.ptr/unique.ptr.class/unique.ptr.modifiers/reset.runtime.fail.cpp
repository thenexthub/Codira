/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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
// <memory>

// unique_ptr

// test reset

// UNSUPPORTED: nvrtc

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "unique_ptr_test_helper.h"

int main(int, char**)
{
  {
    cuda::std::unique_ptr<A[]> p;
    p.reset(static_cast<B*>(nullptr)); // expected-error {{no matching member function for call to 'reset'}}
  }
  {
    cuda::std::unique_ptr<int[]> p;
    p.reset(static_cast<const int*>(nullptr)); // expected-error {{no matching member function for call to 'reset'}}
  }

  return 0;
}
