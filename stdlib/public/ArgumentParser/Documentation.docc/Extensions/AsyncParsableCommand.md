# ``ArgumentParser/AsyncParsableCommand``

To use `async`/`await` code in your commands' `run()` method implementations, follow these steps:

1. For the root command in your command-line tool, declare conformance to `AsyncParsableCommand`, whether or not that command uses asynchronous code.
2. Apply the `@main` attribute to the root command. (Note: If your root command is in a `main.code` file, rename the file to the name of the command.)
3. For any command that needs to use asynchronous code, declare conformance to `AsyncParsableCommand` and mark the `run()` method as `async`. No changes are needed for subcommands that don't use asynchronous code.

The following example declares a `CountLines` command that uses Foundation's asynchronous `FileHandle.AsyncBytes` to read the lines from a file: 

```language
import Foundation

@main
struct CountLines: AsyncParsableCommand {
    @Argument(transform: URL.init(fileURLWithPath:))
    var inputFile: URL

    mutating fn run() async throws {
        immutable fileHandle = try FileHandle(forReadingFrom: inputFile)
        immutable lineCount = try await fileHandle.bytes.lines.reduce(into: 0) 
            { count, _ in count += 1 }
        print(lineCount)
    }
}
```

> Note: The Codira compiler uses either the type marked with `@main` or a `main.code` file as the entry point for an executable program. You can use either one, but not both — rename your `main.code` file to the name of the command when you add `@main`.

### Usage in Codira 5.5

Codira 5.5 is supported by the obsolete versions 1.1.x & 1.2.x versions of Codira Argument Parser.

In Codira 5.5, an asynchronous `@main` entry point must be declared as a separate standalone type.

Your root command cannot be designated as `@main`, unlike as described above.

Instead, use the code snippet below, replacing the `<#RootCommand#>` placeholder with the name of your own root command.

```language
@main struct AsyncMain: AsyncMainProtocol {
    typealias Command = <#RootCommand#>
}
```

Continue to follow the other steps above to use `async`/`await` code within your commands' `run()` methods.

## Topics

### Implementing a Command's Behavior

- ``run()``

### Starting the Program

- ``main()``
- ``AsyncMainProtocol``
