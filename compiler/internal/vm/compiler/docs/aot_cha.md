# Usage of Class Hierarchy during AOT compilation

AOT compiler mode mainly described in [aot.md](../../docs/aot.md), please read it first.

## Class Hierarchy Matching

The class order in class path influences on resulting class hierarchy that would be available for an application. As a
result the order of files in class path and files themselves are enough to verify the class context. The order resolves
situations when several same classes were passed and only the first is considered by VM. To identify ark files the adler
checksum is used.

This class context is represented by a string which is written into AOT file and compared with runtime's one during AOT file loading.

The typical string is:
```
/mnt/scratch/build/panda/pandastdlib/arkstdlib.abc*2239629066:/mnt/scratch/build/panda/Application.abc*1345224566:
```

### Class path

AOT file compiled with class hierarchy requires the complete conformity of class path with runtime. It means that we
have to pass a class path to AOT compiler identical to class path used by runtime.

The example of usage

```
$ bin/ark_aot --boot-panda-files=$LIBDIR/lib1.abc:$LIBDIR/lib2.abc \
              --paoc-panda-files application.abc \
              --paoc-output application.an \
              --paoc-use-cha=true
$ bin/ark --boot-panda-files=$LIBDIR/lib1.abc:$LIBDIR/lib2.abc \
          --aot-files=application.an \
          application.abc \
          Application::Main
```

The wrong usage. Some classes were omitted during AOT compilation due to the fact that they are not used.

```
$ bin/ark_aot --boot-panda-files=$LIBDIR/lib1.abc \
              --paoc-panda-files application.abc \
              --paoc-output application.an \
              --paoc-use-cha=true
$ bin/ark --boot-panda-files=$LIBDIR/lib1.abc:$LIBDIR/lib2.abc \
          --aot-files=application.an \
          application.abc \
          Application::Main
[TID 0073b5] E/aot: Failed to load AoT file: Cannot use 'application.an'.
Runtime class context: '$LIBDIR/lib1.abc*2239629066:$LIBDIR/lib2.abc*2767220441:application.abc*2524155584'.
AoT class context: '$LIBDIR/lib1.abc*2239629066:application.abc*2524155584'
```

The string of class context for AOT file takes into account `--paoc-location` option.

### Boot class path
The boot class path now is a part of class context string. When boot image would be implemented the checksum of the
image should be checked during AOT file verification.

As boot class path represents the core classes of VM it should not depend on any of application code. To obtain AOT file
of boot class file the options `--paoc-boot-output` should be used. The class hierarchy verification for boot libraries
is performed during Runtime initialization when only boot panda files are loaded. The verification of .an
files for boot panda files is needed when the options `--enable-an` is used.

Since the AOT file can be compiled on one host system and executed on different target, we need an option to specify boot
files location according their paths on the target. `--paoc-boot-location` specifies the path where boot panda files are
located on the target system. It works identically to `--paoc-location` but only for boot files used during compilation.

It is important that for boot aot files the entire --boot-panda-files must match between `ark_aot` and `ark`. It means, that if you are compiling an AOT file for boot panda files, then all other entries of --boot-panda-files that will be used during execution should have been passed as well.

## Class Hierarchy Usage

### Virtual Calls

Generally, the default approach of virtual call consists of the following steps:

1) Get class from object
2) Obtain MethodPtr of the callee virtual method
3) Retrieve the index of method in virtual table
4) Load the real MethodPtr from class (obtained in step 1) using index from previous step
5) Invoke method

For JIT compiler steps 2-3-4 is merged into one, because it can obtain the index of method in virtual table during
compilation.

For AOT compilation the method and its index in virtual table is unknown because at runtime the chain of inheritance may
differ. By this reason, the compiled code has to resolve the method during execution and retrieve index from the method.
The [PLT Resolvers](./plt.md) help to avoid this resolution on each call and cache result (index inside virtual table
for each method) into `.aot_table`.

The usage of class hierarchy may help AOT compiler to squash 2-3-4 into one step as JIT-compiler does. It is possible
because the MethodPtr is needed only to obtain the virtual table index. The indices inside virtual tables are still
unchanged until the class hierarchy has changed. Therefore AOT compiler can generate code similar to JIT compiler if the
class hierarchy of runtime matches the class hierarchy during AOT compilation.
