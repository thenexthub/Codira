/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
import { BusinessError } from "@ohos.base"

const IllegalPositionErrorCodeId: number = 10200063;
const EmptyErrorCodeId: number = 10200064;

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Test setting declaration after element throws an error.', testSetDeclarationAfterDeclaration);
    suite.addTest('Test starting element with empty name throws an error.', testStartElementWithEmptyName);
    suite.addTest('Test setting namespace with empty prefix throws an error.', testSetNamespaceWithEmptyPrefix);
    suite.addTest('Test setting namespace with empty URI throws an error.', testSetNamespaceWithEmptyURI);
    suite.addTest('Test adding empty element with empty name throws an error.', testAddEmptyElementWithEmptyName);
    suite.addTest('Test setting empty comment throws an error.', testSetEmptyComment);
    suite.addTest('Test setting empty CDATA throws an error.', testSetEmptyCDATA);
    suite.addTest('Test setting empty text throws an error.', testSetEmptyText);
    suite.addTest('Test setting empty DOCTYPE throws an error.', testSetEmptyDocType);

    return suite.run();
}

function performXmlTest(
    errorCodeId: number,
    errorMessage: string,
    setupFunction: (serializer: xml.XmlDynamicSerializer) => void,
    testFunction: (serializer: xml.XmlDynamicSerializer) => void
) {
    let thatSer = new xml.XmlDynamicSerializer();

    setupFunction(thatSer);

    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == errorCodeId && e.message == errorMessage;
        }
        return false;
    }

    arktest.expectThrow(() => { testFunction(thatSer) }, exceptionCheck);
}

function testSetDeclarationAfterDeclaration() {
    performXmlTest(
        IllegalPositionErrorCodeId,
        'Illegal position for xml',
        (serializer) => { serializer.setDeclaration(); },
        (serializer) => { serializer.setDeclaration(); }
    );
}

function testStartElementWithEmptyName() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {},
        (serializer) => { serializer.startElement(''); }
    );
}

function testSetNamespaceWithEmptyPrefix() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {},
        (serializer) => { serializer.setNamespace('', 'http://www.w3.org/TR/html4/'); }   // CC-OFF(G.OTH.05) test logic
    );
}

function testSetNamespaceWithEmptyURI() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {},
        (serializer) => { serializer.setNamespace('h', ''); }
    );
}

function testAddEmptyElementWithEmptyName() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {
            serializer.setDeclaration();
            serializer.startElement('note');
        },
        (serializer) => { serializer.addEmptyElement(''); }
    );
}

function testSetEmptyComment() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {
            serializer.setDeclaration();
            serializer.startElement('note');
        },
        (serializer) => { serializer.setComment(''); }
    );
}

function testSetEmptyCDATA() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {
            serializer.setDeclaration();
            serializer.startElement('note');
        },
        (serializer) => { serializer.setCdata(''); }
    );
}

function testSetEmptyText() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {
            serializer.setDeclaration();
            serializer.startElement('note');
        },
        (serializer) => { serializer.setText(''); }
    );
}

function testSetEmptyDocType() {
    performXmlTest(
        EmptyErrorCodeId,
        'Parameter error. Parameter cannot be empty',
        (serializer) => {
            serializer.setDeclaration();
            serializer.startElement('note');
        },
        (serializer) => { serializer.setDocType(''); }
    );
}
