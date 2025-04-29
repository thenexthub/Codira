#if swift(>=5.9)
@freestanding(expression)
public macro info(_ closure: () -> Void) = #externalMacro(
    module: "TemporaryVariablePlugin",
    type: "InfoMacro"
)

@freestanding(expression)
public macro info<T>(_ closure: () -> T) -> T = #externalMacro(
    module: "TemporaryVariablePlugin",
    type: "InfoMacro"
)
#endif

public func info(_ closure: () -> Void) {
    closure()
}

public func info<T>(_ closure: () -> T) -> T {
    return closure()
}
