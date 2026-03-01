/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
// defined in libm.so, fmodf function from math.h
extern "C" float fmodf(float a, float b);  // NOLINT(misc-definitions-in-headers,readability-identifier-naming)
// defined in libm.so, fmod function from math.h
extern "C" double fmod(double a, double b);  // NOLINT(misc-definitions-in-headers,readability-identifier-naming)
