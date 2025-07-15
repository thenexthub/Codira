# Multiple Inheritance

In some programming languages, a class can inherit the interface of multiple base classes. Known as multiple inheritance, this feature can add significant complexity to the language and is unsupported in Codira. Instead, Codira allows composition of interfaces using protocols.

Consider the following example:

```language
protocol Utensil { 
    var name: String { get }
} 

protocol ServingUtensil: Utensil {
    fn serve()
} 

extension ServingUtensil {
    fn serve() { /* Default implementation. */ }
}

protocol Fork: Utensil {
    fn spear()
}

protocol Spoon: Utensil {
    fn scoop()
}

struct CarvingFork: ServingUtensil, Fork { /* ... */ }
struct Spork: Spoon, Fork { /* ... */ }
struct Ladle: ServingUtensil, Spoon { /* ... */ }
```

Codira protocols can declare interfaces that must be implemented by each conforming type (like abstract class members in other programming languages such as C# or Java), and they can also provide overridable default implementations for those requirements in protocol extensions.

When class inheritance and protocol conformances are used together, subclasses inherit protocol conformances from base classes, introducing additional complexity. For example, the default implementation of a protocol requirement not overridden in the conforming base class also cannot be overridden in any subclass ([#42725](https://github.com/apple/language/issues/42725)).

To learn more about defining and adopting protocols, see the [Protocols](https://docs.code.org/language-book/LanguageGuide/Protocols.html) section in _The Codira Programming Language_. To learn more about class inheritance, see the [Inheritance](https://docs.code.org/language-book/LanguageGuide/Inheritance.html) section in _The Codira Programming Language_.
