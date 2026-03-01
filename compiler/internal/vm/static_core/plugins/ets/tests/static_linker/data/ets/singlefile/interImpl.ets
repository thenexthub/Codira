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
 
interface Shape {
    area(): double;
    perimeter(): double;
}

class Rectangle implements Shape {
    private width:double = 0.0;
	private height:double = 0.0;
	
    constructor(width: double, height: double) {
	   this.width = width
	   this.height = height
	}

    area(): double {
        return this.width * this.height;
    }

    perimeter(): double {
        return 2 * (this.width + this.height);
    }
}

class Circle implements Shape {
    private radius: double = 0.0;
    constructor(radius: double) {
	   this.radius = radius
	}

    area(): double {
        return 3.14 * this.radius * this.radius;
    }

    perimeter(): double {
        return 2 * 3.14 * this.radius;
    }
}



export function main() {
  const rect = new Rectangle(4, 5);
  const circle = new Circle(3);
  let area = rect.area()
  // CC-OFFNXT(G.AOD.03) for test purposes
  console.log(area)
  let perimeter = circle.perimeter()
  // CC-OFFNXT(G.AOD.03) for test purposes
  console.log(perimeter)
}
