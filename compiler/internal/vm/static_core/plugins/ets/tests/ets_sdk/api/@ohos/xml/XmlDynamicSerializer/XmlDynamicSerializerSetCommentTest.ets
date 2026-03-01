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

    suite.addTest('Write a comment between start and end elements.', testSetCommentBetweenStartAndEnd);
    suite.addTest('Write a comment between end and start elements.', testSetCommentBetweenEndAndStart);
    suite.addTest('Write a comment without any start or end elements.', testSetCommentWithoutStartOrEnd);
    suite.addTest('Write a comment before a start element.', testSetCommentBeforeStart);

    return suite.run();
}

// Common utility function to perform XML tests
function performXmlTest(xmlOperation: (serializer: xml.XmlDynamicSerializer) => void, expectedResult: string): void {
    let thatSer = new xml.XmlDynamicSerializer();
    xmlOperation(thatSer);
    let arrayBuffer = thatSer.getOutput();
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(view1, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_02600
function testSetCommentBetweenStartAndEnd() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.setComment('Hi!');
            serializer.endElement();
        },
        '<note>\r\n  <!--Hi!-->\r\n</note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_02700
function testSetCommentBetweenEndAndStart() {
    performXmlTest(
        (serializer) => {
            serializer.startElement('note');
            serializer.endElement();
            serializer.setComment('Hello, World!');
            serializer.startElement('cat');
            serializer.endElement();
        },
        '<note/>\r\n<!--Hello, World!-->\r\n<cat/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_02800
function testSetCommentWithoutStartOrEnd() {
    performXmlTest(
        (serializer) => {
            serializer.setComment('Hello, World!');
        },
        '<!--Hello, World!-->'
    );
}

// Test cases ported from ArkTS 1.0: XML_02900
function testSetCommentBeforeStart() {
    performXmlTest(
        (serializer) => {
            serializer.setComment('Hello, World!');
            serializer.startElement('note');
            serializer.endElement();
        },
        '<!--Hello, World!-->\r\n<note/>'
    );
}
