// The Swift Programming Language
// https://docs.swift.org/swift-book

/// A macro that adds a `copy()` method into a type bearing the annotation. For example:
///
///     @Copyable
///     struct User {
///         let name: String
///         let age: Int
///     }
///
/// updates the struct as follows:
///
///     struct User {
///         let name: String
///         let age: Int
///
///         func copy(name: String? = nil, age: Int? = nil) -> Self {
///             .init(
///                 name: name ?? self.name
///                 age: age ?? self.age
///             )
///         }
///     }
@attached(member, names: named(copy))
public macro Copyable() = #externalMacro(module: "CopyableMacroMacros", type: "CopyableMacroMacro")
