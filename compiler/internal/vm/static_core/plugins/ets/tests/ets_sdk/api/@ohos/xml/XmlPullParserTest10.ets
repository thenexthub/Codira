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
import { util } from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part10");

    suite.addTest("Testing the XML PullParser without <>.", testXmlPullParserConstructor0020)
    suite.addTest("Testing the XML PullParser with special characters in content. Verifies that it can handle such cases correctly.", testXmlPullParserConstructor0019)
    suite.addTest("Testing the XML PullParser with escaped characters in content. Verifies that it can handle such cases correctly.", testXmlPullParserConstructor0018)
    suite.addTest("Testing the XML PullParser with introduced declaration, parsed correctly.", testXmlPullParserConstructor0017)
    suite.addTest("Testing the XML PullParser with special characters inside elements. Verifies that it can handle such cases correctly.", testXmlPullParserConstructor0016)
    suite.addTest("Testing the XML PullParser with content containing ']]>' within CDATA. Verifies that it handles the input correctly without error.", testXmlPullParserConstructor0015)
    suite.addTest("Testing the XML PullParser with nested CDATA sections.Verifies that it handles nested CDATA sections correctly.", testXmlPullParserConstructor0014)
    suite.addTest("Testing the XML PullParser with a malformed CDATA section. Verifies that it throws a meaningful error for invalid CDATA.", testXmlPullParserConstructor0013)
    suite.addTest("Testing the XML PullParser with a CDATA section without end. Verifies that it throws a meaningful error for invalid CDATA.", testXmlMethod018)
    suite.addTest("Testing the XML PullParser with names containing special characters.", testXmlPullParserConstructor0008)
    suite.addTest("Testing the XML PullParser with tag names that contain spaces.", testXmlPullParserConstructor0007)
    suite.addTest("Testing the XML PullParser with UTF-8", testXmlPullParserConstructor0021)

    return suite.run()
}

function rawStringToBuffer( str: string ) : ArrayBuffer {
    return new util.TextEncoder().encodeInto(str).buffer;
}

let testString = "";
function func21(name: string, value: string): boolean {
    testString += name + ':' + value;
    return true;
}

function testXmlPullParserConstructor0007() {
    const TAG = 'testXmlPullParserConstructor0007';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8" ?>' +
            '<book tag with spaces>' +
            '  <title>XML for Dummies</title>' +
            '  <author>John Doe</author>' +
            '  <price>19.99</price>' +
            '</book>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserConstructor0008() {
    const TAG = 'testXmlPullParserConstructor0008';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<book>' +
            '  <ti&>@tle>XML for Dummies</ti-tle>' +
            '  <author>John Doe</author>' +
            '  <price>19.99</price>' +
            '  <description>This book is about XML.</description>' +
            '</book>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0013() {
    const TAG = 'testXmlPullParserConstructor0013';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <!%[CDATA[内容 <<<<<< >>>>>> ]]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'


        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0014() {
    const TAG = 'testXmlPullParserConstructor0014';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <![CDATA[内容<![CDATA[嵌套 ]]> ]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0015() {
    const TAG = 'testXmlPullParserConstructor0015';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <![CDATA[内容都发给对方]]>分隔符发 ]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0016() {
    const TAG = 'testXmlPullParserConstructor0016';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <![CDATA[内容%$^# ]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserConstructor0017() {
    const TAG = 'testXmlPullParserConstructor0017';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<?xml-stylesheet type="text/css" href="cd_catalog.css"?>' +
            '<CATALOG>' +
            '<CD>' +
            '<TITLE>Empire Burlesque</TITLE>' +
            '<ARTIST>Bob Dylan</ARTIST>' +
            '<COUNTRY>USA</COUNTRY>' +
            '<COMPANY>Columbia</COMPANY>' +
            '<PRICE>10.90</PRICE>' +
            '<YEAR>1985</YEAR>' +
            '</CD>' +
            '</CATALOG>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserConstructor0018() {
    const TAG = 'testXmlPullParserConstructor0018';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<NOTE>' +
            '<TO>John</TO>' +
            '<FROM>Jane</FROM>' +
            '<MESSAGE>Hello & Welcome to the \"XML\" World!</MESSAGE>' +
            '</NOTE>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0019() {
    const TAG = 'testXmlPullParserConstructor0019';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!-- This is a comment -->' +
            '<root><>&<root>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0020() {
    const TAG = 'testXmlPullParserConstructor0020';
    try {
        let testXml =
            'xml version="1.0" encoding="utf-8"' +
            '<note importance="high" logged="true">' +
            ' <title>Happy</title>' +
            ' <todo>Work</todo>' +
            ' <todo>Play</todo>' +
            '</note>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
          arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0021() {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
          '<!DOCTYPE note [\n<!ENTITY foo "baa">]>' +
          '<note importance="high" logged="true">' +
          '     <![CDATA[\r\nfunccrion matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>' +
          '     <!--Hello, World!-->' +
          '     <company>John &amp; Hans</company>' +
          '     <title>Happy</title>' +
          '     <title>Happy</title>' +
          '     <lens>Work</lens>' +
          '     <lens>Play</lens>' +
          '     <?go there?>' +
          '     <a><b/></a>' +
          '     <h:table xmlns:h="http://www.w3.org/TR/html4/">' +
          '         <h:tr>' +
          '             <h:td>Apples</h:td>' +
          '             <h:td>Bananas</h:td>' +
          '         </h:tr>' +
          '     </h:table>' +
          '</note>'
    let textEncoder = new util.TextEncoder();
    let arrbuffer = textEncoder.encodeInto(strXml);
    let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer, 'UTF-8');
    testString = '';
    let options: xml.ParseOptions = {
        supportDoctype: true,
        ignoreNameSpace: true,
        attributeValueCallbackFunction: func21
    }
    that.parseXml(options);
    let str1 = 'importance:highlogged:truexmlns:h:http://www.w3.org/TR/html4/';
    arktest.assertEQ(testString, str1);
}

function func18(name: string, value: string): boolean {
    testString += name + value;
    return true;
}

function testXmlMethod018() {
    const TAG = 'testXmlMethod018';
    try {
        let strXml =
            '<?xml version="1.0" encoding="UTF-8"?>' +
            '<note importance="high" logged="true">' +
            '<company><![CDATA[';
        let textEncoder = new util.TextEncoder();
        let uint8 = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
        testString = '';
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: func18
        };
        that.parseXml(options);
        arktest.assertTrue(false);
    } catch (err) {
        if (!(err instanceof BusinessError)) {
            arktest.assertTrue(false);
        }
        arktest.assertEQ(err.toString(), "BusinessError: Cannot find the ']]>' in xml string.");
        arktest.assertEQ(err.code, 401);
    }
}
