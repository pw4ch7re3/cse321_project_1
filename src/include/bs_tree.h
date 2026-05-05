/**
 * @file src/include/bs_tree.h
 * 
 * @author Sungho Kim
 */
#ifndef BS_TREE_H
#define BS_TREE_H

#include <cstdio>
#include <cstddef>
#include "node.h"
#include "statistics.h"

class BSTree
{
  size_t d;
  Node *root;
  size_t n_splits; // Counter for split operation
  size_t n_merges; // Counter for split operation

  /* Helper functions */

  void
  delete_tree (Node *x)
  {
    if (!x) return;

    if (!x->leaf)
      for (size_t i = 0; i <= x->n; i++)
        delete_tree (x->p[i]);

    delete x;
  }

  // Create
  void create_tree () { root = allocate_node (d); }

  // Search
  Node *search_node (Node *x, int k, size_t &i);

  // Insert
  void split_child (Node *x, size_t i);
  void insert_nonfull (Node *x, int k, Record *v);

  // Delete
  void merge_siblings (Node *x, size_t i);
  void delete_node (Node *x, int k);

public:
  BSTree (size_t d) : d (d), root (allocate_node (d)),
                      n_splits (0), n_merges (0) {}
  ~BSTree () { delete_tree (root); }

  /* Basic operations */
  
  // search_node() wrapper function
  Record *
  search_item (int k)
  {
    size_t i;
    Node *x = search_node (root, k, i);
    return x ? x->v[i] : nullptr;
  }

  void insert_item (int k, Record *v);
  void delete_item (int k);

  /* Getter functions */

  size_t get_split_counter () const { return n_splits; }
  size_t get_merge_counter () const { return n_merges; }
  Statistics get_statistics () const;

  /* Test and debug */

  bool validate () const;
  void print_tree (FILE *out=stdout) const;
};

#endif // BS_TREE_H
