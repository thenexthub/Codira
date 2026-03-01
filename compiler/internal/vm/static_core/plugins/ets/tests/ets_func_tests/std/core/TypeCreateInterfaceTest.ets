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

interface I1 {
	i(): int { return 0 }
}

interface I2 {
	i(x: int): int { return 0 }
}

class C implements I1, I2 {
	public i(): int { return 0  }
	public i(x: int): int { return x }
}

function getIfaces(): (InterfaceType | undefined)[] {
	const c = Type.of(new C()) as ClassType
	let n = c.getInterfacesNum().toInt()
	let r = new (InterfaceType | undefined)[n]
	for (let i = 0; i < n; i++) {
		r[i] = c.getInterface(i)
	}
	return r
}

function main(): void {
	const ifaces = getIfaces()
 arktest.assertEQ( ifaces.length, 2)
	const intType = Type.of(0)
	const I3 = new InterfaceTypeCreator("I3")
		.addInterface(ifaces[0]!)
		.addInterface(ifaces[1]!)
	const CI3 = new ClassTypeCreator("CI3");
	const i1Body: (self: I1) => int = (self: I1): int => { return 30 }
	const i2Body: (self: I2, x: int) => int = (self: I2, x: int): int => { return x * x + 3 }
	CI3
		.addInterface(I3)
		.addMethod(
			new MethodCreator("i")
				.addResult(intType)
				.addBody(new CallableBodyFunction(i1Body as Object))
		)
		.addMethod(
			new MethodCreator("i")
				.addParameter(new ParameterCreator(intType))
				.addResult(intType)
				.addBody(new CallableBodyFunction(i2Body as Object))
		)
		.addMethod(
			new MethodCreator("constructor")
			.addConstructor()
			.addBody(new CallableBodyDefault())
		)
	const ty = CI3.create()
	const inst = ty.make()
 arktest.assertEQ( (inst as I1).i(), 0)
 arktest.assertEQ( (inst as I2).i(3), 0)
}
