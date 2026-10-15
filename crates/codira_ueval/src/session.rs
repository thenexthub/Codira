//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Compiling a version of a workload into libraries (`Builder`), starting a
//!   runtime on a set of libraries (`Deployment`), and delivering an update the
//!   way today's hot reload receives one: new library files appear in the
//!   watched directory, the compiler's lock file is released, and the host
//!   calls `Runtime::update`.

use std::{
    collections::BTreeMap,
    path::{Path, PathBuf},
    sync::atomic::{AtomicBool, Ordering},
    time::{Duration, Instant},
};

use codira_compiler::{Config, DisplayColor, Driver};
use codira_runtime::Runtime;

/// The libraries of one build: file name (`table.codiralib`) to its bytes.
pub type LibSet = BTreeMap<String, Vec<u8>>;

static RELINK_FAILED: AtomicBool = AtomicBool::new(false);

struct RelinkLogger;

impl log::Log for RelinkLogger {
    fn enabled(&self, metadata: &log::Metadata<'_>) -> bool {
        metadata.level() == log::Level::Error
    }

    fn log(&self, record: &log::Record<'_>) {
        if self.enabled(record.metadata())
            && record
                .args()
                .to_string()
                .starts_with("Failed to relink assemblies")
        {
            RELINK_FAILED.store(true, Ordering::SeqCst);
        }
    }

    fn flush(&self) {}
}

/// Installs the logger that lets [`Deployment::apply`] see that the runtime
/// rejected a relink. Safe to call twice.
pub fn install_relink_logger() {
    static LOGGER: RelinkLogger = RelinkLogger;
    if log::set_logger(&LOGGER).is_ok() {
        log::set_max_level(log::LevelFilter::Error);
    }
}

/// Options that change what the compiler produces.
#[derive(Clone, Copy, Debug, Default)]
pub struct BuildOptions {
    /// Build without hot reload: direct calls between modules, no dispatch
    /// table for them (see `--no-hot-reload`).
    pub no_hot_reload: bool,
}

/// A compiler that stays alive between builds, so that producing many
/// variants of one version only recompiles what changed.
pub struct Builder {
    driver: Driver,
    _src: tempfile::TempDir,
    out: tempfile::TempDir,
    /// Current text of every `.code` file, keyed by path below `src/`.
    texts: BTreeMap<String, String>,
}

fn source_files(files: &BTreeMap<String, String>) -> BTreeMap<String, String> {
    files
        .iter()
        .filter_map(|(path, text)| {
            path.strip_prefix("src/")
                .map(|rel| (rel.to_owned(), text.clone()))
        })
        .collect()
}

impl Builder {
    /// Compiles the version whose files are `files` (as read by
    /// `Manifest::read_version`). Returns the builder and the first library
    /// set, or the compiler's diagnostics.
    pub fn new(
        files: &BTreeMap<String, String>,
        options: BuildOptions,
    ) -> Result<(Builder, LibSet), String> {
        let src = tempfile::TempDir::new().map_err(|e| e.to_string())?;
        let out = tempfile::TempDir::new().map_err(|e| e.to_string())?;
        for (path, text) in files {
            let full = src.path().join(path);
            std::fs::create_dir_all(full.parent().expect("has a parent"))
                .and_then(|()| std::fs::write(&full, text))
                .map_err(|e| format!("writing {path}: {e}"))?;
        }
        let config = Config {
            out_dir: Some(out.path().to_path_buf()),
            ..Config::default()
        };
        let (_package, mut driver) =
            Driver::with_package_path(src.path().join("codira.toml"), config)
                .map_err(|e| format!("{e:#}"))?;
        options.apply(&mut driver);
        let mut builder = Builder {
            driver,
            _src: src,
            out,
            texts: source_files(files),
        };
        let libs = builder.compile()?;
        Ok((builder, libs))
    }

    fn compile(&mut self) -> Result<LibSet, String> {
        if let Some(errors) = self
            .driver
            .emit_diagnostics_to_string(DisplayColor::Disable)
            .map_err(|e| format!("{e:#}"))?
        {
            return Err(errors);
        }
        self.driver
            .write_all_assemblies(true)
            .map_err(|e| format!("{e:#}"))?;
        read_libs(self.out.path()).map_err(|e| e.to_string())
    }

    /// Compiles a variant of the version this builder was created for:
    /// `files` replaces the text of every `.code` file that differs. A file
    /// that did not exist at creation cannot be added.
    pub fn build_variant(&mut self, files: &BTreeMap<String, String>) -> Result<LibSet, String> {
        let wanted = source_files(files);
        for (rel, text) in &wanted {
            match self.texts.get(rel) {
                None => return Err(format!("variant adds a file ({rel}); not supported")),
                Some(old) if old == text => {}
                Some(_) => {
                    self.driver
                        .set_file_text(rel.as_str(), text)
                        .map_err(|e| format!("{e:#}"))?;
                }
            }
        }
        self.texts = wanted;
        self.compile()
    }
}

