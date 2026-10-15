//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Reads a workload's `manifest.toml` (the labelled version history) and the
//!   source files of each version directory.

use std::{
    collections::BTreeMap,
    path::{Path, PathBuf},
};

use anyhow::{anyhow, Context, Result};

/// One update of a history: `from` to `to`, with the labels the authoring
/// script recorded.
#[derive(Clone, Debug)]
pub struct UpdateRecord {
    /// Stable identifier, for example `kv-u05`.
    pub id: String,
    /// Directory name of the old version, for example `v05`.
    pub from: String,
    /// Directory name of the new version.
    pub to: String,
    /// bug-fix, refactor, performance, feature, regression, contract-change
    /// or effect-change.
    pub category: String,
    /// One-line description.
    pub title: String,
    /// `invented` or `pattern` (a known class of change, not a copied commit).
    pub provenance: String,
    /// How the update was derived.
    pub derivation: String,
    /// `preserved`, `changed` or `new-api`: what the author intends.
    pub behaviour: String,
    /// How a published contract changes (`none` if it does not).
    pub contract: String,
    /// How an effect summary changes (`none` if it does not).
    pub effect: String,
    /// `none`, `zero-default-ok` or `needs-migrate: ...`.
    pub migrate: String,
    /// Whether a type declaration changed.
    pub layout_change: bool,
    /// Files that differ, relative to the version directory.
    pub files: Vec<String>,
    /// Functions whose text changed, as `module.name`.
    pub changed_functions: Vec<String>,
    /// Functions that were added.
    pub added_functions: Vec<String>,
    /// Functions that were removed.
    pub removed_functions: Vec<String>,
    /// Types whose declaration changed.
    pub changed_types: Vec<String>,
    /// Types that were added.
    pub added_types: Vec<String>,
    /// Free-form notes.
    pub notes: String,
}

/// A workload's history.
#[derive(Clone, Debug)]
pub struct Manifest {
    /// Workload name (`kv`, `ledger`, `stream`).
    pub name: String,
    /// What the workload is and is not.
    pub description: String,
    /// Number of versions (updates + 1).
    pub versions: usize,
    /// Directory that holds `manifest.toml` and `vNN/`.
    pub root: PathBuf,
    /// The updates in order.
    pub updates: Vec<UpdateRecord>,
}

fn string(t: &toml::Table, key: &str) -> Result<String> {
    t.get(key)
        .and_then(toml::Value::as_str)
        .map(str::to_owned)
        .ok_or_else(|| anyhow!("manifest: missing string `{key}`"))
}

fn strings(t: &toml::Table, key: &str) -> Result<Vec<String>> {
    t.get(key)
        .and_then(toml::Value::as_array)
        .ok_or_else(|| anyhow!("manifest: missing array `{key}`"))?
        .iter()
        .map(|v| {
            v.as_str()
                .map(str::to_owned)
                .ok_or_else(|| anyhow!("manifest: `{key}` holds a non-string"))
        })
        .collect()
}

impl Manifest {
    /// Loads `<root>/manifest.toml`.
    pub fn load(root: &Path) -> Result<Manifest> {
        let path = root.join("manifest.toml");
        let text = std::fs::read_to_string(&path)
            .with_context(|| format!("reading {}", path.display()))?;
        let table: toml::Table = text
            .parse()
            .with_context(|| format!("parsing {}", path.display()))?;
        let w = table
            .get("workload")
            .and_then(toml::Value::as_table)
            .ok_or_else(|| anyhow!("manifest: no [workload] table"))?;
        let versions = w
            .get("versions")
            .and_then(toml::Value::as_integer)
            .ok_or_else(|| anyhow!("manifest: no `versions`"))? as usize;
        let mut updates = Vec::new();
        for u in table
            .get("update")
            .and_then(toml::Value::as_array)
            .ok_or_else(|| anyhow!("manifest: no [[update]] entries"))?
        {
            let u = u
                .as_table()
                .ok_or_else(|| anyhow!("manifest: bad [[update]]"))?;
            updates.push(UpdateRecord {
                id: string(u, "id")?,
                from: string(u, "from")?,
                to: string(u, "to")?,
                category: string(u, "category")?,
                title: string(u, "title")?,
                provenance: string(u, "provenance")?,
                derivation: string(u, "derivation")?,
                behaviour: string(u, "behaviour")?,
                contract: string(u, "contract")?,
                effect: string(u, "effect")?,
                migrate: string(u, "migrate")?,
                layout_change: u
                    .get("layout_change")
                    .and_then(toml::Value::as_bool)
                    .ok_or_else(|| anyhow!("manifest: missing `layout_change`"))?,
                files: strings(u, "files")?,
                changed_functions: strings(u, "changed_functions")?,
                added_functions: strings(u, "added_functions")?,
                removed_functions: strings(u, "removed_functions")?,
                changed_types: strings(u, "changed_types")?,
                added_types: strings(u, "added_types")?,
                notes: u
                    .get("notes")
                    .and_then(toml::Value::as_str)
                    .unwrap_or("")
                    .to_owned(),
            });
        }
        if updates.len() + 1 != versions {
            return Err(anyhow!(
                "manifest: {} versions but {} updates",
                versions,
                updates.len()
            ));
        }
        Ok(Manifest {
            name: string(w, "name")?,
            description: string(w, "description")?,
            versions,
            root: root.to_path_buf(),
            updates,
        })
    }

    /// Directory of version `v` (for example `v05`).
    pub fn version_dir(&self, v: &str) -> PathBuf {
        self.root.join(v)
    }

    /// Name of version number `n` (1-based): `v01`, `v02`, ...
    pub fn version_name(n: usize) -> String {
        format!("v{n:02}")
    }

    /// The update that goes from version `from`.
    pub fn update_from(&self, from: &str) -> Option<&UpdateRecord> {
        self.updates.iter().find(|u| u.from == from)
    }

    /// Reads every file of version `v` into a map from path relative to the
    /// version directory (with `/` separators) to its text.
    pub fn read_version(&self, v: &str) -> Result<BTreeMap<String, String>> {
        read_tree(&self.version_dir(v))
    }
}

/// Reads all files below `dir` into a map keyed by relative path.
pub fn read_tree(dir: &Path) -> Result<BTreeMap<String, String>> {
    fn walk(base: &Path, dir: &Path, out: &mut BTreeMap<String, String>) -> Result<()> {
        for entry in std::fs::read_dir(dir).with_context(|| format!("reading {}", dir.display()))? {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() {
                if path.file_name().is_some_and(|n| n == "target") {
                    continue;
                }
                walk(base, &path, out)?;
            } else {
                let rel = path
                    .strip_prefix(base)
                    .expect("inside base")
                    .to_string_lossy()
                    .replace('\\', "/");
                out.insert(
                    rel,
                    std::fs::read_to_string(&path)
                        .with_context(|| format!("reading {}", path.display()))?,
                );
            }
        }
        Ok(())
    }
    let mut out = BTreeMap::new();
    walk(dir, dir, &mut out)?;
    Ok(out)
}

/// The number of updates per category, in alphabetical order.
pub fn category_counts(m: &Manifest) -> Vec<(String, usize)> {
    let mut counts: BTreeMap<String, usize> = BTreeMap::new();
    for u in &m.updates {
        *counts.entry(u.category.clone()).or_default() += 1;
    }
    counts.into_iter().collect()
}
