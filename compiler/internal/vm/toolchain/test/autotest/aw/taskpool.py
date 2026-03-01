#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: A task pool that can execute tasks asynchronously.
"""

import asyncio
from queue import Queue
from threading import Thread, current_thread
from time import time


class TaskPool(object):
    def __init__(self):
        self.task_queue = Queue()
        self.event_loop = None
        self.task_exception = None
        self.event_loop_thread = None
        self._start_event_loop()

    def submit(self, coroutine, callback=None):
        # add item to the task queue
        self._task_add()
        future = asyncio.run_coroutine_threadsafe(coro=coroutine, loop=self.event_loop)
        future.add_done_callback(callback) if callback else None
        # remove item from the task queue after the task is done
        future.add_done_callback(self._task_done)

    def await_taskpool(self):
        asyncio.run_coroutine_threadsafe(coro=self._stop_loop(), loop=self.event_loop)

    def task_join(self):
        self.task_queue.join()

    def _task_add(self, item=1):
        self.task_queue.put(item)

    def _task_done(self, future):
        # clear the task queue and stop the task pool once an exception occurs in the task
        if future.exception():
            while not self.task_queue.empty():
                self.task_queue.get()
                self.task_queue.task_done()
            self.task_exception = future.exception()
            return
        self.task_queue.get()
        self.task_queue.task_done()

    def _set_and_run_loop(self, loop):
        self.event_loop = loop
        asyncio.set_event_loop(loop)
        loop.run_forever()

    async def _stop_loop(self, interval=1):
        # wait for all tasks in the event loop is done, then we can close the loop
        while True:
            if self.task_queue.empty():
                self.event_loop.stop()
                return
            await asyncio.sleep(interval)

    def _start_event_loop(self):
        loop = asyncio.new_event_loop()
        self.event_loop_thread = Thread(target=self._set_and_run_loop, args=(loop,))
        self.event_loop_thread.daemon = True
        # Specifies the thread name to be able to save log of thread to the module_run.log file
        self.event_loop_thread.name = current_thread().name + "-" + str(time()).replace('.', '')[-5:]
        self.event_loop_thread.start()
