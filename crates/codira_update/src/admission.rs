//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The interface between a running process and the update checker:
//!   [`UpdateAdmission`] (what the runtime calls), the real implementation
//!   ([`Z3Admission`], backed by [`crate::check`]), the bundle format an update
//!   travels in ([`UpdateBundle`]) and a test double ([`TestAdmission`]) the
//!   runtime can integrate against without proving anything.
//!
//! # How a runtime uses it
//!
//! ```text
//! let admission = Z3Admission::new(current_library_bytes);
//! match admission.check(&bundle_bytes) {
//!     Admission::Rejected(reasons) => { /* state Rejected: leave everything */ }
//!     Admission::Admitted(report) => {
//!         // 1. before the swap, scan the live heap for every
//!         //    `report.migrations` entry whose outcome is `Guarded`
//!         //    (migration::value_survives); a value outside the guard
//!         //    rejects the update;
//!         // 2. quiesce; refuse to complete while a frame of any function in
//!         //    `report.affected` (or, if the library may have been inlined,
//!         //    `inlining_exposed`) is live: `report.affected.blockers(..)`;
//!         // 3. swap with the new library bytes in the bundle;
//!         // 4. on commit: admission = admission.committed(new_library_bytes)
//!         //    so the next update is checked against this one's result.
//!     }
//! }
//! ```
//!
//! A timeout is never an admission: a query that exhausts the checker's
//! budget is a failure of the check. The checker runs Z3 on the calling
//! thread; a runtime that must not stall should call it from a worker.

use std::{collections::VecDeque, sync::Mutex};

use crate::{
    affected::AffectedSet,
    cert::UpdateCertificate,
    check::{check, CheckOptions, Report},
};

/// The verdict on an update.
#[derive(Debug, Clone, PartialEq)]
pub enum Admission {
    /// Every obligation re-proved; the report carries the affected set and
    /// the migration plan the runtime must honour.
    Admitted(Box<Report>),
    /// Do not swap. One reason per failed check.
    Rejected(Vec<String>),
}

impl Admission {
    pub fn is_admitted(&self) -> bool {
        matches!(self, Admission::Admitted(_))
    }
}

/// What the runtime calls before it quiesces for an update.
pub trait UpdateAdmission {
    /// Decides on `update`, the bytes of an [`UpdateBundle`]: the update
    /// certificate and the new library. Malformed bytes are `Rejected`.
    fn check(&self, update: &[u8]) -> Admission;
}

const MAGIC: &[u8] = b"CODIRA-UPDATE-BUNDLE\n1\n";

/// An update as it travels: the certificate (JSON) and the new library.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct UpdateBundle {
    pub certificate: String,
    pub new_artifact: Vec<u8>,
}

impl UpdateBundle {
    /// `magic, u64 LE length + certificate, u64 LE length + library`.
    pub fn pack(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(MAGIC.len() + 16 + self.certificate.len());
        out.extend_from_slice(MAGIC);
        out.extend_from_slice(&(self.certificate.len() as u64).to_le_bytes());
        out.extend_from_slice(self.certificate.as_bytes());
        out.extend_from_slice(&(self.new_artifact.len() as u64).to_le_bytes());
        out.extend_from_slice(&self.new_artifact);
        out
    }

    /// The inverse of [`UpdateBundle::pack`]; `Err` for anything malformed.
    pub fn unpack(bytes: &[u8]) -> Result<UpdateBundle, String> {
        let rest = bytes
            .strip_prefix(MAGIC)
            .ok_or("not an update bundle (bad magic)")?;
        let take = |rest: &[u8]| -> Result<(usize, usize), String> {
            let header: [u8; 8] = rest
                .get(..8)
                .and_then(|h| h.try_into().ok())
                .ok_or("truncated update bundle")?;
            let len = usize::try_from(u64::from_le_bytes(header))
                .map_err(|_too_large| "update bundle part too large")?;
            if rest.len() - 8 < len {
                return Err("truncated update bundle".to_string());
            }
            Ok((8, len))
        };
        let (skip, len) = take(rest)?;
        let certificate = std::str::from_utf8(&rest[skip..skip + len])
            .map_err(|_not_utf8| "the certificate in the bundle is not UTF-8")?
            .to_string();
        let rest = &rest[skip + len..];
        let (skip, len) = take(rest)?;
        if rest.len() != skip + len {
            return Err("trailing bytes after the update bundle".to_string());
        }
        Ok(UpdateBundle {
            certificate,
            new_artifact: rest[skip..].to_vec(),
        })
    }
}

/// The real checker: the update certificate re-decided with Z3, against the
/// library the process is running now.
#[derive(Debug, Clone)]
pub struct Z3Admission {
    current: Vec<u8>,
    options: CheckOptions,
}

impl Z3Admission {
    /// An admission check for a process running the library `current`.
    pub fn new(current: Vec<u8>) -> Z3Admission {
        Z3Admission {
            current,
            options: CheckOptions::default(),
        }
    }

    pub fn with_options(mut self, options: CheckOptions) -> Z3Admission {
        self.options = options;
        self
    }

    /// The same check for the process after a swap committed: the next
    /// update is proven against `new_current`, the library now running.
    pub fn committed(self, new_current: Vec<u8>) -> Z3Admission {
        Z3Admission {
            current: new_current,
            options: self.options,
        }
    }
}

