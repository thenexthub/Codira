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

package std.core;

export namespace StdProcess {
    const SIGNAL_MAP: Map<string, int> = {
        "SIGHUP": 1,
        "SIGINT": 2,
        "SIGQUIT": 3,
        "SIGILL": 4,
        "SIGTRAP": 5,
        "SIGABRT": 6,
        "SIGBUS": 7,
        "SIGFPE": 8,
        "SIGKILL": 9,
        "SIGUSR1": 10,
        "SIGSEGV": 11,
        "SIGUSR2": 12,
        "SIGPIPE": 13,
        "SIGALRM": 14,
        "SIGTERM": 15,
        "SIGSTKFLT": 16,
        "SIGCHLD": 17,
        "SIGCONT": 18,
        "SIGSTOP": 19,
        "SIGTSTP": 20,
        "SIGTTIN": 21,
        "SIGTTOU": 22,
        "SIGURG": 23,
        "SIGXCPU": 24,
        "SIGXFSZ": 25,
        "SIGVTALRM": 26,
        "SIGPROF": 27,
        "SIGWINCH": 28,
        "SIGIO": 29,
        "SIGPWR": 30,
        "SIGSYS": 31,
    };

    const MAX_BUFFER_SIZE: int = 1024 * 1024;

    const DEFAULT_JOB_HANDLER_CALLBACK = (reason: Error, job: Object) => { console.error("Unhandled Job fail:", reason) };

    const DEFAULT_ERROR_HANDLER_CALLBACK = (error: Object): void => {
        console.error('Unhandled exception:', Class.of(error).getName());
        let out = new String();
        let toStr = new String();
        let stack = new String();
        if (error instanceof Error) {
            error = error as Error;
            try {
                toStr = error.toString() as string;
            } catch (e) {
                toStr = 'invoke toString failed';
            }
            out += toStr;
            try {
                stack = error.stack as string;
            } catch (e) {
                stack = 'exception dump failed';
            }
            out += '\n' + stack;
        } else {
            throw new Error('unknown error type');
        }
        console.error(out);
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    };

    /**
     * @class Represents child process created by executing process.runCmd()
     */
    export class ChildProcess {
        protected constructor(bufferSize: int) {
            this.outBuffer = new ArrayBuffer(bufferSize);
            this.errBuffer = new ArrayBuffer(bufferSize);
            this.pid = -1;
            this.ppid = -1;
            this.exitCode = -1;
            this.killed = false;
            this.outBytesRead = 0;
            this.errBytesRead = 0;
            this.outFd = -1;
            this.errorFd = -1;
        };

        /**
         * Wait for child process to terminate
         *
         * @returns { Promise<int> } Promise containing child process's exit code value
         */
        wait(): Promise<int> {
            this.waitImpl();
            return Promise.resolve <int> (this.exitCode);
        }

        /**
         * Get child process's output to stdout
         *
         * @returns { Promise<Uint8Array> } Promise containing bytes read from child process's stdout in Uint8Array
         */
        getOutput(): Promise<Uint8Array> {
            this.readOutput();
            return Promise.resolve < Uint8Array > (new Uint8Array(this.outBuffer, 0, this.outBytesRead));
        }

        /**
         * Get child process's output to stderr
         *
         * @returns { Promise<Uint8Array> } Promise containing bytes read from child process's stderr in Uint8Array
         */
        getErrorOutput(): Promise<Uint8Array> {
            this.readErrorOutput();
            return Promise.resolve < Uint8Array > (new Uint8Array(this.errBuffer, 0, this.errBytesRead));
        }

        /**
         * Close the child process
         */
        native close(): void;

        /**
         * Send a signal to the child process
         *
         * @param { int | string } signal int or string indicating a kill signal
         *
         */
        kill(signal: int | string): void {
            let killSignal: int;
            if (signal instanceof string) {
                if (SIGNAL_MAP.has(signal as string)) {
                    killSignal = SIGNAL_MAP.get(signal as string)! as int;
                } else {
                    killSignal = SIGNAL_MAP.get("SIGTERM")! as int;
                }
            } else {
                if ((signal as int) < 1 || (signal as int) > 31) {
                    throw new RangeError("Invalid signal value");
                }
                killSignal = signal;
            }
            this.killImpl(killSignal);
        }

        protected native spawn(command: string, timeout: int, killSignal: int): void;

        private native killImpl(signal: int): void;
        private native waitImpl(): number;
        private native readOutput(): void;
        private native readErrorOutput(): void;

        /**
         * Buffer for reading child process's stdout
         */
        private outBuffer: ArrayBuffer;

        /**
         * Buffer for reading child process's stderr
         */
        private errBuffer: ArrayBuffer;

        /**
         * Child process's pid as double value
         */
        readonly pid: int;

        /**
         * Child process's ppid as double value
         */
        readonly ppid: int;

        /**
         * Child process's exit code as double value
         */
        readonly exitCode: int;

        private outBytesRead: int;

        private errBytesRead: int;

        private outFd: int;

        private errorFd: int;

        /**
         * Indicates if the child process has been killed
         */
        readonly killed: boolean;
    }


