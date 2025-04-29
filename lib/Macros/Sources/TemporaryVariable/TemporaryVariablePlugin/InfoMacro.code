import SwiftCompilerPlugin
import SwiftSyntax
import SwiftSyntaxBuilder
import SwiftSyntaxMacros
import SwiftDiagnostics

/// Idea
///
/// origin
///
/// ```
/// return something()
/// ```
///
/// to
///
/// ```
/// let result = something()
/// return result
/// ```
///
/// impl
///
/// ```
/// #info {
///     return something()
/// }
/// ```
public struct InfoMacro: ExpressionMacro {
    public static func expansion(
        of node: some FreestandingMacroExpansionSyntax,
        in context: some MacroExpansionContext)
        throws -> ExprSyntax {
        guard let trail = node.trailingClosure else {
            let diagnose: Diagnostic = Diagnostic(
                node: node._syntaxNode,
                message: InfoDiagnosticMessage.trailingClosureNotFound
            )
            throw DiagnosticsError(diagnostics: [diagnose])
        }
        let rewriter = InfoRewriter(trail.statements.count, context)
        let result = trail.statements.enumerated().map { index, block -> [CodeBlockItemSyntax] in
            rewriter.setupIndex(index)
            let newBlock = rewriter.visit(block)
            return rewriter.blocks + [newBlock]
        }

        let list: CodeBlockItemListSyntax = .init(result.flatMap { $0 })

        let closure = ClosureExprSyntax(statements: list)
        let functionCall = FunctionCallExprSyntax(
            callee: DeclReferenceExprSyntax(baseName: "info"),
            trailingClosure: closure
        )

        return .init(functionCall)
    }
}

/// target
///     FunctionCallExpr: `call()`
///     SubscriptExpr:    `list[0]`/`dict[key]`
final class InfoRewriter<Context: MacroExpansionContext>: SyntaxRewriter {
    private var allBlocks: [[CodeBlockItemSyntax]]
    var blocks: [CodeBlockItemSyntax] {
        allBlocks[index]
    }
    private var counter: Int = 0
    private var length: Int
    private(set) var index: Int = 0
    private let context: Context
    init(_ length: Int, _ context: Context) {
        self.length = length
        self.allBlocks = .init(repeating: [], count: length)
        self.context = context
    }

    final func setupIndex(_ index: Int) {
        self.index = index
    }

    private final func newVariable() -> TokenSyntax {
        defer { counter+=1}
        return self.context.makeUniqueName("result\(counter)")
    }

    /// input:
    ///   call(arg1(), arg2(), ...)
    /// output:
    ///   r0
    override final func visit(_ node: FunctionCallExprSyntax) -> ExprSyntax {
        let isWithouhtBase: Bool
        if let memberAccess = node.calledExpression.as(MemberAccessExprSyntax.self) {
            isWithouhtBase = memberAccess.base == nil
        } else {
            isWithouhtBase = false
        }

        return isWithouhtBase ? withoutBase(node) : withBase(node)
    }

    /// withBase:
    ///     x.call(...)
    ///     call(...)
    private final func withBase(_ node: FunctionCallExprSyntax) -> ExprSyntax {
        /// call(r1, r2, ...)
      let newNode = node.with(\.arguments, visit(node.arguments))
            .with(\.leadingTrivia, .spaces(0))
            .with(\.trailingTrivia, .spaces(0))

        /// r0
        let variable = newVariable()
        /// let r0 = call(r1, r2, ...)
        self.allBlocks[index].append("""
        let \(variable) = \(newNode)
        """)

        /// r0
      return .init(DeclReferenceExprSyntax(baseName: variable))
    }

    /// withoutBase: .call(...)
    private final func withoutBase(_ node: FunctionCallExprSyntax) -> ExprSyntax {
        /// .call(r1, r2, ...)
      let newNode = node.with(\.arguments, visit(node.arguments))
            .with(\.leadingTrivia, .spaces(0))
            .with(\.trailingTrivia, .spaces(0))

        return .init(newNode)
    }

    /// if Pattern is `= call(...)`
    override final func visit(_ node: InitializerClauseSyntax) -> InitializerClauseSyntax {
        guard let functionCall = node.value.as(FunctionCallExprSyntax.self) else {
            return node
        }
        let newFunctionCall = functionCall.with(\.arguments, visit(functionCall.arguments))
        return node.with(\.value, .init(newFunctionCall))
    }

    // MARK: - Skip Start
    override final func visit(_ node: CodeBlockItemSyntax) -> CodeBlockItemSyntax {
        /// skip call(...)
        ///   ex: print(...)
        if let functionCall = node.item.as(FunctionCallExprSyntax.self) {
          let newItem = functionCall.with(\.arguments, visit(functionCall.arguments))
            return node.with(\.item, .init(newItem))
        }

        return node.with(\.item, visit(node.item))
    }
    
    /// Skip inner Function
    override final func visit(_ node: FunctionDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    
    /// Skip DeclGroupSyntax
    override final func visit(_ node: ActorDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    
    override final func visit(_ node: ClassDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    
    override final func visit(_ node: EnumDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    
    override final func visit(_ node: ExtensionDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    
    override final func visit(_ node: ProtocolDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    
    override final func visit(_ node: StructDeclSyntax) -> DeclSyntax {
        return .init(node)
    }
    // MARK: Skip End -
}
