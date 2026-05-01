/**
 * @file src/container/bs_tree.cc
 * 
 * @brief B* Tree Implementation
 * @author Sungho Kim
 * 
 * @todo Lazy implementation.
 */
#include "../include/bs_tree.h"

/* Search */

Node *
BSTree::search_node (Node *x, int k, size_t &i)
{
    i = 0;
    while (i < x->n && k > x->k[i]) i++;

    if (i < x->n && k == x->k[i]) return x;
    else if (x->leaf)             return nullptr;
    else                          return search_node (x->p[i], k, i);
}

/* Insertion */

void
BSTree::split_child (Node *x, size_t i)
{
  if (i == x->n) i--; 

  Node *y = x->p[i];
  Node *z = x->p[i + 1];
  size_t j;

  if (z->n < 2 * d - 1)
    {
      for (j = z->n; j > 0; j--)
        {
          z->k[j] = z->k[j - 1];
          z->v[j] = z->v[j - 1];
        }

      if (!z->leaf)
        for (j = z->n + 1; j > 0; j--)
          z->p[j] = z->p[j - 1];

      z->k[0] = x->k[i];
      z->v[0] = x->v[i];

      if (!z->leaf) z->p[0] = y->p[y->n];

      z->n++;

      x->k[i] = y->k[y->n - 1];
      x->v[i] = y->v[y->n - 1];

      y->n--;
    }
  else if (y->n < 2 * d - 1)
    {
      y->k[y->n] = x->k[i];
      y->v[y->n] = x->v[i];

      if (!y->leaf) y->p[y->n + 1] = z->p[0];

      y->n++;

      x->k[i] = z->k[0];
      x->v[i] = z->v[0];

      for (j = 0; j < z->n - 1; j++)
        {
          z->k[j] = z->k[j + 1];
          z->v[j] = z->v[j + 1];
        }

      if (!z->leaf)
        for (j = 0; j < z->n; j++)
          z->p[j] = z->p[j + 1];

      z->n--;
    }
  else
    {
      size_t n_children = 4 * d - 1;

      /* New node */

      Node *w = allocate_node (d);

      w->leaf = y->leaf;

      /* Temporary containers */

      Node **p = new Node *[n_children + 1];
      int *k = new int[n_children];
      Record **v = new Record *[n_children];

      for (j = 0; j < y->n; j++)
        {
          k[j] = y->k[j];
          v[j] = y->v[j];

          p[j] = y->p[j];
        }

      k[y->n] = x->k[i];
      v[y->n] = x->v[i];

      p[y->n] = y->p[y->n];

      for (j = 0; j < z->n; j++)
        {
          k[y->n + 1 + j] = z->k[j];
          v[y->n + 1 + j] = z->v[j];

          p[y->n + 1 + j] = z->p[j];
        }

      p[n_children] = z->p[z->n];

      /* Number of children at each node */

      size_t n_y = (n_children - 2) / 3;
      size_t n_z = (n_children - 2 - n_y) / 2;
      size_t n_w = n_children - 2 - n_y - n_z;

      y->n = n_y;
      z->n = n_z;
      w->n = n_w;

      /* Redistribute children nodes. */
      
      size_t c = 0; // cursor

      for (j = 0; j < y->n; j++)
        {
          y->k[j] = k[c];
          y->v[j] = v[c];

          y->p[j] = p[c];

          c++;
        }

      y->p[y->n] = p[c];

      int k_yz = k[c];
      Record *v_yz = v[c];

      c++;

      for (j = 0; j < z->n; j++)
        {
          z->k[j] = k[c];
          z->v[j] = v[c];

          z->p[j] = p[c];

          c++;
        }

      z->p[z->n] = p[c];

      int k_zw = k[c];
      Record *v_zw = v[c];
      
      c++;

      for (j = 0; j < w->n; j++)
        {
          w->k[j] = k[c];
          w->v[j] = v[c];
          
          w->p[j] = p[c];
          
          c++;
        }

      w->p[w->n] = p[c];

      /* Update parent node. */

      for (j = x->n; j > i; j--)
        {
          x->k[j] = x->k[j - 1];
          x->v[j] = x->v[j - 1];

          x->p[j + 1] = x->p[j];
        }

      x->k[i] = k_yz;
      x->v[i] = v_yz;

      x->k[i + 1] = k_zw;
      x->v[i + 1] = v_zw;

      x->p[i + 2] = w;

      x->n++;

      delete[] p;
      delete[] k;
      delete[] v;
    }
}

void
BSTree::insert_nonfull (Node *x, int k, Record *v)
{
  size_t i = 0, j = x->n;
  while (i < x->n && k > x->k[i]) i++;

  if (i < x->n && k == x->k[i]) x->v[i] = v;
  else if (x->leaf)
    {
      while (j > 0 && k < x->k[j - 1])
        {
          x->k[j] = x->k[j - 1];
          x->v[j] = x->v[j - 1];
          j--;
        }

      x->k[i] = k;
      x->v[i] = v;
      x->n++;
    }
  else
    {
      if (x->p[i]->n == 2 * d - 1)
        {
          split_child (x, i);

          i = 0;
          while (i < x->n && k > x->k[i]) i++;

          if (i < x->n && k == x->k[i]) x->v[i] = v;
          else insert_nonfull (x->p[i], k, v);
        }
      else insert_nonfull (x->p[i], k, v);
    }
}

