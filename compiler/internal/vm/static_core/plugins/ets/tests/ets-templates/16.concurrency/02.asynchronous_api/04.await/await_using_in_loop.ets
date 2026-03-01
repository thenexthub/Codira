/*
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

{% from '16.concurrency/02.asynchronous_api/predefined_types.jinja' import numeric_types_cases, boolean_types_cases, symbol_types_cases, no_value_types_cases %}
{% from '16.concurrency/02.asynchronous_api/await_using_in_loop_predefined0.base.jinja' import generate_complete_test %}

{{ generate_complete_test(boolean_types_cases + symbol_types_cases + numeric_types_cases + no_value_types_cases) }}