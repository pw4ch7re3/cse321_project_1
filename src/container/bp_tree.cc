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
#include <climits>
#include <queue>

/* === Search === */

List *
BPTree::search_node (int k, size_t &i)
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

/* === Insertion === */

void
BPTree::split_child (List *x, size_t i)
{
  n_splits++;

  List *y = x->p[i];
  List *z = allocate_list (d, y->leaf, y->next);
  size_t j;

  if (y->leaf)
    {
      for (j = 0; j < d - 1; j++)
        {
          z->k[j] = y->k[d + j];
          z->rid[j] = y->rid[d + j];
        }
      
      y->n = d;
      y->next = z;
    }
  else
    {
      for (j = 0; j < d - 1; j++)
        z->k[j] = y->k[d + j];
        
      for (j = 0; j < d; j++)
        z->p[j] = y->p[d + j];
      
      y->n = d - 1;
    }

  z->n = d - 1;

  for (j = x->n; j > i; j--)
    x->k[j] = x->k[j - 1];
  
  if (y->leaf) x->k[i] = z->k[0];
  else x->k[i] = y->k[d - 1];

  for (j = x->n; j > i; j--)
    x->p[j + 1] = x->p[j];
  x->p[i + 1] = z;

  x->n++;
}

void
BPTree::insert_nonfull (List *x, int k, int &v)
{
  size_t i = x->n;

  if (x->leaf)
    {
      while (i > 0 && k < x->k[i - 1])
        {
          x->k[i] = x->k[i - 1];
          x->rid[i] = x->rid[i - 1];

          i--;
        }
      x->k[i] = k;
      x->rid[i] = v;

      x->n++;
    }
  else
    {
      while (i > 0 && k < x->k[i - 1]) i--;
      
      if (x->p[i]->n >= 2 * d - 1)
        {
          split_child (x, i);
          
          i = x->n;
          while (i < x->n && k < x->k[i]) i--;
        }

      insert_nonfull (x->p[i], k, v);
    }
}

void
BPTree::insert_item (int k, int rid)
{
  if (!root) create_tree ();

  List *r = root;
  if (r->n >= 2 * d - 1)
    {
      List *s = allocate_list (d, false);
      
      root = s;

      s->p[0] = r;

      split_child (s, 0);

      insert_nonfull (s, k, rid);
    }
  else insert_nonfull (r, k, rid);
}

