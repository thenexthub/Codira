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
import buffer from '@ohos.buffer';
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: int = 401;
const BufferOverFlowErrorCodeId: int = 10200062;
const IllegalPositionErrorCodeId: int = 10200063;
const EmptyErrorCodeId: int = 10200064;
const NoElementMatchErrorCodeId: int = 10200065;
const EncodingErrorCodeId: int = 10200066;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export namespace xml {

    export enum EventType {
        START_DOCUMENT,
        END_DOCUMENT,
        START_TAG,
        END_TAG,
        TEXT,
        CDSECT,
        COMMENT,
        DOCDECL,
        INSTRUCTION,
        ENTITY_REFERENCE,
        WHITESPACE
    }

    class ParseInfoImpl implements ParseInfo {
        private depth: int
        private columnNumber: int
        private lineNumber: int
        private attributeCount: int
        private name: string
        private namespace: string
        private prefix: string
        private text: string
        private emptyElementTag: boolean
        private whitespace: boolean

        override getAttributeCount(): int { return this.attributeCount; }
        override getColumnNumber(): int { return this.columnNumber; }
        override getDepth(): int { return this.depth; }
        override getLineNumber(): int { return this.lineNumber; }
        override getName(): string { return this.name; }
        override getNamespace(): string { return this.namespace; }
        override getPrefix(): string { return this.prefix; }
        override getText(): string { return this.text; }
        override isEmptyElementTag(): boolean { return this.emptyElementTag; }
        override isWhitespace(): boolean { return this.whitespace; }
    }

    export interface ParseInfo {
        getAttributeCount(): int
        getColumnNumber(): int
        getDepth(): int
        getLineNumber(): int
        getName(): string
        getNamespace(): string
        getPrefix(): string
        getText(): string
        isEmptyElementTag(): boolean
        isWhitespace(): boolean
    }

    export interface ParseOptions {
        attributeValueCallbackFunction?: (name: string, value: string) => boolean
        ignoreNameSpace?: boolean
        supportDoctype?: boolean
        tagValueCallbackFunction?: (name: string, value: string) => boolean
        tokenValueCallbackFunction?: (eventType: EventType, value: ParseInfo) => boolean
    }

    export class XmlPullParser {

        constructor(bufferIn: ArrayBuffer | DataView, encoding?: string) {
            let strIn: string
            let b: buffer.Buffer
            if (bufferIn instanceof DataView) {
                b = buffer.from((bufferIn as DataView).buffer as ArrayBuffer)
            }
            else {
                b = buffer.from(bufferIn)
            }
            if (encoding != undefined) {
                encoding = encoding.toLowerCase()
                if (encoding !== "utf-8") {
                    throw createBusinessError(TypeErrorCodeId, 'Parameter error.Just support utf-8')
                }
                strIn = b.toString(encoding! as buffer.BufferEncoding)
            } else {
                strIn = b.toString()
            }
            this.helper = new XmlParseHelper(strIn)
        }

        private helper: XmlParseHelper
        public parseXml(option: ParseOptions): void {
            if (!this.helper.parseXml(option)) {
                throw createBusinessError(TypeErrorCodeId, this.helper.xmlPullParserError());
            }
        }
    }

    type Attributes = Map<string, string>

    interface XmlNode {
        name?: string
        attributes?: Attributes
        content?: string
        lineNumber?: int
        columnNumber?: int
        pos?: int
        depth?: int
        prefix?: string
        namespace_?: string
        type?: EventType
        isSelfClose?: boolean
        isServiceTagNode?: boolean
    }

    export const MaxXmlLength = 100000;
    const OffsetDepth = 1;
    const SpacesIndentation = 2;

    enum XmlDynamicType {
        NONE,
        START_AND_ATTRIBUTES,
        DECLARATION,
        TEXT,
        OTHERS
    }

    export class XmlDynamicSerializer {
        private iPos: int = 0;
        private buffer: ArrayBuffer;
        private encoding: string = 'utf-8';
        private depth: int = 0;
        private type = XmlDynamicType.NONE;
        private elementStack = new Array<XmlNode>();
        private multNsp = new Map<int, XmlNode>();
        private elementNum: int = 0;

        /**
         * Constructs an instance of the class with the specified buffer and encoding.
         *
         * @param {string} encoding - The encoding format of the data. Only 'utf-8' is supported.
         * @throws {BusinessError} Throws an error if the encoding is not 'utf-8'.
         */
        constructor(encoding: string) {
            if (encoding !== 'utf-8') {
                throw createBusinessError(EncodingErrorCodeId, 'Parameter error.Just support utf-8');
            }

            this.buffer = new ArrayBuffer(MaxXmlLength);
            this.increaseStackLength();
        }

        /**
         * Constructs an instance of the class with the specified buffer.
         */
        constructor() {
            this.buffer = new ArrayBuffer(MaxXmlLength);
            this.increaseStackLength();
        }

        /**
         * Sets an attribute for the current XML element.
         *
         * @param {string} name - The name of the attribute to set.
         * @param {string} value - The value of the attribute to set.
         * @throws {BusinessError} Throws an error if the method is called in an illegal position and
         * the `name` parameter is an empty string.
         */
        public setAttributes(name: string, value: string): void {
            this.checkEmptyParameter(name);
            if (this.type !== XmlDynamicType.START_AND_ATTRIBUTES) {
                this.checkPosition();
            }

            let stringBuilder = new StringBuilder(` ${name}=\"`);
            this.writeEscaped(stringBuilder, value);
            stringBuilder.append(`\"`);
            this.type = XmlDynamicType.START_AND_ATTRIBUTES;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Sets the XML declaration for the current document.
         *
         * @throws {BusinessError} Throws an error if the declaration is set in an illegal position.
         */
        public setDeclaration(): void {
            if (this.iPos > 0) {
                this.checkPosition();
            }

            let stringBuilder = new StringBuilder(`<?xml version=\"1.0\" encoding=\"${this.encoding}\"?>`);
            this.type = XmlDynamicType.DECLARATION;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Handles the start of an XML element.
         *
         * @param {string} name - The name of the XML element being started.
         * @throws {BusinessError} Throws an error if the `name` parameter is an empty string.
         */
        public startElement(name: string): void {
            let stringBuilder = this.prepareBufferForNewItem(name);

            this.elementStack[this.depth].name = name;
            stringBuilder.append('<');
            if (this.elementStack[this.depth].prefix !== '') {
                stringBuilder.append(`${this.elementStack[this.depth].prefix}:`);
            } else if (this.depth !== 0) {
                if (this.elementStack[this.depth - OffsetDepth].prefix !== '') {
                    this.elementStack[this.depth].prefix = this.elementStack[this.depth - OffsetDepth].prefix;
                    stringBuilder.append(`${this.elementStack[this.depth].prefix}:`);
                }
            }
            stringBuilder.append(`${this.elementStack[this.depth].name}`);
            this.type = XmlDynamicType.START_AND_ATTRIBUTES;
            this.depth++;
            this.elementNum++;
            this.increaseStackLength();
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Handles the end of an XML element during parsing.
         *
         * @throws {BusinessError} Throws an error if there is no matching start element for the current end element.
         */
        public endElement(): void {
            if (this.elementNum < 1) {
                throw createBusinessError(NoElementMatchErrorCodeId,
                    'There is no match between the startElement and the endElement');
            }
            this.elementNum--;

            let stringBuilder = new StringBuilder();
            if (this.type == XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('/>');
                this.type = XmlDynamicType.OTHERS;
                this.depth--;
                this.writeToBuffer(stringBuilder);
                return;
            }
            this.depth--;
            if (this.type !== XmlDynamicType.TEXT) {
                this.nextItem(stringBuilder);
            }

            stringBuilder.append('</');
            if (this.elementStack[this.depth].prefix !== '') {
                stringBuilder.append(`${this.elementStack[this.depth].prefix}:`);
            }
            stringBuilder.append(`${this.elementStack[this.depth].name}`);
            this.elementStack[this.depth].prefix = '';
            this.elementStack[this.depth].namespace_ = '';
            this.type = XmlDynamicType.OTHERS;
            stringBuilder.append('>');
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Sets the namespace for the current XML element.
         *
         * @param {string} prefix - The namespace prefix to be set.
         * @param {string} namespace - The namespace URI associated with the prefix.
         * @throws {BusinessError} Throws an error if the `prefix` or `name` parameter is an empty string.
         */
        public setNamespace(prefix: string, namespace: string): void {
            this.checkEmptyParameter(prefix);
            this.checkEmptyParameter(namespace);

            let stringBuilder = new StringBuilder();
            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }

            this.elementStack[this.depth].prefix = prefix;
            this.elementStack[this.depth].namespace_ = namespace;
            let node: XmlNode = {
                prefix: this.elementStack[this.depth]!.prefix,
                namespace_: this.elementStack[this.depth]!.namespace_
            }
            this.multNsp.set(this.depth, node);
            this.type = XmlDynamicType.OTHERS;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Adds an empty XML element with the specified name to the output buffer.
         *
         * @param {string} name - The name of the empty XML element to be added.
         * @throws {BusinessError} Throws an error if the `name` parameter is an empty string.
         */
        public addEmptyElement(name: string): void {
            let stringBuilder = this.prepareBufferForNewItem(name);

            stringBuilder.append(`<${name}/>`);
            this.type = XmlDynamicType.OTHERS;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Sets a comment in the XML output.
         *
         * @param {string} comment - The comment string to be added to the XML output.
         * @throws {BusinessError} Throws an error if the `text` parameter is an empty string.
         */
        public setComment(text: string): void {
            let stringBuilder = this.prepareBufferForNewItem(text);

            stringBuilder.append(`<!--${text}-->`);
            this.type = XmlDynamicType.OTHERS;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Sets the CDATA section for the XML content.
         *
         * @param {string} data - The string data to be wrapped in a Cdata section.
         * @throws {BusinessError} Throws an error if the `text` parameter is an empty string.
         */
        public setCdata(text: string): void {
            let stringBuilder = this.prepareBufferForNewItem(text);

            text = text.replaceAll(']]>', ']]]]><![CDATA[>');
            stringBuilder.append(`<![CDATA[${text}]]>`);
            this.type = XmlDynamicType.OTHERS;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Sets the text content for the current XML element.
         *
         * @param {string} text - The text content to set for the current XML element.
         * @throws {BusinessError} Throws an error if the `text` parameter is an empty string.
         */
        public setText(text: string): void {
            this.checkEmptyParameter(text);
            let stringBuilder = new StringBuilder();
            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }

            this.writeEscaped(stringBuilder, text);
            this.type = XmlDynamicType.TEXT;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Sets the document type (DOCTYPE) for the XML output.
         *
         * @param {string} text - The DOCTYPE declaration to be added to the XML output.
         * @throws {BusinessError} Throws an error if the `text` parameter is an empty string.
         */
        public setDocType(text: string): void {
            let stringBuilder = this.prepareBufferForNewItem(text);

            stringBuilder.append(`<!DOCTYPE ${text}>`);
            this.type = XmlDynamicType.OTHERS;
            this.writeToBuffer(stringBuilder);
        }

        /**
         * Retrieves a copy of the internal buffer as an ArrayBuffer.
         *
         * @returns {ArrayBuffer} A new ArrayBuffer containing the data from the internal buffer.
         */
        public getOutput(): ArrayBuffer {
            return this.buffer.slice(0, this.iPos);
        }

        /**
         * Increases the length of the element stack by creating a new `XmlNode` object with default empty values for
         * `name`, `prefix`, and `namespace_`, and pushing it onto the `elementStack`.
         */
        private increaseStackLength(): void {
            let node: XmlNode = {
                name: '',
                prefix: '',
                namespace_: ''
            }
            this.elementStack.push(node);
        }

        /**
         * Escapes special characters in the given string and appends the result to the provided StringBuilder.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder to which the escaped string will be appended.
         * @param {string} s - The input string to be escaped.
         */
        private writeEscaped(stringBuilder: StringBuilder, s: string): void {
            s = s.replaceAll('&', '&amp;');
            s = s.replaceAll('\'', '&apos;');
            s = s.replaceAll('\"', '&quot;');
            s = s.replaceAll('>', '&gt;');
            s = s.replaceAll('<', '&lt;');
            stringBuilder.append(s);
        }

        /**
         * Writes the content of the `stringBuilder` array to the `buffer` starting at the current position (`iPos`).
         * If the buffer does not have enough space to accommodate the data, an error is thrown.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder will be write.
         * @param {string} name - The name used in the error message if the operation fails.
         * @throws {BusinessError} Throws an error if the buffer does not have enough space to write the data.
         */
        private writeToBuffer(stringBuilder: StringBuilder): void {
            let tempString = stringBuilder.toString();
            let iLenTemp = tempString.length;
            if (this.iPos + iLenTemp <= MaxXmlLength) {
                for (let i = 0; i < iLenTemp; i++) {
                    this.buffer.set((i + this.iPos) as int, tempString.charAt(i).toByte());
                }
                this.iPos += iLenTemp;
            } else {
                throw createBusinessError(BufferOverFlowErrorCodeId, 'The length has exceeded the upper limit');
            }
        }

        /**
         * Handles the namespace processing for the current element in the stack.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append
         * namespace declarations to the XML output.
         */
        private spliceNamespace(stringBuilder: StringBuilder): void {
            if (this.depth === 0) {
                return;
            }

            this.elementStack[this.depth].prefix = this.elementStack[this.depth - OffsetDepth].prefix;
            this.elementStack[this.depth].namespace_ = this.elementStack[this.depth - OffsetDepth].namespace_;

            let prevDepthInfo = this.multNsp.get(this.depth - OffsetDepth);
            if (prevDepthInfo == undefined) {
                return;
            }

            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                let prefix = prevDepthInfo.prefix;
                let namespace = prevDepthInfo.namespace_;
                stringBuilder.append(` xmlns:${prefix}=\"${namespace}\"`);
                this.multNsp.get(this.depth - OffsetDepth)!.prefix = '';
                this.multNsp.get(this.depth - OffsetDepth)!.namespace_ = '';
            }
        }

        /**
         * Appends a newline character followed by indentation spaces to the `out` property.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append spaces to the XML output.
         */
        private nextItem(stringBuilder: StringBuilder): void {
            stringBuilder.append('\r\n');
            stringBuilder.append(' '.repeat(SpacesIndentation * this.depth));
        }

        /**
         * Checks if the provided parameter is an empty string.
         * Throws a `BusinessError` if the parameter is empty.
         *
         * @param {string} para - The parameter to be checked.
         */
        private checkEmptyParameter(para: string): void {
            if (para.length === 0) {
                throw createBusinessError(EmptyErrorCodeId, 'Parameter error. Parameter cannot be empty');
            }
        }

        /**
         * Checks the position of the XML serializer and throws an error if the position is illegal.
         */
        private checkPosition(): void {
            throw createBusinessError(IllegalPositionErrorCodeId, 'Illegal position for xml');
        }

        /**
         * Prepares the buffer for adding a new item by handling namespace line break and indentation.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder instance to append content to.
         */
        private prepareBufferForNewItem(name: string): StringBuilder {
            this.checkEmptyParameter(name);
            let stringBuilder = new StringBuilder();
            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }
            if (this.type !== XmlDynamicType.NONE) {
                this.nextItem(stringBuilder);
            }

            return stringBuilder;
        }
    }


    export class XmlParseHelper {
        private static finalizationRegistry = new FinalizationRegistry<long>(
            (pointer: long) => { XmlParseHelper.releaseNative(pointer); }
        );
        private nativeBinding: long;

        static { loadLibrary('ets_sdk_native'); }

        constructor(content: string) {
            this.nativeBinding = XmlParseHelper.bindNative(content);
            XmlParseHelper.finalizationRegistry.register(this, this.nativeBinding);
        }

        public parseXml(option: ParseOptions): boolean {
            return XmlParseHelper.parseXmlInner(this.nativeBinding,
                option.ignoreNameSpace ?? false,
                option.supportDoctype ?? false,
                option.attributeValueCallbackFunction,
                option.tagValueCallbackFunction,
                option.tokenValueCallbackFunction
            );
        }

        private static native bindNative(content: string): long;
        private static native releaseNative(pointer: long): boolean;
        private static native parseXmlInner(pointer: long,
            ignoreNameSpace: boolean,
            supportDoctype: boolean,
            attributeValueCallbackFunction?: (name: string, value: string) => boolean,
            tagValueCallbackFunction?: (name: string, value: string) => boolean,
            tokenValueCallbackFunction?: (eventType: EventType, value: ParseInfo) => boolean
        ): boolean;
        private static native parserErrorInfo(pointer: long): string;

        public xmlPullParserError(): string {
            return XmlParseHelper.parserErrorInfo(this.nativeBinding);
        }
    }

    const DepthOffset = 1;
    const BufferIndexOffset = 1;
    const IndentationSpaces = 2;

    enum XmlType {
        NONE,
        START_AND_ATTRIBUTES,
        DECLARATION,
        TEXT,
        OTHERS
    }

    export class XmlSerializer {
        private iPos: int = 0;
        private buffer: ArrayBuffer | DataView;
        private encoding: string = 'utf-8';
        private depth: int = 0;
        private type = XmlType.NONE;
        private elementStack = new Array<XmlNode>();
        private multNsp = new Map<int, XmlNode>();
        private isHasDecl: boolean = false;

        /**
         * Constructs an instance of the class with the specified buffer and encoding.
         *
         * @param {ArrayBuffer | DataView} buffer - The buffer containing the data to be processed.
         * @param {string} encoding - The encoding format of the data. Only 'utf-8' is supported.
         * @throws {BusinessError} Throws an error if the encoding is not 'utf-8'.
         */
        constructor(buffer: ArrayBuffer | DataView, encoding: string) {
            if (encoding !== 'utf-8') {
                throw createBusinessError(TypeErrorCodeId, 'Parameter error.Just support utf-8');
            }

            this.buffer = buffer;
            this.increaseStackLength();
        }

        /**
         * Constructs an instance of the class with the specified buffer.
         *
         * @param {ArrayBuffer | DataView} buffer - The buffer containing the data to be processed.
         */
        constructor(buffer: ArrayBuffer | DataView) {
            this.buffer = buffer;
            this.increaseStackLength();
        }

        /**
         * Sets an attribute for the current XML element.
         *
         * @param {string} name - The name of the attribute to set.
         * @param {string} value - The value of the attribute to set.
         * @throws {BusinessError} Throws an error if the method is called in an illegal position.
         */
        public setAttributes(name: string, value: string): void {
            this.checkEmptyParameter(name);
            if (this.type !== XmlType.START_AND_ATTRIBUTES) {
                throw createBusinessError(TypeErrorCodeId, 'Illegal position for attribute');
            }

            let stringBuilder = new StringBuilder(` ${name}=\"`);
            this.writeEscaped(stringBuilder, value);
            stringBuilder.append(`\"`);
            this.type = XmlType.START_AND_ATTRIBUTES;
            this.writeToBuffer(stringBuilder, 'SetAttributes');
        }

        /**
         * Handles the start of an XML element.
         *
         * @param {string} name - The name of the XML element being started.
         */
        public startElement(name: string): void {
            this.checkEmptyParameter(name);
            let stringBuilder = new StringBuilder();
            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }
            if (this.type !== XmlType.NONE && this.type !== XmlType.DECLARATION) {
                this.nextItem(stringBuilder);
            }

            this.elementStack[this.depth].name = name;
            stringBuilder.append('<');
            if (this.elementStack[this.depth].prefix !== '') {
                stringBuilder.append(`${this.elementStack[this.depth].prefix}:`);
            } else if (this.depth !== 0) {
                if (this.elementStack[this.depth - DepthOffset].prefix !== '') {
                    this.elementStack[this.depth].prefix = this.elementStack[this.depth - DepthOffset].prefix;
                    stringBuilder.append(`${this.elementStack[this.depth].prefix}:`);
                }
            }
            stringBuilder.append(`${this.elementStack[this.depth].name}`);
            this.type = XmlType.START_AND_ATTRIBUTES;
            this.depth++;
            this.increaseStackLength();
            this.writeToBuffer(stringBuilder, 'StartElement');
        }

        /**
         * Handles the end of an XML element during parsing.
         */
        public endElement(): void {
            let stringBuilder = new StringBuilder();
            if (this.type == XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('/>');
                this.type = XmlType.OTHERS;
                this.depth--;
                this.writeToBuffer(stringBuilder, 'EndElement');
                return;
            }
            this.depth--;
            if (this.type !== XmlType.TEXT) {
                this.nextItem(stringBuilder);
            }

            stringBuilder.append('</');
            if (this.elementStack[this.depth].prefix !== '') {
                stringBuilder.append(`${this.elementStack[this.depth].prefix}:`);
            }
            stringBuilder.append(`${this.elementStack[this.depth].name}`);
            this.elementStack[this.depth].prefix = '';
            this.elementStack[this.depth].namespace_ = '';
            this.type = XmlType.OTHERS;
            stringBuilder.append('>');
            this.writeToBuffer(stringBuilder, 'EndElement');
        }

        /**
         * Sets the namespace for the current XML element.
         *
         * @param {string} prefix - The namespace prefix to be set.
         * @param {string} namespace - The namespace URI associated with the prefix.
         */
        public setNamespace(prefix: string, namespace: string): void {
            this.checkEmptyParameter(prefix);
            if (namespace.length === 0) {
                throw createBusinessError(TypeErrorCodeId,
                    `Parameter error. The type of ${namespace} must be string`);
            }
            let stringBuilder = new StringBuilder();
            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }

            this.elementStack[this.depth].prefix = prefix;
            this.elementStack[this.depth].namespace_ = namespace;
            let node: XmlNode = {
                prefix: this.elementStack[this.depth]!.prefix,
                namespace_: this.elementStack[this.depth]!.namespace_
            }
            this.multNsp.set(this.depth, node);
            this.type = XmlType.OTHERS;
            this.writeToBuffer(stringBuilder, 'SetNamespace');
        }

        /**
         * Adds an empty XML element with the specified name to the output buffer.
         *
         * @param {string} name - The name of the empty XML element to be added.
         */
        public addEmptyElement(name: string): void {
            let stringBuilder = this.prepareBufferForNewItem(name);

            stringBuilder.append(`<${name}/>`);
            this.type = XmlType.OTHERS;
            this.writeToBuffer(stringBuilder, 'AddEmptyElement');
        }

        /**
         * Sets the XML declaration for the current document.
         *
         * @throws {BusinessError} Throws an error if the declaration is set in an illegal position.
         */
        public setDeclaration(): void {
            if (this.isHasDecl) {
                throw createBusinessError(TypeErrorCodeId, 'illegal position for declaration');
            }

            let stringBuilder = new StringBuilder();
            stringBuilder.append(`<?xml version=\"1.0\" encoding=\"${this.encoding}\"?>`);
            this.isHasDecl = true;
            this.writeToBuffer(stringBuilder, 'SetDeclaration');
        }

        /**
         * Sets a comment in the XML output.
         *
         * @param {string} comment - The comment string to be added to the XML output.
         */
        public setComment(text: string): void {
            let stringBuilder = this.prepareBufferForNewItem(text);

            stringBuilder.append(`<!--${text}-->`);
            this.type = XmlType.OTHERS;
            this.writeToBuffer(stringBuilder, 'SetComment');
        }

        /**
         * Sets the CDATA section for the XML content.
         *
         * @param {string} data - The string data to be wrapped in a Cdata section.
         */
        public setCDATA(text: string): void {
            let stringBuilder = this.prepareBufferForNewItem(text);

            text = text.replaceAll(']]>', ']]]]><![CDATA[>');
            stringBuilder.append(`<![CDATA[${text}]]>`);
            this.type = XmlType.OTHERS;
            this.writeToBuffer(stringBuilder, 'SetCdata');
        }

        /**
         * Sets the text content for the current XML element.
         *
         * @param {string} text - The text content to set for the current XML element.
         */
        public setText(text: string): void {
            this.checkEmptyParameter(text);
            let stringBuilder = new StringBuilder();
            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }

            this.writeEscaped(stringBuilder, text);
            this.type = XmlType.TEXT;
            this.writeToBuffer(stringBuilder, 'SetText');
        }

        /**
         * Sets the document type (DOCTYPE) for the XML output.
         *
         * @param {string} text - The DOCTYPE declaration to be added to the XML output.
         */
        public setDocType(text: string): void {
            let stringBuilder = this.prepareBufferForNewItem(text);

            stringBuilder.append(`<!DOCTYPE ${text}>`);
            this.type = XmlType.OTHERS;
            this.writeToBuffer(stringBuilder, 'SetDocType');
        }

        /**
         * Increases the length of the element stack by creating a new `XmlNode` object with default empty values for
         * `name`, `prefix`, and `namespace_`, and pushing it onto the `elementStack`.
         */
        private increaseStackLength(): void {
            let node: XmlNode = {
                name: '',
                prefix: '',
                namespace_: ''
            }
            this.elementStack.push(node);
        }

        /**
         * Escapes special characters in the given string and appends the result to the provided StringBuilder.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder to which the escaped string will be appended.
         * @param {string} s - The input string to be escaped.
         */
        private writeEscaped(stringBuilder: StringBuilder, s: string): void {
            s = s.replaceAll('&', '&amp;');
            s = s.replaceAll('\'', '&apos;');
            s = s.replaceAll('\"', '&quot;');
            s = s.replaceAll('>', '&gt;');
            s = s.replaceAll('<', '&lt;');
            stringBuilder.append(s);
        }

        /**
         * Writes the content of the `stringBuilder` array to the `buffer` starting at the current position (`iPos`).
         * If the buffer does not have enough space to accommodate the data, an error is thrown.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder will be write.
         * @param {string} name - The name used in the error message if the operation fails.
         * @throws {BusinessError} Throws an error if the buffer does not have enough space to write the data.
         */
        private writeToBuffer(stringBuilder: StringBuilder, name: string): void {
            let tempString = stringBuilder.toString();
            let iLenTemp = tempString.length;
            if (this.buffer.byteLength > this.iPos + iLenTemp - BufferIndexOffset) {
                for (let i = 0; i < iLenTemp; i++) {
                    if (this.buffer instanceof ArrayBuffer) {
                        (this.buffer as ArrayBuffer).set((i + this.iPos) as int, tempString.charAt(i).toByte());
                    } else {
                        ((this.buffer as DataView).buffer as ArrayBuffer).set((i + this.iPos) as int,
                            tempString.charAt(i).toByte());
                    }
                }
                this.iPos += iLenTemp;
            } else {
                throw createBusinessError(TypeErrorCodeId, `XmlSerializer:: ${name} memcpy_s failed`);
            }
        }

        /**
         * Handles the namespace processing for the current element in the stack.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append
         * namespace declarations to the XML output.
         */
        private spliceNamespace(stringBuilder: StringBuilder): void {
            if (this.depth === 0) {
                return;
            }

            this.elementStack[this.depth].prefix = this.elementStack[this.depth - DepthOffset].prefix;
            this.elementStack[this.depth].namespace_ = this.elementStack[this.depth - DepthOffset].namespace_;

            let prevDepthInfo = this.multNsp.get(this.depth - DepthOffset);
            if (prevDepthInfo === undefined) {
                return;
            }

            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                let prefix = prevDepthInfo.prefix;
                let namespace = prevDepthInfo.namespace_;
                stringBuilder.append(` xmlns:${prefix}=\"${namespace}\"`);
                this.multNsp.get(this.depth - DepthOffset)!.prefix = '';
                this.multNsp.get(this.depth - DepthOffset)!.namespace_ = '';

            }
        }

        /**
         * Appends a newline character followed by indentation spaces to the `out` property.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append spaces to the XML output.
         */
        private nextItem(stringBuilder: StringBuilder): void {
            stringBuilder.append('\r\n');
            stringBuilder.append(' '.repeat(IndentationSpaces * this.depth));
        }

        /**
         * Checks if the provided parameter is an empty string.
         * Throws a `BusinessError` if the parameter is empty.
         *
         * @param {string} para - The parameter to be checked.
         */
        private checkEmptyParameter(para: string): void {
            if (para.length === 0) {
                throw createBusinessError(TypeErrorCodeId, 'Parameter error. Parameter cannot be empty');
            }
        }

        /**
         * Prepares the buffer for adding a new item by handling namespace line break and indentation.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder instance to append content to.
         */
        private prepareBufferForNewItem(name: string): StringBuilder {
            this.checkEmptyParameter(name);
            let stringBuilder = new StringBuilder();
            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(stringBuilder);
                stringBuilder.append('>');
            }
            if (this.type !== XmlType.NONE) {
                this.nextItem(stringBuilder);
            }

            return stringBuilder;
        }
    }
}
