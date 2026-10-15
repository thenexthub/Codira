//! Linking inside another program.
//!
//! mold is a one-shot process: it keeps its state in globals, ends by calling
//! `exit`, leaks what it maps and allocates, forks to hide its exit latency and
//! builds rayon's global thread pool, which can be built only once. A compiler
//! or build daemon that links thousands of times in one process can do none of
//! that. A [`Session`] is one link run that way:
//!
//! * the process never exits: where the command line would exit, the link
//!   unwinds with a [`Terminate`] payload, which the caller of
//!   [`catch_termination`] turns into a result;
//! * diagnostics, and what the command line prints to stdout (`--version`,
//!   `--help`, `--stats`), are collected instead of written to the process's
//!   streams (unless asked to echo them);
//! * everything the link leaks (see `mem`) is recorded and freed when the
//!   session ends, the input files are unmapped, and the process-wide state in
//!   `error`, `mapped_file` and `output_file` is put back as it was;
//! * threads come from a pool built for this link ([`run_in_pool`]) and are gone
//!   when it returns.
//!
//! The globals are still process-wide, so only one session can be active at a
//! time: [`Session::begin`] waits for the previous one to end, and concurrent
//! links from several threads are serialised.

use std::any::Any;
use std::fmt;
use std::panic::{AssertUnwindSafe, catch_unwind, resume_unwind};
use std::path::PathBuf;
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::sync::{Arc, Condvar, Mutex, MutexGuard};

/// How serious a diagnostic is.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Severity {
    /// A condition the linker cannot continue past.
    Fatal,
    /// The link fails, but the linker went on to report more problems.
    Error,
    Warning,
    /// A line of a report (`-why_load` and the like) with no prefix.
    Notice,
}

/// One message the linker would have written to stderr.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Diagnostic {
    pub severity: Severity,
    pub message: String,
}

impl fmt::Display for Diagnostic {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.severity {
            Severity::Fatal => write!(f, "fatal: {}", self.message),
            Severity::Error => write!(f, "error: {}", self.message),
            Severity::Warning => write!(f, "warning: {}", self.message),
            Severity::Notice => f.write_str(&self.message),
        }
    }
}

/// The payload of the unwind that replaces `exit` in a session.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Terminate {
    /// The status the process would have exited with.
    pub status: i32,
}

/// What a link tells its caller besides the output file.
#[derive(Clone, Debug, Default)]
pub struct LinkInfo {
    /// The output file, as the command line named it.
    pub output: Option<PathBuf>,
    /// The linker's target, such as "x86_64" or "arm64".
    pub target: Option<&'static str>,
    /// How many threads the link used.
    pub threads: usize,
}

#[derive(Default)]
struct Sink {
    diagnostics: Vec<Diagnostic>,
    stdout: String,
    echo: bool,
    unsupported: Option<String>,
    info: LinkInfo,
    hooks: Vec<fn()>,
}

static ACTIVE: AtomicBool = AtomicBool::new(false);
static SINK: Mutex<Sink> = Mutex::new(Sink {
    diagnostics: Vec::new(),
    stdout: String::new(),
    echo: false,
    unsupported: None,
    info: LinkInfo { output: None, target: None, threads: 0 },
    hooks: Vec::new(),
});
static SESSION_LOCK: Mutex<()> = Mutex::new(());

fn sink() -> MutexGuard<'static, Sink> {
    SINK.lock().unwrap_or_else(|e| e.into_inner())
}

/// Whether a library session is active, that is, whether the linker is to
/// return to its caller where the command line would exit.
pub fn is_library() -> bool {
    ACTIVE.load(Ordering::Relaxed)
}

/// Records a diagnostic if a session is active. Returns false, doing nothing,
/// when the command line is running, which writes to stderr itself.
pub(crate) fn capture(severity: Severity, message: &fmt::Arguments, prefix: &str) -> bool {
    if !is_library() {
        return false;
    }
    let message = message.to_string();
    let mut sink = sink();
    if sink.echo {
        use std::io::Write;
        let _ = writeln!(std::io::stderr(), "{prefix}{message}");
    }
    sink.diagnostics.push(Diagnostic { severity, message });
    true
}

/// Records what the command line would print to stdout.
pub(crate) fn capture_stdout(message: &fmt::Arguments) -> bool {
    if !is_library() {
        return false;
    }
    let mut sink = sink();
    if sink.echo {
        println!("{message}");
    }
    use fmt::Write;
    let _ = writeln!(sink.stdout, "{message}");
    true
}

/// Notes that the link stopped because it was asked for something this linker
/// does not implement (an option, a target, link-time optimisation), as
/// opposed to an error in the inputs. The caller may then use another linker.
pub(crate) fn mark_unsupported(message: String) {
    sink().unsupported = Some(message);
}

/// Sets what the link reports about itself.
pub fn set_info(update: impl FnOnce(&mut LinkInfo)) {
    if is_library() {
        update(&mut sink().info);
    }
}

/// Runs `hook` when the session ends, after the link is over and before its
/// leaked allocations are freed. For caches that hold references into them.
pub fn on_end(hook: fn()) {
    if is_library() {
        let mut sink = sink();
        if !sink.hooks.contains(&hook) {
            sink.hooks.push(hook);
        }
    }
}

/// What a finished session collected.
#[derive(Debug, Default)]
pub struct Captured {
    pub diagnostics: Vec<Diagnostic>,
    pub stdout: String,
    /// Set if the link stopped on something this linker does not implement.
    pub unsupported: Option<String>,
    pub info: LinkInfo,
}

