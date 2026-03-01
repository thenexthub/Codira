/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

import { BusinessError } from '@ohos.base';
import { util } from './@ohos.util';
import buffer from '@ohos.buffer';

/**
 * The stream module provides a comprehensive set of stream processing capabilities, including four types of streams:
 * - Writable: streams designed for writing data to.
 * - Readable: streams designed for reading data from.
 * - Duplex: streams that are both readable and writable.
 * - Transform: a specialized type of duplex stream that can modify or transform data as it's being written and read.
 *
 */
export namespace stream {
    const DEFAULT_ENCODING = 'utf-8';
    const DEFAULT_HIGH_WATER_MARK = 16 * 1024;
    const ENCODING_SET: string[] = [
        'ascii', 'utf-8', 'UTF-8', 'gbk', 'GBK', 'GB2312', 'gb2312',
        'GB18030', 'gb18030', 'ibm866', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13',
        'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u', 'macintosh',
        'windows-874', 'windows-1250', 'windows-1251', 'windows-1252',
        'windows-1253', 'windows-1254', 'windows-1255', 'windows-1256',
        'windows-1257', 'windows-1258', 'big5', 'euc-jp', 'iso-2022-jp',
        'shift_jis', 'euc-kr', 'x-mac-cyrillic', 'utf-16be', 'utf-16le'];

    // BusinessError
    const ERR_DOWRITE_NOT_IMPLEMENTED: BusinessError =
        createBusinessError(10200035, 'The doWrite() method is not implemented.');
    const ERR_WRITE_AFTER_END: BusinessError =
        createBusinessError(10200036, 'Write after end.');
    const ERR_STREAM_ALREADY_FINISHED: BusinessError =
        createBusinessError(10200036, 'Stream already finished.');
    const ERR_MULTIPLE_CALLBACK: BusinessError =
        createBusinessError(10200037, 'Callback called multiple times.');
    const ERR_DOREAD_NOT_IMPLEMENTED: BusinessError =
        createBusinessError(10200038, 'The doRead() method is not implemented.');
    const ERR_DOTRANSFORM_NOT_IMPLEMENTED: BusinessError =
        createBusinessError(10200039, 'The doTransform() method is not implemented.');

    enum WritableEvent {
        CLOSE = 'close',
        DRAIN = 'drain',
        ERROR = 'error',
        FINISH = 'finish',
        PIPE = 'pipe',
        UNPIPE = 'unpipe',
    }

    enum ReadableEvent {
        CLOSE = 'close',
        DATA = 'data',
        END = 'end',
        ERROR = 'error',
        PAUSE = 'pause',
        READABLE = 'readable',
        RESUME = 'resume',
    }

    function createBusinessError(code: int, message: string) {
        let err: BusinessError = new BusinessError();
        err.name = 'BusinessError';
        err.message = message;
        err.code = code;
        return err;
    }

    function processErrOrClose(stream: Readable, err: Error, sync: boolean = false): void {
        if (stream.closed) {
            return;
        }

        if (err) {
            stream.errored = true;
            stream.closed = true;
            if (sync) {
                Promise.resolve().then((): void => {
                    emitError(stream, err);
                    emitClose(stream);
                });
            } else {
                emitError(stream, err);
                emitClose(stream);
            }
        }
    }

    function emitError(stream: Readable, err: Error): void {
        if(stream.errorEmitted) {
            return;
        }

        stream.errorEmitted = true;
        stream.listener?.emit(ReadableEvent.ERROR, err);
    }

    function emitClose(stream: Readable): void {
        if(stream.closedEmitted) {
            return;
        }

        stream.closedEmitted = true;
        stream.listener?.emit(ReadableEvent.CLOSE);
    }

    function onceWrapper(state: OnceState) {
        if (!state.fired) {
            state.target!.off(state.event!, state.wrapFn!);
            state.fired = true;
            return state.callback!.unsafeCall();
        } else {
            const func = (): void => {};
            return func;
        }
    }

    function _onceWrap(target: EventEmitter, event: string, callback: Function): Function {
        const onceState: OnceState = {
            fired: false,
            wrapFn: undefined,
            target: undefined,
            event: undefined,
            callback: undefined
        };
        onceState.fired = false;
        onceState.wrapFn = undefined;
        onceState.target = target;
        onceState.event = event;
        onceState.callback = callback;

        const wrapped = () => {
            return onceWrapper(onceState);
        };

        onceState.wrapFn = wrapped;

        return wrapped;
    }

    function advanceListener(stream: Writable, event: WritableEvent, callback: Function): void {
        if (!stream.listener?.handlers || !stream.listener?.handlers.has(event)) {
            stream.on(event, callback);
        } else {
            stream.listener?.handlers.get(event)?.push(callback);
        }
    }

    interface OnceState {
        fired: boolean;
        wrapFn: Function | undefined;
        target: EventEmitter | undefined;
        event: string | undefined;
        callback: Function | undefined;
    }

    interface WritableBuffer {
        encoding?: string;
        chunk: string | Uint8Array;
        callback: Function;
    }

    /**
     * Return readable options.
     *
     */
    export interface ReadableOptions {
        /**
         * Specifies the encoding format of the data. If this parameter is provided,
         * the readable stream decodes the data into a string in the specified encoding format. Default: utf8.
         * If an invalid string is entered, a 401 exception is thrown in the Readable constructor.
         * Supported encoding formats: utf-8, ibm866, iso-8859-2, iso-8859-3, iso-8859-4, iso-8859-5, iso-8859-6,
         * iso-8859-7, iso-8859-8, iso-8859-8-i, iso-8859-10, iso-8859-13, iso-8859-14, iso-8859-15, koi8-r, koi8-u,
         * macintosh, windows-874, windows-1250, windows-1251, windows-1252, windows-1253, windows-1254, windows-1255,
         * windows-1256, windows-1257, windows-1258, x-mac-cyrillic, gbk, gb18030, big5, euc-jp, iso-2022-jp, shift_jis,
         * euc-kr, utf-16be, utf-16le.
         *
         */
        encoding?: string;
    }

    /**
     * EventEmitter to which event callbacks can be held.
     *
     */
    class EventEmitter {
        public handlers: Map<string, Array<Function>>;

        /**
         * The EventEmitter constructor.
         *
         */
        constructor() {
            this.handlers = new Map<string, Array<Function>>();
        }

        /**
         * Registering event messages.
         *
         * @param { string } event - Register event.
         * @param { Function } callback - Event callbacks.
         */
        on(event: string, callback: Function): void {
            if (!this.handlers.has(event)) {
                const funcList = new Array<Function>();
                this.handlers.set(event, funcList);
            }
            const funcList = this.handlers.get(event);
            funcList!.push(callback);
        }

        /**
         * Unregistering event messages.
         *
         * @param { string } event - Unregister event.
         * @param { Function } callback - Event callbacks.
         */
        off(event: string, callback: Function): void {
            if (this.handlers.has(event)) {
                const funcList = this.handlers.get(event);
                const pos = funcList!.findIndex((element) => element === callback);
                if (pos !== -1) {
                    funcList!.splice(pos, 1);
                }
            }
        }

        /**
         * Clear event messages.
         *
         * @param { string } event - Clear event.
         */
        clear(event: string): void {
            if (this.handlers.has(event)) {
                const funcList = this.handlers.get(event);
                funcList!.splice(0);
            }
        }

        /**
         * Emit event messages.
         *
         * @param { string } event - Emit event.
         * @param { Object } [param] - The parameter of event callbacks.
         */
        emit(event: string, param?: Object): void {
            if (this.handlers.has(event)) {
                const funcList = this.handlers.get(event);
                funcList!.forEach((callback: Function) => {
                    callback.unsafeCall(param);
                })
            }
        }

        /**
         * Judge the event is on or not.
         *
         * @param { string } event - The event to be judged is on or not.
         * @returns { boolean } Event is on returns true, event is off returns false.
         */
        isOn(event: string): boolean {
            return this.handlers.has(event) ? this.handlers.get(event)!.length > 0 : false;
        }

        /**
         * Find the event count in the listener.
         *
         * @param { string } event - The event to be found its count in the listener.
         * @returns { int } The event count in the listener.
         */
        listenerCount(event: string): int {
            return this.handlers.has(event) ? this.handlers.get(event)!.length.toInt() : 0;
        }
    }

    /**
     * Streams to which data can be written.
     *
     */
    export class Writable {
        public doWriteFunc: ((chunk: string | Uint8Array, encoding: string, callback: Function) => void) | null;
        public doWritevFunc: ((chunks: string[] | Uint8Array[], callback: Function) => void) | null;
        public listener: EventEmitter | undefined;

        protected encoder = new util.TextEncoder();

