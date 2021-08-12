#ifndef STACK_GRAPHS_H_
#define STACK_GRAPHS_H_

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// The handle of an empty list.
#define SG_LIST_EMPTY_HANDLE 4294967295

// The local_id of the singleton root node.
#define SG_ROOT_NODE_ID 0

// The local_id of the singleton "jump to scope" node.
#define SG_JUMP_TO_NODE_ID 1

// Describes in which direction the content of a deque is stored in memory.
enum sg_deque_direction {
    SG_DEQUE_FORWARDS,
    SG_DEQUE_BACKWARDS,
};

// The different kinds of node that can appear in a stack graph.
enum sg_node_kind {
    // Removes everything from the current scope stack.
    SG_NODE_KIND_DROP_SCOPES,
    // A node that can be referred to on the scope stack, which allows "jump to" nodes in any
    // other part of the graph can jump back here.
    SG_NODE_KIND_EXPORTED_SCOPE,
    // A node internal to a single file.  This node has no effect on the symbol or scope stacks;
    // it's just used to add structure to the graph.
    SG_NODE_KIND_INTERNAL_SCOPE,
    // The singleton "jump to" node, which allows a name binding path to jump back to another part
    // of the graph.
    SG_NODE_KIND_JUMP_TO,
    // Pops a scoped symbol from the symbol stack.  If the top of the symbol stack doesn't match
    // the requested symbol, or if the top of the symbol stack doesn't have an attached scope
    // list, then the path is not allowed to enter this node.
    SG_NODE_KIND_POP_SCOPED_SYMBOL,
    // Pops a symbol from the symbol stack.  If the top of the symbol stack doesn't match the
    // requested symbol, then the path is not allowed to enter this node.
    SG_NODE_KIND_POP_SYMBOL,
    // Pushes a scoped symbol onto the symbol stack.
    SG_NODE_KIND_PUSH_SCOPED_SYMBOL,
    // Pushes a symbol onto the symbol stack.
    SG_NODE_KIND_PUSH_SYMBOL,
    // The singleton root node, which allows a name binding path to cross between files.
    SG_NODE_KIND_ROOT,
};

// Manages the state of a collection of partial paths to be used in the path-stitching algorithm.
struct sg_partial_path_arena;

// Contains a "database" of partial paths.
//
// This type is meant to be a lazily loaded "view" into a proper storage layer.  During the
// path-stitching algorithm, we repeatedly try to extend a currently incomplete path with any
// partial paths that are compatible with it.  For large codebases, or projects with a large
// number of dependencies, it can be prohibitive to load in _all_ of the partial paths up-front.
// We've written the path-stitching algorithm so that you have a chance to only load in the
// partial paths that are actually needed, placing them into a sg_partial_path_database instance
// as they're needed.
struct sg_partial_path_database;

// A list of paths found by the path-finding algorithm.
struct sg_partial_path_list;

// Manages the state of a collection of paths built up as part of the path-finding algorithm.
struct sg_path_arena;

// A list of paths found by the path-finding algorithm.
struct sg_path_list;

// Contains all of the nodes and edges that make up a stack graph.
struct sg_stack_graph;

// A name that we are trying to resolve using stack graphs.
//
// This typically represents a portion of an identifier as it appears in the source language.  It
// can also represent some other "operation" that can occur in source code, and which needs to be
// modeled in a stack graph — for instance, many languages will use a "fake" symbol named `.` to
// represent member access.
struct sg_symbol {
    const char *symbol;
    size_t symbol_len;
};

// An array of all of the symbols in a stack graph.  Symbol handles are indices into this array.
// There will never be a valid symbol at index 0; a handle with the value 0 represents a missing
// symbol.
struct sg_symbols {
    const struct sg_symbol *symbols;
    size_t count;
};

// A handle to a symbol in a stack graph.  A zero handle represents a missing symbol.
//
// We deduplicate symbols in a stack graph — that is, we ensure that there are never multiple
// `struct sg_symbol` instances with the same content.  That means that you can compare symbol
// handles using simple equality, without having to dereference them.
typedef uint32_t sg_symbol_handle;

