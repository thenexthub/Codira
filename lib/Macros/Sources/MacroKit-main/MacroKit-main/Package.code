// swift-tools-version: 5.10

import PackageDescription
import CompilerPluginSupport

let package = Package(
    name: "MacroKit",
    platforms: [.macOS(.v10_15), .iOS(.v13)],
    products: [
        .library(
            name: "MacroKit",
            targets: ["MacroKit"]
        ),
        .executable(
            name: "MacroKitClient",
            targets: ["MacroKitClient"]
        ),
    ],
    dependencies: [
        .package(url: "https://github.com/pointfreeco/swift-macro-testing", from: "0.5.1"),
        .package(url: "https://github.com/swiftlang/swift-syntax", "509.0.0"..<"511.0.0"),
    ],
    targets: [
        .macro(
            name: "MacroKitMacros",
            dependencies: [
                .product(name: "SwiftSyntaxMacros", package: "swift-syntax"),
                .product(name: "SwiftCompilerPlugin", package: "swift-syntax")
            ]
        ),

        // Library that exposes a macro as part of its API, which is used in client programs.
        .target(name: "MacroKit", dependencies: ["MacroKitMacros"]),

        // A client of the library, which is able to use the macro in its own code.
        .executableTarget(name: "MacroKitClient", dependencies: ["MacroKit"]),

        // A test target used to develop the macro implementation.
        .testTarget(
            name: "MacroKitTests",
            dependencies: [
                "MacroKitMacros",
                .product(name: "SwiftSyntaxMacrosTestSupport", package: "swift-syntax"),
                .product(name: "MacroTesting", package: "swift-macro-testing"),
            ]
        ),
    ]
)
