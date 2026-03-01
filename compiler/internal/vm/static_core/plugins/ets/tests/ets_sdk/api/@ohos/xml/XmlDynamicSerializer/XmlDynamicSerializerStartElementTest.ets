/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { xml } from "@ohos.xml"

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Testing single element start element serialization.', testSingleElementStartElement);
    suite.addTest('Testing multiple element start and end elements serialization.',
        testMultipleElementStartAndEndElements);
    suite.addTest('Testing nested element start and end elements serialization.', testNestedElementStartAndEndElements);
    suite.addTest('Testing deeply nested element start and end elements serialization.',
        testDeeplyNestedElementStartAndEndElements);
    suite.addTest('Testing sibling elements within a parent element serialization.', testSiblingElementsWithinParent);

    return suite.run();
}

// Helper function to perform XML serialization and verification
function performXmlTest(operations: (serializer: xml.XmlDynamicSerializer) => void, expectedResult: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    operations(thatSer);
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_00600
function testSingleElementStartElement() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note1');
            serializer.endElement();
        },
        '<note1/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_00700
function testMultipleElementStartAndEndElements() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note1');
            serializer.endElement();
            serializer.startElement('note2');
            serializer.endElement();
        },
        '<note1/>\r\n<note2/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_00800
function testNestedElementStartAndEndElements() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note1');
            serializer.startElement('note2');
            serializer.endElement();
            serializer.endElement();
        },
        '<note1>\r\n  <note2/>\r\n</note1>'
    );
}

// Test cases ported from ArkTS 1.0: XML_00900
function testDeeplyNestedElementStartAndEndElements() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note1');
            serializer.startElement('note2');
            serializer.startElement('note3');
            serializer.endElement();
            serializer.endElement();
            serializer.endElement();
        },
        '<note1>\r\n  <note2>\r\n    <note3/>\r\n  </note2>\r\n</note1>'
    );
}

// Test cases ported from ArkTS 1.0: XML_01000
function testSiblingElementsWithinParent() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note1');
            serializer.startElement('note2');
            serializer.endElement();
            serializer.startElement('note3');
            serializer.endElement();
            serializer.endElement();
        },
        '<note1>\r\n  <note2/>\r\n  <note3/>\r\n</note1>'
    );
}