// A source file that we have extracted stack graph data from.
//
// It's up to you to choose what names to use for your files, but they must be unique within a
// stack graph.  If you are analyzing files from the local filesystem, the file's path is a good
// choice.  If your files belong to packages or repositories, they should include the package or
// repository IDs to make sure that files in different packages or repositories don't clash with
// each other.
struct sg_file {
    const char *name;
    size_t name_len;
};

// An array of all of the files in a stack graph.  File handles are indices into this array.
// There will never be a valid file at index 0; a handle with the value 0 represents a missing
// file.
struct sg_files {
    const struct sg_file *files;
    size_t count;
};

// A handle to a file in a stack graph.  A zero handle represents a missing file.
//
// We deduplicate files in a stack graph — that is, we ensure that there are never multiple
// `struct sg_file` instances with the same filename.  That means that you can compare file
// handles using simple equality, without having to dereference them.
typedef uint32_t sg_file_handle;

// Uniquely identifies a node in a stack graph.
//
// Each node (except for the _root node_ and _jump to scope_ node) lives in a file, and has a
// _local ID_ that must be unique within its file.
struct sg_node_id {
    sg_file_handle file;
    uint32_t local_id;
};

// A handle to a node in a stack graph.  A zero handle represents a missing node.
typedef uint32_t sg_node_handle;

// A node in a stack graph.
struct sg_node {
    enum sg_node_kind kind;
    struct sg_node_id id;
    // The symbol associated with this node.  For push nodes, this is the symbol that will be
    // pushed onto the symbol stack.  For pop nodes, this is the symbol that we expect to pop off
    // the symbol stack.  For all other node types, this will be null.
    sg_symbol_handle symbol;
    // The scope associated with this node.  For push scope nodes, this is the scope that will be
    // attached to the symbol before it's pushed onto the symbol stack.  For all other node types,
    // this will be null.
    sg_node_handle scope;
    // Whether this node is "clickable".  For push nodes, this indicates that the node represents
    // a reference in the source.  For pop nodes, this indicates that the node represents a
    // definition in the source.  For all other node types, this field will be unused.
    bool is_clickable;
};

// An array of all of the nodes in a stack graph.  Node handles are indices into this array.
// There will never be a valid node at index 0; a handle with the value 0 represents a missing
// node.
struct sg_nodes {
    const struct sg_node *nodes;
    size_t count;
};

// Connects two nodes in a stack graph.
//
// These edges provide the basic graph connectivity that allow us to search for name binding paths
// in a stack graph.  (Though not all sequence of edges is a well-formed name binding: the nodes
// that you encounter along the path must also satisfy all of the rules for maintaining correct
// symbol and scope stacks.)
struct sg_edge {
    sg_node_handle source;
    sg_node_handle sink;
    int32_t precedence;
};

// A handle to an element of a scope stack.  A zero handle represents a missing scope stack.  A
// UINT32_MAX handle represents an empty scope stack.
typedef uint32_t sg_scope_stack_cell_handle;

// A sequence of exported scopes, used to pass name-binding context around a stack graph.
struct sg_scope_stack {
    // The handle of the first element in the scope stack, or SG_LIST_EMPTY_HANDLE if the list is
    // empty, or 0 if the list is null.
    sg_scope_stack_cell_handle cells;
};

// A symbol with a possibly empty list of exported scopes attached to it.
struct sg_scoped_symbol {
    sg_symbol_handle symbol;
    struct sg_scope_stack scopes;
};

// A handle to an element of a symbol stack.  A zero handle represents a missing symbol stack.  A
// UINT32_MAX handle represents an empty symbol stack.
typedef uint32_t sg_symbol_stack_cell_handle;

// An element of a symbol stack.
struct sg_symbol_stack_cell {
    // The scoped symbol at this position in the symbol stack.
    struct sg_scoped_symbol head;
    // The handle of the next element in the symbol stack, or SG_LIST_EMPTY_HANDLE if this is the
    // last element.
    sg_symbol_stack_cell_handle tail;
};

// The array of all of the symbol stack content in a path arena.
struct sg_symbol_stack_cells {
    const struct sg_symbol_stack_cell *cells;
    size_t count;
};

