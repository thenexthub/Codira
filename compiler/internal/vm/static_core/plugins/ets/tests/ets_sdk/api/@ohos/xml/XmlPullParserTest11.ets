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
import { xml } from "@ohos.xml";
import { BusinessError } from "@ohos.base";
import { util } from "@ohos.util";

function LogStack(e: Object): void {
    if (!(e instanceof Error)) {
        return;
    }
    const err = e as Error;
    if (err?.stack) {
        console.error(err!.stack)
    }
}

function testXtsActsXml001(): void {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<!DOCTYPE note [\n<!ENTITY foo "baa">]>' +
        '<note importance="high" logged="true">' +
        '    <![CDATA[\r\nfuncrion matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>' +
        '    <!--Hello, World!-->' +
        '    <company>John &amp; Hans</company>' +
        '    <title>Happy</title>' +
        '    <title>Happy</title>' +
        '    <lens>Work</lens>' +
        '    <lens>Play</lens>' +
        '    <?go there?>' +
        '    <a><b/></a>' +
        '    <h:table xmlns:h="http://www.w3.org/TR/html4/">' +
        '        <h:tr>' +
        '            <h:td>Apples</h:td>' +
        '            <h:td>Bananas</h:td>' +
        '        </h:tr>' +
        '    </h:table>' +
        '</note>';
    try {
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = '';
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string): boolean => {
                str += name + value + ' ';
                return true;
            }
        }
        that.parseXml(options);
        let str3 = '  note [\n<!ENTITY foo "baa">] note      ' +
            '\r\nfuncrion matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n      Hello, World!      company John ' +
            '& Hans company      title Happy title      title Happy title      lens Work lens      ' +
            'lens Play lens      go there      a b b a      h:table          h:tr              h:td Apples h:td' +
            '              h:td Bananas h:td          h:tr      h:table note  ';
        arktest.assertEQ(str, str3);
    } catch (e) {
        console.error(`testXtsActsXml001 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml002(): void {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '    <title>Happy</title>' +
        '    <todo>Work</todo>' +
        '    <todo>Play</todo>' +
        '</note>';
    try {
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key.valueOf() + ' value:' + value.getColumnNumber() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:1  key:2 value:77  key:10 value:81  key:2 value:88  key:4 value:93  ' +
            'key:3 value:101  key:10 value:105  key:2 value:111  key:4 value:115  key:3 value:122  key:10 value:126  ' +
            'key:2 value:132  key:4 value:136  key:3 value:143  key:3 value:150  key:1 value:150  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml002 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml003(): void {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '    <title>Happy</title>' +
        '    <todo>Work</todo>' +
        '    <todo>Play</todo>' +
        '</note>';
    try {
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getDepth() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:0  key:2 value:1  key:10 value:1  key:2 value:2  key:4 value:2  key:3 value:2  ' +
            'key:10 value:1  key:2 value:2  key:4 value:2  key:3 value:2  key:10 value:1  key:2 value:2  ' +
            'key:4 value:2  key:3 value:2  key:3 value:1  key:1 value:0  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml003 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml004(): void {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '    <title>Happy</title>' +
        '    <todo>Work</todo>' +
        '    <todo>Play</todo>' +
        '</note>';
    try {
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getLineNumber() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:1  key:2 value:1  key:10 value:1  key:2 value:1  key:4 value:1  key:3 value:1  ' +
            'key:10 value:1  key:2 value:1  key:4 value:1  key:3 value:1  key:10 value:1  key:2 value:1  ' +
            'key:4 value:1  key:3 value:1  key:3 value:1  key:1 value:1  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml004 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml005(): void {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '    <title>Happy</title>' +
        '    <todo>Work</todo>' +
        '    <todo>Play</todo>' +
        '</note>';
    try {
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getName() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:  key:2 value:note  key:10 value:  key:2 value:title  key:4 value:  ' +
            'key:3 value:title  key:10 value:  key:2 value:todo  key:4 value:  key:3 value:todo  key:10 value:  ' +
            'key:2 value:todo  key:4 value:  key:3 value:todo  key:3 value:note  key:1 value:  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml005 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml006(): void {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '    <title>Happy</title>' +
        '    <todo>Work</todo>' +
        '    <todo>Play</todo>' +
        '</note>';
    try {
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";

        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getNamespace() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:  key:2 value:  key:10 value:  key:2 value:  key:4 value:  key:3 value:  ' +
            'key:10 value:  key:2 value:  key:4 value:  key:3 value:  key:10 value:  key:2 value:  key:4 value:  ' +
            'key:3 value:  key:3 value:  key:1 value:  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml006 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml007() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <todo>Play</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getPrefix() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:  key:2 value:  key:10 value:  key:2 value:  key:4 value:  key:3 value:  ' +
            'key:10 value:  key:2 value:  key:4 value:  key:3 value:  key:10 value:  key:2 value:  key:4 value:  ' +
            'key:3 value:  key:3 value:  key:1 value:  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml007 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml008() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <todo>Play</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getText() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:  key:2 value:  key:10 value:      key:2 value:  key:4 value:Happy  ' +
            'key:3 value:  key:10 value:      key:2 value:  key:4 value:Work  key:3 value:  key:10 value:      ' +
            'key:2 value:  key:4 value:Play  key:3 value:  key:3 value:  key:1 value:  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml008 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml009() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <todo>Play</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.isEmptyElementTag() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:false  key:2 value:false  key:10 value:false  key:2 value:false  ' +
            'key:4 value:false  key:3 value:false  key:10 value:false  key:2 value:false  key:4 value:false  ' +
            'key:3 value:false  key:10 value:false  key:2 value:false  key:4 value:false  key:3 value:false  ' +
            'key:3 value:false  key:1 value:false  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml009 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml010() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <todo>Play</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.isWhitespace() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:true  key:2 value:false  key:10 value:true  key:2 value:true  ' +
            'key:4 value:false  key:3 value:true  key:10 value:true  key:2 value:true  key:4 value:false  ' +
            'key:3 value:true  key:10 value:true  key:2 value:true  key:4 value:false  key:3 value:true  ' +
            'key:3 value:true  key:1 value:true  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml010 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml011() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <todo>Play</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";

        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                str += 'key:' + key + ' value:' + value.getAttributeCount() + '  ';
                return true;
            }
        }
        that.parseXml(options);
        let result = 'key:0 value:0  key:2 value:2  key:10 value:0  key:2 value:0  key:4 value:0  ' +
            'key:3 value:0  key:10 value:0  key:2 value:0  key:4 value:0  key:3 value:0  key:10 value:0  ' +
            'key:2 value:0  key:4 value:0  key:3 value:0  key:3 value:0  key:1 value:0  ';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml011 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml012() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Happy</title>' +
            '    <todo>Work</todo>' +
            '    <mess><![CDATA[This is a \r CDATA\n section]]></mess>' +
            '    <todo>Play</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";
        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                if (key === 5) {
                    str += value.getText();
                }
                return true;
            }
        }
        that.parseXml(options);
        let result = 'This is a \r CDATA\n section';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml012 failed, error: ${e}`);
    }
}
function testXtsActsXml013() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="high" logged="true">' +
            '    <title>Hello\rWorld\n</title>' +
            '    <todo>Work\r\n</todo>' +
            '    <mess><![CDATA[This is a \r\n CDATA section]]></mess>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let str = "";

        let options: xml.ParseOptions =
        {
            supportDoctype: true,
            ignoreNameSpace: true,
            tokenValueCallbackFunction: (key: xml.EventType, value: xml.ParseInfo): boolean => {
                if (key === 4 || key === 5) {
                    str += value.getText();
                }
                return true;
            }
        }
        that.parseXml(options);
        let result = 'Hello\nWorld\nWork\nThis is a \r\n CDATA section';
        arktest.assertEQ(str, result);
    } catch (e) {
        console.error(`testXtsActsXml013 failed, error: ${e}`);
        LogStack(e);
    }
}
function testXtsActsXml014() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="utf-8"?>' +
            '<note importance="hi&amp;gh" logged="&lt;true">' +
            '    <title>Hello&gt;World&apos;</title>' +
            '    <todo>Wo&quot;rk</todo>' +
            '</note>';
        let textEncoder = new util.TextEncoder();
        let arrbuffer = textEncoder.encodeInto(strXml);
        let that = new xml.XmlPullParser(arrbuffer.buffer as object as ArrayBuffer);
        let strAttr = "";
        let str = "";
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            attributeValueCallbackFunction: (name: string, value: string): boolean => {
                strAttr += name + ', ' + value + ' ';
                return true;
            },
            tagValueCallbackFunction: (name: string, value: string): boolean => {
                str += name + value + ' ';
                return true;
            }
        }
        that.parseXml(options);
        let result = ' note      title Hello>World\' title      todo Wo"rk todo note  ';
        let resAttr = 'importance, hi&gh logged, <true ';
        arktest.assertEQ(str, result);
        arktest.assertEQ(strAttr, resAttr);
    } catch (e) {
        console.error(`testXtsActsXml014 failed, error: ${e}`);
        LogStack(e);
    }
}

