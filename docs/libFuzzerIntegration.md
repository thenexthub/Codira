# libFuzzer Integration

Codira has built-in `libFuzzer` integration. In order to use it on a file
`myfile.code`, define an entry point fuzzing function with a
`@_cdecl("LLVMFuzzerTestOneInput")` annotation:

```language
@_cdecl("LLVMFuzzerTestOneInput")
public fn test(_ start: UnsafeRawPointer, _ count: Int) -> CInt {
  let bytes = UnsafeRawBufferPointer(start: start, count: count)
  // TODO: Test the code using the provided bytes.
  return 0
}
```

To compile it, use the `-sanitize=fuzzer` flag to link `libFuzzer`
and enable code coverage information; and the `-parse-as-library` flag
to omit the `main` symbol, so that the fuzzer entry point can be used:

```bash
% languagec -sanitize=fuzzer -parse-as-library myfile.code
```

`libFuzzer` can be combined with other sanitizers:

```bash
% languagec -sanitize=fuzzer,address -parse-as-library myfile.code
```

Finally, launch the fuzzing process:

```bash
% ./myfile
```

Refer to the official `libFuzzer` documentation at
<https://toolchain.org/docs/LibFuzzer.html#options>
for a description of the fuzzer's command line options.
