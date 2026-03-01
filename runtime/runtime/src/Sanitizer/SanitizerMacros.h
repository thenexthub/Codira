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


#ifndef CODIRARUNTIME_SANITIZERMACROS_H
#define CODIRARUNTIME_SANITIZERMACROS_H

// definitions copy from linux/syscalls.h
// these defs are needed to expand function param declaration

// ARG_MAP0 is a special macro for functions has no args
#define ARG_MAP0(m, t, a, ...) m(t,)
#define ARG_MAP1(m, t, a, ...) m(t, a)
#define ARG_MAP2(m, t, a, ...) m(t, a), ARG_MAP1(m, __VA_ARGS__)
#define ARG_MAP3(m, t, a, ...) m(t, a), ARG_MAP2(m, __VA_ARGS__)
#define ARG_MAP4(m, t, a, ...) m(t, a), ARG_MAP3(m, __VA_ARGS__)
#define ARG_MAP5(m, t, a, ...) m(t, a), ARG_MAP4(m, __VA_ARGS__)
#define ARG_MAP6(m, t, a, ...) m(t, a), ARG_MAP5(m, __VA_ARGS__)
#define ARG_MAP(n, ...) ARG_MAP##n(__VA_ARGS__)

#define ARG_DECL(t, a) t a
#define ARG_TYPE(t, a) t
#define ARG_NAME(t, a) a

#define FUNC_TYPE(x) x##_type
#define SANITIZER_FUNC_TYPE(x) ::MapleRuntime::Sanitizer::FUNC_TYPE(x)
#define PTR_TO_REAL(x) x##_fn
#define REAL(x) ::MapleRuntime::Sanitizer::PTR_TO_REAL(x)
#endif // CODIRARUNTIME_SANITIZERMACROS_H
