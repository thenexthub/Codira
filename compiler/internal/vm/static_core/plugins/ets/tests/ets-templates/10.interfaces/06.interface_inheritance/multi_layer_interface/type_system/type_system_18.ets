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
desc: The union type of interfaces and classes is used to verify the correctness of the type system.
---*/

interface Car {
    drive(): string;
}

interface Boat {
    sail(): string;
}

class AmphibiousVehicle implements Car, Boat {
    drive(): string {
        return "Driving!";
    }
    
    sail(): string {
        return "Sailing!";
    }
}

function main(): void {
    const vehicle: Car | Boat = new AmphibiousVehicle();
    arktest.assertEQ(vehicle.drive(), "Driving!");
    arktest.assertEQ(vehicle.sail(), "Sailing!");
}
