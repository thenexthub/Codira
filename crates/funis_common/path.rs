//! Lexical path manipulation, which doesn't consult the file system.

/// Normalizes a path lexically, resolving `.` and `..` components without
/// consulting the file system.
pub fn path_clean(path: &str) -> String {
    clean_path(std::path::Path::new(path)).to_string_lossy().into_owned()
}

/// The bytes of a path, as the file system and Mach-O load commands
/// hold them.
pub fn path_bytes(path: &std::path::Path) -> &[u8] {
    path.as_os_str().as_encoded_bytes()
}

/// Normalizes an OS path without resolving symlinks.
pub fn clean_path(path: &std::path::Path) -> std::path::PathBuf {
    use std::path::{Component, PathBuf};
    let mut out = PathBuf::new();
    for component in path.components() {
        match component {
            Component::CurDir => {}
            Component::ParentDir => match out.components().next_back() {
                Some(Component::RootDir) => {}
                None | Some(Component::ParentDir) => out.push(".."),
                _ => {
                    out.pop();
                }
            },
            other => out.push(other),
        }
    }
    if out.as_os_str().is_empty() { PathBuf::from(".") } else { out }
}

#[cfg(test)]
mod tests {
    use super::*;

    // Windows spells the separator with a backslash; the cases are written with slashes.
    fn clean(path: &str) -> String {
        path_clean(path).replace(std::path::MAIN_SEPARATOR, "/")
    }

    #[test]
    fn clean_paths() {
        assert_eq!(clean("a/./b/../c"), "a/c");
        assert_eq!(clean("/a/../.."), "/");
        assert_eq!(clean("../a"), "../a");
        assert_eq!(clean("../../a/b"), "../../a/b");
        assert_eq!(clean("a/../../b"), "../b");
        assert_eq!(clean("a/b/../../../c"), "../c");
        assert_eq!(clean(".."), "..");
        assert_eq!(clean("/.."), "/");
    }
}
