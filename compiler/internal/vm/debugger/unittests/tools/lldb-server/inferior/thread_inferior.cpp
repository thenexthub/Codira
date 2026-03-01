//===-- thread_inferior.cpp -----------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[]) {
  int thread_count = 2;
  if (argc > 1) {
    thread_count = std::stoi(argv[1], nullptr, 10);
  }

  std::atomic<bool> delay(true);
  std::vector<std::thread> threads;
  for (int i = 0; i < thread_count; i++) {
    threads.push_back(std::thread([&delay] {
      while (delay.load())
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }));
  }

  // Cause a break.
  volatile char *p = nullptr;
  *p = 'a';

  delay.store(false);
  for (std::thread& t : threads) {
    t.join();
  }

  return 0;
}