// A sequence of symbols that describe what we are currently looking for while in the middle of
// the path-finding algorithm.
struct sg_symbol_stack {
    // The handle of the first element in the symbol stack, or SG_LIST_EMPTY_HANDLE if the list is
    // empty, or 0 if the list is null.
    sg_symbol_stack_cell_handle cells;
    size_t length;
};

// An element of a scope stack.
struct sg_scope_stack_cell {
    // The exported scope at this position in the scope stack.
    sg_node_handle head;
    // The handle of the next element in the scope stack, or SG_LIST_EMPTY_HANDLE if this is the
    // last element.
    sg_scope_stack_cell_handle tail;
};

// The array of all of the scope stack content in a path arena.
struct sg_scope_stack_cells {
    const struct sg_scope_stack_cell *cells;
    size_t count;
};

// Details about one of the edges in a name-binding path
struct sg_path_edge {
    struct sg_node_id source_node_id;
    int32_t precedence;
};

// A handle to an element of a path edge list.  A zero handle represents a missing path edge list.
// A UINT32_MAX handle represents an empty path edge list.
typedef uint32_t sg_path_edge_list_cell_handle;

// An element of a path edge list.
struct sg_path_edge_list_cell {
    // The path edge at this position in the path edge list.
    struct sg_path_edge head;
    // The handle of the next element in the path edge list, or SG_LIST_EMPTY_HANDLE if this is
    // the last element.
    sg_path_edge_list_cell_handle tail;
    // The handle of the reversal of this list.
    sg_path_edge_list_cell_handle reversed;
};

// The array of all of the path edge list content in a path arena.
struct sg_path_edge_list_cells {
    const struct sg_path_edge_list_cell *cells;
    size_t count;
};

// The edges in a path keep track of precedence information so that we can correctly handle
// shadowed definitions.
struct sg_path_edge_list {
    // The handle of the first element in the edge list, or SG_LIST_EMPTY_HANDLE if the list is
    // empty, or 0 if the list is null.
    sg_path_edge_list_cell_handle cells;
    enum sg_deque_direction direction;
    size_t length;
};

// A sequence of edges from a stack graph.  A _complete_ path represents a full name binding in a
// source language.
struct sg_path {
    sg_node_handle start_node;
    sg_node_handle end_node;
    struct sg_symbol_stack symbol_stack;
    struct sg_scope_stack scope_stack;
    struct sg_path_edge_list edges;
};

// A handle to an element of a partial scope stack.  A zero handle represents a missing partial
// scope stack.  A UINT32_MAX handle represents an empty partial scope stack.
typedef uint32_t sg_partial_scope_stack_cell_handle;

// Represents an unknown list of exported scopes.
typedef uint32_t sg_scope_stack_variable;

// A pattern that might match against a scope stack.  Consists of a (possibly empty) list of
// exported scopes, along with an optional scope stack variable.
struct sg_partial_scope_stack {
    // The handle of the first element in the partial scope stack, or SG_LIST_EMPTY_HANDLE if the
    // list is empty, or 0 if the list is null.
    sg_partial_scope_stack_cell_handle cells;
    enum sg_deque_direction direction;
    // The scope stack variable representing the unknown content of a partial scope stack, or 0 if
    // the variable is missing.  (If so, this partial scope stack can only match a scope stack
    // with exactly the list of scopes in `cells`, instead of any scope stack with those scopes as
    // a prefix.)
    sg_scope_stack_variable variable;
};

// A symbol with an unknown, but possibly empty, list of exported scopes attached to it.
struct sg_partial_scoped_symbol {
    sg_symbol_handle symbol;
    struct sg_partial_scope_stack scopes;
};

// A handle to an element of a partial symbol stack.  A zero handle represents a missing partial
// symbol stack.  A UINT32_MAX handle represents an empty partial symbol stack.
typedef uint32_t sg_partial_symbol_stack_cell_handle;

// An element of a partial symbol stack.
struct sg_partial_symbol_stack_cell {
    // The partial scoped symbol at this position in the partial symbol stack.
    struct sg_partial_scoped_symbol head;
    // The handle of the next element in the partial symbol stack, or SG_LIST_EMPTY_HANDLE if this
    // is the last element.
    sg_partial_symbol_stack_cell_handle tail;
    // The handle of the reversal of this partial scope stack.
    sg_partial_symbol_stack_cell_handle reversed;
};

