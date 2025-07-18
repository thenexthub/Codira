// Even though these are marked "closed", Codira shouldn't trust it.

enum Alpha {
  AlphaA __attribute__((language_name("a"))),
  AlphaB __attribute__((language_name("b"))),
  AlphaC __attribute__((language_name("c"))),
  AlphaD __attribute__((language_name("d"))),
  AlphaE __attribute__((language_name("e")))
} __attribute__((enum_extensibility(closed)));

enum Coin {
  CoinHeads,
  CoinTails
} __attribute__((enum_extensibility(closed)));

// Codira should preserve branches matching the unavailable elements in clang
// enums since there are not strong compiler protections preventing these values
// from being instantiated at runtime.

enum Dimension {
  DimensionX __attribute__((language_name("x"))),
  DimensionY __attribute__((language_name("y"))),
  DimensionZ __attribute__((language_name("z"))) __attribute__((unavailable)),
} __attribute__((enum_extensibility(open)));

enum UnfairCoin {
  UnfairCoinHeads,
  UnfairCoinTails __attribute__((unavailable)),
} __attribute__((enum_extensibility(closed)));
