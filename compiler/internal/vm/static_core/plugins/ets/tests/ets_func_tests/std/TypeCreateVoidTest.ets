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

function foo(): Void { return Void.void_instance }

function test(a: Object): void {
 arktest.assertTrue((a instanceof Void))
}

class C {
	foo(): Void { throw new Error("not overriden") }
	bar(): Void { throw new Error("not overriden") }
}

function main(): void {
	test(foo());

	const called = new boolean[1];

	const release: (a: Object) => Void = (a: Object): Void => {
		called[0] = true;
		return Void.void_instance
	}

	const returnVoid: (a: Object) => Void = (a: Object): Void => {
		return Void.void_instance
	}

	const creator =
		new ClassTypeCreator("C1")
			.addBaseType(Type.of(new C()) as ClassType)
			.addMethod(
				new MethodCreator("foo")
					.addResult(VoidType.REF)
					.addBody(new CallableBodyFunction(release))
			)
			.addMethod(
				new MethodCreator("bar")
					.addResult(VoidType.REF)
					.addBody(new CallableBodyFunction(returnVoid))
			)
			.addMethod(
				new MethodCreator("constructor")
					.addConstructor()
					.addResult(VoidType.REF)
					.addBody(new CallableBodyDefault())
			)
			;

	const C1 = creator.create()
	const c1 = C1.make() as C

	test(c1.foo())
	test(c1.bar())

	// TODO(kprokopenko): as this function is called multiple times,
	// same class creation will lead to mismatched lambda context...
	// assertTrue called on called[0]
}