// The array of all of the partial symbol stack content in a partial path arena.
struct sg_partial_symbol_stack_cells {
    const struct sg_partial_symbol_stack_cell *cells;
    size_t count;
};

// A pattern that might match against a symbol stack.  Consists of a (possibly empty) list of
// partial scoped symbols.
//
// (Note that unlike partial scope stacks, we don't store any "symbol stack variable" here.  We
// could!  But with our current path-finding rules, every partial path will always have exactly
// one symbol stack variable, which will appear at the end of its precondition and postcondition.
// So for simplicity we just leave it out.  At some point in the future we might add it in so that
// the symbol and scope stack formalisms and implementations are more obviously symmetric.)
struct sg_partial_symbol_stack {
    // The handle of the first element in the partial symbol stack, or SG_LIST_EMPTY_HANDLE if the
    // list is empty, or 0 if the list is null.
    sg_partial_symbol_stack_cell_handle cells;
    enum sg_deque_direction direction;
};

// An element of a partial scope stack.
struct sg_partial_scope_stack_cell {
    // The exported scope at this position in the partial scope stack.
    sg_node_handle head;
    // The handle of the next element in the partial scope stack, or SG_LIST_EMPTY_HANDLE if this
    // is the last element.
    sg_path_edge_list_cell_handle tail;
    // The handle of the reversal of this partial scope stack.
    sg_path_edge_list_cell_handle reversed;
};

// The array of all of the partial scope stack content in a partial path arena.
struct sg_partial_scope_stack_cells {
    const struct sg_partial_scope_stack_cell *cells;
    size_t count;
};

// Details about one of the edges in a partial path
struct sg_partial_path_edge {
    struct sg_node_id source_node_id;
    int32_t precedence;
};

// A handle to an element of a partial path edge list.  A zero handle represents a missing partial
// path edge list.  A UINT32_MAX handle represents an empty partial path edge list.
typedef uint32_t sg_partial_path_edge_list_cell_handle;

// An element of a partial path edge list.
struct sg_partial_path_edge_list_cell {
    // The partial path edge at this position in the partial path edge list.
    struct sg_partial_path_edge head;
    // The handle of the next element in the partial path edge list, or SG_LIST_EMPTY_HANDLE if
    // this is the last element.
    sg_partial_path_edge_list_cell_handle tail;
    // The handle of the reversal of this list.
    sg_partial_path_edge_list_cell_handle reversed;
};

// The array of all of the partial path edge list content in a partial path arena.
struct sg_partial_path_edge_list_cells {
    const struct sg_partial_path_edge_list_cell *cells;
    size_t count;
};

// The edges in a path keep track of precedence information so that we can correctly handle
// shadowed definitions.
struct sg_partial_path_edge_list {
    // The handle of the first element in the edge list, or SG_LIST_EMPTY_HANDLE if the list is
    // empty, or 0 if the list is null.
    sg_partial_path_edge_list_cell_handle cells;
    enum sg_deque_direction direction;
    size_t length;
};

// A portion of a name-binding path.
//
// Partial paths can be computed _incrementally_, in which case all of the edges in the partial
// path belong to a single file.  At query time, we can efficiently concatenate partial paths to
// yield a name-binding path.
//
// Paths describe the contents of the symbol stack and scope stack at the end of the path.
// Partial paths, on the other hand, have _preconditions_ and _postconditions_ for each stack.
// The precondition describes what the stack must look like for us to be able to concatenate this
// partial path onto the end of a path.  The postcondition describes what the resulting stack
// looks like after doing so.
//
// The preconditions can contain _scope stack variables_, which describe parts of the scope stack
// (or parts of a scope symbol's attached scope list) whose contents we don't care about.  The
// postconditions can _also_ refer to those variables, and describe how those variable parts of
// the input scope stacks are carried through unmodified into the resulting scope stack.
struct sg_partial_path {
    sg_node_handle start_node;
    sg_node_handle end_node;
    struct sg_partial_symbol_stack symbol_stack_precondition;
    struct sg_partial_symbol_stack symbol_stack_postcondition;
    struct sg_partial_scope_stack scope_stack_precondition;
    struct sg_partial_scope_stack scope_stack_postcondition;
    struct sg_partial_path_edge_list edges;
};

