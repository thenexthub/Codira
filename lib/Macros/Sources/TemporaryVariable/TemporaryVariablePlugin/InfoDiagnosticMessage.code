//
//  InfoDiagnosticMessage.swift
//  
//
//  Created by Yume on 2023/7/24.
//

import Foundation
import SwiftDiagnostics

enum InfoDiagnosticMessage: String, DiagnosticMessage {
    case trailingClosureNotFound
    var message: String {
        return "Can't find trail closure"
    }
    var diagnosticID: MessageID {
        MessageID(domain: "TemporaryVariable", id: rawValue)
    }
    var severity: DiagnosticSeverity { .error }
}