/* === Deletion === */

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
          y->rid[y->n + j] = z->rid[j];
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
              x->rid[j] = x->rid[j + 1];
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

      if (y->n <= d - 1 && z->n >= d)
        {
          if (out_of_bounds)
            {
              if (y->leaf)
                {
                  for (j = y->n; j > 0; j--)
                    {
                      y->k[j] = y->k[j - 1];
                      y->rid[j] = y->rid[j - 1];
                    }
                  y->k[0] = z->k[z->n - 1];
                  y->rid[0] = z->rid[z->n - 1];

                  x->k[i - 1] = z->k[z->n - 1];
                }
              else
                {
                  for (j = y->n; j > 0; j--)
                    y->k[j] = y->k[j - 1];
                  y->k[0] = x->k[i - 1];
                  
                  for (j = y->n + 1; j > 0; j--)
                    y->p[j] = y->p[j - 1];
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
                  y->rid[y->n] = z->rid[0];

                  x->k[i] = z->k[1];

                  for (j = 0; j < z->n - 1; j++)
                    {
                      z->k[j] = z->k[j + 1];
                      z->rid[j] = z->rid[j + 1];
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
      else if (y->n <= d - 1 && z->n <= d - 1)
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
  if (!root) goto done;

  delete_node (root, k);

  if (!root->leaf && root->n == 0)
    {
      List *r = root;

      root = root->p[0];

      delete r;
    }

  done: return;
}

void
BPTree::range_query (int start, int end, int *buffer, size_t n)
{
  if (!root || !buffer || n == 0) return;

  List *x = root;
  size_t i, j = 0;

  while (!x->leaf)
    {
      i = 0;
      while (i < x->n && start >= x->k[i]) i++;

      x = x->p[i];
    }

  i = 0;
  while (i < x->n && x->k[i] < start) i++;

  while (x && j + 1 < n)
    {
      while (i < x->n && j + 1 < n)
        {
          if (x->k[i] > end)
            {
              buffer[j] = -1;
              return;
            }

          buffer[j++] = x->rid[i++];
        }

      x = x->next;
      i = 0;
    }

  buffer[j] = -1;
}

/* === Getter functions === */

Statistics
BPTree::get_statistics () const
{
  size_t internal_nodes;
  size_t internal_bytes;
  size_t leaf_bytes;
  Statistics s;
  size_t leaves = 0;
  
  collect_list_stats (root, 1, s.n_nodes, leaves, s.n_keys, s.height);

  internal_nodes = s.n_nodes - leaves;
  internal_bytes = internal_nodes * (
    sizeof (List) +
    sizeof (List *) * (2 * d + 1) +
    sizeof (int) * (2 * d));
  leaf_bytes = leaves * (
    sizeof (List) +
    sizeof (List *) * (2 * d + 1) +
    sizeof (int) * (2 * d) +
    sizeof (int) * (2 * d));
  s.n_bytes = internal_bytes + leaf_bytes;
  s.utilization = s.n_nodes == 0 ? 0.0 :
    static_cast<double> (s.n_keys) /
    static_cast<double> (s.n_nodes * (2 * d - 1));
  
  return s;
}

/* === Test and debug === */

static bool
validate_list (List *x, bool is_root, size_t d, int min_key,
               int max_key, size_t depth, size_t &leaf_depth)
{
  /* Properties */
  if (!x ||
      (!is_root && x->n < d - 1) ||
      x->n > 2 * d - 1)
    return false;

  size_t i;

  /* Ordering */
  for (i = 0; i < x->n; i++)
    {
      if (x->k[i] < min_key || x->k[i] > max_key) return false;
      if (i > 0 && x->k[i - 1] >= x->k[i]) return false;
    }

  if (x->leaf) // Base case
    {
      if (leaf_depth == 0) leaf_depth = depth;

      return leaf_depth == depth;
    }
  else // Recursive step
    {
      for (i = 0; i <= x->n; i++)
        {
          int child_min = i == 0 ? min_key : x->k[i - 1];
          int child_max = i == x->n ? max_key : x->k[i];
          if (!validate_list (x->p[i], false, d,
                              child_min, child_max,
                              depth + 1, leaf_depth))
            return false;
        }

      return true;
    }
}

bool
BPTree::validate () const
{
  size_t leaf_depth = 0;
  if (!validate_list (root, true, d,
                      INT_MIN, INT_MAX,
                      1, leaf_depth))
    return false;

  int prev = INT_MIN;
  bool is_head = true;
  List *x = root;
  size_t i;

  while (x && !x->leaf) x = x->p[0];

  while (x)
    {
      for (i = 0; i < x->n; i++)
        {
          if (!is_head && prev >= x->k[i]) return false;

          prev = x->k[i];
          is_head = false;
        }

      x = x->next;
    }

  return true;
}

void
BPTree::print_tree (FILE *out) const
{
  if (!root)
    fprintf (out, "(empty)\n");
  else
    {
      std::queue<List *> q;
      List *x;
      size_t n;
      size_t i, j;

      q.push (root);

      for (i = 0; !q.empty (); i++)
        {
          fprintf (out, "depth %zu:", i);

          n = q.size ();

          for (i = 0; i < n; i++)
            {
              x = q.front ();
              q.pop ();

              fprintf (out, " %c[", x->leaf ? 'leaf' : 'internal');

              for (j = 0; j < x->n; j++)
                fprintf (out, "%s%d", j == 0 ? "" : " ", x->k[j]);

              fprintf (out, "]");

              if (!x->leaf)
                for (j = 0; j <= x->n; j++)
                  q.push (x->p[j]);
            }

          fprintf (out, "\n");
        }
    }
}