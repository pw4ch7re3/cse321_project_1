/**
 * @file src/include/bp_tree.h
 * 
 * @author Sungho Kim
 */
#ifndef BP_TREE_H
#define BP_TREE_H

#include <cstddef>
#include <cstdio>
#include "list.h"
#include "statistics.h"

class BPTree
{
  size_t d;
  List *root;
  size_t n_splits; // Counter for split operation

  /* Helper functions */

  void
  delete_tree (List *x)
  {
    if (!x) return;

    if (!x->leaf)
      for (size_t i = 0; i <= x->n; i++)
        delete_tree (x->p[i]);

    delete x;
  }

  // Create
  void create_tree () { root = allocate_list (d); }

  // Search
  List *search_node (int k, size_t &i);

  // Insert
  void split_child (List *x, size_t i);
  void insert_nonfull (List *x, int k, int &v);

  // Delete
  void merge_siblings (List *x, size_t i);
  void delete_node (List *x, int k);

public:
  BPTree (size_t d) : d(d), root (allocate_list (d)), n_splits (0) {}
  ~BPTree () { delete_tree (root); }

  /* Basic operations */

  void insert_item (int k, int rid);
  void delete_item (int k);

  // search_node() wrapper function
  int
  search_item (int k)
  {
    size_t i;
    List *x = search_node (k, i);
    return x ? x->rid[i] : -1;
  }

  void range_query (int start, int end, int *buffer, size_t n);

  /* Getter functions */

  size_t get_split_counter () const { return n_splits; }
  Statistics get_statistics () const;

  /* Test and debug */

  bool validate () const;
  void print_tree (FILE *out=stdout) const;
};

#endif // BP_TREE_H
