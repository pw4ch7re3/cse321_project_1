/**
 * @file src/container/bp_tree.cc
 * 
 * @brief B+ Tree Implementation
 * @author Sungho Kim
 * 
 * @todo Lazy implementation.
 */
#include "../include/bp_tree.h"

/* Search */

List *
BPTree::search_tree (int k, size_t &i)
{
  List *c = root;
  while (!c->leaf)
    {
      i = 0;
      while (i < c->n && k > c->k[i]) i++;
      
      c = c->p[i];
    }

  return k == c->k[i] ? c : nullptr;
}

/* Insertion */

void
BPTree::split_child (List *x, size_t i)
{
}

void
BPTree::insert_item (int k, Record *v)
{
  List *l = root;
  size_t i;

  while (!l->leaf)
    {
      i = 0;
      while (i < l->n && k > l->k[i]) i++;

      l = l->p[i];
    }

  if (k != l->k[i])
    {

    }
  else
    {
    
    }
}

/* Deletion */

void
BPTree::merge_siblings (List *x, size_t i)
{
}

void
BPTree::redistribute_pointers (List *x, size_t i)
{
}

void
BPTree::delete_item (int k)
{
}