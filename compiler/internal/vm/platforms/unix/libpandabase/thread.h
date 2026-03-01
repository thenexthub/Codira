/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PLATFORMS_UNIX_LIBPANDABASE_THREAD_H
#define PLATFORMS_UNIX_LIBPANDABASE_THREAD_H

namespace panda::os::thread {
constexpr int LOWEST_PRIORITY = 19;

int GetPriority(int thread_id);
int SetPriority(int thread_id, int prio);
}  // namespace panda::os::thread

#endif  // PLATFORMS_UNIX_LIBPANDABASE_THREAD_H
