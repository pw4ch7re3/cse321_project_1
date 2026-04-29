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
      while (i < c->n && k >= c->k[i]) i++;
      
      c = c->p[i];
    }

  i = 0;
  while (i < c->n && k > c->k[i]) i++;

  return i < c->n && k == c->k[i] ? c : nullptr;
}

/* Insertion */

void
BPTree::split_child (List *x, size_t i)
{
  List *y = x->p[i];
  List *z = allocate_list (y->next, d, y->leaf);
  size_t j;

  if (y->leaf)
    {
      y->n = d;
      z->n = d - 1;

      for (j = 0; j < d - 1; j++)
        {
          z->k[j] = y->k[d + j];
          z->v[j] = y->v[d + j];
        }
      
      y->next = z;
    }
  else
    {
      y->n = d - 1;
      z->n = d - 1;

      for (j = 0; j < d - 1; j++)
        z->k[j] = y->k[d + j];
        
      for (j = 0; j < d; j++)
        z->p[j] = y->p[d + j];
    }

  for (j = x->n; j > i; j--)
    x->p[j + 1] = x->p[j];

  x->p[i + 1] = z;

  for (j = x->n; j > i; j--)
    x->k[j] = x->k[j - 1];

  if (y->leaf) x->k[i] = z->k[0];
  else x->k[i] = y->k[d - 1];

  x->n++;
}

void
BPTree::insert_nonfull (List *x, int k, Record *v)
{
  size_t i = 0;

  if (x->leaf)
    {
      while (i < x->n && k > x->k[i]) i++;

      if (i < x->n && k == x->k[i]) x->v[i] = v;
      else
        {
          for (size_t j = x->n; j > i; j--)
            {
              x->k[j] = x->k[j - 1];
              x->v[j] = x->v[j - 1];
            }

          x->k[i] = k;
          x->v[i] = v;
          x->n++;
        }
    }
  else
    {
      while (i < x->n && k >= x->k[i]) i++;
      
      if (x->p[i]->n == 2 * d - 1)
        {
          split_child (x, i);
          
          if (k >= x->k[i]) i++;
        }

      insert_nonfull (x->p[i], k, v);
    }
}

void
BPTree::insert_item (int k, Record *v)
{
  if (!root) create_tree ();

  List *r = root;
  if (r->n == 2 * d - 1)
    {
      List *s = allocate_list (nullptr, d, false);
      
      root = s;
      s->p[0] = r;

      split_child (s, 0);
      insert_nonfull (s, k, v);
    }
  else insert_nonfull (r, k, v);
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