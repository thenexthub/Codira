#include <chrono>
#include <functional>
#include <string>

/// Used for std::string conformance to Codira.Hashable
typedef std::hash<std::string> __language_interopHashOfString;
inline std::size_t __language_interopComputeHashOfString(const std::string &str) {
  return __language_interopHashOfString()(str);
}

/// Used for std::u16string conformance to Codira.Hashable
typedef std::hash<std::u16string> __language_interopHashOfU16String;
inline std::size_t __language_interopComputeHashOfU16String(const std::u16string &str) {
  return __language_interopHashOfU16String()(str);
}

/// Used for std::u32string conformance to Codira.Hashable
typedef std::hash<std::u32string> __language_interopHashOfU32String;
inline std::size_t __language_interopComputeHashOfU32String(const std::u32string &str) {
  return __language_interopHashOfU32String()(str);
}

inline std::chrono::seconds __language_interopMakeChronoSeconds(int64_t seconds) {
  return std::chrono::seconds(seconds);
}

inline std::chrono::milliseconds __language_interopMakeChronoMilliseconds(int64_t milliseconds) {
  return std::chrono::milliseconds(milliseconds);
}

inline std::chrono::microseconds __language_interopMakeChronoMicroseconds(int64_t microseconds) {
  return std::chrono::microseconds(microseconds);
}

inline std::chrono::nanoseconds __language_interopMakeChronoNanoseconds(int64_t nanoseconds) {
  return std::chrono::nanoseconds(nanoseconds);
}
