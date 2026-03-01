/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

const {
    getTestClass,
} = require('ets_proxy.test.abc');

const ListNode = getTestClass('ListNode');

{
	// test proxy-ctor capabilities
	ASSERT_TRUE(Object.isSealed(ListNode));

	let expected = ['next', 'tag', 'getNext', 'setNext', 'constructor'];
	let props = Object.getOwnPropertyNames(ListNode.prototype);
	ASSERT_TRUE(expected.every((p) => props.includes(p)));

	ASSERT_TRUE(!Object.hasOwn(ListNode, '<cctor>'));
}

{
	// test basic proxy-object capabilities
	let n1 = new ListNode(1);
	ASSERT_EQ(n1.tag, 1);
	ASSERT_EQ(n1.next, undefined);

	// napi_xref_wrap add _proxynapiwrapper property
	ASSERT_TRUE(Object.getOwnPropertyNames(n1).length === 1);
	ASSERT_TRUE(Object.isSealed(n1));
}

{
	// test proxy-reference wrap/unwrap invariant
	let n1 = new ListNode(1);
	let n2 = new ListNode(2, n1);
	ASSERT_EQ(n2.tag, 2);
	ASSERT_EQ(n2.next, n1);
	ASSERT_EQ(n2.getNext(), n1);
	n2.setNext(n1);
	ASSERT_EQ(n2.next, n1);
}

{
	// create loop and walk it
	let head = new ListNode(0);
	head.next = new ListNode(1, new ListNode(2, new ListNode(3, head)));

	for (let n = head, i = 0; i < 256; ++i, n = n.next) {
		ASSERT_EQ(n.tag, i % 4);
	}
}

{
	// proxy-object escape ctor
	const ProxyEscapeCtor = getTestClass('ProxyEscapeCtor');
	let capt;
	let obj = new ProxyEscapeCtor((x) => (capt = x));
	ASSERT_EQ(capt, obj);
}

{
	// proxy-method throws
	const ProxyClassCalls = getTestClass('ProxyClassCalls');
	class TestErr extends Error {}
	let doThrow = function () {
		throw new TestErr();
	};

	ASSERT_THROWS(TestErr, () => new ProxyClassCalls(doThrow));

	ASSERT_THROWS(TestErr, () => ProxyClassCalls.call(doThrow));
}
