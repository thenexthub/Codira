This directory holds the Codira Standard Library's Glibc Module, comprised of

- The *overlay* library, which amends some APIs imported from Clang module.
- The clang [module map] which specifies which headers need to be imported
  from Glibc for bare minimum functionality.

Note: Darwin platforms provide their own overlays and module maps in the
system SDK.

[module map]: http://clang.toolchain.org/docs/Modules.html
