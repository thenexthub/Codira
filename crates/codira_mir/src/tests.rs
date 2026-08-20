use crate::{Attr, Body, Generator, GeneratorParam, GeneratorStore, OpKind, Region};

#[test]
fn builds_straight_line_arithmetic() {
    // `2 + 2`
    let mut body = Body::new();
    let two_a = body.push(OpKind::Const(Attr::Int(2)), []);
    let two_b = body.push(OpKind::Const(Attr::Int(2)), []);
    let sum = body.push(OpKind::Add, [two_a, two_b]);

    assert_eq!(body.result(), Some(sum));
    assert_eq!(body.get(sum).operands.as_slice(), &[two_a, two_b]);
}

#[test]
fn builds_if_with_nested_regions() {
    // `if true { 1 } else { 0 }`
    let mut body = Body::new();
    let cond = body.push(OpKind::Const(Attr::Bool(true)), []);

    let mut then_body = Body::new();
    then_body.push(OpKind::Const(Attr::Int(1)), []);
    let mut else_body = Body::new();
    else_body.push(OpKind::Const(Attr::Int(0)), []);

    let if_op = body.push_with_regions(
        OpKind::If,
        [cond],
        [Region::new(then_body), Region::new(else_body)],
    );

    let op = body.get(if_op);
    assert_eq!(op.regions.len(), 2);
    assert!(op.regions[0].body.result().is_some());
    assert!(op.regions[1].body.result().is_some());
}

#[test]
fn generator_store_round_trips() {
    let mut store = GeneratorStore::new();
    let mut body = Body::new();
    body.push(OpKind::ParamRef("N".into()), []);

    let id = store.add_generator(Generator {
        name: "identity".into(),
        params: vec![GeneratorParam { name: "N".into() }],
        body,
    });

    let generator = store.generator(id);
    assert_eq!(generator.name, "identity");
    assert_eq!(generator.params.len(), 1);
    assert_eq!(generator.params[0].name, "N");
}

#[test]
fn empty_body_has_no_result() {
    let body = Body::new();
    assert!(body.is_empty());
    assert_eq!(body.result(), None);
}
