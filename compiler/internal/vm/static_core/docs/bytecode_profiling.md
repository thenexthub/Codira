# Bytecode profiling in Dynamic languages

## Support in ISA

Each bytecode instruction, that supports profiling, contains additional field - profile index. It denotes index of
a corresponding profile data in the profile vector.

Currently, it has 2 bytes size, but we can reduce it for small methods, where size of a profiling vector will be less
than 256 bytes.

## Support in Runtime

For each method, that contains instructions with profiling data, Runtime should allocate profiling vector.

Profiling vector is a plain data of bytes, which content is determined by the language of a given bytecode. Core part
of Runtime knows nothing about how to interpret the profiling vector.

Profile elements are sorted in descending order, from largest Profiles to smallest. It avoids alignment issues.

Each interpreter handler reads profiling field from bytecode instruction and update corresponding slot in profiling
vector.

## Support in AOT compilation

Profile information, gathered during run, can be saved into special profile file, that can be specified by option
`--profile-ouptut`. After VM execution is finished, all profile information will be saved into this file.

Format of a profile file is determined by the language plugin.

AOT compiler accesses this file via runtime interface. CoreVM doesn't implement profiling interface, it must be
implemented by all language plugins, that supports profiling.
