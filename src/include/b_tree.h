/**
 * @file src/include/b_tree.h
 * 
 * @author Sungho Kim
 */
#ifndef B_TREE_H
#define B_TREE_H

#include "node.h"

class BTree
{
  size_t d;
  Node *root;

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

  Node *search_node (Node *x, int k, size_t &i);

  void split_child (Node *x, size_t i);
  void insert_nonfull (Node *x, int k, Record *v);

  void merge_siblings (Node *x, size_t i);
  void delete_node (Node *x, int k);

public:
  BTree (size_t d) : d (d) { root = allocate_node (d); }
  ~BTree () { delete_tree (root); }

  /* Basic operations */

  Node *search_tree (int k, size_t &i);
  void insert_item (int k, Record *v);
  void delete_item (int k);

  /* Wrapper functions */

  Record *
  search_item (int k)
  {
    size_t i;
    Node *x = search_tree (k, i);
    return x ? x->v[i] : nullptr;
  }
};

#endif // B_TREE_H