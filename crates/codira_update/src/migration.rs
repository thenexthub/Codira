//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The heap-migration part of an update: how each changed field of a
//!   layout-changed type carries its live values over, whether that is total,
//!   and for a field that narrows, the exact class of live values that does
//!   *not* convert -- which the runtime checks for, and rejects the swap on,
//!   before it changes anything. The checker recomputes every verdict itself
//!   from the two field types; it does not take the certificate's word.
//!
//! # What the runtime does (`codira_memory::diff`, `codira_memory::cast`)
//!
//! Fields are matched by name (or by `@renamed(from: ..)`). A field present
//! only in the new type starts zeroed; a field present only in the old type
//! is dropped with its values; a field that keeps its name and changes its
//! primitive type is converted, never reinterpreted, and an integer that
//! does not fit the new type becomes the nearest value that does
//! (saturation -- `300u16` to `127i8`, `-1i32` to `0u32`).
//!
//! # What is proven
//!
//! For integer-to-integer retyping (up to 64 bits) a conversion is *exact*
//! when the saturated value equals the old one. Exactness is an interval of
//! the old type; the obligation proves, for every value of the old type, that
//! the interval the certificate states is precisely the exact class
//! (`guard(v) <=> new_min <= v <= new_max`, over a 128-bit view of `v`). When
//! the old type's range lies inside the new one the guard is everything and
//! the migration is *total*: the query proves no old value fails. Otherwise
//! it is *guarded*: a runtime that has the guard rejects the update before
//! the swap if any live value lies outside it -- or, when the intent says
//! `migrate = "saturating"`, the author accepts the clamping and the swap
//! proceeds.
//!
//! Not modelled, and reported as such: a retype involving floats, `bool` or
//! 128-bit integers, and any change that is not a primitive retype (a field
//! becoming a different struct, a layout change inside a nested value
//! struct). `isize`/`usize` are 64 bits (the host's width). A field whose
//! values are lost (removed) needs `drop = true` in the intent, and a type
//! removed outright is rejected: live objects of it have nowhere to go.

use codira_certificate::Query;
use codira_smt::Context;
use serde_derive::{Deserialize, Serialize};

use crate::intent::MigrationPolicy;

/// What happened to a field between the versions.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum FieldChange {
    Added,
    Removed,
    Retyped,
}

/// How a field's live values come across.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(tag = "outcome", rename_all = "snake_case")]
pub enum MigrationOutcome {
    /// Every live value converts exactly.
    Total,
    /// Values in `lo..=hi` convert exactly; the runtime must reject the swap
    /// before it starts if a live value lies outside.
    Guarded { lo: String, hi: String },
    /// As `Guarded`, but the author accepted the runtime's clamping of the
    /// values outside.
    Saturating { lo: String, hi: String },
    /// A new field: starts zeroed.
    ZeroInit,
    /// A removed field, which the author said to drop.
    Discards,
    /// The migration is not modelled.
    Unsupported { reason: String },
}

/// One changed field of a layout-changed type.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct MigrationRecord {
    pub type_name: String,
    pub field: String,
    pub change: FieldChange,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub old_ty: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub new_ty: Option<String>,
    pub policy: MigrationPolicy,
    pub outcome: MigrationOutcome,
    /// The obligation that proves the guard, for a retyped field.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub obligation: Option<String>,
}

impl MigrationRecord {
    /// `Type.field`.
    pub fn subject(&self) -> String {
        format!("{}.{}", self.type_name, self.field)
    }

    /// Whether the runtime may swap given this record, assuming the guard (if
    /// any) holds of the live heap.
    pub fn admissible(&self) -> bool {
        !matches!(self.outcome, MigrationOutcome::Unsupported { .. })
    }
}

/// A primitive integer type of at most 64 bits.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct IntType {
    pub signed: bool,
    pub bits: u32,
}

impl IntType {
    /// Parses `i8` .. `i64`, `u8` .. `u64`, `isize`, `usize`. 128-bit types
    /// are not modelled.
    pub fn parse(name: &str) -> Option<IntType> {
        let (signed, rest) = match name.as_bytes().first()? {
            b'i' => (true, &name[1..]),
            b'u' => (false, &name[1..]),
            _ => return None,
        };
        let bits = match rest {
            "8" => 8,
            "16" => 16,
            "32" => 32,
            "64" | "size" => 64,
            _ => return None,
        };
        Some(IntType { signed, bits })
    }

