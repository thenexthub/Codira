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

    suite.addTest('Testing namespace without setting attributes.', testNamespaceWithoutAttributes);
    suite.addTest('Testing namespace with a single attribute.', testNamespaceWithSingleAttribute);
    suite.addTest('Testing namespace with nested elements.', testNamespaceWithNestedElements);
    suite.addTest('Testing namespace, attributes, and text for an element.', testSetNamespaceWithAttributesAndText);

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

// Test cases ported from ArkTS 1.0: XML_02100
function testNamespaceWithoutAttributes() {
    performXmlTest(
        (serializer) => {
            serializer.setDeclaration();
            serializer.setNamespace('h', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
            serializer.startElement('note');
            serializer.endElement();
        },
        '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note xmlns:h="http://www.w3.org/TR/html4/"/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_02200
function testNamespaceWithSingleAttribute() {
    performXmlTest(
        (serializer) => {
            serializer.setDeclaration();
            serializer.setNamespace('b', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
            serializer.startElement('note');
            serializer.setAttributes('a', '1');
            serializer.endElement();
        },
        '<?xml version="1.0" encoding="utf-8"?>\r\n<b:note a="1" xmlns:b="http://www.w3.org/TR/html4/"/>'
    );
}

// Test cases ported from ArkTS 1.0: XML_02300
function testSetNamespaceWithAttributesAndText() {
    performXmlTest(
        (serializer) => {
            serializer.setDeclaration();
            serializer.setNamespace('b', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
            serializer.startElement('note');
            serializer.setAttributes('a','1');
            serializer.setText('cat');
            serializer.endElement();
        },
        '<?xml version="1.0" encoding="utf-8"?>\r\n<b:note a="1" xmlns:b="http://www.w3.org/TR/html4/">cat</b:note>'
    );
}

// Test cases ported from ArkTS 1.0: XML_02400
function testNamespaceWithNestedElements() {
    performXmlTest(
        (serializer) => {
            serializer.setDeclaration();
            serializer.setNamespace('h', 'http://www.w3.org/TR/html4/');  // CC-OFF(G.OTH.05) test logic
            serializer.startElement('note1');
            serializer.startElement('note2');
            serializer.endElement();
            serializer.endElement();
        },
        '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note1 xmlns:h="http://www.w3.org/TR/html4/">\r\n  <h:note2/>\r\n</h:note1>'
    );
}
