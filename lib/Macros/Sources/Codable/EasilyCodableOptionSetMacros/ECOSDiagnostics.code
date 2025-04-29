//
//  COSDiagnostics.swift
//  
//
//  Created by Maciej Burdzicki on 04/10/2023.
//

import Foundation
import SwiftSyntax
import SwiftDiagnostics
import SwiftSyntaxBuilder

enum EasilyCodableOptionSetError: ECOSDiagnostics, CustomStringConvertible, Error {
    case badInheritance
    
    var domain: String { "CEasilyodableOptionSet macro" }
    
    var description: String {
        switch self {
        case .badInheritance:
            return "@EasilyCodableOptionSet can only be applied to a struct which conforms to 'OptionSet' protocol"
        }
    }
}

protocol ECOSDiagnostics {
    var domain: String { get }
    var description: String { get }
    func diagnostic(for node: SyntaxProtocol, severity: DiagnosticSeverity, fixIts: [FixIt]) -> Diagnostic
}

extension ECOSDiagnostics {
    func diagnostic(for node: SyntaxProtocol, severity: DiagnosticSeverity = .error, fixIts: [FixIt] = []) -> Diagnostic {
        .init(
            node: Syntax(node),
            message: ECOSDiagnosticMessage(
                diagnosticID: .init(domain: domain,id: String(describing: self)),
                message: description, severity: severity),
            fixIts: fixIts)
    }
}

fileprivate struct ECOSDiagnosticMessage: DiagnosticMessage {
    var diagnosticID: MessageID
    var message: String
    var severity: DiagnosticSeverity
}
