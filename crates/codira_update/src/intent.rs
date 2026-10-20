//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The intent file of a live update (`intent.toml`): which functions the
//!   author declares changed and what relation each must keep to its old
//!   version, which fields may differ, and how a retyped field migrates. Also a
//!   small expression language for the `when` precondition of an `equal`
//!   function, parsed here so that the checker needs no compiler front end.
//!
//! # The file
//!
//! ```toml
//! [update]
//! name = "clamp the upper bound"
//!
//! [[function]]
//! name = "clamp"            # the function's path, e.g. `clamp` or `Point.len`
//! relation = "equal"        # equal | refines | replaces | added | removed
//! when = "lo <= hi"         # optional; `equal` is required only where this holds
//!
//! [[function]]
//! name = "area"
//! relation = "refines"      # the new contract is no stricter to call and no weaker to rely on
//!
//! [[field]]
//! type = "Counter"
//! name = "total"
//! may_differ = true         # a changed function may leave a different value here
//! migrate = "saturating"    # saturating | reject (default): a retyped field's old values
//! ```
//!
//! Everything the file does not name is *unchanged*, and the update is
//! admitted only if the certificate proves it unchanged.
//!
//! # The relations
//!
//! * `equal` -- the new function computes the same result, performs the same
//!   effects (allocation, output, panics) in the same order and leaves the same
//!   heap as the old one, for every input satisfying `when` (every input, when
//!   there is no `when`). Outside `when` it may do anything its own contract
//!   allows.
//! * `refines` -- the new function meets the old function's contract:
//!   `requires` no stronger, `ensures` no weaker, and its body proves the new
//!   contract. Its behaviour may differ otherwise.
//! * `replaces` -- the new function meets a new contract the author states in
//!   the source; its body proves it. Callers are not assumed to keep working: a
//!   caller that depends on more than the old contract promises has to be named
//!   too.
//! * `added` / `removed` -- the function exists only in the new / old version.

use std::fmt;

use serde_derive::{Deserialize, Serialize};

/// What the author wants of one changed function.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum Relation {
    Equal,
    Refines,
    Replaces,
    Added,
    Removed,
}

impl fmt::Display for Relation {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(match self {
            Relation::Equal => "equal",
            Relation::Refines => "refines",
            Relation::Replaces => "replaces",
            Relation::Added => "added",
            Relation::Removed => "removed",
        })
    }
}

/// How the live values of a retyped field are carried over when some of them
/// do not fit the new type.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum MigrationPolicy {
    /// The runtime rejects the update before the swap if any live value does
    /// not fit (the default).
    #[default]
    Reject,
    /// The author accepts the runtime's conversion, which clamps to the new
    /// type's nearest value.
    Saturating,
}

/// One function the author declares changed.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct FunctionIntent {
    pub name: String,
    pub relation: Relation,
    /// For `equal`: the inputs the equality is required on, as an expression
    /// over the parameters (see [`parse_expr`]).
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub when: Option<String>,
    /// The function's effect trace (allocations, output, the order of its
    /// calls) may differ from the old one's. `equal` otherwise demands the
    /// same trace.
    #[serde(default)]
    pub effects_may_differ: bool,
}

/// A field the author declares changed.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct FieldIntent {
    /// The type that declares it.
    #[serde(rename = "type")]
    pub type_name: String,
    #[serde(rename = "name")]
    pub field: String,
    /// A changed function may leave a different value in this field than
    /// its old version did.
    #[serde(default)]
    pub may_differ: bool,
    #[serde(default)]
    pub migrate: MigrationPolicy,
    /// The field is removed on purpose; its live values are discarded.
    #[serde(default)]
    pub drop: bool,
}

#[derive(Debug, Default, Deserialize)]
#[serde(deny_unknown_fields)]
struct RawUpdate {
    #[serde(default)]
    name: Option<String>,
}

#[derive(Debug, Default, Deserialize)]
#[serde(deny_unknown_fields)]
struct RawIntent {
    #[serde(default)]
    update: RawUpdate,
    #[serde(default)]
    function: Vec<FunctionIntent>,
    #[serde(default)]
    field: Vec<FieldIntent>,
}

