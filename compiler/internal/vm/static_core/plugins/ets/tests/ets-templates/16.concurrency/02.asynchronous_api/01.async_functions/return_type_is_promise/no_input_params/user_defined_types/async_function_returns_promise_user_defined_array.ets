/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% from "16.concurrency/02.asynchronous_api/user_defined_types.jinja" import class_cases, enum_cases, function_cases, tuple_cases, union_cases %}
{% from "16.concurrency/02.asynchronous_api/async_returns_array.base.jinja" import generate_complete_test %}

{{ generate_complete_test(class_cases + enum_cases + function_cases + tuple_cases + union_cases, 'function') }}
