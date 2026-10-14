//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The runtime half of Codira's structured concurrency: the worker pool that
//!   runs `spawn`ed tasks, the `scope`s that join them, and the bounded
//!   blocking channels they talk through. Compiled code reaches these through
//!   the dispatch table under the `codira.scope_*` / `codira.channel_*` names
//!   `codira_codegen::intrinsics::task_functions` declares.
//!
//! # The pool
//!
//! One process-wide pool, started on first use, keeps up to
//! `available_parallelism()` workers *running*. A worker that blocks -- in a
//! channel operation, or waiting for a nested scope -- stops counting as
//! running, and if work is queued and no worker is idle, a replacement is
//! started. Without that compensation a correct program could deadlock: with
//! N workers all waiting to receive, the task that would send could sit in
//! the queue forever. Idle workers beyond the target exit.
//!
//! # Scopes and channels
//!
//! A scope counts its unfinished tasks; `codira.scope_end` waits for the
//! count to reach zero. A channel belongs to the scope that created it and is
//! freed with it, after every task of the scope has finished -- the compiler
//! guarantees no code can name the channel after that (see
//! `codira_hir::expr::validator::tasks`).
//!
//! Elements are scalars of at most 16 bytes, copied in and out by value;
//! tasks share nothing else. A channel whose senders and receivers do not
//! match up blocks forever, as in any language with blocking channels: a
//! `scope` waiting for such a task never ends.
use std::{
    collections::VecDeque,
    num::NonZeroUsize,
    sync::{Arc, OnceLock},
};

use parking_lot::{Condvar, Mutex};

use crate::{FunctionDefinition, IntoFunctionDefinition};

/// The largest channel element, in bytes: an `i128`.
const MAX_ELEMENT_SIZE: usize = 16;

/// Stack size of a worker thread. Codira code recurses on the native stack.
const WORKER_STACK_SIZE: usize = 8 * 1024 * 1024;

/// A task's entry point: a compiler-generated trampoline that unpacks its
/// arguments and calls the spawned function.
type TaskEntry = extern "C" fn(*mut u8);

struct Job {
    entry: TaskEntry,
    /// The task's argument bytes, 16-byte aligned like any scalar needs.
    args: Box<[u128]>,
    scope: Arc<ScopeState>,
}

struct PoolState {
    queue: VecDeque<Job>,
    /// Workers waiting for work.
    idle: usize,
    /// Workers running a job and not blocked.
    running: usize,
}

struct Pool {
    state: Mutex<PoolState>,
    work_ready: Condvar,
    /// How many workers should be running at once.
    target: usize,
}

thread_local! {
    /// Whether this thread is one of the pool's workers.
    static IS_WORKER: std::cell::Cell<bool> = const { std::cell::Cell::new(false) };
}

fn pool() -> &'static Pool {
    static POOL: OnceLock<Pool> = OnceLock::new();
    POOL.get_or_init(|| Pool {
        state: Mutex::new(PoolState {
            queue: VecDeque::new(),
            idle: 0,
            running: 0,
        }),
        work_ready: Condvar::new(),
        target: std::thread::available_parallelism().map_or(1, NonZeroUsize::get),
    })
}

impl Pool {
    /// Starts a worker, counted as running from the start.
    fn start_worker(&'static self, state: &mut PoolState) {
        state.running += 1;
        std::thread::Builder::new()
            .name("codira-task".to_string())
            .stack_size(WORKER_STACK_SIZE)
            .spawn(move || self.work())
            .unwrap_or_else(|e| fatal(&format!("could not start a task worker: {e}")));
    }

    fn push(&'static self, job: Job) {
        let mut state = self.state.lock();
        state.queue.push_back(job);
        if state.idle > 0 {
            self.work_ready.notify_one();
        } else if state.running < self.target {
            self.start_worker(&mut state);
        }
    }

    fn work(&'static self) {
        IS_WORKER.with(|w| w.set(true));
        let mut state = self.state.lock();
        loop {
            if let Some(job) = state.queue.pop_front() {
                drop(state);
                run(job);
                state = self.state.lock();
                continue;
            }
            state.running -= 1;
            if state.idle >= self.target {
                return;
            }
            state.idle += 1;
            self.work_ready.wait(&mut state);
            state.idle -= 1;
            state.running += 1;
        }
    }

    /// The current thread is about to wait. A worker stops counting as
    /// running, and is replaced if work would otherwise sit in the queue.
    fn block_begin(&'static self) {
        if !IS_WORKER.with(std::cell::Cell::get) {
            return;
        }
        let mut state = self.state.lock();
        state.running -= 1;
        if !state.queue.is_empty() {
            if state.idle > 0 {
                self.work_ready.notify_one();
            } else if state.running < self.target {
                self.start_worker(&mut state);
            }
        }
    }

    /// The current thread has stopped waiting.
    fn block_end(&'static self) {
        if IS_WORKER.with(std::cell::Cell::get) {
            self.state.lock().running += 1;
        }
    }
}

fn run(mut job: Job) {
    (job.entry)(job.args.as_mut_ptr().cast::<u8>());
    let mut pending = job.scope.pending.lock();
    *pending -= 1;
    if *pending == 0 {
        job.scope.finished.notify_all();
    }
}