        private buffer: Array<WritableBuffer>;
        private closedInner: boolean;
        private defaultEncoding: string | undefined;
        private encoding: string | undefined;
        private endCallback: Function | undefined;
        private ending: boolean;
        private erroredInner: Error | null;
        private isDoWritevOverride: boolean;
        private writableBufferLength: int;
        private writableCb: int;
        private writableCorkedInner: int;
        private writableEndedInner: boolean;
        private writableFinishedInner: boolean;
        private writableHighWatermarkInner: int;
        private writableInner: boolean;
        private writableLengthInner: int;
        private writableNeedDrainInner: boolean;
        private writableObjectModeInner: boolean;
        private writableSync: boolean;
        private writeCallbackBuffer: Array<Function>;
        private writaCallbackMove: boolean;
        private writeCallbacks: Array<Function>;
        private writing: boolean;

        /**
         * The Writable constructor.
         *
         */
        constructor() {
            this.doWriteFunc = this.noWriteOpes;
            this.doWritevFunc = null;
            this.listener = new EventEmitter();
            this.buffer = [];
            this.closedInner = false;
            this.defaultEncoding = DEFAULT_ENCODING;
            this.encoding = DEFAULT_ENCODING;
            this.endCallback = undefined;
            this.ending = false;
            this.erroredInner = null;
            this.isDoWritevOverride = true;
            this.writableBufferLength = 0;
            this.writableCb = 0;
            this.writableCorkedInner = 0;
            this.writableEndedInner = false;
            this.writableFinishedInner = false;
            this.writableHighWatermarkInner = DEFAULT_HIGH_WATER_MARK;
            this.writableInner = true;
            this.writableLengthInner = 0;
            this.writableNeedDrainInner = false;
            this.writableObjectModeInner = false;
            this.writableSync = true;
            this.writeCallbackBuffer = [];
            this.writaCallbackMove = false;
            this.writeCallbacks = [];
            this.writing = false;

            this.doInitialize((): void => {
                this.listener?.emit(WritableEvent.ERROR);
            });
        }

        private getChunkLength(chunk: string | Uint8Array): int {
            if (chunk instanceof Uint8Array) {
                return (chunk as Uint8Array).byteLength.toInt();
            } else if ((chunk as string) === '') {
                throw createBusinessError(401, 'Cannot read property byteLength of undefined');
            } else {
                return (chunk as string).length.toInt();
            }
        }

        private setEncoding(encoding: string): boolean {
            let encodingLowCase: string = encoding.toLowerCase();
            if (encodingLowCase === 'utf8' || encodingLowCase === 'ascii') {
                encoding = DEFAULT_ENCODING;
                encodingLowCase = DEFAULT_ENCODING;
            }

            if (ENCODING_SET.indexOf(encodingLowCase) !== -1) {
                this.encoding = encodingLowCase;
                try {
                    this.encoder = new util.TextEncoder(encodingLowCase);
                } catch (e) {
                    this.throwError(e as Error);
                    this.errorFresh();
                    return false;
                }
                return true;
            } else {
                this.throwError(createBusinessError(401, `Parameter error. The type of ${encoding} must be string.`));
                this.errorFresh();
                return false;
            }
        }

        private writeUint8Array(chunk: Uint8Array, encoding?: string, callback?: Function): boolean {
            const chunkLength = this.getChunkLength(chunk);
            if (this.encoding !== this.defaultEncoding) {
                this.setEncoding(this.defaultEncoding!);
            }
            this.writableLengthInner = this.writableLengthInner! + chunkLength;
            const hasRemaining = this.writableLengthInner! < this.writableHighWatermark!;
            let executed: boolean = false;
            const fnBack = (...param: Object[]): void => {
                if (!executed) {
                    executed = true;
                    if (param.length > 0) {
                        callback?.unsafeCall(param[0]);
                    } else {
                        callback?.unsafeCall();
                    }
                    this.writing = false;
                    this.writableCb--;

                    if (param.length > 0 && param[0] instanceof Error && !this.erroredInner) {
                        this.writableInner = false;
                        this.erroredInner = param[0] as Error;
                        Promise.resolve().then((): void => {
                            this.throwError(param[0] as Error);
                            this.errorFresh();
                        });
                        return;
                    }
                    if (this.erroredInner) {
                        this.errorFresh();
                        return;
                    }

                    this.writableLengthInner = this.writableLengthInner! - chunkLength;
                    if (!this.writableSync) {
                        this.freshCacheV();
                        this.afterWrite();
                    } else {
                        Promise.resolve().then((): void => {
                            this.afterWrite();
                        });
                    }
                } else {
                    if (!this.erroredInner) {
                        this.writableInner = false;
                        this.erroredInner = ERR_MULTIPLE_CALLBACK;
                        Promise.resolve().then((): void => {
                            this.throwError(ERR_MULTIPLE_CALLBACK);
                        });
                    }
                    this.errorFresh();
                }
            };
            if (this.writableCorkedInner === 0) {
                if (!this.writing) {
                    this.writing = true;
                    this.writableCb++;
                    this.writableSync = true;
                    this.doWrite(chunk, encoding ?? DEFAULT_ENCODING, fnBack);
                    this.writableSync = false;
                } else {
                    const buffInfo: WritableBuffer = {
                        encoding: encoding,
                        chunk: chunk,
                        callback: fnBack
                    };
                    this.buffer.push(buffInfo);
                    if (callback instanceof Function) {
                        this.writeCallbackBuffer.push(callback);
                    } else {
                        this.writeCallbackBuffer.push((): void => {});
                    }
                    this.writableBufferLength += chunkLength;
                }
            } else {
                const buffInfo: WritableBuffer = {
                    encoding: encoding,
                    chunk: chunk,
                    callback: fnBack
                };
                this.buffer.push(buffInfo);
                if (callback instanceof Function) {
                    this.writeCallbackBuffer.push(callback);
                } else {
                    this.writeCallbackBuffer.push((): void => {});
                }
                this.writableBufferLength += chunkLength;
            }
            return this.erroredInner ? false : hasRemaining;
        }

        private writeString(chunk: string, encoding?: string, callback?: Function): boolean {
            const chunkLength = this.getChunkLength(chunk);
            if (this.encoding !== this.defaultEncoding) {
                this.setEncoding(this.defaultEncoding!);
            }
            this.writableLengthInner = this.writableLengthInner! + chunkLength;
            const hasRemaining = this.writableLengthInner! < this.writableHighWatermark!;
            let executed: boolean = false;
            const fb = (...param: Object[]): void => {
                if (!executed) {
                    executed = true;
                    if (param.length > 0) {
                        callback?.unsafeCall(param[0]);
                    } else {
                        callback?.unsafeCall();
                    }
                    this.writing = false;
                    this.writableCb--;
                    if (param.length > 0 && param[0] instanceof Error && !this.erroredInner) {
                        this.writableInner = false;
                        this.erroredInner = param[0] as Error;
                        Promise.resolve().then((): void => {
                            this.throwError(param[0] as Error);
                            this.errorFresh();
                        });
                        return;
                    }
                    if (this.erroredInner) {
                        this.errorFresh();
                        return;
                    }
                    this.writableLengthInner = this.writableLengthInner! - chunkLength;
                    if (!this.writableSync) {
                        this.freshCacheV();
                        this.afterWrite();
                    } else {
                        Promise.resolve().then((): void => {
                            this.afterWrite();
                        });
                    }
                } else {
                    if (!this.erroredInner) {
                        this.writableInner = false;
                        this.erroredInner = ERR_MULTIPLE_CALLBACK;
                        Promise.resolve().then((): void => {
                            this.throwError(ERR_MULTIPLE_CALLBACK);
                        });
                    }
                    this.errorFresh();
                }
            };

            if (this.writableCorkedInner === 0) {
                if (!this.writing) {
                    this.writing = true;
                    this.writableCb++;
                    this.writableSync = true;
                    this.doWrite(chunk, encoding ?? DEFAULT_ENCODING, fb);
                    this.writableSync = false;
                } else {
                    const buffInfo: WritableBuffer = {
                        encoding: encoding,
                        chunk: chunk,
                        callback: fb
                    };
                    this.buffer.push(buffInfo);
                    if (callback instanceof Function) {
                        this.writeCallbackBuffer.push(callback);
                    } else {
                        this.writeCallbackBuffer.push((): void => {});
                    }
                    this.writableBufferLength += chunkLength;
                }
            } else {
                const buffInfo: WritableBuffer = {
                    encoding: encoding,
                    chunk: chunk,
                    callback: fb
                };
                this.buffer.push(buffInfo);
                if (callback instanceof Function) {
                    this.writeCallbackBuffer.push(callback);
                } else {
                    this.writeCallbackBuffer.push((): void => {});
                }
                this.writableBufferLength += chunkLength;
            }
            return this.erroredInner ? false : hasRemaining;
        }

        private errorFresh(): void {
            if (this.writaCallbackMove) {
                this.writeCallbacks.forEach(callback => {
                    if (callback instanceof Function) {
                        callback.unsafeCall(this.erroredInner ?? null);
                    }
                });
            }
            this.writeCallbackBuffer.forEach(callback => {
                if (callback instanceof Function) {
                    callback.unsafeCall(this.erroredInner);
                }
            });
            this.endCallback?.unsafeCall(this.erroredInner);
            this.endCallback = undefined;
            this.buffer = [];
            this.writeCallbacks = [];
            this.writeCallbackBuffer = [];
            this.writableBufferLength = 0;
            this.writableLengthInner = 0;
        }

