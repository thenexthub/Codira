import SwiftCompilerPlugin
import SwiftSyntax
import SwiftSyntaxBuilder
import SwiftSyntaxMacros

/// A macro that produces extends OptionSet to conform to Codable (Encodable & Decodable) protocol
public struct EasilyCodableOptionSetMacro: ExtensionMacro {
    public static func expansion(of node: AttributeSyntax, attachedTo declaration: some DeclGroupSyntax, providingExtensionsOf type: some TypeSyntaxProtocol, conformingTo protocols: [TypeSyntax], in context: some MacroExpansionContext) throws -> [ExtensionDeclSyntax] {
        let inheritedType = declaration.as(StructDeclSyntax.self)?.inheritanceClause?.inheritedTypes.trimmedDescription
        
        guard inheritedType == "OptionSet" else {
            context.diagnose(EasilyCodableOptionSetError.badInheritance.diagnostic(for: declaration))
            return []
        }
        
        let staticMembers = declaration.memberBlock.members
            .compactMap { $0.decl.as(VariableDeclSyntax.self) }
            .filter { $0.modifiers.first?.name.text == "static" }
        
        let optionNames = staticMembers
            .compactMap { $0.bindings.first }
            .filter {
                if $0.typeAnnotation?.type.as(IdentifierTypeSyntax.self)?.name.text == type.description {
                    return true
                }
                
                let initializer = $0.initializer?.value.as(FunctionCallExprSyntax.self)
                
                return initializer?.calledExpression.as(DeclReferenceExprSyntax.self)?.baseName.text == type.description
            }
            .compactMap { $0.pattern.as(IdentifierPatternSyntax.self)?.identifier.text }
        
        let mappingExpression = prepareMappingExpression(for: optionNames, type: type.description)
        let initExpression = prepareInitExpression(type: type.description)
        let encodingExpression = prepareEncodingExpression(withAllOption: optionNames.contains("all"), type: type.description)
        
        let syntax = try ExtensionDeclSyntax("""
        extension \(raw: type): Codable {
            \(raw: mappingExpression)
            
            \(raw: initExpression)
        
            \(raw: encodingExpression)
        }
        """)
        
        return [syntax]
    }
    
    private static func prepareMappingExpression(for optionNames: [String], type: String) -> String {
        var mappingExpression = optionNames.isEmpty ? [":"] : optionNames.enumerated().map { index, value in
            var line = "\"\(value)\": .\(value)"
            if index != optionNames.count - 1 {
                line += ","
            }
            return line
        }
        
        mappingExpression.insert("private static let mapping: [String: \(type)] = [", at: 0)
        mappingExpression.append("]")
        
        return mappingExpression.joined(separator: "\n")
    }
    
    private static func prepareInitExpression(type: String) -> String {
        """
        init(from decoder: Decoder) throws {
                var container = try decoder.unkeyedContainer()
                var result: \(type) = []
                while !container.isAtEnd {
                    let optionName = try container.decode(String.self)
                    guard let opt = Self.mapping[optionName] else {
                        let context = DecodingError.Context(
                            codingPath: decoder.codingPath,
                            debugDescription: "Option not recognised: \\(optionName)"
                        )
                        throw DecodingError.typeMismatch(String.self, context)
                    }
                    result.insert(opt)
                }
                self = result
        }
        """
    }
    
    private static func prepareEncodingExpression(withAllOption: Bool, type: String) -> String {
        let conditionalFilter = withAllOption ? ".filter { $0.key != \"all\" }\n" : ""
        
        let encodingMapping = """
        optionsRaw = Self.mapping
                \(conditionalFilter).compactMap { self.contains($0.value) ? $0.key : nil }
        """
        
        let encodingPart = !withAllOption ? encodingMapping : """
        if self == .all {
                    optionsRaw = ["all"]
        } else {
                    \(encodingMapping)
        }
        """
        
        return """
        func encode(to encoder: Encoder) throws {
                var container = encoder.unkeyedContainer()
        
                let optionsRaw: [String]
                \(encodingPart)
                try container.encode(contentsOf: optionsRaw)
        }
        """
    }
}

@main
struct CodableOptionSetPlugin: CompilerPlugin {
    let providingMacros: [Macro.Type] = [
        EasilyCodableOptionSetMacro.self,
    ]
}