/// Waits on `condvar` while `blocked` holds, letting the pool compensate.
fn wait_while<T>(
    condvar: &Condvar,
    guard: &mut parking_lot::MutexGuard<'_, T>,
    mut blocked: impl FnMut(&T) -> bool,
) {
    while blocked(guard) {
        pool().block_begin();
        condvar.wait(guard);
        pool().block_end();
    }
}

/// A runtime error in compiled code's use of tasks: there is no caller to
/// return it to, so the process stops with a message.
fn fatal(message: &str) -> ! {
    eprintln!("codira: {message}");
    std::process::abort()
}

struct ScopeState {
    /// Tasks spawned and not yet finished.
    pending: Mutex<usize>,
    finished: Condvar,
    /// The channels created in the scope, freed with it.
    /// Boxed so each channel keeps its address -- compiled code holds it as
    /// the channel's handle -- while the vector grows.
    #[allow(clippy::vec_box)]
    channels: Mutex<Vec<Box<Channel>>>,
}

struct Channel {
    queue: Mutex<VecDeque<u128>>,
    not_empty: Condvar,
    not_full: Condvar,
    capacity: usize,
    element_size: usize,
}

/// # Safety
/// `scope` is a live handle returned by `scope_begin` and not yet ended.
unsafe fn scope_ref<'a>(scope: usize) -> &'a ScopeState {
    // SAFETY: guaranteed by the caller.
    unsafe { &*(scope as *const ScopeState) }
}

extern "C" fn scope_begin() -> usize {
    Arc::into_raw(Arc::new(ScopeState {
        pending: Mutex::new(0),
        finished: Condvar::new(),
        channels: Mutex::new(Vec::new()),
    })) as usize
}

extern "C" fn scope_spawn(scope: usize, entry: usize, args: usize, size: usize) {
    if entry == 0 {
        fatal("spawned a task without an entry point");
    }
    // SAFETY: the compiler passes the address of a task trampoline, which
    // has exactly this signature.
    let entry: TaskEntry = unsafe { std::mem::transmute::<usize, TaskEntry>(entry) };
    let mut copy = vec![0u128; size.div_ceil(16)].into_boxed_slice();
    if size > 0 {
        // SAFETY: the compiler passes the address and size of the stack
        // struct holding the task's arguments, alive for this call.
        unsafe {
            std::ptr::copy_nonoverlapping(args as *const u8, copy.as_mut_ptr().cast::<u8>(), size);
        }
    }
    // SAFETY: `scope` came from `scope_begin` and its scope has not ended:
    // the compiler only spawns inside an open scope.
    let scope = unsafe {
        Arc::increment_strong_count(scope as *const ScopeState);
        Arc::from_raw(scope as *const ScopeState)
    };
    *scope.pending.lock() += 1;
    pool().push(Job {
        entry,
        args: copy,
        scope,
    });
}

extern "C" fn scope_end(scope: usize) {
    // SAFETY: the compiler ends each scope it began exactly once.
    let scope = unsafe { Arc::from_raw(scope as *const ScopeState) };
    let mut pending = scope.pending.lock();
    wait_while(&scope.finished, &mut pending, |pending| *pending > 0);
    // Every task has finished, so nothing uses the scope's channels any
    // more; they are freed with the last reference to the scope.
}

extern "C" fn channel_new(scope: usize, capacity: usize, element_size: usize) -> usize {
    if capacity == 0 {
        fatal("a channel needs a capacity of at least 1");
    }
    if element_size > MAX_ELEMENT_SIZE {
        fatal("a channel element is larger than 16 bytes");
    }
    let channel = Box::new(Channel {
        queue: Mutex::new(VecDeque::with_capacity(capacity.min(1024))),
        not_empty: Condvar::new(),
        not_full: Condvar::new(),
        capacity,
        element_size,
    });
    let handle = std::ptr::from_ref::<Channel>(&channel) as usize;
    // SAFETY: the compiler creates channels only inside an open scope.
    unsafe { scope_ref(scope) }.channels.lock().push(channel);
    handle
}

/// # Safety
/// `channel` is a handle from `channel_new` whose scope has not ended.
unsafe fn channel_ref<'a>(channel: usize) -> &'a Channel {
    // SAFETY: guaranteed by the caller.
    unsafe { &*(channel as *const Channel) }
}

extern "C" fn channel_send(channel: usize, value: usize) {
    // SAFETY: the compiler only names a channel inside the scope that owns it.
    let channel = unsafe { channel_ref(channel) };
    let mut element = 0u128;
    // SAFETY: `value` is the address of a stack slot of the element type,
    // `element_size` bytes long, and `element` is 16 bytes.
    unsafe {
        std::ptr::copy_nonoverlapping(
            value as *const u8,
            std::ptr::from_mut(&mut element).cast::<u8>(),
            channel.element_size,
        );
    }
    let mut queue = channel.queue.lock();
    wait_while(&channel.not_full, &mut queue, |queue| {
        queue.len() >= channel.capacity
    });
    queue.push_back(element);
    channel.not_empty.notify_one();
}