    pub fn min(self) -> i128 {
        if self.signed {
            -(1i128 << (self.bits - 1))
        } else {
            0
        }
    }

    pub fn max(self) -> i128 {
        if self.signed {
            (1i128 << (self.bits - 1)) - 1
        } else {
            (1i128 << self.bits) - 1
        }
    }
}

/// The values of `old` that convert exactly to `new`, and whether that is all
/// of them.
pub fn exact_interval(old: IntType, new: IntType) -> (i128, i128, bool) {
    let lo = old.min().max(new.min());
    let hi = old.max().min(new.max());
    (lo, hi, lo == old.min() && hi == old.max())
}

const WIDE: u32 = 128;

/// The query that proves the verdict for `old -> new` (see the module docs):
/// premise `true`, negated goal, `unsat` when the verdict is right.
pub fn obligation_query(old: IntType, new: IntType) -> Query {
    let ctx = Context::new();
    let value = ctx.bv_var("old_value", old.bits);
    let wide = value.resize(WIDE, old.signed);
    let lit = |x: i128| ctx.bv_lit(x as u128, WIDE);
    let exact = lit(new.min()).sle(wide).and(wide.sle(lit(new.max())));
    let (lo, hi, total) = exact_interval(old, new);
    let goal = if total {
        exact
    } else {
        lit(lo).sle(wide).and(wide.sle(lit(hi))).iff(exact)
    };
    Query {
        smtlib2: ctx.to_smtlib2(&[ctx.bool_true()], goal.not()),
        expected: "unsat".to_string(),
    }
}

/// The outcome the checker expects for a retyped field, from the types and
/// the intent alone.
pub fn expected_outcome(old_ty: &str, new_ty: &str, policy: MigrationPolicy) -> MigrationOutcome {
    let (Some(old), Some(new)) = (IntType::parse(old_ty), IntType::parse(new_ty)) else {
        return MigrationOutcome::Unsupported {
            reason: format!(
                "retyping `{old_ty}` to `{new_ty}` is not modelled (integers of at most 64 bits \
                 only)"
            ),
        };
    };
    let (lo, hi, total) = exact_interval(old, new);
    if total {
        MigrationOutcome::Total
    } else if policy == MigrationPolicy::Saturating {
        MigrationOutcome::Saturating {
            lo: lo.to_string(),
            hi: hi.to_string(),
        }
    } else {
        MigrationOutcome::Guarded {
            lo: lo.to_string(),
            hi: hi.to_string(),
        }
    }
}

/// Whether a live value `v` of the old type survives the migration of a
/// field with this outcome (for a runtime scanning the heap).
pub fn value_survives(outcome: &MigrationOutcome, v: i128) -> bool {
    match outcome {
        MigrationOutcome::Guarded { lo, hi } | MigrationOutcome::Saturating { lo, hi } => {
            match (lo.parse::<i128>(), hi.parse::<i128>()) {
                (Ok(lo), Ok(hi)) => lo <= v && v <= hi,
                _ => false,
            }
        }
        MigrationOutcome::Total | MigrationOutcome::ZeroInit | MigrationOutcome::Discards => true,
        MigrationOutcome::Unsupported { .. } => false,
    }
}