    /**
     * @class Wrapper for parameters used by process.RunCmd function to create a ChildProcess
     */
    export final class ConditionType {
        /**
        * Time after which a kill signal will be send to child process
        */
        timeout?: int;

        /**
        * Kill signal that is sent to child process after timeout
        */
        killSignal?: int | string;

        /**
        * Maximum buffer size for both sdout and stderr of child process
        */
        maxBuffer?: int;
    }

    /**
     * @class Interface that allows to interact with processes in the system, retrieve process-specific and system-specific information
     */
    export final class ProcessManager {
        /**
         * Determine if a given uid belongs to an app
         *
         * @param { int } uid
         *
         * @returns { boolean } resulting boolean
         */
        native isAppUid(uid: int): boolean;

        /**
         * Determine the uid corresponding to a user name string
         *
         * @param { string } v user name string
         *
         * @returns { int } user id as int
         */
        native getUidForName(v: string): int;

        /**
         * Determine the priority for a given pid or tid
         *
         * @param { int } v pid or tid
         *
         * @returns { int } priority value as int
         */
        native getThreadPriority(v: int): int;

        /**
        * Get configuration information
        *
        * @param { int } name constant corresponding to a configuration parameter
        *
        * @returns { long } value of the configuration parameter
        */
        native getSystemConfig(name: int): long;

        /**
         * Get value of an environment variable
         *
         * @param { string } name environment variable
         *
         * @returns { string } value of the environment variable
         */
        native getEnvironmentVar(name: string): string;

        /**
         * Exit the program
         *
         * @param { int } code exit code
         *
         */
        native exit(code: int): void;

        /**
         * Send a kill singnal to a process with a given pid
         *
         * @param { int } signal signal to send
         *
         * @param { int } pid pid of receiving process
         *
         * @returns { boolean } value indicating if the signal was sent succesfully
         *
         */
        native kill(signal: int, pid: int): boolean;
    }

    export type RejectedObjectListener = (reason: Error, obj: Object) => void;
    export type UncaughtErrorListener = (error: Object) => void;

    export type EventListener = RejectedObjectListener | UncaughtErrorListener;

    /**
    * Get effective group id of the current process
    *
    * @returns { int } egid as int
    */
    export native function egid(): int;

    /**
    * Get effective user id of the current process
    *
    * @returns { int } euid as int
    */
    export native function euid(): int;

    /**
    * Get group id of the current process
    *
    * @returns { int } gid as int
    */
    export native function gid(): int;

    /**
    * Get user id of the current process
    *
    * @returns { int } uid as int
    */
    export native function uid(): int;

    /**
    * Get groups to which the current process belongs
    *
    * @returns { int[] } groups as int array
    */
    export native function groups(): int[];

    /**
    * Get process id of the current process
    *
    * @returns { int } pid as int
    */
    export native function pid(): int;

    /**
    * Get parent process id of the current process
    *
    * @returns { int } ppid as int
    */
    export native function ppid(): int;

    /**
    * Get thread id of the current process
    *
    * @returns { int } tid as int
    */
    export native function tid(): int;

    /**
    * Determine if the current process is isolated
    *
    * @returns { boolean } value indicating if the current process is isolated
    */
    export native function isIsolatedProcess(): boolean;

    /**
    * Determine if the current process is a 64-bit process
    *
    * @returns { boolean } value indicating if the current process is a 64-bit process
    */
    export native function is64Bit(): boolean;

    /**
    * Get start time of the current process
    *
    * @returns { long } current process's start time milliseconds as int
    */
    export native function getStartRealtime(): long;

    /**
    * Get past cpu time of the current process
    *
    * @returns { long } current process's past cpu time in milliseconds as long
    */
    export native function getPastCpuTime(): long;


    /**
    * Create a child process to execute a given command
    *
    * @param { string } command command to execute
    *
    * @param { ConditionType } [options] specifies timeout, kill signal and maximum buffer size for child process created
    *
    * @returns { ChildProcess } instance of ChildProcess
    *
    */
    export function runCmd(command: string, options?: ConditionType): ChildProcess {
        let args: int[] = getCmdOptions(options);
        let childProcess = new ChildProcessHelper(args[2]);
        childProcess.spawn(command, args[0], args[1]);
        return childProcess as ChildProcess;
    }

    /**
    * Abort the program
    */
    export native function abort(): void;


    /**
    * Store user-triggered event
    *
    * @param { string } type type of storage event
    *
    * @param { EventListener } listener callback for event handling
    *
    */
    export function on(type: "unhandledPromiseRejection" | "uncaughtError", listener: EventListener): void {
        if (type == string("unhandledJobRejection")) {
            listener = listener as RejectedObjectListener;
            setFailedJobHandler(listener);
            return;
        }
        if (type == "unhandledPromiseRejection") {
            listener = listener as RejectedObjectListener;
            setRejectedPromiseHandler(listener);
            return;
        }
        if (type == "uncaughtError") {
            listener = listener as UncaughtErrorListener;
            setErrorHandler(listener);
            return;
        }
        throw new Error("not implemented");
    }