impl UpdateAdmission for Z3Admission {
    fn check(&self, update: &[u8]) -> Admission {
        let bundle = match UpdateBundle::unpack(update) {
            Ok(bundle) => bundle,
            Err(error) => return Admission::Rejected(vec![error]),
        };
        let certificate = match UpdateCertificate::from_json(&bundle.certificate) {
            Ok(certificate) => certificate,
            Err(error) => return Admission::Rejected(vec![error]),
        };
        let report = check(
            &certificate,
            &self.current,
            &bundle.new_artifact,
            &self.options,
        );
        if report.passed() {
            Admission::Admitted(Box::new(report))
        } else {
            let mut reasons = report.failures;
            reasons.extend(report.rejections);
            Admission::Rejected(reasons)
        }
    }
}

/// A scripted [`UpdateAdmission`] for runtime tests: it returns what it was
/// told to, in order (the last answer repeats), and remembers the bytes it
/// was asked about.
#[derive(Debug)]
pub struct TestAdmission {
    script: Mutex<VecDeque<Admission>>,
    last: Mutex<Admission>,
    received: Mutex<Vec<Vec<u8>>>,
}

impl TestAdmission {
    fn with(first: Admission) -> TestAdmission {
        TestAdmission {
            script: Mutex::new(VecDeque::new()),
            last: Mutex::new(first),
            received: Mutex::new(Vec::new()),
        }
    }

    /// Admits every update, reporting `affected` as the set of functions
    /// that must not be active.
    pub fn admitting(affected: AffectedSet) -> TestAdmission {
        TestAdmission::with(Admission::Admitted(Box::new(Report {
            affected,
            ..Report::default()
        })))
    }

    /// Rejects every update for these reasons.
    pub fn rejecting(reasons: &[&str]) -> TestAdmission {
        TestAdmission::with(Admission::Rejected(
            reasons.iter().map(ToString::to_string).collect(),
        ))
    }

    /// Answers `admission` to the next call that has no earlier answer queued.
    pub fn then(self, admission: Admission) -> TestAdmission {
        self.script
            .lock()
            .expect("test double lock")
            .push_back(admission);
        self
    }

    /// The updates it was asked about, in order.
    pub fn received(&self) -> Vec<Vec<u8>> {
        self.received.lock().expect("test double lock").clone()
    }
}

impl UpdateAdmission for TestAdmission {
    fn check(&self, update: &[u8]) -> Admission {
        self.received
            .lock()
            .expect("test double lock")
            .push(update.to_vec());
        let mut script = self.script.lock().expect("test double lock");
        let mut last = self.last.lock().expect("test double lock");
        if let Some(next) = script.pop_front() {
            *last = next;
        }
        last.clone()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::affected::{AffectedNode, Reason};

    #[test]
    fn a_bundle_round_trips_and_malformed_bytes_are_refused() {
        let bundle = UpdateBundle {
            certificate: "{\"a\": 1}".to_string(),
            new_artifact: vec![1, 2, 3, 0, 255],
        };
        let bytes = bundle.pack();
        assert_eq!(UpdateBundle::unpack(&bytes).unwrap(), bundle);
        assert!(UpdateBundle::unpack(b"").is_err());
        assert!(UpdateBundle::unpack(&bytes[..bytes.len() - 1]).is_err());
        let mut longer = bytes.clone();
        longer.push(0);
        assert!(UpdateBundle::unpack(&longer).is_err());
        let mut corrupt = bytes;
        corrupt[0] ^= 1;
        assert!(UpdateBundle::unpack(&corrupt).is_err());
    }

    #[test]
    fn the_real_checker_rejects_what_is_not_an_update() {
        let admission = Z3Admission::new(vec![0; 4]);
        let Admission::Rejected(reasons) = admission.check(b"garbage") else {
            panic!("garbage must be rejected")
        };
        assert!(reasons[0].contains("bundle"), "{reasons:?}");
        let not_a_certificate = UpdateBundle {
            certificate: "{}".to_string(),
            new_artifact: vec![],
        }
        .pack();
        let Admission::Rejected(reasons) = admission.check(&not_a_certificate) else {
            panic!("an empty certificate must be rejected")
        };
        assert!(reasons[0].contains("certificate"), "{reasons:?}");
    }

    #[test]
    fn the_test_double_answers_in_order_and_remembers_what_it_saw() {
        let affected = AffectedSet {
            functions: vec![AffectedNode {
                id: "tick".into(),
                reasons: vec![Reason::Changed],
            }],
            ..AffectedSet::default()
        };
        let double =
            TestAdmission::admitting(affected.clone()).then(Admission::Rejected(vec!["no".into()]));
        // The queued answer comes first; then the last one repeats.
        assert_eq!(double.check(b"a"), Admission::Rejected(vec!["no".into()]));
        assert_eq!(double.check(b"b"), Admission::Rejected(vec!["no".into()]));
        assert_eq!(double.received(), [b"a".to_vec(), b"b".to_vec()]);

        let double = TestAdmission::admitting(affected);
        let Admission::Admitted(report) = double.check(b"x") else {
            panic!("scripted to admit")
        };
        assert!(report.passed());
        assert_eq!(report.affected.blockers(&["tick", "main"], false), ["tick"]);
        assert!(TestAdmission::rejecting(&["because"]).check(b"x") != Admission::Rejected(vec![]));
    }
}