// Implements a phased forward path-stitching algorithm.
//
// Our overall goal is to start with a set of _seed_ paths, and to repeatedly extend each path by
// appending a compatible partial path onto the end of it.  (If there are multiple compatible
// partial paths, we append each of them separately, resulting in more than one extension for the
// current path.)
//
// We perform this processing in _phases_.  At the start of each phase, we have a _current set_ of
// paths that need to be processed.  As we extend those paths, we add the extensions to the set of
// paths to process in the _next_ phase.  Phases are processed one at a time, each time you invoke
// `sg_forward_path_stitcher_process_next_phase`.
//
// After each phase has completed, the `previous_phase_paths` and `previous_phase_paths_length`
// fields give you all of the paths that were discovered during that phase.  That gives you a
// chance to add to the `sg_partial_path_database` all of the partial paths that we might need to
// extend those paths with before invoking the next phase.
struct sg_forward_path_stitcher {
    // The new candidate paths that were discovered in the most recent phase.
    const struct sg_path *previous_phase_paths;
    // The number of new candidate paths that were discovered in the most recent phase.  If this
    // is 0, then the path stitching algorithm is complete.
    size_t previous_phase_paths_length;
};

// The handle of the singleton root node.
#define SG_ROOT_NODE_HANDLE 1

// The handle of the singleton "jump to scope" node.
#define SG_JUMP_TO_NODE_HANDLE 2

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Creates a new, initially empty stack graph.
struct sg_stack_graph *sg_stack_graph_new(void);

// Frees a stack graph, and all of its contents.
void sg_stack_graph_free(struct sg_stack_graph *graph);

// Creates a new, initially empty path arena.
struct sg_path_arena *sg_path_arena_new(void);

// Frees a path arena, and all of its contents.
void sg_path_arena_free(struct sg_path_arena *paths);

// Creates a new, initially empty partial path arena.
struct sg_partial_path_arena *sg_partial_path_arena_new(void);

// Frees a path arena, and all of its contents.
void sg_partial_path_arena_free(struct sg_partial_path_arena *partials);

// Creates a new, initially empty partial path database.
struct sg_partial_path_database *sg_partial_path_database_new(void);

// Frees a partial path database, and all of its contents.
void sg_partial_path_database_free(struct sg_partial_path_database *db);

// Returns a reference to the array of symbol data in this stack graph.  The resulting array
// pointer is only valid until the next call to any function that mutates the stack graph.
struct sg_symbols sg_stack_graph_symbols(const struct sg_stack_graph *graph);

// Adds new symbols to the stack graph.  You provide an array of symbol content, and an output
// array, which must have the same length.  We will place each symbol's handle in the output
// array.
//
// We ensure that there is only ever one copy of a particular symbol stored in the graph — we
// guarantee that identical symbols will have the same handles, meaning that you can compare the
// handles using simple integer equality.
//
// We copy the symbol data into the stack graph.  The symbol content you pass in does not need to
// outlive the call to this function.
//
// Each symbol must be a valid UTF-8 string.  If any symbol isn't valid UTF-8, it won't be added
// to the stack graph, and the corresponding entry in the output array will be the null handle.
void sg_stack_graph_add_symbols(struct sg_stack_graph *graph,
                                size_t count,
                                const char *const *symbols,
                                const size_t *lengths,
                                sg_symbol_handle *handles_out);

// Returns a reference to the array of file data in this stack graph.  The resulting array pointer
// is only valid until the next call to any function that mutates the stack graph.
struct sg_files sg_stack_graph_files(const struct sg_stack_graph *graph);

// Adds new files to the stack graph.  You provide an array of file content, and an output array,
// which must have the same length.  We will place each file's handle in the output array.
//
// There can only ever be one file with a particular name in the graph.  If you try to add a file
// with a name that already exists, you'll get the same handle as a result.
//
// We copy the filenames into the stack graph.  The filenames you pass in do not need to outlive
// the call to this function.
//
// Each filename must be a valid UTF-8 string.  If any filename isn't valid UTF-8, it won't be
// added to the stack graph, and the corresponding entry in the output array will be the null
// handle.
void sg_stack_graph_add_files(struct sg_stack_graph *graph,
                              size_t count,
                              const char *const *files,
                              const size_t *lengths,
                              sg_file_handle *handles_out);

