/**
 * @file src/container/b_tree.cc
 * 
 * @brief B Tree Implementation.
 * @author Sungho Kim
 * 
 * @todo Lazy implementation.
 * 
 * @ref T. H. Cormen, C. E. Leiserson, R. L. Rivest, and C. Stein,
 * Introduction to Algorithms, third edition. MIT Press, 2009.
 */
#include "../include/b_tree.h"
#include <utility>

/* Search */

Node *
BTree::search_node (Node *x, int k, size_t &i)
{
    i = 0;
    while (i < x->n && k > x->k[i]) i++;

    if (i < x->n && k == x->k[i]) return x;
    else if (x->leaf)             return nullptr;
    else                          return search_node (x->p[i], k, i);
}

Node *
BTree::search_tree (int k, size_t &i)
{
    i = 0;
    return search_node (root, k, i);
}

/* Insertion */

void
BTree::split_child (Node *x, size_t i)
{
  Node *z = allocate_node (d);
  Node *y = x->p[i];
  size_t j;
  
  z->leaf = y->leaf;
  z->n = d - 1;

  for (j = 0; j < d - 1; j++)
    {
      z->k[j] = y->k[d + j];
      z->v[j] = y->v[d + j];
    }

  if (!y->leaf)
    for (j = 0; j < d; j++)
      z->p[j] = y->p[d + j];

  y->n = d - 1;

  for (j = x->n; j > i; j--)
    x->p[j + 1] = x->p[j];

  x->p[i + 1] = z;

  for (j = x->n; j > i; j--)
    {
      x->k[j] = x->k[j - 1];
      x->v[j] = x->v[j - 1];
    }

  x->k[i] = y->k[d - 1];
  x->v[i] = y->v[d - 1];
  x->n++;
}

void
BTree::insert_nonfull (Node *x, int k, Record *v)
{
  size_t i = x->n;
  if (x->leaf)
    {
      while (i > 0 && k < x->k[i - 1])
        {
          x->k[i] = x->k[i - 1];
          x->v[i] = x->v[i - 1];
          i--;
        }

      x->k[i] = k;
      x->v[i] = v;
      x->n++;
    }
  else
    {
      while (i > 0 && k < x->k[i - 1]) i--;

      if (x->p[i]->n == 2 * d - 1)
        {
          split_child (x, i);
          if (k > x->k[i]) i++;
        }

      insert_nonfull (x->p[i], k, v);
    }
}

void
BTree::insert_item (int k, Record *v)
{
  Node *r = root;
  if (r->n == 2 * d - 1)
    {
      Node *s = allocate_node (d);
      
      root = s;
      s->leaf = false;
      s->p[0] = r;

      split_child (s, 0);
      insert_nonfull (s, k, v);
    }
  else insert_nonfull (r, k, v);
}

/* Deletion */

void
BTree::merge_siblings (Node *x, size_t i)
{
  Node *y = x->p[i];
  Node *z = x->p[i + 1];
  size_t j;

  y->k[d - 1] = x->k[i];
  y->v[d - 1] = x->v[i];
  
  for (j = 0; j < z->n; j++)
    {
      y->k[d + j] = z->k[j];
      y->v[d + j] = z->v[j];
    }

  if (!y->leaf)
    for (j = 0; j <= z->n; j++)
      y->p[d + j] = z->p[j];

  for (j = i + 1; j < x->n; j++)
    {
      x->k[j - 1] = x->k[j];
      x->v[j - 1] = x->v[j];
    }

  for (j = i + 2; j <= x->n; j++)
    x->p[j - 1] = x->p[j];

  y->n += z->n + 1;
  x->n--;

  delete z;
}

void
BTree::delete_node (Node *x, int k)
{
  size_t i = 0, j;

  while (i < x->n && k > x->k[i]) i++;

  if (x->leaf && i < x->n && k == x->k[i]) // Case 1
    {
      for (j = i; j < x->n - 1; j++)
        {
          x->k[j] = x->k[j + 1];
          x->v[j] = x->v[j + 1];
        }
      x->n--;
    }
  else if (!x->leaf && i < x->n && k == x->k[i])
    {
      Node *y = x->p[i];
      Node *z = x->p[i + 1];
      Node *w;

      if (y->n >= d) // Case 2a
        {
          w = y;
          while (!w->leaf) w = w->p[w->n];
          
          x->k[i] = w->k[w->n - 1];
          x->v[i] = w->v[w->n - 1];
          
          delete_node (y, w->k[w->n - 1]);
        }
      else if (z->n >= d) // Case 2b
        {
          w = z;
          while (!w->leaf) w = w->p[0];
          
          x->k[i] = w->k[0];
          x->v[i] = w->v[0];
          
          delete_node (z, w->k[0]);
        }
      else // Case 2c
        {
          merge_siblings (x, i);
          delete_node (y, k);
        }
    }
  else if (!x->leaf)
    {
      bool out_of_bounds = i >= x->n;
      Node *y = x->p[i];
      Node *z = out_of_bounds ? x->p[i - 1] : x->p[i + 1];

      if (y->n == d - 1 && z->n >= d) // Case 3a
        {
          if (out_of_bounds)
            {
              std::swap (x->k[i - 1], z->k[z->n - 1]);
              std::swap (x->v[i - 1], z->v[z->n - 1]);

              for (j = y->n; j > 0; j--)
                {
                  y->k[j] = y->k[j - 1];
                  y->v[j] = y->v[j - 1];
                }

              y->k[0] = z->k[z->n - 1];
              y->v[0] = z->v[z->n - 1];

              if (!y->leaf && !z->leaf)
                {
                  for (j = y->n + 1; j > 0; j--)
                    y->p[j] = y->p[j - 1];

                  y->p[0] = z->p[z->n];
                }

              y->n++;
              z->n--;
            }
          else
            {
              std::swap (x->k[i], z->k[0]);
              std::swap (x->v[i], z->v[0]);

              y->k[y->n] = z->k[0];
              y->v[y->n] = z->v[0];

              for (j = 0; j < z->n - 1; j++)
                {
                  z->k[j] = z->k[j + 1];
                  z->v[j] = z->v[j + 1];
                }

              if (!y->leaf && !z->leaf)
                {
                  y->p[y->n + 1] = z->p[0];

                  for (j = 0; j < z->n; j++)
                    z->p[j] = z->p[j + 1];
                }

              y->n++;
              z->n--;
            }
        }
      else if (y->n == d - 1 && z->n == d - 1) // Case 3b
        {
          if (out_of_bounds)
            {
              merge_siblings (x, i - 1);
              y = x->p[i - 1];
            }
          else merge_siblings (x, i);
        }

      delete_node (y, k);
    }
  else return; // Failed to delete the key K.
}

void
BTree::delete_item (int k)
{
  if (!root) return;
  
  delete_node (root, k);

  if (!root->leaf && root->n == 0)
    {
      Node *r = root;
      
      root = root->p[0];
      
      delete r;
    }
}