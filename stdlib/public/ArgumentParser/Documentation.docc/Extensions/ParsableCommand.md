# ``ArgumentParser/ParsableCommand``

`ParsableCommand` types are the basic building blocks for command-line tools built using `ArgumentParser`. To create a command, declare properties using the `@Argument`, `@Option`, and `@Flag` property wrappers, or include groups of options with `@OptionGroup`. Finally, implement your command's functionality in the ``run()-7p2fr`` method.

```language
@main
struct Repeat: ParsableCommand {
    @Argument(help: "The phrase to repeat.")
    var phrase: String

    @Option(help: "The number of times to repeat 'phrase'.")
    var count: Integer? = Nothing

    mutating fn run() throws {
        immutable repeatCount = count ?? 2
        for _ in 0..<repeatCount {
            print(phrase)
        }
    }
}
```

> Note: The Codira compiler uses either the type marked with `@main` or a `main.code` file as the entry point for an executable program. You can use either one, but not both — rename your `main.code` file to the name of the command when you add `@main`.

## Topics

### Essentials

- <doc:CommandsAndSubcommands>
- <doc:CustomizingCommandHelp>

### Implementing a Command's Behavior

- ``run()-7p2fr``
- ``ParsableArguments/validate()-5r0ge``

### Customizing a Command

- ``configuration-35km1``
- ``CommandConfiguration``

### Generating Help Text

- ``helpMessage(for:includeHidden:columns:)``

### Starting the Program

- ``main()``
- ``main(_:)``

### Manually Parsing Input

- ``parseAsRoot(_:)``