// Returns a reference to the array of nodes in this stack graph.  The resulting array pointer is
// only valid until the next call to any function that mutates the stack graph.
struct sg_nodes sg_stack_graph_nodes(const struct sg_stack_graph *graph);

// Adds new nodes to the stack graph.  You provide an array of `struct sg_node` instances.  You
// also provide an output array, which must have the same length as `nodes`, in which we will
// place each node's handle in the stack graph.
//
// We copy the node content into the stack graph.  The array you pass in does not need to outlive
// the call to this function.
//
// You cannot add new instances of the root node or "jump to scope" node, since those are
// singletons and already exist in the stack graph.
//
// If any node that you pass in is invalid, it will not be added to the graph, and the
// corresponding entry in the `handles_out` array will be null.  (Note that includes trying to add
// a node with the same ID as an existing node, since all nodes must have unique IDs.)
void sg_stack_graph_add_nodes(struct sg_stack_graph *graph,
                              size_t count,
                              const struct sg_node *nodes,
                              sg_node_handle *handles_out);

// Adds new edges to the stack graph.  You provide an array of `struct sg_edges` instances.  A
// stack graph can contain at most one edge between any two nodes.  It is not an error if you try
// to add an edge that already exists, but it won't have any effect on the graph.
void sg_stack_graph_add_edges(struct sg_stack_graph *graph,
                              size_t count,
                              const struct sg_edge *edges);

// Returns a reference to the array of symbol stack content in a path arena.  The resulting array
// pointer is only valid until the next call to any function that mutates the path arena.
struct sg_symbol_stack_cells sg_path_arena_symbol_stack_cells(const struct sg_path_arena *paths);

// Adds new symbol stacks to the path arena.  `count` is the number of symbol stacks you want to
// create.  The content of each symbol stack comes from two arrays.  The `lengths` array must have
// `count` elements, and provides the number of symbols in each symbol stack.  The `symbols` array
// contains the contents of each of these symbol stacks in one contiguous array.  Its length must
// be the sum of all of the counts in the `lengths` array.
//
// You must also provide an `out` array, which must also have room for `count` elements.  We will
// fill this array in with the `sg_symbol_stack` instances for each symbol stack that is created.
void sg_path_arena_add_symbol_stacks(struct sg_path_arena *paths,
                                     size_t count,
                                     const struct sg_scoped_symbol *symbols,
                                     const size_t *lengths,
                                     struct sg_symbol_stack *out);

// Returns a reference to the array of scope stack content in a path arena.  The resulting array
// pointer is only valid until the next call to any function that mutates the path arena.
struct sg_scope_stack_cells sg_path_arena_scope_stack_cells(const struct sg_path_arena *paths);

// Adds new scope stacks to the path arena.  `count` is the number of scope stacks you want to
// create.  The content of each scope stack comes from two arrays.  The `lengths` array must have
// `count` elements, and provides the number of scopes in each scope stack.  The `scopes` array
// contains the contents of each of these scope stacks in one contiguous array.  Its length must
// be the sum of all of the counts in the `lengths` array.
//
// You must also provide an `out` array, which must also have room for `count` elements.  We will
// fill this array in with the `sg_scope_stack` instances for each scope stack that is created.
void sg_path_arena_add_scope_stacks(struct sg_path_arena *paths,
                                    size_t count,
                                    const sg_node_handle *scopes,
                                    const size_t *lengths,
                                    struct sg_scope_stack *out);

// Returns a reference to the array of path edge list content in a path arena.  The resulting
// array pointer is only valid until the next call to any function that mutates the path arena.
struct sg_path_edge_list_cells sg_path_arena_path_edge_list_cells(const struct sg_path_arena *paths);

// Adds new path edge lists to the path arena.  `count` is the number of path edge lists you want
// to create.  The content of each path edge list comes from two arrays.  The `lengths` array must
// have `count` elements, and provides the number of edges in each path edge list.  The `edges`
// array contains the contents of each of these path edge lists in one contiguous array.  Its
// length must be the sum of all of the counts in the `lengths` array.
//
// You must also provide an `out` array, which must also have room for `count` elements.  We will
// fill this array in with the `sg_path_edge_list` instances for each path edge list that is
// created.
void sg_path_arena_add_path_edge_lists(struct sg_path_arena *paths,
                                       size_t count,
                                       const struct sg_path_edge *edges,
                                       const size_t *lengths,
                                       struct sg_path_edge_list *out);

