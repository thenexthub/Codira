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

 /*---
 desc: Test a generic method with char constraint used in reading from simulated epoll event
 name: generic_method_with_char_constraint_and_epoll
 params: 'char'
 ---*/

class EpollSimulator {
  private data: string = "";

  // Simulate adding data to epoll monitored socket
  sendData(data: string): void {
    this.data = data;
  }

  // Simulate reading from epoll
  readData(): string {
    return this.data;
  }
}

// Generic method with char constraint to read from simulated epoll
function onEpollRead<X extends string>(simulator: EpollSimulator): X {
  const data = simulator.readData();
  return data.charAt(0) as X;
}

function main(): void {
  const epollSim = new EpollSimulator();

  // Simulate sending 'N' to the socket
  epollSim.sendData("N");

  // Call generic method constrained to char
  const result = onEpollRead<string>(epollSim);

  // Assert the result is 'N'
  if (result !== 'N') {
    throw new Error(`Expected 'N', but got '${result}'`);
  }
}

// Run the test
main();
