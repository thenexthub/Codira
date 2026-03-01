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

function main() {
    // Basic counter
    console.count();
    console.count();
    console.count();
    
    // Named counter
    console.count("userClick");
    console.count("userClick");
    console.count("userClick");
    
    // Multiple named counters
    console.count("loop1");
    console.count("loop2");
    console.count("loop1");
    console.count("loop2");
    
    // Reset default counter
    console.countReset();
    console.count();
    
    // Reset named counter
    console.count("userAction");
    console.count("userAction");
    console.countReset("userAction");
    console.count("userAction");
    
    // Counter in loops
    for (let i = 0; i < 3; i++) {
        console.count("forLoop");
    }
    
    // Reset and reuse
    console.count("test");
    console.countReset("test");
    console.count("test");
    
    // Multiple resets
    console.count("multi");
    console.countReset("multi");
    console.count("multi");
    console.countReset("multi");
}