// Creates a new, empty sg_path_list.
struct sg_path_list *sg_path_list_new(void);

void sg_path_list_free(struct sg_path_list *path_list);

size_t sg_path_list_count(const struct sg_path_list *path_list);

const struct sg_path *sg_path_list_paths(const struct sg_path_list *path_list);

// Finds all complete paths reachable from a set of starting nodes, placing the result into the
// `path_list` output parameter.  You must free the path list when you are done with it by calling
// `sg_path_list_done`.
//
// This function will not return until all reachable paths have been processed, so `graph` must
// already contain a complete stack graph.  If you have a very large stack graph stored in some
// other storage system, and want more control over lazily loading only the necessary pieces, then
// you should use sg_forward_path_stitcher.
void sg_path_arena_find_all_complete_paths(const struct sg_stack_graph *graph,
                                           struct sg_path_arena *paths,
                                           size_t starting_node_count,
                                           const sg_node_handle *starting_nodes,
                                           struct sg_path_list *path_list);

// Returns a reference to the array of partial symbol stack content in a partial path arena.  The
// resulting array pointer is only valid until the next call to any function that mutates the path
// arena.
struct sg_partial_symbol_stack_cells sg_partial_path_arena_partial_symbol_stack_cells(const struct sg_partial_path_arena *partials);

// Adds new partial symbol stacks to the partial path arena.  `count` is the number of partial
// symbol stacks you want to create.  The content of each partial symbol stack comes from two
// arrays.  The `lengths` array must have `count` elements, and provides the number of symbols in
// each partial symbol stack.  The `symbols` array contains the contents of each of these partial
// symbol stacks in one contiguous array.  Its length must be the sum of all of the counts in the
// `lengths` array.
//
// You must also provide an `out` array, which must also have room for `count` elements.  We will
// fill this array in with the `sg_partial_symbol_stack` instances for each partial symbol stack
// that is created.
void sg_partial_path_arena_add_partial_symbol_stacks(struct sg_partial_path_arena *partials,
                                                     size_t count,
                                                     const struct sg_partial_scoped_symbol *symbols,
                                                     const size_t *lengths,
                                                     struct sg_partial_symbol_stack *out);

// Returns a reference to the array of partial scope stack content in a partial path arena.  The
// resulting array pointer is only valid until the next call to any function that mutates the
// partial path arena.
struct sg_partial_scope_stack_cells sg_partial_path_arena_partial_scope_stack_cells(const struct sg_partial_path_arena *partials);

// Adds new partial scope stacks to the partial path arena.  `count` is the number of partial
// scope stacks you want to create.  The content of each partial scope stack comes from three
// arrays.  The `lengths` array must have `count` elements, and provides the number of scopes in
// each scope stack.  The `scopes` array contains the contents of each of these scope stacks in
// one contiguous array.  Its length must be the sum of all of the counts in the `lengths` array.
// The `variables` array must have `count` elements, and provides the optional scope stack
// variable for each partial scope stack.
//
// You must also provide an `out` array, which must also have room for `count` elements.  We will
// fill this array in with the `sg_partial_scope_stack` instances for each partial scope stack
// that is created.
void sg_partial_path_arena_add_partial_scope_stacks(struct sg_partial_path_arena *partials,
                                                    size_t count,
                                                    const sg_node_handle *scopes,
                                                    const size_t *lengths,
                                                    const sg_scope_stack_variable *variables,
                                                    struct sg_partial_scope_stack *out);

// Returns a reference to the array of partial path edge list content in a partial path arena.
// The resulting array pointer is only valid until the next call to any function that mutates the
// partial path arena.
struct sg_partial_path_edge_list_cells sg_partial_path_arena_partial_path_edge_list_cells(const struct sg_partial_path_arena *partials);

