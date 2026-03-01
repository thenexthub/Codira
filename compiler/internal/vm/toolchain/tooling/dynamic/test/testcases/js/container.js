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

var o = {
    "foo" : function() {
        var ArrayList = ArkPrivate.Load(ArkPrivate.ArrayList);
        let arrayList = new ArrayList();
        arrayList.add(5);
        arrayList.insert(4, 0);
        arrayList.add(8);
        arrayList.insert(18, 2);
        arrayList.add(15);
        arrayList.add(3);
        arrayList.add(10);
        arrayList.add(288);
        arrayList.add(188);
        arrayList.removeByIndex(1);
        arrayList.removeByRange(0, 2);

        var Deque = ArkPrivate.Load(ArkPrivate.Deque);
        let deque = new Deque();
        deque.insertFront(5);
        deque.insertFront(1888);
        deque.insertEnd(3);
        deque.insertFront(0);
        deque.insertFront(1);
        deque.insertEnd(13);
        deque.insertEnd(1888);
        deque.insertEnd(17);
        deque.insertEnd(888);
        deque.insertEnd(387);
        deque.insertEnd(666);
        deque.insertEnd(177);
        deque.insertEnd(999);
        deque.popFirst();
        deque.popLast();
        deque.popFirst();
        var boolean0 = deque.has(999);
        var number0 = deque.getFirst();
        var number1 = deque.getLast();

        var HashMap = ArkPrivate.Load(ArkPrivate.HashMap);
        let hashMap = new HashMap();
        hashMap.set(3, 1);
        hashMap.set(0, 19);
        hashMap.set(2, 8);
        hashMap.set(9, 8);
        hashMap.set(38, 1);
        hashMap.set('three', 8);
        hashMap.set(98, 88);
        hashMap.set(0, 8);
        hashMap.set(103, 99);
        hashMap.set(77, 8);
        hashMap.set('two', 88);
        hashMap.set('zero', 1888);
        hashMap.clear();
        hashMap.set(1, 188);
        hashMap.set(98,188);
        hashMap.set('one', 66);
        hashMap.remove(1);
        hashMap.remove('one');

        var HashSet = ArkPrivate.Load(ArkPrivate.HashSet);
        let hashSet = new HashSet();
        hashSet.add(5);
        hashSet.add(18);
        hashSet.add(2);
        hashSet.add(18);
        hashSet.add(8);
        hashSet.add(11);
        hashSet.add(1);
        hashSet.add(99);
        hashSet.add(112);
        hashSet.add(0);
        hashSet.add(433);
        hashSet.add(17);
        hashSet.add('one');
        hashSet.add(537);
        hashSet.add('four');
        hashSet.add(1888);
        hashSet.add('three');
        hashSet.add(588);
        hashSet.add(38);
        hashSet.remove(1);
        hashSet.remove(2);
        hashSet.remove(11);
        hashSet.remove(588);

        var LightWeightMap = ArkPrivate.Load(ArkPrivate.LightWeightMap);
        let lightWeightMap = new LightWeightMap();
        lightWeightMap.set(8, 5);
        lightWeightMap.set(3, 18);
        lightWeightMap.set(7, 3);
        lightWeightMap.set(1, 288);
        lightWeightMap.set('three', 88);
        lightWeightMap.set(9, 7);
        lightWeightMap.set(111, 12);
        lightWeightMap.set(555, 31);
        lightWeightMap.set(777, 18);
        lightWeightMap.set(222, 17);
        lightWeightMap.set(0, 7);
        lightWeightMap.set(1, 12);
        lightWeightMap.set(157, 31);
        lightWeightMap.set(66, 18);
        lightWeightMap.set(45, 17);
        lightWeightMap.set('four', 333);
        lightWeightMap.remove(3);
        lightWeightMap.remove(157);
        lightWeightMap.remove(1999);
        lightWeightMap.removeAt(2);
        lightWeightMap.removeAt(1);
        lightWeightMap.removeAt(1);
        var number2 = lightWeightMap.getIndexOfKey(157);
        var number3 = lightWeightMap.getIndexOfValue(5);
        var number4 = lightWeightMap.getKeyAt(5);
        lightWeightMap.clear();
        lightWeightMap.set(0, 0);

        var LightWeightSet = ArkPrivate.Load(ArkPrivate.LightWeightSet);
        let lightWeightSet = new LightWeightSet();
        lightWeightSet.add(5);
        lightWeightSet.add(188);
        lightWeightSet.add('100');
        lightWeightSet.add(4);
        lightWeightSet.add(388);
        lightWeightSet.add(111);
        lightWeightSet.add(12);
        lightWeightSet.add('four');
        lightWeightSet.add(77);
        lightWeightSet.add(593);
        lightWeightSet.add(66);
        lightWeightSet.add(12);
        lightWeightSet.add(855);
        lightWeightSet.add('three');
        lightWeightSet.add(578);
        lightWeightSet.add(58);
        lightWeightSet.add('one');
        lightWeightSet.add(593);
        lightWeightSet.add(566);
        lightWeightSet.remove(5);
        lightWeightSet.remove(12);
        lightWeightSet.remove(77);
        lightWeightSet.remove(111);
        lightWeightSet.removeAt(1);
        lightWeightSet.removeAt(2);
        var number5 = lightWeightSet.getIndexOf(58);
        var number6 = lightWeightSet.getValueAt(3);

        var LinkedList = ArkPrivate.Load(ArkPrivate.LinkedList);
        let linkedList = new LinkedList();
        linkedList.add(88);
        linkedList.add(4);
        linkedList.add(188);
        linkedList.add(18);
        linkedList.addFirst('one');
        linkedList.add(187);
        linkedList.set(3, 1888);
        linkedList.set(4, 888);
        linkedList.set(2, 3989);
        linkedList.addFirst('one');
        linkedList.addFirst('one');
        linkedList.add('two');
        linkedList.insert(1,588);
        linkedList.add('one');
        linkedList.insert(3,388);
        linkedList.removeFirst();
        linkedList.removeLast();
        linkedList.removeByIndex(1);
        var string0 = linkedList.getLast();
        var number7 = linkedList.get(0);
        var number8 = linkedList.getFirst();
        var number9 = linkedList.getIndexOf(588);

        var List = ArkPrivate.Load(ArkPrivate.List);
        let list = new List();
        list.add(18);
        list.add(1587);
        list.add(527);
        list.add(0);
        list.add('one');
        list.add('three');
        list.add(88);
        list.add('nine');
        list.add(18);
        list.add('100');
        list.add(322);
        list.set(1,'changge');
        list.insert(2, 2);
        list.set(5,'changge');
        list.set(8,'changge');
        list.remove(3);
        var number10 = list.getIndexOf(8);
        var number11 = list.get(7);
        var number12 = list.getFirst();
        var number13 = list.getLast();
        var number14 = list.getLastIndexOf('changge');

        var PlainArray = ArkPrivate.Load(ArkPrivate.PlainArray);
        let plainArray = new PlainArray();
        plainArray.add(0, 5);
        plainArray.add(1, 158);
        plainArray.add(2, 3);
        plainArray.add(3, 8888);
        plainArray.add(857, 88)
        plainArray.add(33, 33);
        plainArray.add(33, 38);
        plainArray.add(588, 188);
        plainArray.add(318, 518)
        plainArray.add(397, 198);
        plainArray.add(8, 8);
        plainArray.add(88, 188);
        plainArray.add(18, 58)
        plainArray.setValueAt(0,857);
        plainArray.setValueAt(3,857);
        var boolean1 = plainArray.has(588);
        var number15 = plainArray.getIndexOfKey(8);
        var number16 = plainArray.getIndexOfValue(518);
        plainArray.clear();
        plainArray.add(8, 8);

        var Queue = ArkPrivate.Load(ArkPrivate.Queue);
        let queue = new Queue();
        queue.add(5);
        queue.add(188);
        queue.add(3);
        queue.add(2888);
        queue.add(1);
        queue.add('555');
        queue.add(857);
        queue.add('eleven');
        queue.add(999);
        queue.add('male');
        var number17 = queue.getFirst();
        queue.pop();
        var number18 = queue.getFirst();
        queue.pop();

        var Stack = ArkPrivate.Load(ArkPrivate.Stack);
        let stack = new Stack();
        stack.push(5);
        stack.push(18);
        stack.push(3);
        stack.push(28);
        stack.push(1);
        stack.push('one');
        stack.push(888);
        stack.push(857);
        stack.push('six');
        stack.push(158);
        stack.push(5);
        stack.push(5);
        stack.pop();
        stack.pop();
        stack.pop();
        stack.pop();
        stack.pop();

        var TreeMap = ArkPrivate.Load(ArkPrivate.TreeMap);
        let treeMap = new TreeMap();
        treeMap.set(0, 5);
        treeMap.set(3, 1888);
        treeMap.set('nine', 588);
        treeMap.set(2, 4);
        treeMap.set('three', 588);
        treeMap.set(99, 5);
        treeMap.set(87, 4);
        treeMap.set(3, 88);
        treeMap.set(388, 588);
        treeMap.set('one', 888);
        treeMap.replace(2, 8);
        treeMap.replace(388, 'change');
        var boolean2 = treeMap.hasKey(888);
        var string1 = treeMap.get(388);
        var string2 = treeMap.getLastKey();
        var number19 = treeMap.get(2);
        var number20 = treeMap.getHigherKey(99);
        var number21 = treeMap.getFirstKey();
        treeMap.clear();
        treeMap.set(88, 88);

        var TreeSet = ArkPrivate.Load(ArkPrivate.TreeSet);
        let treeSet = new TreeSet();
        treeSet.add(5);
        treeSet.add(18);
        treeSet.add(4);
        treeSet.add(2);
        treeSet.add(18);
        treeSet.add(377);
        treeSet.add(99);
        treeSet.add(1388);
        treeSet.add(588);
        treeSet.add('ten');
        treeSet.add(888);
        treeSet.add(11199);
        treeSet.add('one');
        treeSet.add(2);
        treeSet.add(18);
        treeSet.add('array');
        treeSet.add(377);
        treeSet.remove(1);
        treeSet.remove(0);
        treeSet.popFirst();
        treeSet.popFirst();
        treeSet.popLast();
        treeSet.popLast();
        var boolean3 = treeSet.has(11199);
        var string3 = treeSet.getLastValue();
        var number22 = treeSet.getFirstValue();
        var number23 = treeSet.getLowerValue(99);
        var number24 = treeSet.getHigherValue(888);

        var Vector = ArkPrivate.Load(ArkPrivate.Vector);
        let vector = new Vector();
        vector.add(111);
        vector.add('ten');
        vector.add(857);
        vector.add(33);
        vector.add('male');
        vector.add(88);
        vector.add(77);
        vector.add('one');
        vector.insert(4, 0);
        vector.insert('change', 5);
        vector.insert('change', 8);
        vector.set(2, 999);
        vector.set(7, 999);
        vector.set(9, 999);
        vector.remove(88);
        vector.removeByIndex(1);
        vector.removeByRange(0, 2);
        var string4 = vector.get(3);
        var string5 = vector.getLastElement();        
        var number25 = vector.getLastIndexOf(999);
        var number26 = vector.getIndexOf(857);
        var number27 = vector.getFirstElement();

        // sendable
        let sendableArray = new SendableArray(1, 2);
        let sendableArray0 = sendableArray[0];
        let sendableArray1 = sendableArray[1];
        let sendableArraySize = sendableArray.length;
        let sendableMap = new SendableMap([
            [0, 1],
            [1, 2],
        ]);
        let sendableMap0 = sendableMap.get(0);
        let sendableMap1 = sendableMap.get(1);
        let sendableMapSize = sendableMap.size;
        let sendableSet = new SendableSet();
        sendableSet.add(1);
        sendableSet.add(2);
        let sendableSet0 = sendableSet.has(1)
        let sendableSet1 = sendableSet.has(2)
        let sendableSetSize = sendableSet.size;

        var nop = undefined;
    }
}

o.foo()