        private freshCache(): void {
            if (this.writableCorkedInner !== 0) {
                return;
            }
            let currentLength: int = this.buffer.length.toInt();
            let current: WritableBuffer[] = this.buffer.splice(0 as int, 1 as int);
            this.writeCallbackBuffer.splice(0 as int, 1 as int);
            while (currentLength > 0) {
                this.writableBufferLength -= this.getChunkLength(current[0].chunk);
                this.writing = true;
                this.writableCb++;
                this.writableSync = true;
                this.doWrite(current[0].chunk, current[0].encoding ?? DEFAULT_ENCODING, current[0].callback);
                this.writableSync = false;
                if (!this.writing) {
                    currentLength = this.buffer.length.toInt();
                    current = this.buffer.splice(0 as int, 1 as int);
                } else {
                    break;
                }
            }
            if (this.finishMayBe()) {
                this.finishWrite();
            }
        }

        private freshCacheV(): void {
            if (this.writableCorkedInner !== 0) {
                return;
            }
            if (this.buffer.length > 0) {
                const bufferChunkLength = this.writableBufferLength;
                this.writableBufferLength = 0;
                this.writeCallbacks = [];
                let executed: boolean = false;
                const funCallback = (...param: Object[]): void => {
                    if (!executed) {
                        executed = true;
                        if (param.length > 0 && param[0] instanceof Error && !this.erroredInner) {
                            this.throwError(param[0] as Error);
                            this.errorFresh();
                            return;
                        }
                        this.writing = false;
                        this.writableCb--;
                        if (this.erroredInner) {
                            this.errorFresh();
                            return;
                        }
                        this.writableLengthInner = this.writableLengthInner! - bufferChunkLength;
                        this.writeCallbacks.forEach(callback => {
                            if (callback instanceof Function) {
                                callback.unsafeCall();
                            }
                        });
                        this.writeCallbacks = [];

                        if (!this.writableSync) {
                            this.freshCacheV();
                            this.afterWrite();
                        } else {
                            Promise.resolve().then((): void => {
                                this.afterWrite();
                            });
                        }
                    } else {
                        if (!this.erroredInner) {
                            this.throwError(ERR_MULTIPLE_CALLBACK);
                        }
                        this.errorFresh();
                    }
                };
                const oldWriting = this.writing;
                this.writing = true;
                const oldWritableCb = this.writableCb;
                this.writableCb++;
                const oldWritableSync = this.writableSync;
                this.writableSync = true;
                this.writeCallbackBuffer.forEach((callback: Function ) => {
                    this.writeCallbacks.push(callback);
                });
                this.writaCallbackMove = false;
                let strChunks: string[] = new (string)[this.buffer.length];
                let arrayChunks: Uint8Array[] = new (Uint8Array)[this.buffer.length];
                let isString: boolean = false;
                let index: int = 0;
                this.buffer.forEach((value: WritableBuffer) => {
                    if (value.chunk instanceof string) {
                        strChunks[index] = value.chunk as string;
                        isString = true;
                    } else {
                        arrayChunks[index] = value.chunk as Uint8Array;
                    }
                    index++;
                });
                if (isString) {
                    this.doWritev(strChunks, funCallback);
                } else {
                    this.doWritev(arrayChunks, funCallback);
                }
                if (this.isDoWritevOverride) {
                    this.writableSync = false;
                    this.buffer = new Array<WritableBuffer>();
                    this.writeCallbackBuffer = new Array<Function>();
                    this.writaCallbackMove = true;
                    if (this.finishMayBe()) {
                        this.finishWrite();
                    }
                } else {
                    this.isDoWritevOverride = false;
                    this.writableBufferLength = bufferChunkLength;
                    this.writing = oldWriting;
                    this.writableCb = oldWritableCb;
                    this.writableSync = oldWritableSync;
                    this.freshCache();
                }
            } else {
                if (this.finishMayBe()) {
                    this.finishWrite();
                }
            }
        }

        private finishMayBe(): boolean {
            return !this.writing && this.writableCorkedInner === 0 && this.ending;
        }

        private finishWrite(): void {
            if (!this.writableFinishedInner && this.writableCb === 0 && !this.erroredInner) {
                if (this.writableSync) {
                    Promise.resolve().then((): void => {
                        this.writableFinishedInner = true;
                        this.endCallback?.unsafeCall(this.erroredInner);
                        this.listener?.emit(WritableEvent.FINISH);
                    });
                } else {
                    this.writableFinishedInner = true;
                    this.endCallback?.unsafeCall(this.erroredInner);
                    this.listener?.emit(WritableEvent.FINISH);
                }
                Promise.resolve().then((): void => {
                    if (!this.erroredInner) {
                        this.closedInner = true;
                        this.listener?.emit(WritableEvent.CLOSE);
                    }
                });
            }
        }

        private afterWrite(): void {
            if (!this.finishMayBe() && this.writableNeedDrainInner && this.writableLengthInner === 0) {
                this.writableNeedDrainInner = false;
                this.listener?.emit(WritableEvent.DRAIN);
            }
        }

        noWriteOpes(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            if (chunk instanceof string) {
                this.doWritev([chunk as string], callback);
            } else {
                this.doWritev([chunk as Uint8Array], callback);
            }
            if (!this.isDoWritevOverride) {
                this.isDoWritevOverride = true;
                this.throwError(ERR_DOWRITE_NOT_IMPLEMENTED);
                this.errorFresh();
            }
        }

        private throwError(error: Error): void {
            this.writableInner = false;
            this.erroredInner = error;
            if (this.listener && this.listener!.listenerCount(WritableEvent.ERROR) > 0) {
                Promise.resolve().then((): void => {
                    this.listener?.emit(WritableEvent.ERROR, this.erroredInner as Error);
                    if (!this.closedInner) {
                        this.closedInner = true;
                        this.listener?.emit(WritableEvent.CLOSE);
                    }
                });
            } else {
                throw this.erroredInner as Error;
            }
        }

        private endInner(callback?: Function): void {
            if (!this.writableFinishedInner && !this.writableEndedInner && this.writableCb === 0) {
                Promise.resolve().then((): void => {
                    if (!this.erroredInner) {
                        this.writableFinishedInner = true;
                        callback?.unsafeCall(this.erroredInner);
                        this.listener?.emit(WritableEvent.FINISH);
                        this.closedInner = true;
                        this.listener?.emit(WritableEvent.CLOSE);
                    }
                });
            }
        }

        /**
         * Writes a chunk to Writable and invokes callback when the chunk is flushed. The return value indicates
         * whether the internal buffer of the Writable reaches the hightWaterMark. If true is returned, the buffer
         * does not reach the hightWaterMark. If false is returned, the buffer has been reached. The write function
         * should be called after the drain event is triggered. If the write function is called continuously,
         * the chunk is still added to the buffer until the memory overflows.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { boolean } Write success returns true, write failure returns false.
         * @throws { BusinessError } 401 - Parameter error.
         * @throws { BusinessError } 10200035 - The doWrite method has not been implemented.
         * @throws { BusinessError } 10200036 - The stream has been ended.
         * @throws { BusinessError } 10200037 - The callback is invoked multiple times consecutively.
         */
        write(chunk?: string | Uint8Array, encoding?: string, callback?: Function): boolean {
            if (encoding) {
                this.setEncoding(encoding!);
            }
            if (chunk === null || chunk === undefined) {
                throw createBusinessError(401, `Parameter error. The type of ${chunk} must be string or UintArray`);
            }
            if (this.ending && !this.erroredInner) {
                this.erroredInner = ERR_WRITE_AFTER_END;
                this.writableInner = false;
                this.errorFresh();

                Promise.resolve().then((): void => {
                    callback?.unsafeCall(this.erroredInner);
                    if (this.listener && this.listener!.listenerCount(WritableEvent.ERROR) > 0) {
                        this.listener!.emit(WritableEvent.ERROR, this.erroredInner as Error);
                        if (!this.closedInner) {
                            this.closedInner = true;
                            this.listener?.emit(WritableEvent.CLOSE);
                        }
                    } else {
                        throw this.erroredInner as Error;
                    }
                });
                return false;
            }
            if (this.erroredInner) {
                this.errorFresh();
                callback?.unsafeCall(this.erroredInner);
                return false;
            }
            let flag: boolean = false;
            if (chunk instanceof Uint8Array) {
                flag = this.writeUint8Array(chunk, encoding ?? this.encoding, callback);
            } else {
                flag = this.writeString(chunk! as string, encoding ?? this.encoding, callback);
            }
            if (!flag) {
                this.writableNeedDrainInner = true;
            }
            return flag;
        }