/// A parsed intent file.
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct Intent {
    pub name: Option<String>,
    pub functions: Vec<FunctionIntent>,
    pub fields: Vec<FieldIntent>,
}

impl Intent {
    /// Parses the text of an intent file. A function named twice, an unknown
    /// key, an `equal`-only key on another relation, and a `when` that is not
    /// an expression are errors.
    pub fn parse(text: &str) -> Result<Intent, String> {
        let raw: RawIntent = toml::from_str(text).map_err(|e| format!("intent file: {e}"))?;
        let mut seen: Vec<&str> = Vec::new();
        for function in &raw.function {
            if seen.contains(&function.name.as_str()) {
                return Err(format!(
                    "intent file: `{}` is named twice; a function has one relation",
                    function.name
                ));
            }
            seen.push(&function.name);
            if function.when.is_some() && function.relation != Relation::Equal {
                return Err(format!(
                    "intent file: `when` only applies to `equal`, and `{}` is `{}`",
                    function.name, function.relation
                ));
            }
            if let Some(when) = &function.when {
                parse_expr(when)
                    .map_err(|e| format!("intent file: `when` of `{}`: {e}", function.name))?;
            }
        }
        Ok(Intent {
            name: raw.update.name,
            functions: raw.function,
            fields: raw.field,
        })
    }

    /// The declared relation of `name`, or `None` when the author left it
    /// unchanged.
    pub fn function(&self, name: &str) -> Option<&FunctionIntent> {
        self.functions.iter().find(|f| f.name == name)
    }

    /// The declared field, if any.
    pub fn field(&self, type_name: &str, field: &str) -> Option<&FieldIntent> {
        self.fields
            .iter()
            .find(|f| f.type_name == type_name && f.field == field)
    }
}

// ---- the `when` expression language -------------------------------------

/// A unary operator of the `when` language.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum UnOp {
    Not,
    Neg,
    BitNot,
}

/// A binary operator of the `when` language.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BinOp {
    Add,
    Sub,
    Mul,
    Div,
    Rem,
    BitAnd,
    BitOr,
    BitXor,
    Shl,
    Shr,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
    And,
    Or,
}

/// An expression over a function's parameters: integer and boolean literals,
/// parameter names, the C operators on them, and parentheses. Integer
/// literals take the type of the other operand when the expression is
/// evaluated.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Expr {
    Int(i128),
    Bool(bool),
    Var(String),
    Unary(UnOp, Box<Expr>),
    Binary(BinOp, Box<Expr>, Box<Expr>),
}

#[derive(Debug, Clone, PartialEq, Eq)]
enum Token {
    Int(i128),
    Ident(String),
    Op(&'static str),
}

const OPERATORS: &[&str] = &[
    "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "+", "-", "*", "/", "%", "&", "|", "^", "<",
    ">", "!", "~", "(", ")",
];

fn tokenize(text: &str) -> Result<Vec<Token>, String> {
    let chars: Vec<char> = text.chars().collect();
    let mut tokens = Vec::new();
    let mut i = 0;
    'outer: while i < chars.len() {
        let c = chars[i];
        if c.is_whitespace() {
            i += 1;
            continue;
        }
        if c.is_ascii_digit() {
            let start = i;
            let (radix, skip) = match (c, chars.get(i + 1)) {
                ('0', Some('x' | 'X')) => (16, 2),
                ('0', Some('b' | 'B')) => (2, 2),
                _ => (10, 0),
            };
            i += skip;
            let digits_start = i;
            while i < chars.len() && (chars[i].is_digit(radix) || chars[i] == '_') {
                i += 1;
            }
            let digits: String = chars[digits_start..i]
                .iter()
                .filter(|c| **c != '_')
                .collect();
            let value = i128::from_str_radix(&digits, radix).map_err(|_overflow| {
                format!(
                    "`{}` is not a number that fits",
                    chars[start..i].iter().collect::<String>()
                )
            })?;
            tokens.push(Token::Int(value));
            continue;
        }
        if c.is_alphabetic() || c == '_' {
            let start = i;
            while i < chars.len() && (chars[i].is_alphanumeric() || chars[i] == '_') {
                i += 1;
            }
            tokens.push(Token::Ident(chars[start..i].iter().collect()));
            continue;
        }
        for op in OPERATORS {
            let n = op.chars().count();
            if chars[i..].iter().take(n).collect::<String>() == *op {
                tokens.push(Token::Op(op));
                i += n;
                continue 'outer;
            }
        }
        return Err(format!("unexpected `{c}`"));
    }
    Ok(tokens)
}

