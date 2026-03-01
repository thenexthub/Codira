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

package std.concurrency;

import { AtomicFlag } from "std/debug/concurrency";

type CB = () => void;

export namespace concurrency {

    /**
     * Message class for inter-thread communication
     * Supports different types of message construction and sending
     */
    export class Message {
        private handler: concurrency.MessageHandler;
        private what: int;
        private callback: CB | undefined = undefined;
        private obj: Object | undefined = undefined;
        private isSent: AtomicFlag = new AtomicFlag(false);
        private isProcessed: AtomicFlag = new AtomicFlag(false);

        /**
         * Create a basic message instance
         * @param { MessageHandler } handler The message handler
         */
        constructor(handler: concurrency.MessageHandler) {
            this.handler = handler;
        }

        /**
         * Create a message instance with message identifier
         * @param { int } what The message identifier
         * @param { MessageHandler } handler The message handler
         */
        constructor(what: int, handler: concurrency.MessageHandler) {
            this.handler = handler;
            this.what = what;
        }

        /**
         * Create a message instance with message identifier and additional object
         * @param { int } what The message identifier
         * @param { MessageHandler } handler The message handler
         * @param { Object } obj Additional object data
         */
        constructor(what: int, obj: Object, handler: concurrency.MessageHandler) {
            this.handler = handler;
            this.what = what;
            this.obj = obj;
        }

        /**
         * Create a message instance with callback function
         * @param { Task } callback The callback function
         * @param { MessageHandler } handler The message handler
         */
        constructor(callback: CB, handler: concurrency.MessageHandler) {
            this.handler = handler;
            this.callback = callback;
        }

        /**
         * Get the message identifier
         * @returns { int } The message identifier
         */
        public getWhat(): int {
            return this.what;
        }

        /**
         * Get the message callback function
         * @returns { Task } The callback function
         */
        public getCallback(): CB | undefined {
            if (this.callback != undefined) {
                return this.callback as CB;
            }
            return undefined;
        }

        /**
         * Get the additional object of the message
         * @returns { Object } Additional object data
         */
        public getObject(): Object | undefined {
            if (this.obj != undefined) {
                return this.obj as Object;
            }
            return undefined;
        }

        /**
         * Get the message handler
         * @returns { MessageHandler } The message handler
         */
        public getTarget(): concurrency.MessageHandler {
            return this.handler;
        }

        /**
         * Send the message to the target
         */
        public sendToTarget(): void {
            if (this.isSent.get()) {
                throw new Error("Message already sent");
            }
            this.isSent.set(true);
            this.handler.sendMessage(this);
        }

        public equals(other: concurrency.Message): boolean {
            if (this.handler != other.getTarget()) {
                return false;
            }
            if (this.callback != undefined) {
                return this.callback == other.getCallback();
            }
            if (this.what != undefined) {
                return this.what == other.getWhat() && this.obj == other.getObject();
            }
            return false;
        }
    }
}
