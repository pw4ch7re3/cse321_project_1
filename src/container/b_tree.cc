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
#include <climits>
#include <queue>
#include <utility>

/* === Search === */

Node *
BTree::search_node (Node *x, int k, size_t &i)
{
    i = 0;
    while (i < x->n && k > x->k[i]) i++;

    if (i < x->n && k == x->k[i]) return x;
    else if (x->leaf)             return nullptr;
    else                          return search_node (x->p[i], k, i);
}

/* === Insertion === */

/**
 * Before calling split_child(),
 * 
 * x
 * ... | p_i | k | v | ...
 *        |
 *        y
 * 
 * where Y has 2 * d - 1 keys.
 * -----------------------------------
 * After terminating split_child(),
 * 
 * x
 * ... | p_i | k' | v' | p_i+1 | k | v | ...
 *        |               |
 *        y               z
 * 
 * where both Y and Z have d - 1 keys.
 */
void
BTree::split_child (Node *x, size_t i)
{
  n_splits++;

  Node *z = allocate_node (d);
  Node *y = x->p[i];
  size_t j;

  /* Y and Z are in the same depth. */
  z->leaf = y->leaf;

  /* Move half of elements from Y to Z. */
  for (j = 0; j < d - 1; j++)
    {
      z->k[j] = y->k[d + j];
      z->v[j] = y->v[d + j];
    }

  if (!y->leaf)
    for (j = 0; j < d; j++)
      z->p[j] = y->p[d + j];

  z->n = d - 1;

  /* Now, Y has half of elements. */
  y->n = d - 1;

  /* Promote middle key in original Y. */
  for (j = x->n; j > i; j--)
    {
      x->k[j] = x->k[j - 1];
      x->v[j] = x->v[j - 1];
    }
  x->k[i] = y->k[d - 1];
  x->v[i] = y->v[d - 1];
  
  /* Insert Z's pointer into X. */
  for (j = x->n; j > i; j--)
    x->p[j + 1] = x->p[j];
  x->p[i + 1] = z;

  x->n++;
}

/* Implemented in top-down approach. */
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
      
      if (x->p[i]->n >= 2 * d - 1)
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
  if (!root) create_tree ();

  Node *r = root;
  if (r->n >= 2 * d - 1)
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

/* === Deletion === */

void
BTree::merge_siblings (Node *x, size_t i)
{
  Node *y = x->p[i];
  Node *z = x->p[i + 1];
  size_t j;

  /* Move all of elements from Z to Y. */
  size_t n_y = y->n;
  y->k[n_y] = x->k[i];
  y->v[n_y] = x->v[i];
  for (j = 0; j < z->n; j++)
    {
      y->k[n_y + 1 + j] = z->k[j];
      y->v[n_y + 1 + j] = z->v[j];
    }

  if (!y->leaf)
    for (j = 0; j <= z->n; j++)
      y->p[n_y + 1 + j] = z->p[j];

  y->n += z->n + 1;

  /* The key between Y's and Z's pointers was demoted. */
  for (j = i + 1; j < x->n; j++)
    {
      x->k[j - 1] = x->k[j];
      x->v[j - 1] = x->v[j];
    }

  /* Remove Z's pointer from X. */
  for (j = i + 2; j <= x->n; j++)
    x->p[j - 1] = x->p[j];

  x->n--;

  /* Free Z's pointer. */
  delete z;
}

void
BTree::delete_node (Node *x, int k)
{
  size_t i = 0;
  while (i < x->n && k > x->k[i]) i++;

  if (x->leaf && i < x->n && k == x->k[i]) // Case 1
    {
      for (size_t j = i; j < x->n - 1; j++)
        {
          x->k[j] = x->k[j + 1];
          x->v[j] = x->v[j + 1];
        }

      x->n--;
    }
  else if (!x->leaf && i < x->n && k == x->k[i])
    {
      Node *y = x->p[i];
      if (y->n >= d) // Case 2a
        {
          Node *w = y;
          while (!w->leaf) w = w->p[w->n];

          int pred_key = w->k[w->n - 1];
          x->k[i] = pred_key;
          x->v[i] = w->v[w->n - 1];
          
          delete_node (y, pred_key);

          goto done;
        }

      Node *z = x->p[i + 1];
      if (z->n >= d) // Case 2b
        {
          Node *w = z;
          while (!w->leaf) w = w->p[0];

          int succ_key = w->k[0];
          x->k[i] = succ_key;
          x->v[i] = w->v[0];
          
          delete_node (z, succ_key);

          goto done;
        }

      // Case 2c
      merge_siblings (x, i);

      delete_node (y, k);
    }
  else if (!x->leaf && (i >= x->n || k < x->k[i]))
    {
      bool out_of_bounds = i >= x->n;
      Node *y = x->p[i];
      Node *z = out_of_bounds ? x->p[i - 1] : x->p[i + 1];

      if (y->n <= d - 1 && z->n >= d) // Case 3a
        {
          size_t j;

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

              if (!y->leaf)
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
      else if (y->n <= d - 1 && z->n <= d - 1) // Case 3b
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
  
  done: return;
}

void
BTree::delete_item (int k)
{
  if (!root) goto done;
  
  delete_node (root, k);

  if (!root->leaf && root->n == 0)
    {
      Node *r = root;
      
      root = root->p[0];
      
      delete r;
    }

  done: return;
}

/* === Getter functions === */

Statistics
BTree::get_statistics () const
{
  size_t node_bytes = sizeof (Node) +
    sizeof (Node *) * (2 * d + 1) +
    sizeof (int) * (2 * d) +
    sizeof (Record *) * (2 * d);
  Statistics s {};

  collect_node_stats (root, 1, s.n_nodes, s.n_keys, s.height);

  s.n_bytes = s.n_nodes * node_bytes;
  s.utilization = s.n_nodes == 0 ? 0.0 :
    static_cast<double> (s.n_keys) /
    static_cast<double> (s.n_nodes * (2 * d - 1));
  
  return s;
}

/* === Test and debug  === */

static bool
validate_node (Node *x, bool is_root, size_t d,
               int min_key, int max_key,
               size_t depth, size_t &leaf_depth)
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
          if (!validate_node (x->p[i], false, d,
                              child_min, child_max,
                              depth + 1, leaf_depth))
            return false;
        }

      return true;
    }
}

bool
BTree::validate () const
{
  size_t leaf_depth = 0;
  return validate_node (root, true, d,
                        INT_MIN, INT_MAX,
                        1, leaf_depth);
}

void
BTree::print_tree (FILE *out) const
{
  if (!root) fprintf (out, "(empty)\n");
  else
    {
      std::queue<Node *> q;
      Node *x;
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

              fprintf (out, " [");

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