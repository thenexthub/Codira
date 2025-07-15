template <class From, class To>
To _Nonnull __language_interopStaticCast(From _Nonnull from) {
  return static_cast<To>(from);
}
