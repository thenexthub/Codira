/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
#include "ohos.multimedia.image.image.impl.hpp"
#include "ohos.multimedia.image.image.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

namespace {
// To be implemented.

class PixelMapImpl {
public:
    PixelMapImpl() {}
};

class ImageSourceImpl {
public:
    ImageSourceImpl() {}

    ::ohos::multimedia::image::image::PixelMap CreatePixelMapSync(
        ::ohos::multimedia::image::image::DecodingOptions const &options)
    {
        return taihe::make_holder<PixelMapImpl, ::ohos::multimedia::image::image::PixelMap>();
    }
};

::ohos::multimedia::image::image::ImageSource CreateImageSource()
{
    return taihe::make_holder<ImageSourceImpl, ::ohos::multimedia::image::image::ImageSource>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateImageSource(CreateImageSource);
// NOLINTEND