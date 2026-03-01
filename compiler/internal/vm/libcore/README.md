# bionic maintainer overview

[bionic](https://en.wikipedia.org/wiki/Bionic_(software)) is Android's
C library, math library, and dynamic linker.

This document is a high-level overview of making changes to bionic itself.
If you're trying to _use_ bionic, or want more in-depth information about
some part of the implementation, see [all the bionic documentation](docs/).

## What are the big pieces of bionic?

#### libc/ --- libc.so, libc.a

The C library. Stuff like `fopen(3)` and `kill(2)`.

#### libm/ --- libm.so, libm.a

The math library. Traditionally Unix systems kept stuff like `sin(3)` and
`cos(3)` in a separate library to save space in the days before shared
libraries.

#### libdl/ --- libdl.so

The dynamic linker interface library. This is actually just a bunch of stubs
that the dynamic linker replaces with pointers to its own implementation at
runtime. This is where stuff like `dlopen(3)` lives.

#### libstdc++/ --- libstdc++.so

The C++ ABI support functions. The C++ compiler doesn't know how to implement
thread-safe static initialization and the like, so it just calls functions that
are supplied by the system. Stuff like `__cxa_guard_acquire` and
`__cxa_pure_virtual` live here.

#### linker/ --- /system/bin/linker and /system/bin/linker64

The dynamic linker. When you run a dynamically-linked executable, its ELF file
has a `DT_INTERP` entry that says "use the following program to start me".  On
Android, that's either `linker` or `linker64` (depending on whether it's a
32-bit or 64-bit executable). It's responsible for loading the ELF executable
into memory and resolving references to symbols (so that when your code tries to
jump to `fopen(3)`, say, it lands in the right place).

#### tests/ --- unit tests

The `tests/` directory contains unit tests. Roughly arranged as one file per
publicly-exported header file. `tests/headers/` contains compile-only tests
that just check that things are _in_ the headers, whereas the "real" tests
check actual _behavior_.

#### benchmarks/ --- benchmarks

The `benchmarks/` directory contains benchmarks, with its own [documentation](benchmarks/README.md).


## What's in libc/?

```
libc/
  arch-arm/
  arch-arm64/
  arch-common/
  arch-x86/
  arch-x86_64/
    # Each architecture has its own subdirectory for stuff that isn't shared
    # because it's architecture-specific. There will be a .mk file in here that
    # drags in all the architecture-specific files.
    bionic/
      # Every architecture needs a handful of machine-specific assembler files.
      # They live here.
    string/
      # Most architectures have a handful of optional assembler files
      # implementing optimized versions of various routines. The <string.h>
      # functions are particular favorites.
    syscalls/
      # The syscalls directories contain script-generated assembler files.
      # See 'Adding system calls' later.

  include/
    # The public header files on everyone's include path. These are a mixture of
    # files written by us and files taken from BSD.

  kernel/
    # The kernel uapi header files. The "libc" headers that developers actually
    # use are a mixture of headers provided by the C library itself (which,
    # for bionic, are in bionic/libc/include/) and headers provided by the
    # kernel. This is because ISO C and POSIX will say things like "there is
    # a constant called PROT_NONE" or "there is a type called struct stat,
    # and it contains a field called st_size", but they won't necessarily say
    # what _value_ that constant has, or what _order_ the fields in a type
    # are in. Those are left to individual kernels' ABIs. In an effort to --
    # amongst other things, see https://lwn.net/Articles/507794/ for more
    # background -- reduce copy & paste, the Linux kernel makes all the types
    # and constants that make up the "userspace API" (uapi) available as
    # headers separate from their internal-use headers (which contain all kinds
    # of extra stuff that isn't available to userspace). We import the latest
    # released kernel's uapi headers in external/kernel-headers/, but we don't
    # use those headers directly in bionic. The bionic/libc/kernel/ directory
    # contains scrubbed copies of the originals from external/kernel-headers/.
    # The generate_uapi_headers.sh script should be used to go from a kernel
    # tree to external/kernel-headers/ --- this takes care of the
    # architecture-specific details. The update_all.py script should then be
    # used to regenerate bionic's copy from external/kernel-headers/.
    # The files in bionic must not be edited directly because any local changes
    # will be overwritten by the next update. "Updating kernel header files"
    # below has more information on this process.

  private/
    # These are private header files meant for use within bionic itself.

  dns/
    # Contains the DNS resolver (originates from NetBSD code).

  upstream-freebsd/
  upstream-netbsd/
  upstream-openbsd/
    # These directories contain upstream source with no local changes.
    # Any time we can just use a BSD implementation of something unmodified,
    # we should. Ideally these should probably have been three separate git
    # projects in external/, but they're here instead mostly by historical
    # accident (because it wouldn't have been easy to import just the tiny
    # subset of these operating systems that -- unlike Android -- just have
    # one huge repository rather than lots of little ones and a mechanism
    # like our `repo` tool).
    # The structure under these directories mimics the relevant upstream tree,
    # but in order to actually be able to compile this code in our tree
    # _without_ making modifications to the source files directly, we also
    # have the following subdirectories in each one that aren't upstream:
    android/
      include/
        # This is where we keep the hacks necessary to build BSD source
        # in our world. The *-compat.h files are automatically included
        # using -include, but we also provide equivalents for missing
        # header/source files needed by the BSD implementation.

  bionic/
    # This is the biggest mess. The C++ files are files we own, typically
    # because the Linux kernel interface is sufficiently different that we
    # can't use any of the BSD implementations. The C files are usually
    # legacy mess that needs to be sorted out, either by replacing it with
    # current upstream source in one of the upstream directories or by
    # switching the file to C++ and cleaning it up.

  malloc_debug/
    # The code that implements the functionality to enable debugging of
    # native allocation problems.

  stdio/
    # These are legacy files of dubious provenance. We're working to clean
    # this mess up, and this directory should disappear.

  tools/
    # Various tools used to maintain bionic.

  tzcode/
    # A modified superset of the IANA tzcode. Most of the modifications relate
    # to Android's use of a single file (with corresponding index) to contain
    # timezone data.
  zoneinfo/
    # Android-format timezone data.
    # See 'Updating tzdata' later.
```

