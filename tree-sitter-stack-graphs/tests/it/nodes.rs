// -*- coding: utf-8 -*-
// ------------------------------------------------------------------------------------------------
// Copyright © 2021, stack-graphs authors.
// Licensed under either of Apache License, Version 2.0, or MIT license, at your option.
// Please see the LICENSE-APACHE or LICENSE-MIT files in this distribution for license details.
// ------------------------------------------------------------------------------------------------

use pretty_assertions::assert_eq;
use stack_graphs::arena::Handle;
use stack_graphs::graph::File;
use stack_graphs::graph::StackGraph;
use tree_sitter_stack_graphs::LoadError;

use super::build_stack_graph;

fn build_and_check_stack_graph_nodes(
    python_source: &str,
    tsg_source: &str,
    expected_nodes: &[&str],
) {
    let (graph, file) =
        build_stack_graph(python_source, tsg_source).expect("Could not load stack graph");
    check_stack_graph_nodes(&graph, file, expected_nodes);
}

pub(super) fn check_stack_graph_nodes(
    graph: &StackGraph,
    file: Handle<File>,
    expected_nodes: &[&str],
) {
    let actual_nodes = graph
        .nodes_for_file(file)
        .map(|handle| graph[handle].display(&graph).to_string())
        .collect::<Vec<_>>();
    assert_eq!(expected_nodes, actual_nodes);
}

#[test]
fn can_create_definition_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_symbol", symbol = (source-text @id), is_definition
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) definition a]"]);
}

#[test]
fn cannot_create_definition_node_without_symbol() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_symbol", is_definition
      }
    "#;
    let python = "a";
    let result = build_stack_graph(python, tsg);
    assert!(matches!(result, Err(LoadError::MissingSymbol(_))));
}

#[test]
fn can_create_drop_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "drop_scopes"
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) drop scopes]"]);
}

#[test]
fn can_create_exported_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) is_exported
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) exported scope]"]);
}

#[test]
fn can_create_endpoint_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) is_endpoint
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) exported scope]"]);
}

#[test]
fn can_create_implicit_internal_node() {
    let tsg = r#"
      (identifier) @id {
         node result
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) scope]"]);
}

#[test]
fn can_create_explicit_internal_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "scope"
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) scope]"]);
}

#[test]
fn can_create_pop_symbol_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_symbol", symbol = (source-text @id)
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) pop a]"]);
}

#[test]
fn cannot_create_pop_symbol_node_without_symbol() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_symbol"
      }
    "#;
    let python = "a";
    let result = build_stack_graph(python, tsg);
    assert!(matches!(result, Err(LoadError::MissingSymbol(_))));
}

#[test]
fn can_create_pop_scoped_symbol_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_scoped_symbol", symbol = (source-text @id)
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) pop scoped a]"]);
}

#[test]
fn cannot_create_pop_scoped_symbol_node_without_symbol() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_scoped_symbol"
      }
    "#;
    let python = "a";
    let result = build_stack_graph(python, tsg);
    assert!(matches!(result, Err(LoadError::MissingSymbol(_))));
}

#[test]
fn can_create_push_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "push_symbol", symbol = (source-text @id)
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) push a]"]);
}

#[test]
fn cannot_create_push_symbol_node_without_symbol() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "push_symbol"
      }
    "#;
    let python = "a";
    let result = build_stack_graph(python, tsg);
    assert!(matches!(result, Err(LoadError::MissingSymbol(_))));
}

#[test]
fn can_create_push_scoped_node() {
    let tsg = r#"
      (identifier) @id {
         node scope
         attr (scope) is_exported
         node result
         attr (result) type = "push_scoped_symbol", symbol = (source-text @id), scope = scope
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(
        python,
        tsg,
        &[
            "[test.py(0) exported scope]", //
            "[test.py(1) push scoped a test.py(0)]",
        ],
    );
}

#[test]
fn cannot_create_push_scoped_symbol_node_without_symbol() {
    let tsg = r#"
      (identifier) @id {
         node scope
         attr (scope) is_exported
         node result
         attr (result) type = "push_scoped_symbol", scope = scope
      }
    "#;
    let python = "a";
    let result = build_stack_graph(python, tsg);
    assert!(matches!(result, Err(LoadError::MissingSymbol(_))));
}

#[test]
fn can_create_reference_node() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "push_symbol", symbol = (source-text @id), is_reference
      }
    "#;
    let python = "a";
    build_and_check_stack_graph_nodes(python, tsg, &["[test.py(0) reference a]"]);
}

#[test]
fn cannot_create_reference_node_without_symbol() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "push_symbol", is_reference
      }
    "#;
    let python = "a";
    let result = build_stack_graph(python, tsg);
    assert!(matches!(result, Err(LoadError::MissingSymbol(_))));
}

#[test]
fn can_calculate_spans() {
    let tsg = r#"
      (identifier) @id {
         node result
         attr (result) type = "pop_symbol", symbol = "test", source_node = @id, is_definition
      }
    "#;
    let python = "  a  ";
    let (graph, file) = build_stack_graph(python, tsg).unwrap();
    let node_handle = graph.nodes_for_file(file).next().unwrap();
    let source_info = graph.source_info(node_handle).unwrap();

    let span = format!(
        "{}:{}-{}:{}",
        source_info.span.start.line,
        source_info.span.start.column.utf8_offset,
        source_info.span.end.line,
        source_info.span.end.column.utf8_offset,
    );
    assert_eq!("0:2-0:3", span);

    let containing_line = source_info.containing_line.into_option().unwrap();
    let containing_line = &graph[containing_line];
    assert_eq!(containing_line, "  a  ");

    let trimmed_line = &python[source_info.span.start.trimmed_line.clone()];
    assert_eq!(trimmed_line, "a");
}