impl BuildOptions {
    fn apply(self, _driver: &mut Driver) {
        // `no_hot_reload` is wired to the compiler in the commit that adds
        // the option; until then the field is always false.
        let _ = self.no_hot_reload;
    }
}

/// Reads every `*.codiralib` in `dir`.
pub fn read_libs(dir: &Path) -> std::io::Result<LibSet> {
    let mut libs = LibSet::new();
    for entry in std::fs::read_dir(dir)? {
        let path = entry?.path();
        if path.extension().is_some_and(|e| e == "codiralib") {
            let name = path
                .file_name()
                .expect("a file has a name")
                .to_string_lossy()
                .into_owned();
            libs.insert(name, std::fs::read(&path)?);
        }
    }
    Ok(libs)
}

/// What delivering an update cost, as seen by the host thread.
#[derive(Clone, Copy, Debug, Default)]
pub struct ApplyTiming {
    /// Copying changed files into the watched directory.
    pub install: Duration,
    /// From the end of the copy until `Runtime::update` reported success.
    pub detect: Duration,
    /// The one `Runtime::update` call that relinked: the time the host
    /// thread cannot serve requests.
    pub relink_call: Duration,
}

/// Why an update did not take effect.
#[derive(Clone, Debug, PartialEq, Eq)]
pub enum ApplyError {
    /// No library differs from the running one.
    NothingToApply,
    /// The runtime logged a failed relink.
    RelinkFailed,
    /// `Runtime::update` did not report a relink in time.
    Timeout,
    /// Filesystem trouble.
    Io(String),
}

/// A directory with a set of libraries in it and a runtime watching it.
pub struct Deployment {
    dir: tempfile::TempDir,
    current: LibSet,
}

impl Deployment {
    /// Writes `libs` into a fresh directory.
    pub fn new(libs: &LibSet) -> std::io::Result<Deployment> {
        let dir = tempfile::TempDir::new()?;
        for (name, bytes) in libs {
            std::fs::write(dir.path().join(name), bytes)?;
        }
        Ok(Deployment {
            dir,
            current: libs.clone(),
        })
    }

    /// The path of the entry library.
    pub fn entry(&self) -> PathBuf {
        self.dir.path().join("mod.codiralib")
    }

    /// Starts a runtime on the libraries in the directory.
    pub fn start(&self) -> Result<Runtime, String> {
        // Safety: the libraries were produced by this crate's own compiler
        // build; loading them runs their initialisers, as for any library.
        unsafe { Runtime::builder(self.entry()).finish() }.map_err(|e| e.to_string())
    }

    /// Delivers `libs` the way the compiler does when it rebuilds: take the
    /// lock file, write the changed libraries, release the lock. Then call
    /// `Runtime::update` until it relinks. Only libraries whose bytes differ
    /// are written.
    pub fn apply(
        &mut self,
        runtime: &mut Runtime,
        libs: &LibSet,
        timeout: Duration,
    ) -> Result<ApplyTiming, ApplyError> {
        let changed: Vec<&String> = libs
            .iter()
            .filter(|(name, bytes)| self.current.get(*name) != Some(*bytes))
            .map(|(name, _)| name)
            .collect();
        if changed.is_empty() {
            return Err(ApplyError::NothingToApply);
        }
        RELINK_FAILED.store(false, Ordering::SeqCst);
        let io = |e: std::io::Error| ApplyError::Io(e.to_string());
        let t0 = Instant::now();
        let lock = self.dir.path().join(codira_project::LOCKFILE_NAME);
        std::fs::write(&lock, b"").map_err(io)?;
        for name in changed {
            std::fs::write(self.dir.path().join(name), &libs[name]).map_err(io)?;
        }
        std::fs::remove_file(&lock).map_err(io)?;
        let install = t0.elapsed();
        self.current = libs.clone();

        let t1 = Instant::now();
        loop {
            let call = Instant::now();
            // Safety: the new libraries were compiled by this crate's
            // compiler from sources it also compiled.
            if unsafe { runtime.update() } {
                return Ok(ApplyTiming {
                    install,
                    detect: t1.elapsed(),
                    relink_call: call.elapsed(),
                });
            }
            if RELINK_FAILED.load(Ordering::SeqCst) {
                return Err(ApplyError::RelinkFailed);
            }
            if t1.elapsed() > timeout {
                return Err(ApplyError::Timeout);
            }
            std::thread::sleep(Duration::from_micros(100));
        }
    }
}
