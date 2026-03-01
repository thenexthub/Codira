/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_PLUGINS_ETS_COMPILER_INTRINSIC_ID_MAPPING_INL_H
#define PANDA_PLUGINS_ETS_COMPILER_INTRINSIC_ID_MAPPING_INL_H

using ark::compiler::RuntimeInterface;

RuntimeInterface::EntrypointId GetEntrypointByIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    RuntimeInterface::EntrypointId entrypoint;
    switch (id) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_INT8_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_INT8_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_INT16_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_INT16_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_INT32_ARRAY_COPY_WITHIN_IMPL:
        case RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT32_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_INT32_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_INT64_ARRAY_COPY_WITHIN_IMPL:
        case RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT64_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_BIG_INT64_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT8_ARRAY_COPY_WITHIN_IMPL:
        case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT8_CLAMPED_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_U_INT8_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT16_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_U_INT16_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_U_INT32_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_U_INT32_COPY_WITHIN;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_BIG_U_INT64_ARRAY_COPY_WITHIN_IMPL:
            entrypoint = RuntimeInterface::EntrypointId::ARRAY_BIG_U_INT64_COPY_WITHIN;
            break;

        default:
            UNREACHABLE();
            break;
    }
    return entrypoint;
}

#endif  // PANDA_PLUGINS_ETS_COMPILER_INTRINSIC_ID_MAPPING_INL_H