        /**
         * Write the last chunk to Writable.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { Writable } Returns the Writable object.
         * @throws { BusinessError } 401 - Parameter error.
         * @throws { BusinessError } 10200035 - The doWrite method has not been implemented.
         */
        end(chunk?: string | Uint8Array, encoding?: string, callback?: Function): Writable {
            if (this.erroredInner) {
                Promise.resolve().then((): void => {callback?.unsafeCall(this.erroredInner!)});
                return this;
            }
            if (this.writableFinishedInner) {
                Promise.resolve().then((): void => {callback?.unsafeCall(this.erroredInner)});
                this.throwError(ERR_STREAM_ALREADY_FINISHED);
                return this;
            } else if (this.writableEndedInner) {
                Promise.resolve().then((): void => {callback?.unsafeCall(this.erroredInner)});
                this.throwError(ERR_WRITE_AFTER_END);
                return this;
            }

            this.writableNeedDrainInner = false;
            this.writableInner = false;

            if (chunk) {
                this.write(chunk, encoding);
            }

            if (this.writableCorkedInner > 0) {
                this.writableCorkedInner = 1;
                this.uncork();
            }

            this.endCallback = callback;
            this.ending = true;
            this.endInner(callback);
            this.writableEndedInner = true;
            return this;
        }

        /**
         * Set the default encoding mode.
         *
         * @param { string } [encoding] - Encoding type. Default: utf-8.
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        setDefaultEncoding(encoding?: string): boolean {
            if (!encoding) {
                return false;
            }

            const ret = this.setEncoding(encoding);
            if (ret) {
                this.defaultEncoding = encoding;
            }
            return ret;
        }

        /**
         * After the call, all Write operations will be forced to write to the buffer instead of being flushed.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        cork(): boolean {
            this.writableCorkedInner += 1;
            return true;
        }

        /**
         * After calling, flush all buffers.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        uncork(): boolean {
            if (this.writableCorkedInner > 0) {
                this.writableCorkedInner -= 1;
                if (this.writableCorkedInner === 0  && !this.writing) {
                    this.freshCacheV();
                }
            }

            return true;
        }

        /**
         * Registering Event Messages.
         *
         * @param { string } event - Register Event.
         * @param { Function } callback - event callbacks.
         */
        on(event: string, callback: Function): void {
            this.listener!.on(event, callback);
        }

        /**
         * Cancel event message.
         *
         * @param { string } event - Register Event.
         * @param { Function } [callback] - event callbacks.
         */
        off(event: string, callback?: Function): void {
            if (callback) {
                this.listener!.off(event, callback);
            } else {
                this.listener!.clear(event);
            }
        }

        /**
         * This method is invoked by the Writable method during initialization and must not be invoked directly.
         * After the resource is initialized in the doInitialize method, the callback () method is invoked.
         *
         * @param { Function } callback - Callback when the stream has completed the initial.
         */
        doInitialize(callback: Function): void {}

        /**
         * Implemented by subclass inheritance. The implementation logic of flushing chunks in the buffer must not be
         * directly called. The call is controlled by Writable.write.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         */
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            if (this.doWriteFunc !== null) {
                this.doWriteFunc?.(chunk, encoding, callback);
                return;
            }
        }