/// One link, run inside this process. See the module documentation.
pub struct Session {
    _lock: MutexGuard<'static, ()>,
}

impl Session {
    /// Starts a session, waiting for any other to end first. With `echo`, the
    /// diagnostics are also written to stderr as they happen.
    pub fn begin(echo: bool) -> Self {
        let lock = SESSION_LOCK.lock().unwrap_or_else(|e| e.into_inner());
        *sink() = Sink { echo, ..Sink::default() };
        crate::mem::arena_start();
        ACTIVE.store(true, Ordering::SeqCst);
        Session { _lock: lock }
    }

    /// Ends the session and returns what it collected.
    pub fn finish(self) -> Captured {
        self.end()
    }

    fn end(&self) -> Captured {
        // The link's data structures are gone by now; what is left are the
        // globals and the allocations they point into.
        let hooks = std::mem::take(&mut sink().hooks);
        for hook in hooks {
            hook();
        }
        crate::error::reset_state();
        crate::perf::Counter::disable();
        crate::output_file::cleanup();
        crate::output_file::reset();
        crate::mapped_file::reset();
        // SAFETY: nothing of the link is used after this; the caller of
        // `Session::begin` owns no reference into it (see the module docs).
        unsafe { crate::mem::arena_release() };
        ACTIVE.store(false, Ordering::SeqCst);
        let mut sink = sink();
        Captured {
            diagnostics: std::mem::take(&mut sink.diagnostics),
            stdout: std::mem::take(&mut sink.stdout),
            unsupported: sink.unsupported.take(),
            info: std::mem::take(&mut sink.info),
        }
    }
}

impl Drop for Session {
    fn drop(&mut self) {
        // Only reached when `finish` was not: a panic in the caller. Free
        // what the session holds all the same.
        if ACTIVE.load(Ordering::SeqCst) {
            self.end();
        }
    }
}

/// What ended a call to [`catch_termination`].
#[derive(Debug)]
pub enum Ended {
    /// The linker would have exited with this status.
    Exited(i32),
    /// Something panicked that is not the linker's way of exiting: a bug, with
    /// its message.
    Panicked(String),
}

/// Runs `f`, turning the linker's would-be exits into a result.
pub fn catch_termination<R>(f: impl FnOnce() -> R) -> Result<R, Ended> {
    match catch_unwind(AssertUnwindSafe(f)) {
        Ok(value) => Ok(value),
        Err(payload) => Err(classify(payload)),
    }
}

fn classify(payload: Box<dyn Any + Send>) -> Ended {
    if let Some(t) = payload.downcast_ref::<Terminate>() {
        return Ended::Exited(t.status);
    }
    let message = if let Some(s) = payload.downcast_ref::<&str>() {
        (*s).to_owned()
    } else if let Some(s) = payload.downcast_ref::<String>() {
        s.clone()
    } else {
        "a panic with a payload that is not a string".to_owned()
    };
    Ended::Panicked(message)
}

/// Unwinds to the caller of [`catch_termination`], as `exit(status)` would end
/// the process. Skips the panic hook, so nothing is printed.
pub(crate) fn terminate(status: i32) -> ! {
    resume_unwind(Box::new(Terminate { status }))
}

struct Threads {
    live: AtomicUsize,
    gone: Mutex<()>,
    changed: Condvar,
}

/// A thread pool that, when dropped, waits until every one of its threads has
/// exited: after a link nothing of it may be left running, because the next
/// thing to happen is that the memory its threads read is freed.
struct Pool {
    pool: Option<rayon::ThreadPool>,
    threads: Arc<Threads>,
}

impl Drop for Pool {
    fn drop(&mut self) {
        drop(self.pool.take());
        let mut guard = self.threads.gone.lock().unwrap_or_else(|e| e.into_inner());
        while self.threads.live.load(Ordering::SeqCst) != 0 {
            guard = self.threads.changed.wait(guard).unwrap_or_else(|e| e.into_inner());
        }
    }
}

/// Runs `f` on a thread pool of `threads` threads built for this call and gone
/// when it returns (or unwinds). rayon's global pool, which the command line
/// uses, can be built only once per process and never goes away.
pub fn run_in_pool<R: Send>(threads: usize, f: impl FnOnce() -> R + Send) -> R {
    let threads = threads.max(1);
    let state = Arc::new(Threads {
        live: AtomicUsize::new(threads),
        gone: Mutex::new(()),
        changed: Condvar::new(),
    });
    let exiting = Arc::clone(&state);
    let built = rayon::ThreadPoolBuilder::new()
        .num_threads(threads)
        .thread_name(|i| format!("funis-{i}"))
        // The linker recurses (symbol resolution, ICF), and a thread of the
        // default size has 2 MiB, a quarter of what the main thread of the
        // command line has.
        .stack_size(8 << 20)
        .exit_handler(move |_| {
            exiting.live.fetch_sub(1, Ordering::SeqCst);
            let _guard = exiting.gone.lock().unwrap_or_else(|e| e.into_inner());
            exiting.changed.notify_all();
        })
        .build();
    let pool = match built {
        Ok(pool) => pool,
        Err(err) => crate::error::fatal(format_args!("failed to build linker thread pool: {err}")),
    };
    let pool = Pool { pool: Some(pool), threads: state };
    let result = pool.pool.as_ref().expect("the pool is built").install(f);
    drop(pool);
    result
}