/// Re-derives a record's verdict from its types and policy; `Err` says what
/// disagrees. Used by the checker, which does not trust the record.
pub fn verify_record(record: &MigrationRecord) -> Result<(), String> {
    match record.change {
        FieldChange::Added => {
            if record.outcome != MigrationOutcome::ZeroInit {
                return Err(format!(
                    "{}: a new field starts zeroed, the record says {:?}",
                    record.subject(),
                    record.outcome
                ));
            }
        }
        FieldChange::Removed => {
            if !matches!(
                record.outcome,
                MigrationOutcome::Discards | MigrationOutcome::Unsupported { .. }
            ) {
                return Err(format!(
                    "{}: a removed field is discarded, the record says {:?}",
                    record.subject(),
                    record.outcome
                ));
            }
        }
        FieldChange::Retyped => {
            let (Some(old), Some(new)) = (&record.old_ty, &record.new_ty) else {
                return Err(format!(
                    "{}: a retyped field needs both types",
                    record.subject()
                ));
            };
            let expected = expected_outcome(old, new, record.policy);
            if expected != record.outcome {
                return Err(format!(
                    "{}: retyping `{old}` to `{new}` is {expected:?}, the record says {:?}",
                    record.subject(),
                    record.outcome
                ));
            }
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use codira_certificate::{recheck, Recheck};

    use super::*;

    fn ty(name: &str) -> IntType {
        IntType::parse(name).unwrap()
    }

    fn proves(old: &str, new: &str) -> bool {
        let query = obligation_query(ty(old), ty(new));
        matches!(
            recheck(&query.smtlib2, 5_000_000),
            Ok(Recheck::Unsat { vacuous: false })
        )
    }

    #[test]
    fn widening_is_total_and_narrowing_has_an_exact_guard() {
        for (old, new, total) in [
            ("i32", "i64", true),
            ("u8", "i16", true),
            ("u32", "u64", true),
            ("i8", "i8", true),
            ("i64", "i32", false),
            ("u16", "i8", false),
            ("i32", "u32", false),
            ("u64", "i64", false),
            ("usize", "u32", false),
        ] {
            let (lo, hi, is_total) = exact_interval(ty(old), ty(new));
            assert_eq!(is_total, total, "{old} -> {new}");
            assert!(lo <= hi);
            assert!(proves(old, new), "{old} -> {new}: the verdict must prove");
        }
        // The interval is exact: u16 -> i8 keeps 0..=127.
        assert_eq!(exact_interval(ty("u16"), ty("i8")).0, 0);
        assert_eq!(exact_interval(ty("u16"), ty("i8")).1, 127);
        assert_eq!(exact_interval(ty("i32"), ty("u32")).0, 0);
    }

    #[test]
    fn a_wrong_guard_does_not_prove() {
        // The guard for i32 -> i8 is -128..=127; one too wide is refuted.
        let ctx = Context::new();
        let old = ty("i32");
        let value = ctx.bv_var("old_value", old.bits);
        let wide = value.resize(WIDE, true);
        let lit = |x: i128| ctx.bv_lit(x as u128, WIDE);
        let exact = lit(-128).sle(wide).and(wide.sle(lit(127)));
        let too_wide = lit(-128).sle(wide).and(wide.sle(lit(128)));
        let text = ctx.to_smtlib2(&[ctx.bool_true()], too_wide.iff(exact).not());
        assert!(matches!(recheck(&text, 5_000_000), Ok(Recheck::Sat)));
    }

    #[test]
    fn the_checker_rederives_every_verdict() {
        let record = |old: &str, new: &str, policy, outcome| MigrationRecord {
            type_name: "Counter".into(),
            field: "n".into(),
            change: FieldChange::Retyped,
            old_ty: Some(old.into()),
            new_ty: Some(new.into()),
            policy,
            outcome,
            obligation: None,
        };
        let honest = record(
            "i64",
            "i32",
            MigrationPolicy::Reject,
            MigrationOutcome::Guarded {
                lo: "-2147483648".into(),
                hi: "2147483647".into(),
            },
        );
        assert_eq!(verify_record(&honest), Ok(()));
        // Claiming totality for a narrowing is caught.
        let lying = record(
            "i64",
            "i32",
            MigrationPolicy::Reject,
            MigrationOutcome::Total,
        );
        assert!(verify_record(&lying).unwrap_err().contains("Guarded"));
        // A widening that claims a guard is caught as well.
        let odd = record(
            "i32",
            "i64",
            MigrationPolicy::Reject,
            MigrationOutcome::Guarded {
                lo: "0".into(),
                hi: "1".into(),
            },
        );
        assert!(verify_record(&odd).is_err());
        // Saturating is only a policy for a narrowing.
        let sat = record(
            "i64",
            "i32",
            MigrationPolicy::Saturating,
            MigrationOutcome::Saturating {
                lo: "-2147483648".into(),
                hi: "2147483647".into(),
            },
        );
        assert_eq!(verify_record(&sat), Ok(()));
        let float = record(
            "f64",
            "i32",
            MigrationPolicy::Reject,
            MigrationOutcome::Total,
        );
        assert!(verify_record(&float).is_err());
    }

    #[test]
    fn a_live_value_is_checked_against_the_guard() {
        let guard = expected_outcome("i64", "i8", MigrationPolicy::Reject);
        assert!(value_survives(&guard, 127));
        assert!(value_survives(&guard, -128));
        assert!(!value_survives(&guard, 128));
        assert!(!value_survives(&guard, i128::from(i64::MIN)));
        assert!(value_survives(&MigrationOutcome::Total, i128::MAX));
    }
}
