/**
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

.extern StackOverflowExceptionEntrypoint

// This trampoline is needed because x86 push LR when executing a call instruction, but `rt_sigreturn` syscall just
// jumps to the given pc, thus, SP become unaligned.
// This trampoline solves this problem by just invoking the entrypoint via regular call.
.global StackOverflowExceptionEntrypointTrampoline
.type StackOverflowExceptionEntrypointTrampoline, %function
StackOverflowExceptionEntrypointTrampoline:
    callq   StackOverflowExceptionEntrypoint@plt
