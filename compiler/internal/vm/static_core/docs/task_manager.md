# Introduction

Internally in VM we have different tasks for two components `GC` and `JIT`. Both of this components can use more than one thread for solving one task or another. To optimize usage of hardware(cores)/OS(threads) resources we introduce TaskManager. The main goal of TaskManager is to use limited number of resources(OS threads) and schedule different types of tasks and achieve optimal metric values for `GC` and `JIT`. I.e. for example if we expect STW(stop-the-world pause) soon - we should increase priority for `GC` tasks. And vice-versa if we do not expect `GC` tasks anytime soon - we should increase priority for `JIT` tasks. 

For Hybrid mode (i.e. we can execute both static and dynamic code) TaskManager also should take into account the language context. I.e. if we are currently executing static code - the `GC` tasks for static instance have higher priority than `GC` tasks for dynamic instance, and vice versa. 

# Implementation details

##Task

It is some limited by time task, which has these properties:
- priority of operation: `background` or `foreground`;
- for which VM instance (`static` or `dynamic`) this task is executed;
- which component is the owner of this task (`GC`, `JIT`, etc.)

The time is required to finish the task should be limited by some reasonable value, to make the whole system with Task management optimal.


##WorkerThread

WorkerThread is the thread which executing tasks. Each WorkerThread has two queues - one for the foreground tasks and one for the background tasks. WorkerThread always try to execute task from the foreground queue first, if it is empty, then it try to execute task from the background queue. If no tasks in both queues - WorkerThread request `TaskManager` for more tasks.

##TaskManager

Is responsible for tasks "scheduling". I.e. each `WorkerThread` request `TaskManager` for new tasks. TaskManager creates array of tasks in proportion based on the current context(i.e. which language currently executed - static or dynamic, do we expect GC soon or not, etc). 

TaskManager make decision about scheduling based on some state of the system. The state of the system is provided by changing priority for TaskQueue(s). For example if we are currently expect a lot of GC because we have a lot of allocations, we will increase priority for TaskQueue for GC tasks. TaskManager choose more tasks from TaskQueues with higher priority. First tasks with `foreground` priority are selected in proportion based on TaskQueues priority, then if there are tasks with `foreground` priority in some TaskQueues they are selected without priority/proportion rules, if not - `background` tasks will be selected in proportion based on TasqQueues priority. 

##TaskQueue

It is a queue for tasks of the same "kind" or "components" (for example "JIT") which is fullfilled by corresponding components of VM.

Each component 

For managing worker threads we should provide machinery for:
1. Managing thread pool - `ThreadPool`, `WorkerThread`
2. Managing tasks - `TaskQueue`, `TaskExecutor` etc.
3. Change priority of worker threads `ThreadManager`
4. Distribute worker threads from `ThreadPool` between static and dynamic VM instances to optimize performance and power efficiency

NB: the minimal entity in this machinery is WorkerThread, i.e. we shouldn't have one `TaskQueue` for tasks from different task producers (such as GC, JIT etc.)

# ThreadPool

This entity is provide functionality for:
1. Creation of thread pool with `N` worker threads
1. Getting available `WorkerThread` from the pool of available worker threads
1. Returning `WorkerThread` back to the pool of available worker threads 

# Thread Manager

Thread Manager is responsible for:
1. Keeping thread pool
1. Distribute threads from pool between consumers in accordance with different policies
1. Provide machinery for changing priority of the threads managed by current Thread Manager

## API

TBD

# WorkerThread

The thread which is responsible for Task execution. 

Has these states:
1. Idle - waiting for task
2. Running - currently executing some task
3. ShuttingDown - currently termination is in progress
