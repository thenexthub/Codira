//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Measures the link step alone, `codira_image` against LLD running
//!   in-process, on object files collected with `CODIRA_DUMP_OBJECTS`. Run it
//!   with `cargo run --release -p codira_codegen --example image_bench --
//!   <directory> [runs]`.
//! - Every object in the directory is linked `runs` times by each linker,
//!   interleaved (the machine is never idle, so single runs mean nothing), and
//!   the minimum and median are printed. Three variants are timed: the image
//!   writer producing bytes in memory, the same plus writing the file, and LLD
//!   in-process writing the file, which is what the compiler does without the
//!   image writer.
#[cfg(not(windows))]
fn main() {
    eprintln!("image_bench measures LLD in-process, which this compiler links only on Windows");
}

#[cfg(windows)]
fn main() {
    imp::run();
}

#[cfg(windows)]
mod imp {
    use std::{
        path::{Path, PathBuf},
        time::{Duration, Instant},
    };

    use codira_image::{link_windows_dll, DllRequest, Outcome};
    use codira_lld::LldFlavor;

    fn summarize(label: &str, mut samples: Vec<Duration>) {
        samples.sort();
        let min = samples[0];
        let median = samples[samples.len() / 2];
        println!(
            "  {label:<34} min {:>9.1} us   median {:>9.1} us",
            min.as_secs_f64() * 1e6,
            median.as_secs_f64() * 1e6
        );
    }

    pub fn run() {
        // Referencing the compiler's library is what makes the example link the
        // LLVM libraries that LLD's static libraries need.
        assert!(
            codira_codegen::in_process_lld_available(),
            "the compiler was built without in-process LLD"
        );
        let mut args = std::env::args().skip(1);
        let dir = PathBuf::from(args.next().expect("a directory of .obj files"));
        let runs: usize = args.next().and_then(|s| s.parse().ok()).unwrap_or(31);

        let mut objects: Vec<PathBuf> = std::fs::read_dir(&dir)
            .expect("readable directory")
            .filter_map(|e| e.ok().map(|e| e.path()))
            .filter(|p| p.extension().is_some_and(|e| e == "obj"))
            .collect();
        objects.sort();

        let scratch = tempfile::tempdir().expect("scratch directory");
        for object_path in objects {
            let object = std::fs::read(&object_path).expect("readable object");
            let extra =
                std::fs::read_to_string(object_path.with_extension("exports")).unwrap_or_default();
            let mut exports: Vec<String> = ["get_info", "get_version", "set_allocator_handle"]
                .iter()
                .map(|s| (*s).to_owned())
                .collect();
            exports.extend(extra.lines().filter(|l| !l.is_empty()).map(str::to_owned));
            exports.sort();
            exports.dedup();
            let export_refs: Vec<&str> = exports.iter().map(String::as_str).collect();

            let out_direct = scratch.path().join("direct.codiralib");
            let out_lld = scratch.path().join("lld.codiralib");
            let lld_args = |out: &Path| -> Vec<String> {
                let mut a = vec![
                    object_path.display().to_string(),
                    "/DLL".into(),
                    "/NOENTRY".into(),
                    format!("/winsysroot:{}", out.with_extension("no-sysroot").display()),
                ];
                a.extend(exports.iter().map(|e| format!("/EXPORT:{e}")));
                a.push("/NOIMPLIB".into());
                a.push(format!("/OUT:{}", out.display()));
                a
            };

            let request = DllRequest {
                object: &object,
                exports: &export_refs,
                dll_name: "bench.codiralib",
                debug_info: false,
                has_native_code: false,
            };
            // Warm-up, and a check that the writer takes this object.
            let Outcome::Image(image) = link_windows_dll(&request) else {
                println!(
                    "{}: declined by the image writer, skipped",
                    object_path.display()
                );
                continue;
            };
            codira_lld::link(LldFlavor::Coff, &lld_args(&out_lld))
                .ok()
                .expect("LLD links the object");
            let lld_size = std::fs::metadata(&out_lld).map_or(0, |m| m.len());

            let (mut memory, mut file, mut lld) = (Vec::new(), Vec::new(), Vec::new());
            for _ in 0..runs {
                let t = Instant::now();
                let outcome = link_windows_dll(&request);
                memory.push(t.elapsed());
                drop(outcome);

                let t = Instant::now();
                if let Outcome::Image(bytes) = link_windows_dll(&request) {
                    std::fs::write(&out_direct, &bytes).expect("writable output");
                }
                file.push(t.elapsed());

                let t = Instant::now();
                let result = codira_lld::link(LldFlavor::Coff, &lld_args(&out_lld));
                lld.push(t.elapsed());
                result.ok().expect("LLD links the object");
            }
            println!(
                "{} ({} bytes of object, image {} bytes, LLD image {} bytes, {} runs)",
                object_path.file_name().unwrap().to_string_lossy(),
                object.len(),
                image.len(),
                lld_size,
                runs
            );
            summarize("codira_image, bytes in memory", memory);
            summarize("codira_image, written to a file", file);
            summarize("LLD in-process, written to a file", lld);
        }
    }
}
