import SwiftCompilerPlugin
import SwiftSyntax
import SwiftSyntaxBuilder
import SwiftSyntaxMacros

public struct ListablePropertiesMacro: ExtensionMacro {
    public static func expansion(of node: SwiftSyntax.AttributeSyntax, attachedTo declaration: some SwiftSyntax.DeclGroupSyntax, providingExtensionsOf type: some SwiftSyntax.TypeSyntaxProtocol, conformingTo protocols: [SwiftSyntax.TypeSyntax], in context: some SwiftSyntaxMacros.MacroExpansionContext) throws -> [SwiftSyntax.ExtensionDeclSyntax] {
        let variables = declaration.memberBlock.members
            .compactMap { $0.decl.as(VariableDeclSyntax.self) }
            .filter { $0.modifiers.first?.name.text != "static" }
        
        let varNames = variables.compactMap {
            $0.bindings.first?.pattern.as(IdentifierPatternSyntax.self)?.identifier.text
        }
        
        let syntax = try ExtensionDeclSyntax("""
        extension \(raw: type) {
            static func getProperties() -> [String] {
                \(raw: varNames)
            }
        }
        """)
        
        return [syntax]
    }
    
    
}

@main
struct ListablePropertiesPlugin: CompilerPlugin {
    let providingMacros: [Macro.Type] = [
        ListablePropertiesMacro.self,
    ]
}
