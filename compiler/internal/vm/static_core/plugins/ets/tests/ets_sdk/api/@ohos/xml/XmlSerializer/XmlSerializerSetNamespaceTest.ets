/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import { xml } from "@ohos.xml"
import { BusinessError } from "@ohos.base"

const TypeErrorCodeId: int = 401;
const MaxBufferLength = 2048;

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlSerializer API tests");

    suite.addTest("Testing empty prefix for namespace", testEmptyPrefixForNamespace);
    suite.addTest("Testing empty namespace value", testEmptyNamespaceValue);
    suite.addTest("Testing namespace setting with arrayBuffer overflow", testNamespaceSettingWithOverflow);
    suite.addTest("Test setting namespace with prefix 'h' and URI 'http://www.w3.org/TR/html4/'.",
        testSetNamespaceWithPrefixH);
    suite.addTest("Test setting namespace with prefix 'b' and URI 'http://www.w3.org/TR/html4/'.",
        testSetNamespaceWithPrefixB);
    suite.addTest("Test setting namespace with prefix 'h' and URI 'http://www.111/'.",
        testSetNamespaceWithDifferentURI);
    suite.addTest("Test nested elements with namespace prefix 'h' and URI 'http://www.w3.org/TR/html4/'.",
        testNestedElementsWithNamespace);

    return suite.run();
}

/**
 * Common function to perform XML serialization tests
 * @param serializerOperation Function to perform operations on the serializer
 * @param expectedResult Expected XML string result
 */
function performXmlTest(serializerOperation: (serializer: xml.XmlSerializer) => void, expectedResult: string): void {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let serializer = new xml.XmlSerializer(arrayBuffer);

    serializerOperation(serializer);

    let view = new Uint8Array(arrayBuffer);
    let result = '';
    for (let i: int = 0; i < expectedResult.length; ++i) {
        result += String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(result, expectedResult);
}

function testEmptyPrefixForNamespace() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Parameter error. Parameter cannot be empty";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.setNamespace("","http://www.w3.org/TR/html4/")}, exceptionCheck);  // CC-OFF(G.OTH.05) test logic
}

function testEmptyNamespaceValue() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "Parameter error. The type of  must be string";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.setNamespace("h","")}, exceptionCheck);
}

function testNamespaceSettingWithOverflow() {
    let arrayBuffer = new ArrayBuffer(35);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    thatSer.endElement();
    thatSer.startElement("note1");
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "XmlSerializer:: SetNamespace memcpy_s failed";
        }
        return false;
    }
    arktest.expectThrow(() => {thatSer.setNamespace("b", "bbb")}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_03100
function testSetNamespaceWithPrefixH() {
    performXmlTest((serializer) => {
        serializer.setDeclaration();
        serializer.setNamespace('h', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
        serializer.startElement('note');
        serializer.endElement();
    }, '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note xmlns:h="http://www.w3.org/TR/html4/"/>');
}

// Test cases ported from ArkTS 1.0: XML_03200
function testSetNamespaceWithPrefixB() {
    performXmlTest((serializer) => {
        serializer.setDeclaration();
        serializer.setNamespace('b', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
        serializer.startElement('note');
        serializer.endElement();
    }, '<?xml version="1.0" encoding="utf-8"?>\r\n<b:note xmlns:b="http://www.w3.org/TR/html4/"/>');
}

// Test cases ported from ArkTS 1.0: XML_03300
function testSetNamespaceWithDifferentURI() {
    performXmlTest((serializer) => {
        serializer.setDeclaration();
        serializer.setNamespace('h', 'http://www.111/');
        serializer.startElement('note');
        serializer.endElement();
    }, '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note xmlns:h="http://www.111/"/>');
}

// Test cases ported from ArkTS 1.0: XML_03400
function testNestedElementsWithNamespace() {
    performXmlTest((serializer) => {
        serializer.setDeclaration();
        serializer.setNamespace('h', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
        serializer.startElement('note1');
        serializer.startElement('note2');
        serializer.endElement();
        serializer.endElement();
    }, '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note1 xmlns:h="http://www.w3.org/TR/html4/">\r\n  <h:note2/>\r\n</h:note1>');
}