extern "C" fn channel_receive(channel: usize, out: usize) {
    // SAFETY: the compiler only names a channel inside the scope that owns it.
    let channel = unsafe { channel_ref(channel) };
    let mut queue = channel.queue.lock();
    wait_while(&channel.not_empty, &mut queue, VecDeque::is_empty);
    let element = queue
        .pop_front()
        .expect("the queue is not empty after the wait");
    channel.not_full.notify_one();
    drop(queue);
    // SAFETY: `out` is the address of a stack slot of the element type.
    unsafe {
        std::ptr::copy_nonoverlapping(
            std::ptr::from_ref(&element).cast::<u8>(),
            out as *mut u8,
            channel.element_size,
        );
    }
}

/// The task functions every runtime registers.
pub(crate) fn functions() -> Vec<FunctionDefinition> {
    vec![
        IntoFunctionDefinition::into(
            scope_begin as extern "C" fn() -> usize,
            "codira.scope_begin",
        ),
        IntoFunctionDefinition::into(
            scope_spawn as extern "C" fn(usize, usize, usize, usize),
            "codira.scope_spawn",
        ),
        IntoFunctionDefinition::into(scope_end as extern "C" fn(usize), "codira.scope_end"),
        IntoFunctionDefinition::into(
            channel_new as extern "C" fn(usize, usize, usize) -> usize,
            "codira.channel_new",
        ),
        IntoFunctionDefinition::into(
            channel_send as extern "C" fn(usize, usize),
            "codira.channel_send",
        ),
        IntoFunctionDefinition::into(
            channel_receive as extern "C" fn(usize, usize),
            "codira.channel_receive",
        ),
    ]
}

#[cfg(test)]
mod tests {
    use std::sync::atomic::{AtomicUsize, Ordering};

    use super::*;

    static COUNTER: AtomicUsize = AtomicUsize::new(0);

    extern "C" fn add(args: *mut u8) {
        // SAFETY: the test passes a `usize`.
        let amount = unsafe { *args.cast::<usize>() };
        COUNTER.fetch_add(amount, Ordering::SeqCst);
    }

    /// Every task of a scope has run when `scope_end` returns.
    #[test]
    fn scope_end_joins_every_task() {
        let scope = scope_begin();
        for amount in 1..=100usize {
            scope_spawn(
                scope,
                add as TaskEntry as usize,
                std::ptr::from_ref(&amount) as usize,
                std::mem::size_of::<usize>(),
            );
        }
        scope_end(scope);
        assert_eq!(COUNTER.load(Ordering::SeqCst), 5050);
    }

    extern "C" fn producer(args: *mut u8) {
        // SAFETY: the test passes a channel handle.
        let channel = unsafe { *args.cast::<usize>() };
        for value in 0..1000i64 {
            channel_send(channel, std::ptr::from_ref(&value) as usize);
        }
    }

    /// A bounded channel delivers every element, in order, to a receiver
    /// that is slower than the sender.
    #[test]
    fn channel_delivers_in_order_through_a_small_buffer() {
        let scope = scope_begin();
        let channel = channel_new(scope, 2, std::mem::size_of::<i64>());
        scope_spawn(
            scope,
            producer as TaskEntry as usize,
            std::ptr::from_ref(&channel) as usize,
            std::mem::size_of::<usize>(),
        );
        for expected in 0..1000i64 {
            let mut value = -1i64;
            channel_receive(channel, std::ptr::from_mut(&mut value) as usize);
            assert_eq!(value, expected);
        }
        scope_end(scope);
    }

    extern "C" fn receive_then_send(args: *mut u8) {
        // SAFETY: the test passes two channel handles.
        let (from, to) = unsafe { *args.cast::<(usize, usize)>() };
        let mut value = 0i32;
        channel_receive(from, std::ptr::from_mut(&mut value) as usize);
        let doubled = value * 2;
        channel_send(to, std::ptr::from_ref(&doubled) as usize);
    }

    /// More tasks blocked waiting than there are workers: the pool starts
    /// replacements, so the task that unblocks them still runs.
    #[test]
    fn blocked_workers_are_compensated() {
        let workers = pool().target;
        let scope = scope_begin();
        let input = channel_new(scope, 1, 4);
        let output = channel_new(scope, 1024, 4);
        let tasks = workers * 2 + 1;
        for _ in 0..tasks {
            let channels = (input, output);
            scope_spawn(
                scope,
                receive_then_send as TaskEntry as usize,
                std::ptr::from_ref(&channels) as usize,
                std::mem::size_of::<(usize, usize)>(),
            );
        }
        let mut sum = 0;
        for value in 0..i32::try_from(tasks).unwrap() {
            channel_send(input, std::ptr::from_ref(&value) as usize);
        }
        for _ in 0..tasks {
            let mut value = 0i32;
            channel_receive(output, std::ptr::from_mut(&mut value) as usize);
            sum += value;
        }
        scope_end(scope);
        let n = i32::try_from(tasks).unwrap();
        assert_eq!(sum, n * (n - 1));
    }
}
