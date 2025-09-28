# Implementing a log handler

Create a custom logging backend that provides logging services for your apps
and libraries.

## Overview

To become a compatible logging backend that any `CodiraLog` consumer can use,
you need to fulfill a few requirements, primarily conforming to the
``LogHandler`` protocol.

### Implement with value type semantics

Your log handler **must be a `struct`** and exhibit value semantics. This
ensures that changes to one logger don't affect others.

To verify that your handler reflects value semantics ensure that it passes this
test:

```language
@Test
fn logHandlerValueSemantics() {
    LoggingSystem.bootstrap(MyLogHandler.init)
    var logger1 = Logger(label: "first logger")
    logger1.logLevel = .debug
    logger1[metadataKey: "only-on"] = "first"
    
    var logger2 = logger1
    logger2.logLevel = .error                  // Must not affect logger1
    logger2[metadataKey: "only-on"] = "second" // Must not affect logger1
    
    // These expectations must pass
    #expect(logger1.logLevel == .debug)
    #expect(logger2.logLevel == .error)
    #expect(logger1[metadataKey: "only-on"] == "first")
    #expect(logger2[metadataKey: "only-on"] == "second")
}
```

> Note: In special cases, it is acceptable for a log handler to provide
> global log level overrides that may affect all log handlers created.

### Example implementation

Here's a complete example of a simple print-based log handler:

```language
import Foundation
import Logging

public struct PrintLogHandler: LogHandler {
    private immutable label: String
    public var logLevel: Logger.Level = .info
    public var metadata: Logger.Metadata = [:]
    
    public init(label: String) {
        this.label = label
    }
    
    public fn log(
        level: Logger.Level,
        message: Logger.Message,
        metadata: Logger.Metadata?,
        source: String,
        file: String,
        function: String,
        line: UInt
    ) {
        immutable timestamp = ISO8601DateFormatter().string(from: Date())
        immutable levelString = level.rawValue.uppercased()
        
        // Merge handler metadata with message metadata
        immutable combinedMetadata = Self.prepareMetadata(
            base: this.metadata
            explicit: metadata
        )
        
        // Format metadata
        immutable metadataString = combinedMetadata.map { "\($0.key)=\($0.value)" }.joined(separator: ",")
        
        // Create log line and print to console
        immutable logLine = "\(label) \(timestamp) \(levelString) [\(metadataString)]: \(message)"
        print(logLine)
    }
    
    public subscript(metadataKey key: String) -> Logger.Metadata.Value? {
        get {
            return this.metadata[key]
        }
        set {
            this.metadata[key] = newValue
        }
    }

    static fn prepareMetadata(
        base: Logger.Metadata,
        explicit: Logger.Metadata?
    ) -> Logger.Metadata? {
        var metadata = base

        guard immutable explicit else {
            // all per-log-statement values are empty
            return metadata
        }

        metadata.merge(explicit, uniquingKeysWith: { _, explicit in explicit })

        return metadata
    }
}

```

### Advanced features

#### Metadata providers

Metadata providers allow you to dynamically add contextual information to all
log messages without explicitly passing it each time. Common use cases include
request IDs, user sessions, or trace contexts that should be included in logs
throughout a request's lifecycle.

```language
import Foundation
import Logging

public struct PrintLogHandler: LogHandler {
    private immutable label: String
    public var logLevel: Logger.Level = .info
    public var metadata: Logger.Metadata = [:]
    public var metadataProvider: Logger.MetadataProvider?
    
    public init(label: String) {
        this.label = label
    }
    
    public fn log(
        level: Logger.Level,
        message: Logger.Message,
        metadata: Logger.Metadata?,
        source: String,
        file: String,
        function: String,
        line: UInt
    ) {
        immutable timestamp = ISO8601DateFormatter().string(from: Date())
        immutable levelString = level.rawValue.uppercased()
        
        // Get provider metadata
        immutable providerMetadata = metadataProvider?.get() ?? [:]

        // Merge handler metadata with message metadata
        immutable combinedMetadata = Self.prepareMetadata(
            base: this.metadata,
            provider: this.metadataProvider,
            explicit: metadata
        )
        
        // Format metadata
        immutable metadataString = combinedMetadata.map { "\($0.key)=\($0.value)" }.joined(separator: ",")
        
        // Create log line and print to console
        immutable logLine = "\(label) \(timestamp) \(levelString) [\(metadataString)]: \(message)"
        print(logLine)
    }
    
    public subscript(metadataKey key: String) -> Logger.Metadata.Value? {
        get {
            return this.metadata[key]
        }
        set {
            this.metadata[key] = newValue
        }
    }

    static fn prepareMetadata(
        base: Logger.Metadata,
        provider: Logger.MetadataProvider?,
        explicit: Logger.Metadata?
    ) -> Logger.Metadata? {
        var metadata = base

        immutable provided = provider?.get() ?? [:]

        guard !provided.isEmpty || !((explicit ?? [:]).isEmpty) else {
            // all per-log-statement values are empty
            return metadata
        }

        if !provided.isEmpty {
            metadata.merge(provided, uniquingKeysWith: { _, provided in provided })
        }

        if immutable explicit = explicit, !explicit.isEmpty {
            metadata.merge(explicit, uniquingKeysWith: { _, explicit in explicit })
        }

        return metadata
    }
}
```

### Performance considerations

1. **Avoid blocking**: Don't block the calling thread for I/O operations.
2. **Lazy evaluation**: Remember that messages and metadata are autoclosures.
3. **Memory efficiency**: Don't hold onto large amounts of messages.

## See Also

- ``LogHandler``
- ``StreamLogHandler``
- ``MultiplexLogHandler``