void
BSTree::insert_item (int k, Record *v)
{
  if (!root) create_tree ();

  Node *r = root;
  if (r->n == 2 * d - 1)
    {
      Node *s = allocate_node (d);
      
      root = s;
      s->leaf = false;
      s->p[0] = r;

      Node *t = allocate_node (d);

      t->leaf = r->leaf;
      t->n = d - 1;
      
      for (size_t j = 0; j < d - 1; j++)
        {
          t->k[j] = r->k[d + j];
          t->v[j] = r->v[d + j];
        }

      if (!r->leaf)
        for (size_t j = 0; j < d; j++)
          t->p[j] = r->p[d + j];
          
      r->n = d - 1;
      
      s->k[0] = r->k[d - 1];
      s->v[0] = r->v[d - 1];

      s->p[1] = t;

      s->n++;

      insert_nonfull (s, k, v);
    }
  else insert_nonfull (r, k, v);
}

/* Deletion */

void
BSTree::merge_siblings (Node *x, size_t i)
{
  size_t n = (4 * d - 2) / 3;
  Node *y = x->p[i];
  size_t j;

  if (y->n >= n) { /* do nothing */ }
  else if (i > 0 && x->p[i - 1]->n > n)
    {
      Node *z = x->p[i - 1];
      
      for (j = y->n; j > 0; j--)
        {
          y->k[j] = y->k[j - 1];
          y->v[j] = y->v[j - 1];
        }

      if (!y->leaf)
        for (j = y->n + 1; j > 0; j--)
          y->p[j] = y->p[j - 1];

      y->k[0] = x->k[i - 1];
      y->v[0] = x->v[i - 1];

      if (!y->leaf) y->p[0] = z->p[z->n];

      y->n++;

      x->k[i - 1] = z->k[z->n - 1];
      x->v[i - 1] = z->v[z->n - 1];

      z->n--;
    }
  else if (i < x->n && x->p[i + 1]->n > n)
    {
      Node *z = x->p[i + 1];
      
      y->k[y->n] = x->k[i];
      y->v[y->n] = x->v[i];

      if (!y->leaf) y->p[y->n + 1] = z->p[0];
      
      y->n++;

      x->k[i] = z->k[0];
      x->v[i] = z->v[0];

      for (j = 0; j < z->n - 1; j++)
        {
          z->k[j] = z->k[j + 1];
          z->v[j] = z->v[j + 1];
        }
      
      if (!z->leaf)
        for (j = 0; j < z->n; j++)
          z->p[j] = z->p[j + 1];
          
      z->n--;
    }
  else if (x->n == 1)
    {
      Node *z = x->p[0];
      Node *w = x->p[1];
      
      z->k[z->n] = x->k[0];
      z->v[z->n] = x->v[0];
      
      for (j = 0; j < w->n; j++)
        {
          z->k[z->n + 1 + j] = w->k[j];
          z->v[z->n + 1 + j] = w->v[j];
        }

      if (!z->leaf)
        for (j = 0; j <= w->n; j++)
          z->p[z->n + 1 + j] = w->p[j];
          
      z->n += w->n + 1;
      x->n = 0;

      delete w;
    }
  else
    {
      size_t i_ = i == x->n ? i - 2 : (i == 0 ? 0 : i - 1);
      
      Node *z = x->p[i_];
      Node *w = x->p[i_ + 1];
      Node *u = x->p[i_ + 2];

      size_t n_children = z->n + w->n + u->n + 2;
      
      Node **p = new Node *[n_children + 1];
      int *k = new int[n_children];
      Record **v = new Record *[n_children];

      size_t c = 0; // cursor
      
      for (j = 0; j < z->n; j++)
        {
          k[c] = z->k[j];
          v[c] = z->v[j];

          p[c] = z->p[j];

          c++;
        }

      p[c] = z->p[z->n];
      
      k[c] = x->k[i_];
      v[c] = x->v[i_];
      
      c++;
      
      for (j = 0; j < w->n; j++)
        {
          k[c] = w->k[j];
          v[c] = w->v[j];

          p[c] = w->p[j];

          c++;
        }

      p[c] = w->p[w->n];
      
      k[c] = x->k[i_ + 1];
      v[c] = x->v[i_ + 1];
      
      c++;
      
      for (j = 0; j < u->n; j++)
        {
          k[c] = u->k[j];
          v[c] = u->v[j];

          p[c] = u->p[j];

          c++;
        }

      p[c] = u->p[u->n];

      size_t n_z = (n_children - 1) / 2;
      size_t n_w = n_children - 1 - n_z;

      z->n = n_z;
      w->n = n_w;

      c = 0;

      for (j = 0; j < z->n; j++)
        {
          z->k[j] = k[c];
          z->v[j] = v[c];

          z->p[j] = p[c];

          c++;
        }
      
      z->p[z->n] = p[c];

      x->k[i_] = k[c];
      x->v[i_] = v[c];
      
      c++;

      for (j = 0; j < w->n; j++)
        {
          w->k[j] = k[c];
          w->v[j] = v[c];
          
          w->p[j] = p[c];

          c++;
        }

      w->p[w->n] = p[c];

      for (j = i_ + 1; j < x->n - 1; j++)
        {
          x->k[j] = x->k[j + 1];
          x->v[j] = x->v[j + 1];

          x->p[j + 1] = x->p[j + 2];
        }

      x->n--;

      delete u;

      delete[] p;
      delete[] k;
      delete[] v;
    }
}

void
BSTree::delete_node (Node *x, int k)
{
  size_t i = 0, j;
  while (i < x->n && k > x->k[i]) i++;

  if (x->leaf && i < x->n && k == x->k[i])
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
      Node *w = x->p[i];
      while (!w->leaf) w = w->p[w->n];
      
      x->k[i] = w->k[w->n - 1];
      x->v[i] = w->v[w->n - 1];
      
      delete_node (x->p[i], x->k[i]);
      merge_siblings (x, i);
    }
  else
    {
      delete_node (x->p[i], k);
      merge_siblings (x, i);
    }
}

void
BSTree::delete_item (int k)
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