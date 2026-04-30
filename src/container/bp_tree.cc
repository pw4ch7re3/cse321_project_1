/**
 * @file src/container/bp_tree.cc
 * 
 * @brief B+ Tree Implementation
 * @author Sungho Kim
 * 
 * @todo Lazy implementation.
 * 
 * @ref Junghoon Kim, "Chapter 11. Indexing", Introduction to
 * Database. UNIST, 2026.
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
  List *y = x->p[i];
  List *z = x->p[i + 1];
  size_t j;

  if (y->leaf)
    {
      for (j = 0; j < z->n; j++)
        {
          y->k[y->n + j] = z->k[j];
          y->v[y->n + j] = z->v[j];
        }

      y->n += z->n;
      y->next = z->next;
    }
  else
    {
      y->k[y->n] = x->k[i];

      for (j = 0; j < z->n; j++)
        y->k[y->n + 1 + j] = z->k[j];
        
      for (j = 0; j <= z->n; j++)
        y->p[y->n + 1 + j] = z->p[j];
        
      y->n += z->n + 1;
    }

  for (j = i + 1; j < x->n; j++)
    x->k[j - 1] = x->k[j];
    
  for (j = i + 2; j <= x->n; j++)
    x->p[j - 1] = x->p[j];

  x->n--;

  delete z;
}

void
BPTree::delete_node (List *x, int k)
{
  size_t i = 0, j;

  if (x->leaf)
    {
      while (i < x->n && k > x->k[i]) i++;

      if (i < x->n && k == x->k[i])
        {
          for (j = i; j < x->n - 1; j++)
            {
              x->k[j] = x->k[j + 1];
              x->v[j] = x->v[j + 1];
            }

          x->n--;
        }
    }
  else
    {
      while (i < x->n && k >= x->k[i]) i++;

      bool out_of_bounds = i >= x->n;
      List *y = x->p[i];
      List *z = out_of_bounds ? x->p[i - 1] : x->p[i + 1];

      if (y->n == d - 1 && z->n >= d)
        {
          if (out_of_bounds)
            {
              if (y->leaf)
                {
                  for (j = y->n; j > 0; j--)
                    {
                      y->k[j] = y->k[j - 1];
                      y->v[j] = y->v[j - 1];
                    }

                  y->k[0] = z->k[z->n - 1];
                  y->v[0] = z->v[z->n - 1];

                  x->k[i - 1] = z->k[z->n - 1];
                }
              else
                {
                  for (j = y->n; j > 0; j--)
                    y->k[j] = y->k[j - 1];
                  
                  for (j = y->n + 1; j > 0; j--)
                  y->p[j] = y->p[j - 1];

                  y->k[0] = x->k[i - 1];
                  y->p[0] = z->p[z->n];

                  x->k[i - 1] = z->k[z->n - 1];
                }

              y->n++;
              z->n--;
            }
          else
            {
              if (y->leaf)
                {
                  y->k[y->n] = z->k[0];
                  y->v[y->n] = z->v[0];

                  x->k[i] = z->k[1];

                  for (j = 0; j < z->n - 1; j++)
                    {
                      z->k[j] = z->k[j + 1];
                      z->v[j] = z->v[j + 1];
                    }
                }
              else
                {
                  y->k[y->n] = x->k[i];
                  y->p[y->n + 1] = z->p[0];

                  x->k[i] = z->k[0];

                  for (j = 0; j < z->n - 1; j++)
                    z->k[j] = z->k[j + 1];

                  for (j = 0; j < z->n; j++)
                    z->p[j] = z->p[j + 1];
                }

              y->n++;
              z->n--;  
            }
        }
      else if (y->n == d - 1 && z->n == d - 1)
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
}

void
BPTree::delete_item (int k)
{
  if (!root) return;

  delete_node (root, k);

  if (!root->leaf && root->n == 0)
    {
      List *r = root;

      root = root->p[0];

      delete r;
    }
}