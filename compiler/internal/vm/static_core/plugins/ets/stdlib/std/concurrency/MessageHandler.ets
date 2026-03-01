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

package std.concurrency;

import { launch } from "std/concurrency";

type Handler = (message: concurrency.Message) => void;
type Task = () => void;

/**
* @enum defines the message status
*/
enum MessageStatus {
    NOT_EXECUTED = 0,
    EXECUTED = 1,
    CANCELLED = 2,
    IN_EXECUTION = 3
}

export namespace concurrency {
    /**
     * MessageHandler class for handling message processing and callback management
     * Provides functionality for posting tasks, sending messages, and managing callbacks
     */
    export class MessageHandler {
        private handler: Handler;
        private worker: EAWorker;
        private messageMutex: Mutex;
        private messages: Map<Message, MessageStatus>;

        /**
         * Create a MessageHandler instance
         * @param { Handler } handler The message handler function
         * @param { EAWorker } [worker=EAWorker.current()] The worker to handle messages
         */
        constructor(handler: Handler, worker: EAWorker | undefined = EAWorker.current()) {
            if (worker == undefined) {
                worker = EAWorker.current();
                if (worker == undefined) {
                    throw new Error("No worker found for MessageHandler");
                }
            }
            this.handler = handler;
            this.worker = worker as EAWorker;
            this.messageMutex = new Mutex();
            this.messages = new Map<Message, MessageStatus>();
        }

        /**
         * Check if a specific callback exists in the handler
         * @param { Task } callback The callback function to check
         * @returns { boolean } True if the callback exists, false otherwise
         */
        public hasCallbacks(callback: Task): boolean {
            if (this.workerIsMain()) {
                return false;
            }
            return this.hasMessage((value: MessageStatus, key: Message) => {
                return key.getCallback() == callback;
            });
        }

        /**
         * Check if messages with a specific identifier exist
         * @param { int } what The message identifier to check
         * @returns { boolean } True if messages with the identifier exist, false otherwise
         */
        public hasMessages(what: int): boolean {
            if (this.workerIsMain()) {
                return false;
            }
            return this.hasMessage((value: MessageStatus, key: Message) => {
                return key.getWhat() == what && value != MessageStatus.CANCELLED;
            });
        }

        /**
         * Check if messages with a specific identifier and object exist
         * @param { int } what The message identifier to check
         * @param { Object } obj The object to check
         * @returns { boolean } True if messages with the identifier and object exist, false otherwise
         */
        public hasMessages(what: int, obj: Object): boolean {
            if (this.workerIsMain()) {
                return false;
            }
            return this.hasMessage((value: MessageStatus, key: Message) => {
                return key.getWhat() == what && key.getObject() == obj && value != MessageStatus.CANCELLED;
            });
        }

        /**
         * Post a callback task to be executed
         * @param { Task } callback The callback function to post
         * @returns { boolean } True if the callback was successfully posted, false otherwise
         */
        public post(callback: Task): boolean {
            try {
                let msg = new concurrency.Message(callback, this);
                return this.sendMessage(msg);
            } catch (e) {
                return false;
            }
        }

        /**
         * Remove a specific callback from the handler
         * @param { Task } callback The callback function to remove
         * @returns { boolean } True if the callback was successfully removed, false otherwise
         */
        public removeCallbacks(callback: Task): boolean {
            if (this.workerIsMain()) {
                return false;
            }
            return this.removeMessages((value: MessageStatus, key: Message) => {
                return key.getCallback() == callback && value == MessageStatus.NOT_EXECUTED;
            });
        }

        /**
         * Remove all messages with a specific identifier
         * @param { int } what The message identifier to remove
         * @returns { boolean } True if messages were successfully removed, false otherwise
         */
        public removeMessages(what: int): boolean {
            if (this.workerIsMain()) {
                return false;
            }
            return this.removeMessages((value: MessageStatus, key: Message) => {
                return key.getWhat() == what && value == MessageStatus.NOT_EXECUTED && key.getCallback() === undefined;
            });
        }

        /**
         * Remove messages with a specific identifier and object
         * @param { int } what The message identifier to remove
         * @param { Object } obj The object to match
         * @returns { boolean } True if messages were successfully removed, false otherwise
         */
        public removeMessages(what: int, obj: Object): boolean {
            if (this.workerIsMain()) {
                return false;
            }
            return this.removeMessages((value: MessageStatus, key: Message) => {
                return key.getWhat() == what && key.getObject() == obj && value == MessageStatus.NOT_EXECUTED && key.getCallback() === undefined;
            });
        }

        /**
         * Send an empty message with a specific identifier
         * @param { int } what The message identifier
         * @returns { boolean } True if the message was successfully sent, false otherwise
         */
        public sendEmptyMessage(what: int): boolean {
            return this.sendMessage(new concurrency.Message(what, this));
        }

        /**
         * Send a message to the handler
         * @param { Message } message The message to send
         * @returns { boolean } True if the message was successfully sent, false otherwise
         */
        public sendMessage(message: Message): boolean {
            try {
                if (message.getTarget() != this || !this.worker.isAlive()) {
                    return false;
                }
                this.messageMutex.lock();
                this.messages.set(message, MessageStatus.NOT_EXECUTED);
                this.messageMutex.unlock();
                let msgHandler = (msg: Message) => {
                    this.messageMutex.lock();
                    if (this.messages.get(msg) == MessageStatus.CANCELLED) {
                        this.messages.delete(msg);
                        this.messageMutex.unlock();
                        return;
                    }
                    this.messages.set(msg, MessageStatus.IN_EXECUTION);
                    this.messageMutex.unlock();
                    this.handler(msg);
                    this.messageMutex.lock();
                    this.messages.set(msg, MessageStatus.EXECUTED);
                    this.messageMutex.unlock();
                };
                this.worker.run<void>(msgHandler, message);
                return true;
            } catch (e) {
                return false;
            }
        }

        /**
         * Get the target worker associated with this handler
         * @returns { EAWorker } The worker that handles messages for this handler
         */
        public getWorker(): EAWorker {
            return this.worker;
        }

        private workerIsMain(): boolean {
            return this.worker === EAWorker.main();
        }

        private hasMessage(pred: (value: MessageStatus, key: Message) => boolean): boolean {
            let res = false;
            this.messageMutex.lock();
            this.messages.forEach((value: MessageStatus, key: Message) => {
                if (!res && pred(value, key)) {
                    res = true;
                }
            });
            this.messageMutex.unlock();
            return res;
        }

        private removeMessages(pred: (value: MessageStatus, key: Message) => boolean): boolean {
            let res = false;
            this.messageMutex.lock();
            this.messages.forEach((value: MessageStatus, key: Message) => {
                if (pred(value, key)) {
                    this.messages.set(key, MessageStatus.CANCELLED);
                    res = true;
                }
            });
            this.messageMutex.unlock();
            return res;
        }
    }
}