    /**
    * Remove user-triggered event
    *
    * @param { string } type type of storage event
    *
    * @returns { boolean } indicating if the event has been removed successfully
    *
    */
    export function off(type: "unhandledPromiseRejection" | "uncaughtError", listener?: RejectedObjectListener): boolean {
        if (type == string("unhandledJobRejection")) {
            if (listener) {
                failedJobHandler.unregister(listener);
            } else {
                failedJobHandler.reset();
            }
            return true;
        }
        if (type == "unhandledPromiseRejection") {
            if (listener) {
                rejectedPromiseHandler.unregister(listener);
            } else {
                rejectedPromiseHandler.reset();
            }
            return true;
        }
        if (type == "uncaughtError") {
            setErrorHandler(DEFAULT_ERROR_HANDLER_CALLBACK);
            return true;
        }
        throw new Error("not implemented");
    }

    /**
    * Get the current working directory of the program
    *
    * @returns { string } current working directory as string
    *
    */
    export native function cwd(): string;

    /**
    * Change the current working directory of the program
    *
    * @param { string } path working directory
    *
    */
    export native function chdir(path: string): void;

    /**
    * Get the system uptime
    *
    * @returns { long } uptime in milliseconds as long
    *
    */
    export native function uptime(): long;

    function getCmdOptions(options?: ConditionType): int[] {
        let timeout: int = 0;
        let killSignal: int = SIGNAL_MAP.get("SIGTERM")! as int;
        let maxBuffer: int = MAX_BUFFER_SIZE;

            if (options == undefined) {
                return [timeout, killSignal, maxBuffer];
            }

        if (options.timeout != undefined && options.timeout as int > 0) {
            timeout = options.timeout as int;
        }

        if (options.killSignal != undefined) {
            if (options.killSignal instanceof string) {
                let key: string = options.killSignal as string;
                if (SIGNAL_MAP.has(key)) {
                    killSignal = SIGNAL_MAP.get(options.killSignal as string)! as int;
                }
            } else {
                if ((options.killSignal as int) < 1 || (options.killSignal as int) > 31) {
                    throw new RangeError("Invalid signal value");
                }
                killSignal = options.killSignal as int;
            }
        }

        if (options.maxBuffer != undefined) {
            if ((options.maxBuffer as int) < 0) {
                throw new RangeError("Invalid buffer size value");
            }
            maxBuffer = min(options.maxBuffer as int, MAX_BUFFER_SIZE);
        }

        let ret: Array<int> = new Array<int>(3);
        ret[0] = timeout
        ret[1] = killSignal
        ret[2] = maxBuffer
        return ret;
    }

    /**
    * This method updates handler that is applied to
    * unhandled failed jobs at the program exit
    *
    * @param { RejectedObjectListener } listener user-provided handler
    *
    */
    function setFailedJobHandler(listener: RejectedObjectListener): void {
        failedJobHandler.register(listener);
    }

    /**
    * This method updates handler that is applied to
    * unhandled rejected promises at the program exit
    *
    * @param { RejectedObjectListener } listener user-provided handler
    *
    */
    function setRejectedPromiseHandler(listener: RejectedObjectListener): void {
        rejectedPromiseHandler.register(listener);
    }

    /**
    * This method updates handler that is applied to uncaught error
    *     right before abnormal program termination
    *
    * @param { EventListener } listener user-provided handler
    */
    function setErrorHandler(listener: UncaughtErrorListener) : void {
        errorHandler.reset(listener);
    }

    /**
    * This method applies default or user-provided handler to
    * all unhandled failed job results
    *
    * @param { Object[][] } objs array of pairs of fail results and failed jobs
    *
    */
    function listUnhandledJobs(objs: Object[][]): void {
        for (let objAndReason of objs) {
            let reason = objAndReason[0] as Error;
            let obj = objAndReason[1];
            failedJobHandler.apply(reason, obj);
        }
    }

    /**
    * This method applies default or user-provided handler to
    * all unhandled rejected promise results
    *
    * @param { Object[][] } objs array of pairs of rejection results and rejected promises
    *
    */
    function listUnhandledPromises(objs: Object[][]): void {
        for (let objAndReason of objs) {
            let reason = objAndReason[0] as Error;
            let obj = objAndReason[1];
            rejectedPromiseHandler.apply(reason, obj);
        }
    }

    /**
     *
     * Thist is noreturn function.
     * Applies default or user-provided handler to an uncaught error and exits the program.
     *
     * @param { Object } error Uncaught error
     *
     */
    function HandleUncaughtError(error: Object) {
        errorHandler.handleUncaughtError(error);
    }

    let failedJobHandler: RejectedHandler = new RejectedHandler(DEFAULT_JOB_HANDLER_CALLBACK);
    let rejectedPromiseHandler: RejectedHandler = new RejectedHandler();
    let errorHandler: ProcessErrorHandler = new ProcessErrorHandler(DEFAULT_ERROR_HANDLER_CALLBACK);
}

final class ChildProcessHelper extends StdProcess.ChildProcess {
    constructor(bufferSize: int) {
        super(bufferSize);
    };

    spawn(command: string, timeout: int, killSignal: int): void {
        super.spawn(command, timeout, killSignal);
    }

}