// Adds new partial path edge lists to the partial path arena.  `count` is the number of partial
// path edge lists you want to create.  The content of each partial path edge list comes from two
// arrays.  The `lengths` array must have `count` elements, and provides the number of edges in
// each partial path edge list.  The `edges` array contains the contents of each of these partial
// path edge lists in one contiguous array.  Its length must be the sum of all of the counts in
// the `lengths` array.
//
// You must also provide an `out` array, which must also have room for `count` elements.  We will
// fill this array in with the `sg_partial_path_edge_list` instances for each partial path edge
// list that is created.
void sg_partial_path_arena_add_partial_path_edge_lists(struct sg_partial_path_arena *partials,
                                                       size_t count,
                                                       const struct sg_partial_path_edge *edges,
                                                       const size_t *lengths,
                                                       struct sg_partial_path_edge_list *out);

// Creates a new, empty sg_partial_path_list.
struct sg_partial_path_list *sg_partial_path_list_new(void);

void sg_partial_path_list_free(struct sg_partial_path_list *partial_path_list);

size_t sg_partial_path_list_count(const struct sg_partial_path_list *partial_path_list);

const struct sg_partial_path *sg_partial_path_list_paths(const struct sg_partial_path_list *partial_path_list);

// Finds all partial paths in a file that are _productive_ and _as complete as possible_, placing
// the result into the `partial_path_list` output parameter.  You must free the path list when you
// are done with it by calling `sg_partial_path_list_done`.
//
// This function will not return until all reachable paths have been processed, so `graph` must
// already contain a complete stack graph.  If you have a very large stack graph stored in some
// other storage system, and want more control over lazily loading only the necessary pieces, then
// you should use sg_forward_path_stitcher.
void sg_partial_path_arena_find_partial_paths_in_file(const struct sg_stack_graph *graph,
                                                      struct sg_partial_path_arena *partials,
                                                      sg_file_handle file,
                                                      struct sg_partial_path_list *partial_path_list);

// Adds new partial paths to the partial path database.  `paths` is the array of partial paths
// that you want to add; `count` is the number of them.
//
// We copy the partial path content into the partial path database.  The array you pass in does
// not need to outlive the call to this function.
//
// You should take care not to add a partial path to the database multiple times.  This won't
// cause an _error_, in that nothing will break, but it will probably cause you to get duplicate
// paths from the path-stitching algorithm.
void sg_partial_path_database_add_partial_paths(const struct sg_stack_graph *graph,
                                                struct sg_partial_path_arena *partials,
                                                struct sg_partial_path_database *db,
                                                size_t count,
                                                const struct sg_partial_path *paths);

// Creates a new forward path stitcher that is "seeded" with a set of starting stack graph nodes.
//
// Before calling this method, you must ensure that `db` contains all of the possible partial
// paths that start with any of your requested starting nodes.
//
// Before calling `sg_forward_path_stitcher_process_next_phase` for the first time, you must
// ensure that `db` contains all possible extensions of any of those initial paths.  You can
// retrieve a list of those extensions via the `previous_phase_paths` and
// `previous_phase_paths_length` fields.
struct sg_forward_path_stitcher *sg_forward_path_stitcher_new(const struct sg_stack_graph *graph,
                                                              struct sg_path_arena *paths,
                                                              struct sg_partial_path_arena *partials,
                                                              struct sg_partial_path_database *db,
                                                              size_t count,
                                                              const sg_node_handle *starting_nodes);

// Runs the next phase of the path-stitching algorithm.  We will have built up a set of
// incomplete paths during the _previous_ phase.  Before calling this function, you must
// ensure that `db` contains all of the possible partial paths that we might want to extend
// any of those paths with.
//
// After this method returns, you can retrieve a list of the (possibly incomplete) paths that were
// encountered during this phase via the `previous_phase_paths` and `previous_phase_paths_length`
// fields.
void sg_forward_path_stitcher_process_next_phase(const struct sg_stack_graph *graph,
                                                 struct sg_path_arena *paths,
                                                 struct sg_partial_path_arena *partials,
                                                 struct sg_partial_path_database *db,
                                                 struct sg_forward_path_stitcher *stitcher);

// Frees a forward path stitcher.
void sg_forward_path_stitcher_free(struct sg_forward_path_stitcher *stitcher);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* STACK_GRAPHS_H_ */
