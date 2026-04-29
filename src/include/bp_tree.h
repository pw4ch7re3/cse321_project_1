/**
 * @file src/include/bp_tree.h
 * 
 * @author Sungho Kim
 */
#ifndef BP_TREE_H
#define BP_TREE_H

#include "list.h"

class BPTree
{
  int d;
  List *root;

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

  void split_child (List *x, size_t i);

  void merge_siblings (List *x, size_t i);
  void redistribute_pointers (List *x, size_t i);

public:
  BPTree ();
  ~BPTree () { delete_tree (root); }

  /* Basic operations */

  List *search_tree (int k, size_t &i);
  void insert_item (int k, Record *v);
  void delete_item (int k);

  /* Wrapper functions */

  Record *
  search_item (int k)
  {
    size_t i;
    List *x = search_tree (k, i);
    return x ? x->v[i] : nullptr;
  }
};

#endif // BP_TREE_H