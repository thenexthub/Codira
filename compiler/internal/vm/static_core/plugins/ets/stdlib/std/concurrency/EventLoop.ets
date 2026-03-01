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

type TimerId = int;
type Callback = () => void;

// NOTE(panferovi): fix signature after #25420
/**
 * @internal
 */
native function launchSameWorker(callback: Object): void;

class EventLoop {
    constructor(workerId: WorkerId) {
        this.workerId = workerId;
        this.initEvent = ConcurrencyHelpers.eventCreate();
        this.timerQueue = new Array<Timer>();
    }

    static createTimer(callback: Callback, delay: int, repeat: boolean): Timer {
        return EventLoop.timerTable.createTimer(callback, delay, repeat, 0);
    }

    static clearTimer(id: TimerId) {
        return EventLoop.timerTable.clearTimer(id);
    }

    static getOrCreate(): EventLoop {
        return EventLoop.registry.getOrCreateEventLoop();
    }

    public init() {
        launchSameWorker(() => { this.pollLoop(); } as Object);
    }

    public registerTimer(timer: Timer) {
        this.timerQueue.push(timer);
        ConcurrencyHelpers.eventFire(this.initEvent);
    }

    private pollLoop() {
        ConcurrencyHelpers.eventWait(this.initEvent);
        this.pollTimers();
        EventLoop.registry.removeEventLoop(this.workerId);
    }

    private pollTimers() {
        while (this.timerQueue.length != 0) {
            this.updateTimers();
            this.invokeTimers();
            Coroutine.Schedule();
        }
    }

    private updateTimers() {
        this.timerQueue = EventLoop.timerTable.removeClearedTimers(this.timerQueue);
        let currentTime = Date.now();
        this.timerQueue.forEach((timer: Timer) => { timer.updateDelay(currentTime); });
    }

    private invokeTimers() {
        this.timerQueue.forEach((timer: Timer) => {
            if (!timer.isExpired()) {
                return;
            }
            timer.invoke();
            if (timer.needRepeat()) {
                timer.reset();
            } else {
                EventLoop.timerTable.clearTimer(timer.getId());
            }
        });
    }

    private workerId: WorkerId;
    private initEvent: Object;
    private timerQueue: Array<Timer>;
    private static registry = new Registry();
    private static timerTable = new TimerTable();
};

class Registry {
    constructor() {
        this.eventLoops = new Map<WorkerId, EventLoop>();
        this.registryLock = ConcurrencyHelpers.spinlockCreate();
    }

    public getOrCreateEventLoop(): EventLoop {
        let workerId = CoroutineExtras.getWorkerId();
        let eLoop: EventLoop | undefined;
        let needInit = false;
        ConcurrencyHelpers.spinlockGuard(this.registryLock, () => {
            eLoop = this.eventLoops.get(workerId);
            needInit = eLoop === undefined;
            if (needInit) {
                eLoop = new EventLoop(workerId);
                this.eventLoops.set(workerId, eLoop!);
            }
        });
        if (needInit) {
            eLoop!.init();
        }
        return eLoop!;
    }

    public removeEventLoop(workerId: WorkerId) {
        ConcurrencyHelpers.spinlockGuard(this.registryLock, () => {
            this.eventLoops.delete(workerId);
        });
    }

    private eventLoops: Map<WorkerId, EventLoop>;
    private registryLock: Object;
};

class Timer {
    constructor(callback: Callback, delay: int, repeat: boolean, linkedTimer: long, id: TimerId) {
        this.callback = callback;
        this.delay = delay;
        this.repeat = repeat;
        this.linkedTimer = linkedTimer;
        this.id = id;
        this.startTime = Date.now();
        this.elapsedTime = 0;
    }

    public updateDelay(time: number) {
        this.elapsedTime += time - this.startTime;
        this.startTime = time;
    }

    public isExpired(): boolean {
        return this.delay <= this.elapsedTime;
    }

    public invoke() {
        launchSameWorker(this.callback as Object);
    }

    public getId(): TimerId {
        return this.id;
    }

    public needRepeat(): boolean {
        return this.repeat;
    }

    public reset() {
        this.startTime = Date.now();
        this.elapsedTime = 0;
    }

    public hasLinkedTimer(): boolean {
        return this.linkedTimer != 0;
    }

    private callback: Callback;
    private delay: int;
    private repeat: boolean;
    private linkedTimer: long;
    private id: TimerId;
    private startTime: number;
    private elapsedTime: number;
};

class TimerTable {
    constructor() {
        // IDs must be positive integers beginning at 1.
        this.nextFreeId = 1;
        this.tableSpinlock = ConcurrencyHelpers.spinlockCreate();
        this.timers = new Map<TimerId, Timer>();
    }

    public createTimer(callback: Callback, delay: int, repeat: boolean, linkedTimer: long): Timer {
        let timer: Timer | undefined;
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            let timerId = this.nextFreeId++;
            timer = new Timer(callback, delay, repeat, linkedTimer, timerId);
            this.timers.set(timerId, timer!);
        });
        return timer!;
    }

    public createTimer(callback: Object, delay: int, repeat: boolean, linkedTimer: long): Timer {
        return this.createTimer(callback as Callback, delay, repeat, linkedTimer);
    }

    public findTimer(id: TimerId): Timer | undefined {
        let timer: Timer | undefined;
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            timer = this.timers.get(id);
        });
        return timer;
    }

    public clearTimer(id: TimerId) {
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            let timer = this.timers.get(id);
            if (timer === undefined) {
                return;
            }
            if (timer!.hasLinkedTimer() && !CoroutineExtras.workerHasExternalScheduler()) {
                throw new Error('Failed to clear timer. Unable to clear interop timer from non-interop worker');
            }
            this.timers.delete(id);
        });
    }

    public removeClearedTimers(timerQueue: Array<Timer>): Array<Timer> {
        let updatedTimers: Array<Timer> | undefined = undefined;
        let startIdx = 0;
        ConcurrencyHelpers.spinlockGuard(this.tableSpinlock, () => {
            for (let idx = 0; idx < timerQueue.length; ++idx) {
                if (!this.timers.has(timerQueue[idx].getId())) {
                    if (updatedTimers === undefined) {
                        updatedTimers = new Array<Timer>();
                    }
                    // copy from start idx to removed timer
                    for (let subIdx = startIdx; subIdx < idx; ++subIdx) {
                        updatedTimers!.push(timerQueue[subIdx]);
                    }
                    startIdx = idx + 1;
                }
            }
        });
        if (updatedTimers !== undefined) {
            // copy from start idx to the end
            for (let subIdx = startIdx; subIdx < timerQueue.length; ++subIdx) {
                updatedTimers!.push(timerQueue[subIdx]);
            }
        }
        return updatedTimers !== undefined ? updatedTimers! : timerQueue;
    }

    private nextFreeId: TimerId;
    private tableSpinlock: Object;
    private timers: Map<TimerId, Timer>;
};