## Adding a new function

### Should I?

The first question you should ask is "should I add this function?".
The answer is usually "no".

The answer is "yes" if the function is part of ISO C or POSIX.

The answer is "probably" if the function is in glibc and musl _and_ the BSDs
(including iOS/macOS).

The answer is "probably not" if the function differs in API or behavior between
them (ABI doesn't matter).

The answer is "maybe" if the function has three/four distinct users in
different unrelated projects, and there isn't a more specific higher-level
library that would make more sense as the place to add the function.

### How?

Adding a function usually involves:

1. Find the right header file to work in by looking up the official
   specification. The POSIX documentation on
   [opengroup.org](https://pubs.opengroup.org/onlinepubs/9799919799/functions/contents.html)
   is the canonical source (though this may lag behind the most recent ISO C).
   This documentation will also be useful in later steps such as writing tests.
1. Add constants (and perhaps types) to the appropriate header file.
   Note that you should check to see whether the constants are already in
   kernel uapi header files, in which case you just need to make sure that
   the appropriate header file in libc/include/ `#include`s the relevant
   `linux/` file or files.
1. Add function declarations to the appropriate header file. Don't forget
   to include the appropriate `__INTRODUCED_IN()`, with the right API level
   for the first release your system call wrapper will be in. See
   libc/include/android/api_level.h for the API levels.
   If the header file doesn't exist, copy all of libc/include/sys/sysinfo.h
   into your new file --- it's a good short example to start from.

   Note also our style for naming arguments: always use two leading
   underscores (so developers are free to use any of the unadorned names as
   macros without breaking things), avoid abbreviations, and ideally try to
   use the same name as an existing system call (to reduce the amount of
   English vocabulary required by people who just want to use the function
   signatures). If there's a similar function already in the C library,
   check what names it already used. Finally, prefer the `void*` orthography we
   use over the `void *` you'll see on man7.org, and don't forget to include
   nullability annotations for any pointers.
1. Add basic documentation to the header file. Again, the existing
   libc/include/sys/sysinfo.h is a good short example that shows the
   expected style.

   Most of the detail should actually be left to the man7.org page, with
   only a brief one-sentence explanation (usually based on the description
   in the NAME section of the man page) in our documentation. Always
   include the return value/error reporting details (you can find out
   what the system call returns from the RETURN VALUE of the man page),
   but try to match the wording and style wording from _our_ existing
   documentation; we're trying to minimize the number of English words
   that readers need to understand by using the exact same wording where
   possible).

   Explicitly say which version of Android the function was added to in
   the documentation because the documentation generation tool doesn't yet
   understand `__INTRODUCED_IN()`.

   Explicitly call out any Android-specific changes/additions/limitations
   because they won't be on the man7.org page.
1. Add the function name to the correct section in libc/libc.map.txt (or
   the corresponding map file for libm or libdl).
   If you don't add your function to the map, it won't be exported.
   Adding the function to the map adds an implicit build-time check that your
   function is defined for all supported architectures.

   Any new function should include an `# introduced=` matching the
   `__INTRODUCED_IN()` from the header. For several versions of Android we
   created separate sections (such as `LIBC_Q`) to make it a little more
   obvious in places like nm(1) output which version of Android is required.
   Theoretically this also gave us the option of having multiple versions of
   the same symbol with different behavior. Other OSes use this for backward
   compatibility but we tend to use API level checks within the implementation
   instead. When marketing moved back to numbers, and then when the alphabet
   wrapped, this started to look like less of a good idea. The move to "2025Q4"
   style releases made this even less convincing, so we've gone back to just
   adding new symbols unversioned (but with the `# introduced=` annotation).
1. Add a basic test. Don't try to test everything; concentrate on just testing
   the code that's actually in *bionic*, not all the functionality that's
   implemented in the kernel. At one end of the spectrum, for string routines
   that's everything; at the other, for simple syscalls, that's just the
   auto-generated argument and return value marshalling.

   Add a test in the right file in tests/. We have one file per header, so if
   your system call is exposed in <unistd.h>, for example, your test would go
   in tests/unistd_test.cpp.

   A trivial test that deliberately supplies an invalid argument helps check
   that we're generating the right symbol and have the right declaration in
   the header file, and that the change to libc.map.txt from step 5 is
   correct. (For system calls, you can use strace(1) manually to confirm that
   the correct system call is being made.)

   For testing the *kernel* side of things, we should prefer to rely on
   https://github.com/linux-test-project/ltp for kernel testing, but you'll
   want to check that external/ltp/ does contain tests for the syscall you're
   adding. Also check that external/ltp/ is using the libc wrapper for the
   syscall rather than calling it "directly" via syscall(3)!

   One common exception is for known kernel bugs where we sometimes have an
   explicit test where the failure message says what SHA to cherrypick.
   This has become less useful with the move to GKI kernels.
1. Implement the function. Similar to tests, we typically try to have one
   file per header, so a function from <sys/foo.h> would normally go in
   sys_foo.cpp. The more complicated the implementation, the more likely it
   warrants its own file.

   Trivial system call wrappers are a special case covered in the next section.

## Adding libc wrappers for system calls

### Should I?

The first question you should ask is "should I add a libc wrapper for
this system call?". The decision tree is similar to "adding a new function"
above, but somewhat stricter because for system calls you can always just use
[syscall(2)](https://man7.org/linux/man-pages/man2/syscall.2.html) instead.

### How?

Adding a system call usually involves following all the steps from
"adding a new function" above, with a couple of small differences:

1. For Linux system calls that aren't in POSIX, you can find the right header
   file to work in by looking up your system call in the system call index on
   [man7.org](https://man7.org/linux/man-pages/dir_section_2.html).
   (If there's no header file given, or an explicit "glibc provides no wrapper",
   see the points above about whether we should really be adding this or not!)
1. To implement a trivial system call wrapper -- one where a caller could just
   use syscall(3) with no extra logic, simply add an entry (or entries,
   in some cases) to SYSCALLS.TXT.

   See SYSCALLS.TXT itself for documentation on the format.

   Some trivial system call wrappers are still harder than others. The most
   common problem is a 64-bit argument such as `off64_t` (a *pointer* to a
   64-bit argument is fine, since pointers are always the "natural" size for
   the architecture regardless of the size of the thing they point to).
   Whenever you have a function that takes `off_t` or `off64_t`, you'll need to
   consider whether you actually need a foo() and a foo64(), and whether they
   will use the same underlying system call or are implemented as two different
   system calls. It's usually easiest to find a similar system call and copy
   and paste from that. You'll definitely need to test both on 32-bit and
   64-bit. (These special cases warrant more testing than the easy cases,
   even if only manual testing with strace. Sadly it isn't always feasible to
   write a working test for the interesting cases -- offsets larger than 2GiB,
   say -- so you may end up just writing a "meaningless" program whose only
   purpose is to give you patterns to look for when run under strace(1).)

  In some cases the syscall you're adding a wrapper for will be new enough
  that it's not available on all supported kernels. In that case you'll need
  to consider two extra issues:

  1. Think about what the fallback behavior should be. There are two main
     choices: fail clearly (usually with -1 and errno set to ENOSYS),
     or have a "best effort" fallback implementation. When deciding between
     these two options, always consider (a) how hard it is for us to maintain
     the other implementation, (b) how hard it is for developers to maintain
     their own implementation (bearing in mind that they won't be able to use
     anything from bionic until their target API level is high enough), and
     (c) any security implications. As an example of the last point, we didn't
     offer fallback implementations for things like accept4() with SOCK_CLOEXEC
     because there's no non-racy way to do that, meaning that a developer might
     write code that _looks_ secure but which we make insecure at runtime by
     pretending to have done what they asked for -- atomic setting of the
     SOCK_CLOEXEC flag on the returned fd -- but not actually doing it.

  1. Make sure that you've written your unit tests so they don't fail when
     run on devices with old (but still supported kernels). Search for ENOSYS
     in the tests to find existing examples of the common idiom for this.

  See https://source.android.com/docs/core/architecture/kernel/android-common#feature-and-launch-kernels
  for details of what kernels are considered supported.

  You can see a general example of adding a trivial system call wrapper here:
  https://android-review.googlesource.com/c/platform/bionic/+/2073827

### Debugging tips

If a test fails to build with an undefined symbol error,
this is most likely the _host_ reference test against glibc,
and you need to add an `#if defined(__GLIBC__)` to the test.
(Search for existing examples to copy & paste,
in particular to make sure you include the `GTEST_SKIP()`.)

When we switch to musl for the host libc, this should be less of a problem.

## Updating kernel header files

As mentioned above, this is currently a two-step process:

  1. Use generate_uapi_headers.sh to go from a Linux source tree to appropriate
     contents for external/kernel-headers/.
  2. Run update_all.py to scrub those headers and import them into bionic.

Note that if you're actually just trying to expose device-specific headers to
build your device drivers, you shouldn't modify bionic. Instead use
`TARGET_DEVICE_KERNEL_HEADERS` and friends described in [config.mk](https://android.googlesource.com/platform/build/+/main/core/config.mk#186).


## Updating tzdata

Tzdata updates are now handled by the libcore team because it needs to be
updated in sync with icu's copy of the data, and they own that.

See
[system/timezone/README.android](https://android.googlesource.com/platform/system/timezone/+/main/README.android)
for more information.


## Verifying changes

If you make a change that is likely to have a wide effect on the tree (such as a
libc header change), you should run `make checkbuild`. A regular `make` will
_not_ build the entire tree; just the minimum number of projects that are
required for the device. Tests, additional developer tools, and various other
modules will not be built. Note that `make checkbuild` will not be complete
either, as `make tests` covers a few additional modules, but generally speaking
`make checkbuild` is enough.


## Running the tests

The tests are all built from the tests/ directory. There is a separate
directory `benchmarks/` containing benchmarks, and that has its own
documentation on [running the benchmarks](benchmarks/README.md).

### Building

We assume you've already checked out the Android source tree.

To build, make sure you're in the right directory and you've set up your environment:

    $ cd main  # Or whatever you called your Android source tree.
    $ source build/envsetup.sh

Then choose an appropriate "lunch". If you're not testing on a device,
two choices are particularly useful.

If you want to be able to run tests and benchmarks directly on your x86-64
host machine, use:

    $ lunch aosp_cf_x86_64_phone-trunk_staging-userdebug

Alternatively, if you want to (say) check generated arm64 code without having
a specific device in mind, use:

    $ lunch aosp_cf_arm64_phone-trunk_staging-userdebug

Note that in both cases,
these targets will also build the corresponding 32-bit variant.
See below for where the 64-bit and 32-bit files end up.

See [Build Android](https://source.android.com/docs/setup/build/building)
for more details.

### Device tests

Once you've completed that setup, you can build:

    $ cd bionic
    $ mm

This will build everything: bionic, the benchmarks, and the tests
(and all the dependencies).

If you want to test on a device,
the first time after flashing your device,
you'll need to remount the filesystems to be writable:

    $ adb root
    $ adb remount
    $ adb reboot
    $ adb wait-for-device
    $ adb root
    $ adb remount

Then you can sync your locally built files across:

    $ adb sync

And then you can run the 32-bit tests (dynamic or static):

    $ adb shell /data/nativetest/bionic-unit-tests/bionic-unit-tests
    $ adb shell \
        /data/nativetest/bionic-unit-tests-static/bionic-unit-tests-static

Or the 64-bit tests (dynamic or static):

    $ adb shell /data/nativetest64/bionic-unit-tests/bionic-unit-tests
    $ adb shell \
        /data/nativetest64/bionic-unit-tests-static/bionic-unit-tests-static

Note that we use our own custom gtest runner that offers a superset of the
options documented at
<https://github.com/google/googletest/blob/main/docs/advanced.md#running-test-programs-advanced-options>,
in particular for test isolation and parallelism (both on by default).

### Device tests via CTS

Most of the unit tests are executed by CTS. By default, CTS runs as
a non-root user, so the unit tests must also pass when not run as root.
Some tests cannot do any useful work unless run as root. In this case,
the test should check `getuid() == 0` and do nothing otherwise (typically
we log in this case to prevent accidents!). Obviously, if the test can be
rewritten to not require root, that's an even better solution.

Currently, the list of bionic CTS tests is generated at build time by
running a host version of the test executable and dumping the list of
all tests. In order for this to continue to work, all architectures must
have the same number of tests, and the host version of the executable
must also have the same number of tests.

Running the gtests directly is orders of magnitude faster than using CTS,
but in cases where you really have to run CTS:

    $ make cts # In $ANDROID_ROOT.
    $ adb unroot # Because real CTS doesn't run as root.
    # This will sync any *test* changes, but not *code* changes:
    $ cts-tradefed \
        run singleCommand cts --skip-preconditions -m CtsBionicTestCases

### Host tests

The host tests require that you have `lunch`ed either an x86 or x86_64 target.

(Obviously, in theory you could build for arm64 and run on an arm64 host,
but we currently only support x86-64 host builds.)

For example:

    $ lunch aosp_cf_x86_64_phone-trunk_staging-userdebug

Then build as normal:

    $ cd bionic
    $ mm

Note that due to ABI limitations (specifically, the size of pthread_mutex_t),
32-bit bionic requires PIDs less than 65536.
To enforce this, set /proc/sys/kernel/pid_max to 65536.
(The tests will remind you if you forget.)

The easiest way to run is to use our provided script.

To run the 32-bit tests on the host:

    $ ./tests/run-on-host.sh 32

To run the 64-bit tests on the host:

    $ ./tests/run-on-host.sh 64

You can supply gtest flags as extra arguments to this script.

This script starts by running build/run-on-host.sh which -- despite the name --
is actually a script to set up your host to look more like an Android device.
In particular, it creates a /system directory with appropriate symlinks to your
"out" directory.

An alternative is to run the static binaries directly from your "out" directory.

To run the static 32-bit tests:

    $ ../out/target/product/vsoc_x86_64/data/nativetest/bionic-unit-tests-static/bionic-unit-tests-static

To run the static 64-bit tests:

    $ ../out/target/product/vsoc_x86_64/data/nativetest64/bionic-unit-tests-static/bionic-unit-tests-static


### Against glibc

As a way to check that our tests do in fact test the correct behavior (and not
just the behavior we think is correct), it is possible to run the tests against
the host's glibc.

    $ ./tests/run-on-host.sh glibc

### Against musl

Another way to verify test behavior is to run against musl on the host. glibc
musl don't always match, so this can be a good way to find the more complicated
corners of the spec. If they *do* match, bionic probably should too!

    $ OUT_DIR=$(ANDROID_BUILD_TOP)/musl-out ./tests/run-on-host.sh musl

Note: the alternate OUT_DIR is used to avoid causing excessive rebuilding when
switching between glibc and musl. The first musl test run will be expensive
because it will not reuse any already built artifacts, but subsequent runs will
be cheaper than if you hadn't used it.

## Gathering test coverage

To get test coverage for bionic, use `//bionic/build/coverage.sh`. Before
running, follow the instructions at the top of the file to rebuild bionic with
coverage instrumentation.

## Attaching GDB to the tests

Bionic's test runner will run each test in its own process by default to prevent
tests failures from impacting other tests. This also has the added benefit of
running them in parallel, so they are much faster.

However, this also makes it difficult to run the tests under GDB. To prevent
each test from being forked, run the tests with the flag `--no-isolate`.


## 32-bit ABI bugs

See [32-bit ABI bugs](docs/32-bit-abi.md).