function testXtsActsXml015() {
    try {
        let strXml =
            '<?xml version="1.0" encoding="UTF-8"?>' +
            '<note importance="high" logged="true">' +
            '<company>John &amp; Hans</company>' +
            '<tod<xml version="1.0" encoding="UTF-8"?><note importance="high" logged="true"></note>' +
            '</note>';
        let textec = new util.TextEncoder()
        let uint8 = textec.encodeInto(strXml);
        let that = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
        let str = ''
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string): boolean => {
                str += name + value;
                return true;
            }
        }
        that.parseXml(options);
        arktest.assertTrue(false);
    } catch (e) {
        e = e as BusinessError;
        try {
            arktest.assertEQ(e.toString(), "BusinessError: The node name contains invalid characters: <");
            arktest.assertEQ(e.code, 401);
        } catch (e) {
            console.error(`testXtsActsXml015 failed, error: ${e}`);
            LogStack(e);
        }
    }
}

function testXtsActsXml016() {
    let strXml =
        '<?xml version="1.0" encoding="UTF-8"?>' +
        '<note importance="high" logged="true">' +
        '<company><![CDATA[SYSTEM</company>' +
        '<todo>work</todo>' +
        '</note>';
    let textEncoder = new util.TextEncoder();
    let uint8 = textEncoder.encodeInto(strXml);
    let pullParse = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
    let str = '';
    let options: xml.ParseOptions = {
        supportDoctype: true,
        ignoreNameSpace: true,
        tagValueCallbackFunction: (name: string, value: string): boolean => {
            str += name + value;
            return true;
        }
    }
    try {
        pullParse.parseXml(options);
        arktest.assertTrue(false);
    } catch (e) {
        if (e instanceof BusinessError) {
            try {
                arktest.assertEQ(e.toString(), "BusinessError: Cannot find the ']]>' in xml string.");
                arktest.assertEQ(e.code, 401);
            } catch (e) {
                console.error(`testXtsActsXml016 failed, error: ${e}`);
                LogStack(e);
            }
        } else {
            throw e;
        }
    }
}
function testXtsActsXml017() {
    let strXml =
        '<?xml version="1.0" encoding="UTF-8"?>' +
        '<note importance="high" logged="true">' +
        '<company><![CDATA[';
    let textEncoder = new util.TextEncoder();
    let uint8 = textEncoder.encodeInto(strXml);
    let pullParse = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
    let str = '';
    let options: xml.ParseOptions = {
        supportDoctype: true,
        ignoreNameSpace: true,
        tagValueCallbackFunction: (name: string, value: string): boolean => {
            str += name + value;
            return true;
        }
    }
    try {
        pullParse.parseXml(options);
        arktest.assertTrue(false);
    } catch (e) {
        e = e as BusinessError;
        try {
            arktest.assertEQ(e.toString(), "BusinessError: Cannot find the ']]>' in xml string.");
            arktest.assertEQ(e.code, 401);
        } catch (e) {
            console.error(`testXtsActsXml017 failed, error: ${e}`);
            LogStack(e);
        }
    }
}
function testXtsActsXml018() {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '<title>Happy</title>' +
        '<?go there' +
        '</note>';
    let textEncoder = new util.TextEncoder();
    let uint8 = textEncoder.encodeInto(strXml);
    let pullParse = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
    let str = '';
    let options: xml.ParseOptions = {
        supportDoctype: true,
        ignoreNameSpace: true,
        tagValueCallbackFunction: (name: string, value: string): boolean => {
            str += name + value;
            return true;
        }
    }
    try {
        pullParse.parseXml(options);
        arktest.assertTrue(false);
    } catch (e) {
        e = e as BusinessError;
        try {
            arktest.assertEQ(e.toString(), "BusinessError: Cannot find the '?>' in xml string.");
            arktest.assertEQ(e.code, 401);
        } catch (e) {
            console.error(`testXtsActsXml017 failed, error: ${e}`);
            LogStack(e);
        }
    }
}
function testXtsActsXml019() {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '<title>Happy</title>' +
        '<?go there';
    let textEncoder = new util.TextEncoder();
    let uint8 = textEncoder.encodeInto(strXml);
    let pullParse = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
    let str = '';
    let options: xml.ParseOptions = {
        supportDoctype: true,
        ignoreNameSpace: true,
        tagValueCallbackFunction: (name: string, value: string): boolean => {
            str += name + value;
            return true;
        }
    }
    try {
        pullParse.parseXml(options);
        arktest.assertTrue(false);
    } catch (e) {
        e = e as BusinessError;
        try {
            arktest.assertEQ(e.toString(), "BusinessError: Cannot find the '?>' in xml string.");
            arktest.assertEQ(e.code, 401);
        } catch (e) {
            console.error(`testXtsActsXml019 failed, error: ${e}`);
            LogStack(e);
        }
    }
}
function testXtsActsXml020() {
    let strXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<note importance="high" logged="true">' +
        '<title>Happy</title>' +
        '<!--comment' +
        '</note>';
    let textEncoder = new util.TextEncoder();
    let uint8 = textEncoder.encodeInto(strXml);
    let pullParse = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
    let str = '';
    let options: xml.ParseOptions = {
        supportDoctype: true,
        ignoreNameSpace: true,
        tagValueCallbackFunction: (name: string, value: string): boolean => {
            str += name + value;
            return true;
        }
    }
    try {
        pullParse.parseXml(options);
        arktest.assertTrue(false);
    } catch (e) {
        e = e as BusinessError;
        try {
            arktest.assertEQ(e.toString(), "BusinessError: Cannot find the '--' in xml string.");
            arktest.assertEQ(e.code, 401);
        } catch (e) {
            console.error(`testXtsActsXml020 failed, error: ${e}`);
            LogStack(e);
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part10");

    suite.addTest("Testing xts acts suit testXml0052.", testXtsActsXml001);
    suite.addTest("Testing xts acts suit testXml0053.", testXtsActsXml002);
    suite.addTest("Testing xts acts suit testXml0054.", testXtsActsXml003);
    suite.addTest("Testing xts acts suit testXml0055.", testXtsActsXml004);
    suite.addTest("Testing xts acts suit testXml0056.", testXtsActsXml005);
    suite.addTest("Testing xts acts suit testXml0057.", testXtsActsXml006);
    suite.addTest("Testing xts acts suit testXml0058.", testXtsActsXml007);
    suite.addTest("Testing xts acts suit testXml0059.", testXtsActsXml008);
    suite.addTest("Testing xts acts suit testXml0060.", testXtsActsXml009);
    suite.addTest("Testing xts acts suit testXml0061.", testXtsActsXml010);
    suite.addTest("Testing xts acts suit testXml0062.", testXtsActsXml011);
    suite.addTest("Testing xts acts suit testXml0063.", testXtsActsXml012);
    suite.addTest("Testing xts acts suit testXml0064.", testXtsActsXml013);
    suite.addTest("Testing xts acts suit testXml0065.", testXtsActsXml014);
    suite.addTest("Testing xts acts suit testXmlMethod016.", testXtsActsXml015);
    suite.addTest("Testing xts acts suit testXmlMethod017.", testXtsActsXml016);
    suite.addTest("Testing xts acts suit testXmlMethod018.", testXtsActsXml017);
    suite.addTest("Testing xts acts suit testXmlMethod019.", testXtsActsXml018);
    suite.addTest("Testing xts acts suit testXmlMethod020.", testXtsActsXml019);
    suite.addTest("Testing xts acts suit testXmlMethod021.", testXtsActsXml020);

    return suite.run();
}