        /**
         * The implementation logic of flushing chunks in the buffer in batches should not be actively called.
         * The call is controlled by Writable.write.
         *
         * @param { string[] | Uint8Array[] } [chunks] - Data to be written.
         * @param { Function } [callback] - Callback after writing.
         */
        doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
            if (this.doWritevFunc !== null) {
                this.doWritevFunc?.(chunks, callback);
                return;
            }
            this.isDoWritevOverride = false;
        }

        /**
         * Returns boolean indicating whether it is in ObjectMode.
         */
        get writableObjectMode(): boolean {
            return this.writableObjectModeInner;
        }

        /**
         * Value of highWatermark.
         */
        get writableHighWatermark(): int {
            return this.writableHighWatermarkInner;
        }

        /**
         * Is true if it is safe to call writable.write(), which means
         * the stream has not been destroyed or emitted 'error' or 'end'.
         *
         */
        get writable(): boolean {
            return this.writableInner;
        }

        /**
         * Size of data that can be flushed, in bytes or objects.
         */
        get writableLength(): int {
            return this.writableLengthInner;
        }

        /**
         * If the buffer of the stream is full and true, otherwise it is false.
         */
        get writableNeedDrain(): boolean {
            return this.writableNeedDrainInner;
        }

        /**
         * Number of times Writable.uncork() needs to be called in order to fully uncork the stream.
         */
        get writableCorked(): int {
            return this.writableCorkedInner;
        };

        /**
         * Whether Writable.end has been called.
         */
        get writableEnded(): boolean {
            return this.writableEndedInner;
        }

        /**
         * Whether Writable.end has been called and all buffers have been flushed.
         */
        get writableFinished(): boolean {
            return this.writableFinishedInner;
        }

        /**
         * Returns error if the stream has been destroyed with an error.
         */
        get errored(): Error | null {
            return this.erroredInner;
        }

        /**
         * Writable completes destroyfile and returns true, otherwise returns false.
         */
        get closed(): boolean {
            return this.closedInner;
        }
    }

    /**
     * The stream from which data can be read.
     *
     * @syscap SystemCapability.Utils.Lang
     */
    export class Readable {
        public listener: EventEmitter | undefined;
        public errorEmitted: boolean;
        public closedEmitted: boolean;

        protected encoder = new util.TextEncoder();
        protected stringDecoderInner = new util.StringDecoder();

        private buf: Array<buffer.Buffer | string | null>;
        private bufIndex: int;
        private awaitDrainWriters: Set<Writable>;
        private callbacks: Map<string, Array<Function>>;
        private readableDecoder: boolean;
        private isInitialized: boolean;
        private pauseInner: boolean;
        private readableHasPaused: boolean;
        private pipeWritableArrayInner: Array<Writable>;
        private readableObjectModeInner: boolean;
        private readableInner: boolean;
        private readableHighWatermarkInner: int;
        private readableFlowingInner: boolean | null;
        private readableLengthInner: int;
        private readableEncodingInner: string | null;
        private readableEndedInner: boolean;
        private erroredInner: boolean;
        private closedInner: boolean | undefined;
        private dataListenning: boolean | undefined;
        private readableListenning: boolean | undefined;
        private readableHasFlowingInner: boolean;
        private readableEndEmitted: boolean;
        private readableNeedReadable: boolean | undefined;
        private readableEmittedReadable: boolean;
        private isReading: boolean | undefined;
        private readableEmittedResume: boolean;
        private readableSync: boolean;
        private isReadingMore: boolean;
        private multiAwaitDrain: boolean;

        /**
         * The Readable constructor.
         *
         */
        constructor() {
            this.initializeMembers();
        }

        /**
         * The Readable constructor.
         *
         * @param { ReadableOptions } options - Provide options.
         * @throws { BusinessError } 401 - Parameter error.
         */
        constructor(options: ReadableOptions) {
            this.initializeMembers();

            if (options.encoding) {
                this.readableEncodingInner = options.encoding!;
                if (this.readableEncodingInner!.toLowerCase() === 'utf8') {
                    this.readableEncodingInner = DEFAULT_ENCODING;
                }

                if (ENCODING_SET.findIndex((element) => element === this.readableEncodingInner!.toLowerCase()) === -1) {
                    throw createBusinessError(401, `Unknown encoding: ${this.readableEncodingInner!.toLowerCase()}`);
                }

                this.stringDecoder = new util.StringDecoder(this.readableEncodingInner!);
            }
        }

        private initializeMembers(): void {
            this.listener = new EventEmitter();
            this.errorEmitted = false;
            this.closedEmitted = false;
            this.buf = new Array<buffer.Buffer | string | null>();
            this.bufIndex = 0;
            this.awaitDrainWriters = new Set<Writable>();
            this.callbacks = new Map<string, Array<Function>>();
            this.readableDecoder = false;
            this.isInitialized = false;
            this.pauseInner = false;
            this.readableHasPaused = false;
            this.pipeWritableArrayInner = new Array<Writable>();
            this.readableObjectModeInner = false;
            this.readableInner = true;
            this.readableHighWatermarkInner = DEFAULT_HIGH_WATER_MARK;
            this.readableFlowingInner = null;
            this.readableLengthInner = 0;
            this.readableEncodingInner = null;
            this.readableEndedInner = false;
            this.erroredInner = false;
            this.closedInner = undefined;
            this.dataListenning = false;
            this.readableListenning = undefined;
            this.readableHasFlowingInner = false;
            this.readableEndEmitted = false;
            this.readableNeedReadable = false;
            this.readableEmittedReadable = false;
            this.isReading = undefined;
            this.readableEmittedResume = false;
            this.readableSync = true;
            this.isReadingMore = false;
            this.multiAwaitDrain = false;
        }

        private computeNewReadableHighWatermark(readSize: int): int {
            readSize--;
            readSize |= readSize >>> 1;
            readSize |= readSize >>> 2;
            readSize |= readSize >>> 4;
            readSize |= readSize >>> 8;
            readSize |= readSize >>> 16;
            readSize++;
            return readSize;
        }

        private flowInner(stream: Readable): void {
            while (this.readableFlowing && stream.read() !== null);
        }

        private emitReadableNextCycle(stream: Readable): void {
            stream.read(0)
        }

        private emitReadableNow(stream: Readable): void {
            stream.readableNeedReadable = false;
            if (!stream.readableEmittedReadable) {
                stream.readableEmittedReadable = true;
                Promise.resolve().then((): void => {
                    stream.emitReadableInner(stream);
                });
            }
        }

        private emitReadableEnd(stream: Readable): void {
            if (!stream.readableEndEmitted) {
                stream.readableEndedInner = true;
                Promise.resolve().then((): void => {
                    this.emitReadableEndNextCycle(stream);
                });
            }
        }

        private emitReadableEndNextCycle(stream: Readable): void {
            if (!stream.erroredInner && !stream.closedInner
                && !stream.readableEndEmitted && stream.readableLengthInner === 0) {
                stream.readableEndEmitted = true;
                stream.listener?.emit(ReadableEvent.END);
            }
            emitClose(stream);
        }

        private emitReadableInner(stream: Readable): void {
            if (!stream.erroredInner && (stream.readableLengthInner || stream.readableEndedInner)) {
                stream.listener?.emit(ReadableEvent.READABLE);
                stream.readableEmittedReadable = false;
            }
            if (!stream.readableFlowingInner && !stream.readableEndedInner
                && stream.readableLengthInner <= stream.readableHighWatermarkInner) {
                stream.readableNeedReadable = true;
            } else {
                stream.readableNeedReadable = false;
            }
            stream.flowInner(stream)
        }

        private resumeInner(stream: Readable): void {
            if (!this.isReading) {
                stream.read(0);
            }
            stream.readableEmittedResume = false;
            stream.listener?.emit(ReadableEvent.RESUME);
            this.flowInner(stream);
            if (stream.readableFlowingInner && !stream.isReading) {
                stream.read(0);
            }
        }

        private endOfFlow(stream: Readable): void {
            if (stream.readableEndedInner) {
                return;
            }
            const stringDecoder = stream.readableDecoder ? stream.stringDecoderInner : null;
            if (stringDecoder) {
                const chunk = stringDecoder.end();
                if(chunk?.length) {
                    stream.buf.push(chunk);
                    stream.readableLengthInner += stream.readableObjectModeInner ? 1 : chunk.length.toInt();
                }
            }
            stream.readableEndedInner = true;

            if (stream.readableSync) {
                stream.emitReadableNow(stream);
            } else {
                stream.readableNeedReadable = false;
                stream.readableEmittedReadable = true;
                stream.emitReadableInner(stream);
            }
        }

        private canBeReadMore(stream: Readable): void {
            if (!stream.isReadingMore) {
                stream.isReadingMore = true;
                Promise.resolve().then((): void => {
                    while ((!stream.isReading && !stream.readableEndedInner) &&
                        (stream.readableLengthInner < stream.readableHighWatermarkInner
                        || (stream.readableFlowingInner && stream.readableLengthInner === 0))) {
                        const preLen = stream.readableLengthInner;
                        stream.read(0);
                        if (preLen === stream.readableLengthInner) {
                            break;
                        }
                        stream.isReadingMore = false;
                    }
                })
            }
        }

        private bufHasSpace(stream: Readable): boolean {
            return !stream.readableEndedInner &&
                (stream.readableLengthInner < stream.readableHighWatermarkInner ||
                stream.readableLengthInner === 0)
        }

        private pushChunkInner(stream: Readable, chunk: buffer.Buffer | string): void {
            if (stream.readableFlowingInner && stream.dataListenning
                && !stream.readableSync && stream.readableLengthInner === 0) {
                this.awaitDrainWriters.clear();
                stream.listener?.emit(ReadableEvent.DATA, chunk);
            } else {
                if (chunk instanceof buffer.Buffer && (chunk as buffer.Buffer).length) {
                    stream.readableLengthInner += (chunk as buffer.Buffer).length.toInt();
                    stream.buf.push(chunk);
                } else if (chunk instanceof string && (chunk as string).length) {
                    stream.readableLengthInner += (chunk as string).length.toInt();
                    stream.buf.push(chunk);
                }
                if (stream.readableNeedReadable) {
                    this.emitReadableNow(stream);
                }
            }
            stream.canBeReadMore(stream);
        }

        private pushByteModeChunk(stream: Readable,
            chunk: Uint8Array | string | undefined | null, encoding?: string): boolean {
            if (chunk === null) {
                stream.isReading = false;
                stream.endOfFlow(stream);
                return false;
            }

            let chunkBuffer: buffer.Buffer | undefined = undefined;
            let chunkString: string | undefined = undefined;
            let chunkIsBuffer: boolean = false;
            if (chunk instanceof string) {
                if (!encoding) {
                    encoding = DEFAULT_ENCODING;
                }
                if (stream.readableEncodingInner !== encoding) {
                    chunkBuffer = buffer.from(chunk, encoding as buffer.BufferEncoding);
                    chunkIsBuffer = true;
                    encoding = '';
                } else {
                    chunkString = chunk as string;
                }
            } else if (chunk instanceof Uint8Array) {
                chunkBuffer = buffer.from(chunk);
                chunkIsBuffer = true;
                encoding = '';
            }

            if (!chunk || (chunkIsBuffer && chunkBuffer!.length < 0) ||
                (!chunkIsBuffer && (chunk as string).length < 0)) {
                stream.isReading = false;
                stream.canBeReadMore(stream);
                return stream.bufHasSpace(stream);
            }

            if (stream.readableEndedInner) {
                processErrOrClose(stream, ERR_WRITE_AFTER_END);
                return false;
            }

            if (stream.erroredInner) {
                return false;
            }

            stream.isReading = false;
            if (stream.readableDecoder && !encoding) {
                if (chunkIsBuffer) {
                    chunkString = stream.stringDecoderInner.write(new Uint8Array(chunkBuffer!.buffer));
                    chunkIsBuffer = false;
                }
                if (chunkString!.length === 0) {
                    stream.canBeReadMore(stream);
                    return stream.bufHasSpace(stream);
                }
            }

            if (chunkIsBuffer) {
                stream.pushChunkInner(stream, chunkBuffer!);
            } else {
                stream.pushChunkInner(stream, chunkString!);
            }
            return stream.bufHasSpace(stream);
        }

        private calculateReadSize(size?: int): int {
            if (size !== undefined && (size! <= 0 || (this.readableLengthInner === 0 && this.readableEndedInner))) {
                return 0;
            }

            if (this.readableObjectModeInner) {
                return 1;
            }

            if (size === undefined) {
                if(this.readableFlowingInner && this.readableLengthInner) {
                    const entry = this.buf[this.bufIndex];
                    if (entry instanceof buffer.Buffer) {
                        return (entry as buffer.Buffer).length.toInt();
                    } else if (entry instanceof string) {
                        return (entry as string).length.toInt();
                    } else {
                        return 0;
                    }
                }
                return this.readableLengthInner;
            }

            if (size !== undefined && size! <= this.readableLengthInner) {
                return size!;
            }
            return this.readableEndedInner ? this.readableLengthInner : 0;
        }

        private readDataFromBuf(size: int): buffer.Buffer | string | null {
            if (this.readableLengthInner === 0) {
                return null;
            }

            let num: int = this.bufIndex;
            let res: buffer.Buffer | string | null = null;

            const buf = this.buf;
            const len: int = buf.length

            if (this.readableObjectModeInner) {
                res = buf[num];
                buf[num++] = null;
            } else if (!size || size >= this.readableLengthInner) {
                if (this.readableDecoder) {
                    res = '';
                    while (num < len) {
                        res = (res as string) + buf[num];
                        buf[num++] = null;
                    }
                } else if (len - num === 0) {
                    res = buffer.alloc(0);
                } else if (len - num === 1) {
                    res = buf[num];
                    buf[num++] = null;
                } else {
                    res = buffer.allocUninitializedFromPool(this.readableLengthInner.toInt()) as buffer.Buffer
                    let index: int = 0;
                    while (num < len) {
                        (buf[num] as buffer.Buffer).copy(res, index, 0 ,(buf[num] as buffer.Buffer).length);
                        index += (buf[num] as buffer.Buffer).length.toInt();
                        buf[num++] = null;
                    }
                }
            } else if (size < (buf[num] as buffer.Buffer).length) {
                res = (buf[num] as buffer.Buffer).subarray(0, size.toInt());
                buf[num] = (buf[num] as buffer.Buffer).subarray(size.toInt());
            } else if (size === (buf[num] as buffer.Buffer).length) {
                res = buf[num];
                buf[num++] = null;
            } else if (this.readableDecoder) {
                res = '';
                while (num < len) {
                    const str: buffer.Buffer | string | null = buf[num];
                    if (size > (str as string).length.toInt()) {
                        res = (res as string) + (str as string);
                        size -= (str as string).length.toInt();
                        buf[num++] = null;
                    } else {
                        if (size === (buf[num] as string).length.toInt()) {
                            res = (res as string) + (str as string);
                            buf[num++] = null;
                        } else {
                            res = (res as string) + (str as buffer.Buffer).subarray(0, size.toInt());
                            buf[num] = (str as buffer.Buffer).subarray(size.toInt());
                        }
                        break;
                    }
                }
            } else {
                res = buffer.allocUninitializedFromPool(size.toInt()) as buffer.Buffer

                const nRetLen = size.toInt();
                while (num < len) {
                    const data = buf[num];
                    const dataLength = (data as buffer.Buffer).length
                    if (size > dataLength) {
                        (data as buffer.Buffer).copy(res, nRetLen - size.toInt(), 0, dataLength);
                        size -= dataLength
                        buf[num++] = null;
                    } else {
                        (data as buffer.Buffer).copy(res, nRetLen - size.toInt(), 0, size.toInt());
                        if (size === dataLength) {
                            buf[num++] = null;
                        } else {
                            let remainBuf = buffer.allocUninitializedFromPool(dataLength - size.toInt());
                            (data as buffer.Buffer).copy(remainBuf, nRetLen - size.toInt(), 0, size.toInt());
                            buf[num] = remainBuf;
                        }
                        break;
                    }
                }
            }

            if (num === len) {
                this.buf.length = 0;
                this.bufIndex = 0;
            } else if (num > 1024) {
                this.buf.splice(0, num);
                this.bufIndex = 0;
            } else {
                this.bufIndex = num;
            }

            return res;
        }

        setEndType(): void {
            Promise.resolve().then((): void => {
                this.readableInner = false;
                this.readableEndedInner = true;
                this.listener?.emit(ReadableEvent.END);
            });
        }

        private throwError(error: Error): void {
            if (this.listener !== undefined && this.listener!.listenerCount(WritableEvent.ERROR) > 0) {
                setTimeout((): void => {
                    this.listener!.emit(WritableEvent.ERROR, error);
                });
            } else {
                throw error;
            }
        }

        /**
         * Reads a buffer of a specified size from the buffer. If the available buffer is sufficient, the result
         * of the specified size is returned. Otherwise, if Readable has ended, all remaining buffers are returned.
         *
         * @param { int } size - Expected length of the data to be read.
         * @returns { Any } The return value is of the buffer and string types.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200038 - The doRead method has not been implemented.
         */
        read(size?: int): buffer.Buffer | string | null {
            const originSize = size;

            if (size !== undefined && size! > this.readableHighWatermarkInner) {
                this.readableHighWatermarkInner = this.computeNewReadableHighWatermark(size!);
            }

            if (size !== 0) {
                this.readableEmittedReadable = false;
            }

            if (size === 0 && this.readableNeedReadable && ((this.readableHighWatermarkInner ?
                this.readableLengthInner >= this.readableHighWatermarkInner :
                this.readableLengthInner > 0) || this.readableEndedInner)) {
                if (this.readableLengthInner === 0 && this.readableEndedInner) {
                    this.emitReadableEnd(this);
                } else {
                    this.emitReadableNow(this);
                }
                return null;
            }

            size = this.calculateReadSize(size);

            if (size === 0 && this.readableEndedInner) {
                if (this.readableLengthInner === 0) {
                    this.emitReadableEnd(this);
                }
                return null;
            }

            let needDoRead: boolean | undefined = this.readableNeedReadable;

            if (this.readableLengthInner === 0 || this.readableLengthInner - size! < this.readableHighWatermarkInner) {
                needDoRead = true;
            }

            if (this.isReading || this.readableEndedInner || this.erroredInner) {
                needDoRead = false;
            } else if (needDoRead) {
                this.isReading = true;
                this.readableSync = true;
                if (this.readableLengthInner === 0) {
                    this.readableNeedReadable = true;
                }

                try {
                    this.doRead(this.readableHighWatermarkInner);
                } catch (error) {
                    processErrOrClose(this, error as Error, false);
                }
                this.readableSync = false;

                if (!this.isReading) {
                    size = this.calculateReadSize(originSize);
                }
            }

            let res : buffer.Buffer | string | null = null;
            if (size! > 0) {
                res = this.readDataFromBuf(size!);
            }

            if (res === null) {
                this.readableNeedReadable = this.readableLengthInner <= this.readableHighWatermarkInner ? true : false;
                size = 0;
            } else {
                this.readableLengthInner -= size!;
                this.awaitDrainWriters.clear();
            }

            if (this.readableLengthInner === 0) {
                if (!this.readableEndedInner) {
                    this.readableNeedReadable = true;
                }

                if (originSize !== size && this.readableEndedInner) {
                    this.emitReadableEnd(this);
                }
            }

            if (res !== null && !this.erroredInner && !this.closedInner) {
                this.listener?.emit(ReadableEvent.DATA, res);
            }

            return res;
        }

        /**
         * Switch Readable to Streaming Mode.
         *
         * @returns { Readable } Return this object.
         */
        resume(): Readable {
            if (!this.readableFlowingInner) {
                this.readableHasFlowingInner = true;
                this.readableFlowingInner = this.readableListenning ? false : true;
                if (!this.readableEmittedResume) {
                    this.readableEmittedResume = true;
                    Promise.resolve().then((): void => {
                        this.resumeInner(this);
                    });
                }
            }
            this.readableHasPaused = true;
            this.pauseInner = false;
            return this;
        }

        /**
         * Toggle Readable to Suspend Mode.
         *
         * @returns { Readable } Return this object.
         */
        pause(): Readable {
            if (!this.readableHasFlowingInner || (this.readableHasFlowingInner && this.readableFlowingInner)) {
                this.readableHasFlowingInner = true;
                this.readableFlowingInner = false;
                this.listener?.emit(ReadableEvent.PAUSE);
            }
            this.pauseInner = true;
            this.readableHasPaused = true;
            return this;
        }

        /**
         * Sets the encoding format of the input binary data.Default: utf8.
         *
         * @param { string } [encoding] - Original Data Encoding Type.
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        setEncoding(encoding?: string): boolean {
            let res: boolean = true;
            if (!encoding) {
                encoding = DEFAULT_ENCODING;
                res = false;
            }
            if (encoding.toLowerCase() === 'utf8') {
                encoding = DEFAULT_ENCODING;
            }

            let encodingLowCase: string = encoding.toLowerCase();
            if (ENCODING_SET.indexOf(encodingLowCase) !== -1) {
                try {
                    this.stringDecoder = new util.StringDecoder(encoding);
                    this.readableEncodingInner = encodingLowCase;

                    // Iterate over current buffer to convert already stored Buffers:
                    let currentBuffer: string = '';
                    for (let i: int = 0; i < this.buf.length; i++) {
                        const decoder = this.stringDecoder as util.StringDecoder;
                        if (this.buf[i] instanceof string) {
                            currentBuffer += decoder.write(this.buf[i] as string);
                        } else {
                            currentBuffer += decoder.write(new Uint8Array((this.buf[i] as buffer.Buffer).buffer));
                        }
                    }
                    this.buf.length = 0;
                    this.bufIndex = 0;

                    if (currentBuffer !== '') {
                        this.buf.push(currentBuffer);
                    }
                    this.readableLengthInner = currentBuffer.length.toInt();
                } catch (e) {
                    this.throwError(e as Error);
                }
            } else {
                this.throwError(createBusinessError(401,
                    `Parameter error. The type of ${encodingLowCase} must be string.`));
                res = false;
            }
            return res;
        }

        /**
         * Query whether it is in pause state.
         *
         * @returns { boolean } Pause state returns true, otherwise returns false.
         */
        isPaused(): boolean {
            return this.pauseInner || (this.readableHasFlowingInner && !this.readableFlowingInner);
        }

        /**
         * Concatenated a Writable to a Readable and switches the Readable to stream mode.
         *
         * @param { Writable } destination - Output writable stream.
         * @param { Object } [options] - Pipeline Options.
         * @returns { Writable } Returns the Writable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        pipe(destination: Writable, options?: Object): Writable {
            const src: Readable = this;

            if (src.pipeWritableArrayInner.length === 1 && !src.multiAwaitDrain) {
                src.multiAwaitDrain = true;
            }

            src.pipeWritableArrayInner.push(destination);

            // Writeable streams are automatically turned off when readable streams are turned off
            const endCallback = (): void => {
                destination.end();
            };
            if (src.readableEndEmitted) {
                Promise.resolve().then((): void => {
                    endCallback();
                });
            } else {
                src.listener?.on(ReadableEvent.END, _onceWrap(src.listener!, ReadableEvent.END, endCallback));
            }

            let ondrainCallback: undefined | Function = undefined;
            let cleand: boolean = false;

            const cleanup = (): void => {
                destination.listener?.clear(WritableEvent.CLOSE);
                destination.listener?.clear(WritableEvent.FINISH);
                destination.listener?.clear(WritableEvent.ERROR);
                destination.listener?.clear(WritableEvent.DRAIN);
                destination.listener?.clear(WritableEvent.UNPIPE);
                src.listener?.clear(ReadableEvent.END);
                src.listener?.clear(ReadableEvent.DATA);
                cleand = true;
                if(ondrainCallback && src.awaitDrainWriters.size > 0 && destination.writableNeedDrain) {
                    ondrainCallback!.unsafeCall();
                }
            }

            const unpipeCallback = (): void => {
                cleanup();
            };
            destination.on(WritableEvent.UNPIPE, unpipeCallback)

            const pause = (): void => {
                if (!cleand) {
                    if (src.pipeWritableArrayInner.length === 1 && src.pipeWritableArrayInner[0] === destination) {
                        src.awaitDrainWriters.add(destination);
                        src.multiAwaitDrain = false;
                    } else if (src.pipeWritableArrayInner.length > 1) {
                        const objIdx: int =
                            src.pipeWritableArrayInner.findIndex((value: Writable) => value === destination).toInt();
                        if (objIdx !== -1) {
                            src.awaitDrainWriters.add(destination);
                        }
                    }
                    src.pause();
                }
                if (!ondrainCallback) {
                    const ondrainFunc: Function = (): void => {
                        src.awaitDrainWriters.delete(destination);
                        if (src.awaitDrainWriters.size === 0 && src.dataListenning) {
                            src.resume();
                        }
                    };
                    ondrainCallback = ondrainFunc;
                    destination.on(WritableEvent.DRAIN, ondrainCallback!);
                }
            };

            const dataCallback = (data: Any): void => {
                const writeData = new Uint8Array((data as buffer.Buffer).buffer);
                const res = destination.write(writeData);
                if (res === false) {
                    pause();
                }
            };

            const errorCallBack = (): void => {
                destination.listener?.clear(WritableEvent.ERROR);
                src.unpipe(destination);
            };

            const closeCallBack = (): void => {
                destination.listener?.clear(WritableEvent.FINISH);
                src.unpipe(destination);
            }

            const finishCallback = (): void => {
                destination.listener?.clear(WritableEvent.CLOSE);
                src.unpipe(destination);
            }

            src.on(ReadableEvent.DATA, dataCallback);

            advanceListener(destination, WritableEvent.ERROR, errorCallBack);
            destination.listener?.on(WritableEvent.CLOSE, _onceWrap(destination.listener!,
                WritableEvent.CLOSE, closeCallBack));
            destination.listener?.on(WritableEvent.FINISH, _onceWrap(destination.listener!,
                WritableEvent.FINISH, finishCallback));
            destination.listener?.emit(WritableEvent.PIPE, src);

            if (destination.writableNeedDrain) {
                pause();
            } else if (!src.readableFlowingInner) {
                src.resume();
            }
            return destination;
        }

        /**
         * Disconnect Writable from Readable.
         *
         * @param { Writable } [destination] - Writable Streams Needing to Be Disconnected.
         * @returns { Readable } Returns the Readable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        unpipe(destination?: Writable): Readable {
            if (this.pipeWritableArrayInner.length === 0) {
                return this;
            }

            if (!destination) {
                // remove all pipelines
                const destinations = this.pipeWritableArrayInner;
                this.pipeWritableArrayInner = [];
                this.pause();

                for (let i: int = 0; i < destinations.length.toInt(); i++) {
                    destinations[i].listener?.emit(WritableEvent.UNPIPE, this);
                }
                return this;
            }

            const objIdx: int =
                this.pipeWritableArrayInner.findIndex((value: Writable) => value === destination).toInt();

            if(objIdx === -1) {
                return this;
            }

            this.pipeWritableArrayInner.splice(objIdx, 1);
            if (this.pipeWritableArrayInner.length === 0) {
                this.pause();
            }
            destination.listener?.emit(WritableEvent.UNPIPE, this);

            return this;
        }

        /**
         * Registering Event Messages.
         *
         * @param { string } event - Registering Events.
         * @param { Function } callback - Event callback.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        on(event: string, callback: Function): void {
            if (!this.isInitialized) {
                this.isInitialized = true;
                this.doInitialize((...data: Object[]): void => {});
            }

            if (!this.callbacks.has(event)) {
                const callbackList = new Array<Function>();
                this.callbacks.set(event, callbackList);
            }

            this.callbacks.get(event)!.push(callback);
            this.listener?.on(event, callback);

            if (event === ReadableEvent.DATA.toString()) {
                this.dataListenning = true;
                if (this.listener && this.listener!.listenerCount(ReadableEvent.READABLE) > 0) {
                    this.readableListenning = true;
                } else {
                    this.readableListenning = false;
                }
                if (!this.readableHasFlowingInner || (this.readableHasFlowingInner && this.readableFlowingInner)) {
                    this.resume();
                }
            } else if (event === ReadableEvent.READABLE.toString()) {
                if (!this.readableEndEmitted || !this.readableListenning) {
                    this.readableHasFlowingInner = true;
                    this.readableListenning = true;
                    this.readableNeedReadable = true;
                    this.readableFlowingInner = false;
                    this.readableEmittedReadable =  false;
                    if (this.readableLengthInner) {
                        this.emitReadableNow(this);
                    } else if (!this.isReading) {
                        Promise.resolve().then((): void => {
                            this.emitReadableNextCycle(this)
                        });
                    }
                }
            }
        }

        /**
         * Cancel event message.
         *
         * @param { string } event - Registering Events.
         * @param { Function } callback - Event callback.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        off(event: string, callback?: Function): void {
            if (callback !== undefined) {
                if (this.callbacks.has(event)) {
                    const callbackList = this.callbacks.get(event);
                    callbackList!.forEach((callbackFunc: Function) => {
                        if (callbackFunc === callback) {
                            this.listener?.off(event, callbackFunc);
                        }
                    })
                }
            } else {
                if (this.callbacks.has(event)) {
                    const calbackList = this.callbacks.get(event);
                    calbackList!.forEach((callbackFunc: Function) => {
                        this.listener?.off(event, callbackFunc);
                    })
                }
            }

            if (event === ReadableEvent.READABLE.toString()) {
                Promise.resolve().then((): void => {
                    if (this.listener!.listenerCount(ReadableEvent.READABLE) > 0) {
                        this.readableListenning = true;
                    } else {
                        this.readableListenning = false;
                    }

                    if (!this.pauseInner && (this.readableHasPaused || this.readableEmittedResume)) {
                        this.readableFlowingInner = true;
                        this.readableHasFlowingInner = true;
                    } else if (this.dataListenning) {
                        this.resume();
                    } else if (!this.readableListenning) {
                        this.readableFlowingInner = false;
                        this.readableHasFlowingInner = false;
                    }
                });
            } else if (event === ReadableEvent.DATA.toString() &&
                this.listener!.listenerCount(ReadableEvent.DATA) === 0) {
                this.dataListenning = false;
            }
        }

        /**
         * It may be implemented by child classes, and if so, will be called by the Readable class methods only.
         * It must not be called directly.
         *
         * @param { Function } callback - Callback when the stream has completed the initial.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doInitialize(callback: Function): void {}

        /**
         * The specific implementation of data production. It must not be actively called.
         * After data production, Readable.push should be called to push the produced data into the buffer.
         * If push is not called, doRead will not be called again.
         *
         * @param { int } size - Expected length of the data to be read.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doRead(size: int): void {
            throw ERR_DOREAD_NOT_IMPLEMENTED;
        }

        /**
         * Adds the generated data to the buffer. The return value indicates whether the data in the buffer has not
         * reached the highWaterMark (similar to Writable.write). If the chunk is null, all data has been generated.
         *
         * @param { Uint8Array | string | undefined | null } chunk - Binary data to be stored in the buffer.
         * @param { string } [encoding] - Binary data encoding type.
         * @returns { boolean } If true is returned, the data in the buffer reaches the highWaterMark. Otherwise, the
         * data in the buffer does not reach the highWaterMark.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types.
         */
        push(chunk: Uint8Array | string | undefined | null, encoding?: string): boolean {
            return this.pushByteModeChunk(this, chunk, encoding);
        }

        /**
         * Returns boolean indicating whether it is in ObjectMode.
         *
         */
        get readableObjectMode(): boolean {
            return this.readableObjectModeInner;
        }

        /**
         * Is true if it is safe to call readable.read(), which means
         * the stream has not been destroyed or emitted 'error' or 'end'.
         *
         */
        get readable(): boolean {
            if (this.readableInner) {
                return this.readableInner && !this.errorEmitted && !this.readableEndEmitted;
            } else if (!this.readableInner && this.readableEndedInner) {
                return false;
            }
            return true;
        }

        /**
         * Returns the value of highWatermark passed when creating this Readable.
         *
         */
        get readableHighWatermark(): int {
            return this.readableHighWatermarkInner;
        }

        /**
         * This property reflects the current state of the readable stream true/false.
         *
         */
        get readableFlowing(): boolean | null {
            return this.readableFlowingInner;
        }

        /**
         * Size of the data that can be read, in bytes or objects.
         *
         */
        get readableLength(): int {
            return this.readableLengthInner;
        }

        /**
         * Getter for the property encoding of a given Readable stream. The encoding property can be set using the
         * readable.setEncoding() method.
         *
         */
        get readableEncoding(): string | null {
            return this.readableEncodingInner;
        }

        /**
         * Whether all data has been generated.
         *
         */
        get readableEnded(): boolean {
            return this.readableEndEmitted;
        }

        /**
         * Returns error if the stream has been destroyed with an error.
         *
         */
        get errored(): boolean {
            return this.erroredInner;
        }

        set errored(value: boolean) {
            this.erroredInner = value;
        }

        /**
         * Readable completes destroyfile and returns true, otherwise returns false.
         *
         */
        get closed(): boolean {
            return this.closedInner === undefined ? false : this.closedInner!;
        }

        set closed(value: boolean) {
            this.closedInner = value;
        }

        get stringDecoder(): Any {
            return this.readableDecoder ? this.stringDecoderInner : null;
        }

        set stringDecoder(value: util.StringDecoder | null) {
            if (value) {
                this.stringDecoderInner = value;
                this.readableDecoder = true;
            } else {
                this.readableDecoder = false;
            }
        }
    }

    /**
     * Duplex streams are streams that inheritance for readable streams and consist of Writable streams.
     *
     * @extends Readable
     * @syscap SystemCapability.Utils.Lang
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    export class Duplex extends Readable {
        _writable: Writable;

        /**
         * The Duplex constructor.
         *
         */
        constructor() {
            super();
            this._writable = new Writable();
            this._writable.doWriteFunc = this.doWrite;
            this._writable.doWritevFunc = this.doWritev;
        }

        /**
         * writes a chunk to Writable and invokes callback when the chunk is flushed. The return value indicates
         * whether the internal buffer of the Writable reaches the hightWaterMark. If true is returned, the buffer
         * does not reach the hightWaterMark. If false is returned, the buffer has been reached. The write function
         * should be called after the drain event is triggered. If the write function is called continuously,
         * the chunk is still added to the buffer until the memory overflows
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { boolean } Write success returns true, write failure returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200036 - The stream has been ended.
         * @throws { BusinessError } 10200037 - The callback is invoked multiple times consecutively.
         * @throws { BusinessError } 10200039 - The doTransform method has not been implemented for
         * a class that inherits from Transform.
         */
        write(chunk?: string | Uint8Array, encoding?: string, callback?: Function): boolean {
            return this._writable.write(chunk, encoding, callback);
        }

        /**
         * Write the last chunk to Writable.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @returns { Writable } Returns the Writable object.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         * @throws { BusinessError } 10200039 - The doTransform method has not been implemented for
         * a class that inherits from Transform.
         */
        end(chunk?: string | Uint8Array, encoding?: string, callback?: Function): Writable {
            super.setEndType();
            return this._writable.end(chunk, encoding, callback);
        }

        on(event: string, callback: Function): void {
            super.on(event, callback);
            this._writable.on(event, callback);
        }

        off(event: string, callback?: Function): void {
            super.off(event);
            this._writable.off(event, callback);
        }

        getListener(): EventEmitter | undefined {
            return this._writable.listener;
        }

        /**
         * Set the default encoding mode.
         *
         * @param { string } [encoding] - Encoding type.Default: utf8.
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        setDefaultEncoding(encoding?: string): boolean {
            return this._writable.setDefaultEncoding(encoding);
        }

        /**
         * After the call, all Write operations will be forced to write to the buffer instead of being flushed.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        cork(): boolean {
            return this._writable.cork();
        }

        /**
         * After calling, flush all buffers.
         *
         * @returns { boolean } Setting successful returns true, setting failed returns false.
         */
        uncork(): boolean {
            return this._writable.uncork();
        }

        doInitialize(callback: Function): void {
            super.doInitialize(callback);
            this._writable.doInitialize(callback);
        }

        /**
         * Implemented by subclass inheritance. The implementation logic of flushing chunks in the buffer must not be
         * directly called. The call is controlled by Writable.write.
         *
         * @param { string | Uint8Array } [chunk] - Data to be written.
         * @param { string } [encoding] - Encoding type.
         * @param { Function } [callback] - Callback after writing.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            this._writable.noWriteOpes(chunk, encoding, callback);
        }

        /**
         * The implementation logic of flushing chunks in the buffer in batches should not be actively called.
         * The call is controlled by Writable.write.
         *
         * @param { string[] | Uint8Array[] } [chunks] - Data to be written.
         * @param { Function } [callback] - Callback after writing.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doWritev(chunks: string[] | Uint8Array[], callback: Function): void {}

        get writableObjectMode(): boolean {
            if (this._writable.writableObjectMode !== undefined) {
                return this._writable.writableObjectMode as boolean;
            } else {
                return false;
            }
        }

        get writableHighWatermark(): int {
            if (this._writable.writableHighWatermark !== undefined) {
                return this._writable.writableHighWatermark as int;
            } else {
                return 0;
            }
        }

        get writable(): boolean {
            if (this._writable.writable !== undefined) {
                return this._writable.writable as boolean;
            } else {
                return false;
            }
        }

        get writableLength(): int {
            if (this._writable.writableLength !== undefined) {
                return this._writable.writableLength as int;
            } else {
                return 0;
            }
        }

        get writableNeedDrain(): boolean {
            if (this._writable.writableNeedDrain !== undefined) {
                return this._writable.writableNeedDrain as boolean;
            } else {
                return false;
            }
        }

        get writableCorked(): int {
            if (this._writable.writableCorked !== undefined) {
                return this._writable.writableCorked as int;
            } else {
                return 0;
            }
        }

        get writableEnded(): boolean {
            if (this._writable.writableEnded !== undefined) {
                return this._writable.writableEnded as boolean;
            } else {
                return false;
            }
        }

        get writableFinished(): boolean {
            if (this._writable.writableFinished !== undefined) {
                return this._writable.writableFinished as boolean;
            } else {
                return false;
            }
        }
    }

    /**
     * Transform stream is a Duplex stream where the output is computed in some way from the input.
     * Transform implementations must implement the doTransform() method and may also implement the doFlush() method.
     *
     * @extends Duplex
     */
    export class Transform extends Duplex {
        /**
         * The Transform constructor.
         *
         */
        constructor() {
            super();
            this._writable.doWriteFunc = this.doWrite;
            this._writable.doWritevFunc = this.doWritev;
        }

        on(event: string, callback: Function): void {
            super.on(event, callback);
        }

        end(chunk?: string | Uint8Array, encoding?: string, callback?: Function): Writable {
            if (!this.doTransform) {
                throw ERR_DOTRANSFORM_NOT_IMPLEMENTED;
            }
            if (chunk instanceof Uint8Array) {
                const chunkString = (this.stringDecoder as util.StringDecoder).write(chunk);
                let encodingVal: string = encoding ? encoding : 'utf8'
                let callbackVal: Function = callback ? callback : (...param: Object[]) => {}
                this.doTransform(chunkString, encodingVal, callbackVal);
            } else if (chunk instanceof string) {
                let encodingVal: string = encoding ? encoding : 'utf8'
                let callbackVal: Function = callback ? callback : (...param: Object[]) => {}
                this.doTransform(chunk as string, encodingVal, callbackVal);
            }
            this.doFlush((...param: Object[]): void => {
                if (param.length > 0) {
                    for (let idx: int = 0; idx < param.length.toInt(); idx++) {
                        const val = param[idx];
                        if (val instanceof Uint8Array) {
                            this.push(val as Uint8Array, encoding);
                        } else if (val instanceof string) {
                            this.push(val as string, encoding);
                        } else {
                            this.push('', encoding);
                        }
                    }
                }
            });
            const write: Writable = super.end(chunk, encoding, callback);
            return write;
        }

        push(chunk: Uint8Array | string | undefined | null, encoding?: string): boolean {
            return super.push(chunk, encoding);
        }

        /**
         * Convert the input data. After the conversion,
         * Transform.push can be called to send the input to the read stream.
         * Transform.push should not be called Transform.write to call.
         *
         * @param { string } chunk - Input data to be converted.
         * @param { string } encoding - If the chunk is a string, then this is the encoding type. If chunk is a buffer,
         * then this is the special value 'buffer'. Ignore it in that case.
         * @param { Function } callback - Callback after conversion.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doTransform(chunk: string, encoding: string, callback: Function): void {
            throw ERR_DOTRANSFORM_NOT_IMPLEMENTED;
        }

        /**
         * After all data is flushed to the write stream,
         * you can use the Transform.doFlush writes some extra data, must
         * not be called directly, only called by Writable after flushing all data.
         *
         * @param { Function } callback - Callback after flush completion.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *     1.Mandatory parameters are left unspecified;
         *     2.Incorrect parameter types;
         *     3.Parameter verification failed.
         */
        doFlush(callback: Function): void {}

        write(chunk?: string | Uint8Array, encoding?: string, callback?: Function): boolean {
            if (chunk instanceof string) {
                let executed: boolean = false;
                const callBackFunction = (...param: Object[]): void => {
                    if (!executed) {
                        executed = true;
                        if (param.length > 0 && param[0] instanceof Error) {
                            this.getListener()?.emit(WritableEvent.ERROR, param[0] as Error);
                        }
                    } else {
                        this.getListener()?.emit(WritableEvent.ERROR, ERR_MULTIPLE_CALLBACK);
                    }
                };
                let chunkVal: string = chunk ? chunk as string : ''
                let encodingVal: string = encoding ? encoding : 'utf8'
                this.doTransform(chunkVal, encodingVal, callBackFunction);
            }
            return super.write(chunk, encoding, callback);
        }

        doRead(size: int): void {}

        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            super.doWrite(chunk, encoding, callback);
        }
    }
}