fn binary_precedence(op: &str) -> Option<(u8, BinOp)> {
    Some(match op {
        "||" => (1, BinOp::Or),
        "&&" => (2, BinOp::And),
        "==" => (3, BinOp::Eq),
        "!=" => (3, BinOp::Ne),
        "<" => (4, BinOp::Lt),
        "<=" => (4, BinOp::Le),
        ">" => (4, BinOp::Gt),
        ">=" => (4, BinOp::Ge),
        "|" => (5, BinOp::BitOr),
        "^" => (6, BinOp::BitXor),
        "&" => (7, BinOp::BitAnd),
        "<<" => (8, BinOp::Shl),
        ">>" => (8, BinOp::Shr),
        "+" => (9, BinOp::Add),
        "-" => (9, BinOp::Sub),
        "*" => (10, BinOp::Mul),
        "/" => (10, BinOp::Div),
        "%" => (10, BinOp::Rem),
        _ => return None,
    })
}

struct Parser {
    tokens: Vec<Token>,
    at: usize,
}

impl Parser {
    fn peek(&self) -> Option<&Token> {
        self.tokens.get(self.at)
    }

    fn expression(&mut self, min: u8) -> Result<Expr, String> {
        let mut lhs = self.unary()?;
        while let Some(Token::Op(op)) = self.peek() {
            let Some((precedence, binary)) = binary_precedence(op) else {
                break;
            };
            if precedence < min {
                break;
            }
            self.at += 1;
            let rhs = self.expression(precedence + 1)?;
            lhs = Expr::Binary(binary, Box::new(lhs), Box::new(rhs));
        }
        Ok(lhs)
    }

    fn unary(&mut self) -> Result<Expr, String> {
        match self.peek().cloned() {
            Some(Token::Op("!")) => {
                self.at += 1;
                Ok(Expr::Unary(UnOp::Not, Box::new(self.unary()?)))
            }
            Some(Token::Op("-")) => {
                self.at += 1;
                Ok(Expr::Unary(UnOp::Neg, Box::new(self.unary()?)))
            }
            Some(Token::Op("~")) => {
                self.at += 1;
                Ok(Expr::Unary(UnOp::BitNot, Box::new(self.unary()?)))
            }
            Some(Token::Op("(")) => {
                self.at += 1;
                let inner = self.expression(1)?;
                match self.peek() {
                    Some(Token::Op(")")) => {
                        self.at += 1;
                        Ok(inner)
                    }
                    _ => Err("a `(` is never closed".to_string()),
                }
            }
            Some(Token::Int(value)) => {
                self.at += 1;
                Ok(Expr::Int(value))
            }
            Some(Token::Ident(name)) => {
                self.at += 1;
                Ok(match name.as_str() {
                    "true" => Expr::Bool(true),
                    "false" => Expr::Bool(false),
                    _ => Expr::Var(name),
                })
            }
            Some(Token::Op(op)) => Err(format!("unexpected `{op}`")),
            None => Err("the expression ends where an operand is expected".to_string()),
        }
    }
}

/// Parses a `when` expression. Precedence and associativity are C's, with
/// `&&`/`||` for the boolean connectives.
pub fn parse_expr(text: &str) -> Result<Expr, String> {
    let tokens = tokenize(text)?;
    if tokens.is_empty() {
        return Err("the expression is empty".to_string());
    }
    let mut parser = Parser { tokens, at: 0 };
    let expr = parser.expression(1)?;
    match parser.peek() {
        None => Ok(expr),
        Some(Token::Op(op)) => Err(format!("unexpected `{op}` after the expression")),
        Some(Token::Ident(name)) => Err(format!("unexpected `{name}` after the expression")),
        Some(Token::Int(_)) => Err("unexpected number after the expression".to_string()),
    }
}

