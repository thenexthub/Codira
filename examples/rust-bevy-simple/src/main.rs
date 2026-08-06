//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
//!
use std::{env, path::PathBuf};

use bevy::prelude::*;
use codira_runtime::Runtime as CodiraRuntime;

// Minimal Bevy application that demonstrates how to insert the Codira runtime into a Bevy world object and
// utilizes the Codira runtime inside of Bevy systems.
fn main() {
    let lib_dir = PathBuf::from(env::args().nth(1).expect("Expected path to a Codira library."));

    App::new()
        .add_plugins(MinimalPlugins)
        .add_plugin(CodiraPlugin { lib_dir })
        .run();
}

// A Bevy best-practice is to build your logical separations of code into "Plugins".
// This example will build the core Codira functionality into a CodiraPlugin. This plugin is loaded into
// Bevy in the "main()" function above. Through this plugin we will load and access the various
// Codira pieces with Bevy.
struct CodiraPlugin {
    lib_dir: PathBuf,
}

impl Plugin for CodiraPlugin {
    fn build(&self, app: &mut App) {
        // A "resource" is similar to a global variable of the Bevy "world" (main application).
        // Bevy handles most systems (functions) in a parallel / multi-threaded way by default.
        // Actions such as Initializing, Updating, or Calling functions using the Codira runtime
        // can not be performed in parallel with other Bevy systems. At least not without
        // additional overhead.
        // We are using "exclusive_system()" to load the initial setup that inserts the Codira
        // runtime as a Bevy world resource, as well as other resources.
        // "exclusive_system()" is only necessary for loading the Codira runtime.
        app.insert_resource(self.lib_dir.clone())
            .add_startup_system(setup.exclusive_system())
            .add_system(print_from_codira)
            .add_system(reload_codiralib_every_frame);
    }
}

struct PrintTimer(Timer);

// Insert the Codira runtime, and associated timers as Bevy world resources.
fn setup(world: &mut World) {
    let builder = CodiraRuntime::builder(
        world
            .get_resource::<PathBuf>()
            .expect("Lib path must be added as resource"),
    );
    // We assume the Codira runtime is safe.
    let codira: CodiraRuntime = unsafe { builder.finish() }.expect("Failed to spawn Runtime");
    // Codira does not implement the send/sync trait so it needs to be inserted into Bevy as a
    // "non_send_resource".
    world.insert_non_send_resource(codira);
    world.insert_resource(PrintTimer(Timer::from_seconds(1.0, true)));

    world.remove_resource::<PathBuf>();
}

fn reload_codiralib_every_frame(
    // Since Codira was loaded with "non_send_resource" it is accessed as "NonSend" / "NonSendMut"
    // instead of "Res" / "ResMut". "NonSend" is a data type that must be accessed from Bevy's main
    // thread. This is typically reserved for data that is not safe to access in a multi-threaded
    // environment.
    mut codira: NonSendMut<CodiraRuntime>,
) {
    let _ = unsafe { codira.update() };
}

fn print_from_codira(codira: NonSend<CodiraRuntime>, time: Res<Time>, mut timer: ResMut<PrintTimer>) {
    // Call the function defined in Codira named "codira_func"
    let result: usize = codira.invoke("codira_func", ()).unwrap();
    if timer.0.tick(time.delta()).just_finished() {
        println!("Printing value from `codira_func`: {}", result);
    }
}

