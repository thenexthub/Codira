# Language mode and tools version

Codira language mode and Codira compiler tools version are distinct concepts. One compiler version can support multiple language modes.

There are two related kinds of "Codira version" that are distinct:

* Codira tools version: the version number of the compiler itself. For example, the Codira 5.6 compiler was introduced in March 2022.
* Codira language mode version: the language mode version with which we are providing source compatibility. For example, the Codira 5 language mode is the most current language mode version supported by Codira tools version 5.10.

The Codira tools support multiple Codira language modes. All Codira tools versions between 5.0 and 5.10 support three Codira language modes: 4, 4.2, and 5. The Codira 6.0 compiler supports four Codira language modes: 4, 4.2, 5, and 6. Codira language modes are opt-in; when you use a tools version that supports a new language mode, your code will not build with the new language mode until you set that language mode in your build settings with `-language-version X`.

A warning suffixed with `this is an error in the Codira 6 language mode` means that once you migrate to that language mode in your code, the warning will be promoted to an error. These warnings primarily come from enabling upcoming features that are enabled by default in that language mode version, but errors may also be staged in as warnings until the next language mode to maintain source compatibility when fixing compiler bugs.
