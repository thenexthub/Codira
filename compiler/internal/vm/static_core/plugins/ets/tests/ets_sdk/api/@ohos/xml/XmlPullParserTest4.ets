/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import {xml} from "@ohos.xml"
import { BusinessError } from "@ohos.base";


function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part4");
    suite.addTest("Testing XML ParseInfo getDepth with ENTITY_REFERENCE event.", testXmlParseInfoGetDepth0010)
    suite.addTest("Testing XML ParseInfo getDepth with WHITESPACE event.", testXmlParseInfoGetDepth0011)
    suite.addTest("Testing XML ParseInfo getDepth with INSTRUCTION event.", testXmlParseInfoGetDepth0009)
    suite.addTest("Testing XML ParseInfo getDepth with DOCDECL event.", testXmlParseInfoGetDepth0008)
    suite.addTest("Testing XML ParseInfo getDepth with COMMENT event.", testXmlParseInfoGetDepth0007)
    suite.addTest("Testing XML ParseInfo getDepth with CDSECT event.", testXmlParseInfoGetDepth0006)
    suite.addTest("Testing XML ParseInfo getDepth with TEXT event.", testXmlParseInfoGetDepth0005)
    suite.addTest("Testing XML ParseInfo getDepth with end tag event.", testXmlParseInfoGetDepth0004)
    suite.addTest("Testing XML ParseInfo getDepth with start tag event.", testXmlParseInfoGetDepth0003)
    suite.addTest("Testing XML ParseInfo getDepth with END_DOCUMENT event.", testXmlParseInfoGetDepth0002)
    suite.addTest("Testing XML ParseInfo getDepth with START_DOCUMENT event.", testXmlParseInfoGetDepth0001)
    suite.addTest("Test XmlPullParser getDepth", testGetDepth)

    return suite.run()
}

const testXml =
    "<?xml version='1.0' encoding=\"utf-8\"?><!DOCTYPE example [<!ENTITY exampleEntity 'some entity'>]>\
    <!-- XMLjghk --><example><!-- fghj -->\
    <startDocument><startTag><element><!-- yesdryug -->\
    <tagName>tfyugyi</tagName><!-- xtgh --><content>ydtyfugihj</content>" +
    "<!-- CDATAguhi -->" +
    "<cdata><![CDATA[fyghjk]]></cdata>" +
    "<!-- ugh -->" +
    "<?processingInstruction example='cfgvhj'?>" +
    "<!-- gvhblkm -->" +
    "<entityReference>&exampleEntity;</entityReference>" +
    "<!-- fcguhjk -->" +
    "<whitespace>     </whitespace>" +
    "</element>" +
    "<!-- ufyui -->" +
    "</startTag>" +
    "</startDocument>" +
    "<!-- ufygo -->" +
    "<endDocument/>" +
    "</example>";

function rawStringToBuffer( str: string ) : ArrayBuffer {
    let idx = 0
    let len = str.length
    let arr = new Uint8Array( len )
    for ( idx = 0 ; idx < len ; ++idx ) {
        arr[ idx ] = str.charCodeAt(idx) & 0xFF;
    }

    return arr.buffer as ArrayBuffer
}

function testXmlParseInfoGetDepth0001() {
    const TAG = 'testXmlParseInfoGetDepth0001';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:0 value:0"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0002() {
    const TAG = 'testXmlParseInfoGetDepth0002';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:1 value:0"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0003() {
    const TAG = 'testXmlParseInfoGetDepth0003';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:2"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0004() {
    const TAG = 'testXmlParseInfoGetDepth0004';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:3 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0005() {
    const TAG = 'testXmlParseInfoGetDepth0005';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:5"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0006() {
    const TAG = 'testXmlParseInfoGetDepth0006';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:5 value:5"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0007() {
    const TAG = 'testXmlParseInfoGetDepth0007';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:6 value:0"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0008() {
    const TAG = 'testXmlParseInfoGetDepth0008';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:7 value:0"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0009() {
    const TAG = 'testXmlParseInfoGetDepth0009';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:8 value:4"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0011() {
    const TAG = 'testXmlParseInfoGetDepth0011';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:10 value:5"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetDepth0010() {
    const TAG = 'testXmlParseInfoGetDepth0010';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getDepth() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:9 value:5"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

// testParseXml004
function testGetDepth() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",  // key0
    "<note importance='high'>",                    // key2
    "    <title>Happy</title>",                    // key10 key2 key4 key3
    "    <todo>Work</todo>",                       // key10 key2 key4 key3
    "    <todo>Play</todo>",                       // key10 key2 key4 key3
    "</note>")                                     // key3 key2

    let textStr = ''
    let oldStr = ''
    let itemsCount = 0
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        textStr += "{ " + value.getName() + " : " + value.getDepth() + " }; "
        oldStr += "key:" + eventType + " value:" + value.getDepth() + " "
        itemsCount += 1
        return true
    }

    //assertEQ(itemsCount, 16) - bug - increment in callback doesn't work

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{  : 0 }; { note : 1 }; {  : 1 }; { title : 2 }; {  : 2 }; ' +
        '{ title : 2 }; {  : 1 }; { todo : 2 }; {  : 2 }; { todo : 2 }; {  : 1 }; ' +
        '{ todo : 2 }; {  : 2 }; { todo : 2 }; { note : 1 }; {  : 0 }; '
    arktest.assertEQ(textStr, expectedTextStr)

    // key0 - xml, key2 - start, key3 - close, key 4 - text, key10 - whitespace
    let oldExpected = "key:0 value:0 key:2 value:1 key:10 value:1 key:2 value:2 key:4 value:2 " +
        "key:3 value:2 key:10 value:1 key:2 value:2 key:4 value:2 key:3 value:2 key:10 value:1 " +
        "key:2 value:2 key:4 value:2 key:3 value:2 key:3 value:1 key:1 value:0 ";

    arktest.assertEQ(oldStr, oldExpected)
}

