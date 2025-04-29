import MacroKitMacros
import MacroTesting
import SwiftSyntaxMacros
import SwiftSyntaxMacrosTestSupport
import XCTest

// edges cases:
//  - overloaded functions

class GenerateMockMacroTests: XCTestCase {

    override func invokeTest() {
        withMacroTesting(record: false, macros: [GenerateMockMacro.self]) {
            super.invokeTest()
        }
    }

    func testGenerateMock_HappyPath_SimpleProperty() {
        let proto = """
        protocol TestProtocol {
            var property: String {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: String {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, String> = .init()
                }
                internal var property: String {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ThrowingProperty() {
        let proto = """
        protocol TestProtocol {
            var property: String {
                get throws
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: String {
                    get throws
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, Result<String, Error>> = .init()
                }
                internal var property: String {
                    get throws {
                        try mocks.property.getter()
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_AsyncProperty() {
        let proto = """
        protocol TestProtocol {
            var property: String {
                get async
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: String {
                    get async
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, String> = .init()
                }
                internal var property: String {
                    get async {
                        mocks.property.getter()
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_AsyncThrowingProperty() {
        let proto = """
        protocol TestProtocol {
            var property: String {
                get async throws
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: String {
                    get async throws
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, Result<String, Error>> = .init()
                }
                internal var property: String {
                    get async throws {
                        try mocks.property.getter()
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_SimpleProperty_GetSet() {
        let proto = """
        protocol TestProtocol {
            var property: String {
                get
                set
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: String {
                    get
                    set
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, String> = .init()
                }
                internal var property: String {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_ClosureProperty_Void() {
        let proto = """
        protocol TestProtocol {
            var property: () -> Void {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: () -> Void {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<() -> Void, () -> Void> = .init()
                }
                internal var property: () -> Void {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ClosureProperty_OneParam() {
        let proto = """
        protocol TestProtocol {
            var property: (Int) -> Void {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: (Int) -> Void {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<(Int) -> Void, (Int) -> Void> = .init()
                }
                internal var property: (Int) -> Void {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ClosureProperty_TwoParam() {
        let proto = """
        protocol TestProtocol {
            var property: (Int, Bool) -> Void {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: (Int, Bool) -> Void {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<(Int, Bool) -> Void, (Int, Bool) -> Void> = .init()
                }
                internal var property: (Int, Bool) -> Void {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_ThrowingClosureProperty_TwoParam() {
        let proto = """
        protocol TestProtocol {
            var property: (Int, Bool) throws -> Void {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: (Int, Bool) throws -> Void {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<(Int, Bool) throws -> Void, (Int, Bool) throws -> Void> = .init()
                }
                internal var property: (Int, Bool) throws -> Void {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ThrowingAsyncClosureProperty_TwoParam() {
        let proto = """
        protocol TestProtocol {
            var property: (Int, Bool) async throws -> Void {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: (Int, Bool) async throws -> Void {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<(Int, Bool) async throws -> Void, (Int, Bool) async throws -> Void> = .init()
                }
                internal var property: (Int, Bool) async throws -> Void {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_ImplicitVoidFunction() {
        let proto = """
        protocol TestProtocol {
            func function()
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function()
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(), Void> = .init()
                }
                internal func function() {
                    return mocks.function.execute(())
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ExplicitVoidFunction() {
        let proto = """
        protocol TestProtocol {
            func function() -> Void
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function() -> Void
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(), Void> = .init()
                }
                internal func function() -> Void {
                    return mocks.function.execute(())
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ImplicitVoidFunction_WithParam() {
        let proto = """
        protocol TestProtocol {
            func function(value: Int)
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(value: Int)
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Int), Void> = .init()
                }
                internal func function(value arg0: Int) {
                    return mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ExplicitVoidFunction_WithParam() {
        let proto = """
        protocol TestProtocol {
            func function(value: Int) -> Void
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(value: Int) -> Void
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Int), Void> = .init()
                }
                internal func function(value arg0: Int) -> Void {
                    return mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_NonVoidFunction_WithAnonParam() {
        let proto = """
        protocol TestProtocol {
            func function(_ value: Int) -> Bool
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(_ value: Int) -> Bool
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Int), Bool> = .init()
                }
                internal func function(_ arg0: Int) -> Bool {
                    return mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_NonVoidFunction_WithInternalAndExternalParams() {
        let proto = """
        protocol TestProtocol {
            func function(with value: Int) -> Bool
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(with value: Int) -> Bool
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Int), Bool> = .init()
                }
                internal func function(with arg0: Int) -> Bool {
                    return mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_ClosureFunction_WithAnonParam() {
        let proto = """
        protocol TestProtocol {
            func function(_ value: Int) -> () -> Void
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(_ value: Int) -> () -> Void
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Int), () -> Void> = .init()
                }
                internal func function(_ arg0: Int) -> () -> Void {
                    return mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_ClosureFunctionWithParam_WithAnonParam() {
        let proto = """
        protocol TestProtocol {
            func function(_ value: Int) -> (Bool) -> Void
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(_ value: Int) -> (Bool) -> Void
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Int), (Bool) -> Void> = .init()
                }
                internal func function(_ arg0: Int) -> (Bool) -> Void {
                    return mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_ThrowingFunction() {
        let proto = """
        protocol TestProtocol {
            func function() throws
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function() throws
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(), Result<Void, Error>> = .init()
                }
                internal func function() throws {
                    return try mocks.function.execute(())
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_AsyncFunction() {
        let proto = """
        protocol TestProtocol {
            func function() async
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function() async
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(), Void> = .init()
                }
                internal func function() async {
                    return mocks.function.execute(())
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_AsyncThrowingFunction() {
        let proto = """
        protocol TestProtocol {
            func function() async throws
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function() async throws
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(), Result<Void, Error>> = .init()
                }
                internal func function() async throws {
                    return try mocks.function.execute(())
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_AsyncThrowingFunctionWithBothParamsAndReturnType() {
        let proto = """
        protocol TestProtocol {
            func function(with value: Bool) async throws -> String
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(with value: Bool) async throws -> String
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<(Bool), Result<String, Error>> = .init()
                }
                internal func function(with arg0: Bool) async throws -> String {
                    return try mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_FunctionWithParamsWithAttributes() {
        let proto = """
        protocol TestProtocol {
            func function(_ closure: @Sendable @escaping (Bool) -> Void) async throws -> String
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                func function(_ closure: @Sendable @escaping (Bool) -> Void) async throws -> String
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var function: MockMember<((Bool) -> Void), Result<String, Error>> = .init()
                }
                internal func function(_ arg0: @Sendable @escaping (Bool) -> Void) async throws -> String {
                    return try mocks.function.execute((arg0))
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_AssociatedType() {
        let proto = """
        protocol TestProtocol {
            associatedtype Value

            var property: Value {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                associatedtype Value

                var property: Value {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock<Value>: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<Value, Value> = .init()
                }
                internal var property: Value {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_AssociatedTypeWithConstraint() {
        let proto = """
        protocol TestProtocol {
            associatedtype Value: Codable

            var property: Value {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                associatedtype Value: Codable

                var property: Value {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock<Value: Codable>: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<Value, Value> = .init()
                }
                internal var property: Value {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_MultipleAssociatedTypes() {
        let proto = """
        protocol TestProtocol {
            associatedtype Foo
            associatedtype Bar: Codable
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                associatedtype Foo
                associatedtype Bar: Codable
            }

            #if DEBUG

            internal class TestProtocolMock<Foo, Bar: Codable>: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_HappyPath_Inheritance() {
        let proto = """
        protocol TestProtocol: Equatable {
            var property: String {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol: Equatable {
                var property: String {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol, Equatable {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, String> = .init()
                }
                internal var property: String {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }
    func testGenerateMock_HappyPath_MultipleInheritance() {
        let proto = """
        protocol TestProtocol: Equatable, Codable {
            var property: String {
                get
            }
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol: Equatable, Codable {
                var property: String {
                    get
                }
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol, Equatable, Codable {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, String> = .init()
                }
                internal var property: String {
                    get {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
            }

            #endif
            """
        }
    }

    func _testGenerateMock_HappyPath_OverloadedFunctions() {
        let proto = """
        protocol TestProtocol {
            func function() -> String
            func function() -> Int
        }
        """
        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        }
    }

    func testGenerateMock_whenInternalProtocol_allMockPropertiesAndFunctionsAreMadeInternal() {
        let proto = """
        protocol TestProtocol {
            var property: String { get set }
            func doWork() -> Int
            static func doMoreWork() -> String
        }
        """

        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            protocol TestProtocol {
                var property: String { get set }
                func doWork() -> Int
                static func doMoreWork() -> String
            }

            #if DEBUG

            internal class TestProtocolMock: TestProtocol {
                internal let mocks = Members()
                internal class Members {
                    internal var property: MockMember<String, String> = .init()
                    internal var doWork: MockMember<(), Int> = .init()
                    internal var doMoreWork: MockMember<(), String> = .init()
                }
                internal var property: String {
                    get  {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
                internal func doWork() -> Int {
                    return mocks.doWork.execute(())
                }
                static internal func doMoreWork() -> String {
                    return mocks.doMoreWork.execute(())
                }
            }

            #endif
            """
        }
    }

    func testGenerateMock_whenPublicProtocol_allMockPropertiesAndFunctionsAreMadePublic() {
        let proto = """
        public protocol TestProtocol {
            var property: String { get set }
            func doWork() -> Int
            static func doMoreWork() -> String
        }
        """

        assertMacro {
            """
            @GenerateMock
            \(proto)
            """
        } expansion: {
            """
            public protocol TestProtocol {
                var property: String { get set }
                func doWork() -> Int
                static func doMoreWork() -> String
            }

            #if DEBUG

            open class TestProtocolMock: TestProtocol {
                public let mocks = Members()
                public class Members {
                    public var property: MockMember<String, String> = .init()
                    public var doWork: MockMember<(), Int> = .init()
                    public var doMoreWork: MockMember<(), String> = .init()
                }
                public init() {
                }
                open var property: String {
                    get  {
                        mocks.property.getter()
                    }
                    set {
                        mocks.property.setter(newValue)
                    }
                }
                open func doWork() -> Int {
                    return mocks.doWork.execute(())
                }
                static public func doMoreWork() -> String {
                    return mocks.doMoreWork.execute(())
                }
            }

            #endif
            """
        }
    }
}
