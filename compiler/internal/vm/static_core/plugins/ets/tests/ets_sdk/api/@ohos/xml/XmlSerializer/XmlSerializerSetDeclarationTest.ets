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
    const suite = new arktest.ArkTestsuite('XmlSerializer API tests');

    suite.addTest('Test writing XML declaration with UTF-8 encoding.', testWriteDeclarationWithUTF8);
    suite.addTest('Test writing XML declaration without specifying encoding.', testWriteDeclarationWithoutEncoding);
    suite.addTest('Test writing XML declaration at an illegal position throws an error.',
        testWriteDeclarationAtIllegalPositionThrowsError);

    return suite.run();
}

/**
 * Common function to perform XML serialization tests
 * @param serializerOperation Function to perform operations on the serializer
 * @param expectedResult Expected XML string result
 * @param encoding Optional encoding parameter for XmlSerializer
 */
function performXmlTest(
    serializerOperation: (serializer: xml.XmlSerializer) => void, 
    expectedResult: string, 
    encoding?: string
): void {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let serializer: xml.XmlSerializer;
    if (encoding == undefined) {
        serializer = new xml.XmlSerializer(arrayBuffer);
    } else {
        serializer = new xml.XmlSerializer(arrayBuffer, encoding);
    }
    
    serializerOperation(serializer);
    
    let view = new Uint8Array(arrayBuffer);
    let result = '';
    for (let i = 0; i < expectedResult.length; ++i) {
        result += String.fromCodePoint(view[i].toInt());
    }
    arktest.assertEQ(result, expectedResult);
}

// Test cases ported from ArkTS 1.0: XML_00100
function testWriteDeclarationWithUTF8() {
    performXmlTest(
        (serializer) => {
            serializer.setDeclaration();
        },
        '<?xml version="1.0" encoding="utf-8"?>',
        'utf-8'
    );
}

// Test cases ported from ArkTS 1.0: XML_00400
function testWriteDeclarationWithoutEncoding() {
    performXmlTest(
        (serializer) => {
            serializer.setDeclaration();
        },
        '<?xml version="1.0" encoding="utf-8"?>'
    );
}

function testWriteDeclarationAtIllegalPositionThrowsError() {
    let arrayBuffer = new ArrayBuffer(MaxBufferLength);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setDeclaration();
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == TypeErrorCodeId &&
                e.message == "illegal position for declaration";
        }
        return false;
    }
    arktest.expectThrow(() => { thatSer.setDeclaration() }, exceptionCheck);
}