impl Expr {
    /// Every parameter name the expression mentions, in order of first use.
    pub fn variables(&self) -> Vec<&str> {
        fn walk<'a>(e: &'a Expr, out: &mut Vec<&'a str>) {
            match e {
                Expr::Var(name) if !out.contains(&name.as_str()) => out.push(name),
                Expr::Unary(_, inner) => walk(inner, out),
                Expr::Binary(_, a, b) => {
                    walk(a, out);
                    walk(b, out);
                }
                _ => {}
            }
        }
        let mut out = Vec::new();
        walk(self, &mut out);
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn an_intent_file_names_changed_functions_and_fields() {
        let intent = Intent::parse(
            r#"
            [update]
            name = "fix"

            [[function]]
            name = "clamp"
            relation = "equal"
            when = "lo <= hi && x != 0"

            [[function]]
            name = "area"
            relation = "refines"

            [[field]]
            type = "Counter"
            name = "total"
            may_differ = true
            migrate = "saturating"
            "#,
        )
        .unwrap();
        assert_eq!(intent.name.as_deref(), Some("fix"));
        assert_eq!(intent.function("clamp").unwrap().relation, Relation::Equal);
        assert_eq!(intent.function("area").unwrap().relation, Relation::Refines);
        assert!(intent.function("other").is_none());
        let field = intent.field("Counter", "total").unwrap();
        assert!(field.may_differ);
        assert_eq!(field.migrate, MigrationPolicy::Saturating);
    }

    #[test]
    fn mistakes_in_an_intent_file_are_errors_not_guesses() {
        let twice = "[[function]]\nname='f'\nrelation='equal'\n[[function]]\nname='f'\nrelation='refines'\n";
        assert!(Intent::parse(twice).unwrap_err().contains("named twice"));
        let misplaced = "[[function]]\nname='f'\nrelation='replaces'\nwhen='x > 0'\n";
        assert!(Intent::parse(misplaced)
            .unwrap_err()
            .contains("only applies to `equal`"));
        let unknown = "[[function]]\nname='f'\nrelation='equal'\nwhy='because'\n";
        assert!(Intent::parse(unknown).is_err());
        let bad_when = "[[function]]\nname='f'\nrelation='equal'\nwhen='x >'\n";
        assert!(Intent::parse(bad_when).unwrap_err().contains("when"));
        assert!(Intent::parse("[[function]]\nname='f'\nrelation='sometimes'\n").is_err());
        assert!(Intent::parse("").unwrap().functions.is_empty());
    }

    #[test]
    fn when_expressions_parse_with_c_precedence() {
        let e = parse_expr("a + 2 * b < 10 && !(c == 0) || d").unwrap();
        // `||` is the loosest.
        let Expr::Binary(BinOp::Or, left, right) = e else {
            panic!("expected `||` at the root")
        };
        assert_eq!(*right, Expr::Var("d".into()));
        let Expr::Binary(BinOp::And, cmp, not) = *left else {
            panic!("expected `&&` below it")
        };
        assert!(matches!(*cmp, Expr::Binary(BinOp::Lt, ..)));
        assert!(matches!(*not, Expr::Unary(UnOp::Not, _)));
        assert_eq!(
            parse_expr("x - 1 - 1").unwrap(),
            Expr::Binary(
                BinOp::Sub,
                Box::new(Expr::Binary(
                    BinOp::Sub,
                    Box::new(Expr::Var("x".into())),
                    Box::new(Expr::Int(1))
                )),
                Box::new(Expr::Int(1))
            ),
            "left associative"
        );
        assert_eq!(parse_expr("0xff").unwrap(), Expr::Int(255));
        assert_eq!(
            parse_expr("a & b | c").unwrap().variables(),
            ["a", "b", "c"]
        );
        for bad in [
            "",
            "1 +",
            "(1",
            "1 2",
            "a $ b",
            "99999999999999999999999999999999999999999",
        ] {
            assert!(parse_expr(bad).is_err(), "{bad:?}");
        }
    }
}
