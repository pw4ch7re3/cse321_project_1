/**
 * @file src/container/bs_tree.cc
 * 
 * @brief B* Tree Implementation
 * @author Sungho Kim
 * 
 * @todo Lazy implementation.
 */
#include "../include/bs_tree.h"
#include <climits>
#include <queue>

/* === Search === */

Node *
BSTree::search_node (Node *x, int k, size_t &i)
{
    i = 0;
    while (i < x->n && k > x->k[i]) i++;

    if (i < x->n && k == x->k[i]) return x;
    else if (x->leaf)             return nullptr;
    else                          return search_node (x->p[i], k, i);
}

/* === Insertion === */

static void
redistribute_y_to_z (Node *x, Node *y, Node *z, size_t i)
{
  size_t j;

  for (j = z->n; j > 0; j--)
    {
      z->k[j] = z->k[j - 1];
      z->v[j] = z->v[j - 1];
    }
  z->k[0] = x->k[i];
  z->v[0] = x->v[i];

  if (!z->leaf)
    {
      for (j = z->n + 1; j > 0; j--)
        z->p[j] = z->p[j - 1];
      z->p[0] = y->p[y->n];
    }

  z->n++;

  y->n--;

  x->k[i] = y->k[y->n];
  x->v[i] = y->v[y->n];
}

static void
redistribute_z_to_y (Node *x, Node *y, Node *z, size_t i)
{
  size_t j;

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

static void
split_single_child (Node *x, size_t i, size_t d)
{
  Node *z = allocate_node (d);
  Node *y = x->p[i];
  size_t j;

  z->leaf = y->leaf;

  for (j = 0; j < d - 1; j++)
    {
      z->k[j] = y->k[d + j];
      z->v[j] = y->v[d + j];
    }

  if (!y->leaf)
    for (j = 0; j < d; j++)
      z->p[j] = y->p[d + j];

  z->n = d - 1;
  y->n = d - 1;

  for (j = x->n; j > i; j--)
    {
      x->k[j] = x->k[j - 1];
      x->v[j] = x->v[j - 1];
      x->p[j + 1] = x->p[j];
    }

  x->k[i] = y->k[d - 1];
  x->v[i] = y->v[d - 1];
  x->p[i + 1] = z;
  x->n++;
}

void
BSTree::split_child (Node *x, size_t i)
{
  while (i >= x->n) i--;

  Node *y = x->p[i];
  Node *z = x->p[i + 1];

  /**
   * Before calling split_child(),
   * 
   * x
   * ... | p_i | k_2 | v_2 | p_i+1 | ...
   *        |                 |
   *        y                 z
   * 
   * y
   * ... | k_0 | v_0 | p_1 | k_1 | v_1 | p_2
   * 
   * z
   * p_3 | k_3 | v_3 | ...
   * 
   * where k_0 < k_1 < k_2 < k_3.
   * ---------------------------------------
   * After terminating split_child(),
   * 
   * x
   * ... | p_i | k_1 | v_1 | p_i+1 | ...
   *        |                 |
   *        y                 z
   * 
   * y
   * ... | k_0 | v_0 | p_1
   * 
   * z
   * p_2 | k_2 | v_2 | p_3 | k_3 | v_3 | ...
   * 
   * where k_0 < k_1 < k_2 < k_3, still.
   */
  if (z->n < 2 * d - 1)
    redistribute_y_to_z (x, y, z, i);

  /**
   * Before calling split_child(),
   * 
   * x
   * ... | p_i | k_1 | v_1 | p_i+1 | ...
   *        |                 |
   *        y                 z
   * 
   * y
   * ... | k_0 | v_0 | p_1
   * 
   * z
   * p_2 | k_2 | v_2 | p_3 | k_3 | v_3 | ...
   * 
   * where k_0 < k_1 < k_2 < k_3.
   * ---------------------------------------
   * After terminating split_child(),
   * 
   * x
   * ... | p_i | k_2 | v_2 | p_i+1 | ...
   *        |                 |
   *        y                 z
   * 
   * y
   * ... | k_0 | v_0 | p_1 | k_1 | v_1 | p_2
   * 
   * z
   * p_3 | k_3 | v_3 | ...
   * 
   * where k_0 < k_1 < k_2 < k_3, still.
   */
  else if (y->n < 2 * d - 1)
    redistribute_z_to_y (x, y, z, i);

  else /* y->n >= 2 * d - 1 && z->n >= 2 * d - 1 */
    {
      /* Expected as 4 * d - 1. */
      size_t n_children = y->n + z->n + 1;
      /* New node to be splitted. */
      Node *w = allocate_node (d);
      /* Temporary containers. */
      Node **tp = new Node *[n_children + 1];
      int *tk = new int[n_children];
      Record *tv = new Record[n_children];
      /* Numbers of children. */
      size_t n_y = (n_children - 2) / 3;
      size_t n_z = (n_children - 2 - n_y) / 2;
      size_t n_w = n_children - 2 - n_y - n_z;
      /* Promoted keys and values. */
      int k_yz, k_zw;
      Record v_yz, v_zw;
      /* Indices. */
      size_t j, k = 0;

      /* Y, Z, and W are in the same depth. */
      w->leaf = y->leaf;      

      /* Store data into temporary containers. */
      for (j = 0; j < y->n; j++)
        {
          tk[j] = y->k[j];
          tv[j] = y->v[j];
          tp[j] = y->p[j];
        }
      tk[y->n] = x->k[i];
      tv[y->n] = x->v[i];
      tp[y->n] = y->p[y->n];

      for (j = 0; j < z->n; j++)
        {
          tk[y->n + 1 + j] = z->k[j];
          tv[y->n + 1 + j] = z->v[j];
          tp[y->n + 1 + j] = z->p[j];
        }
      tp[n_children] = z->p[z->n];

      /* Difference between numbers of data is at most 1. */
      y->n = n_y;
      z->n = n_z;
      w->n = n_w;

      /* Fill Y. */
      for (j = 0; j < y->n; j++)
        {
          y->k[j] = tk[k];
          y->v[j] = tv[k];
          y->p[j] = tp[k];

          k++;
        }
      y->p[y->n] = tp[k];

      /* These will be promoted. */
      k_yz = tk[k];
      v_yz = tv[k];

      k++;

      /* Fill Z. */
      for (j = 0; j < z->n; j++)
        {
          z->k[j] = tk[k];
          z->v[j] = tv[k];
          z->p[j] = tp[k];

          k++;
        }
      z->p[z->n] = tp[k];

      /* These will be promoted. */
      k_zw = tk[k];
      v_zw = tv[k];
      
      k++;

      /* Fill W. */
      for (j = 0; j < w->n; j++)
        {
          w->k[j] = tk[k];
          w->v[j] = tv[k];
          w->p[j] = tp[k];
          
          k++;
        }
      w->p[w->n] = tp[k];

      /* Do promotion. */
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

      /* Insert W's pointer into X. */
      x->p[i + 2] = w;

      x->n++;

      /* Free temporary containers. */
      delete[] tp;
      delete[] tk;
      delete[] tv;

      n_splits++;
    }
}

/* Implemented in top-down approach. */
void
BSTree::insert_nonfull (Node *x, int k, Record &v)
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

          i = x->n;
          while (i > 0 && k < x->k[i - 1]) i--;

          if (x->p[i]->n >= 2 * d - 1)
            {
              split_single_child (x, i, d);
              n_splits++;

              if (k > x->k[i]) i++;
            }
        }
      
      insert_nonfull (x->p[i], k, v);
    }
}

void
BSTree::insert_item (int k, Record v)
{
  if (!root) create_tree ();

  Node *r = root;
  if (r->n >= 2 * d - 1)
    {
      Node *s = allocate_node (d);
      Node *t = allocate_node (d);

      root = s;

      s->leaf = false;
      s->p[0] = r;

      /* R and T are in the same depth. */
      t->leaf = r->leaf;
      
      /* Move half of elements from R to T. */
      for (size_t j = 0; j < d - 1; j++)
        {
          t->k[j] = r->k[d + j];
          t->v[j] = r->v[d + j];
        }

      if (!r->leaf)
        for (size_t j = 0; j < d; j++)
          t->p[j] = r->p[d + j];

      t->n = d - 1;

      /* Now, R has half of elements. */
      r->n = d - 1;
      
      /* Promote middle key in original R. */
      s->k[0] = r->k[d - 1];
      s->v[0] = r->v[d - 1];

      /* Insert T's pointer into S. */
      s->p[1] = t;

      s->n++;

      insert_nonfull (s, k, v);

      n_splits++;
    }
  else insert_nonfull (r, k, v);
}

/* === Deletion === */

void
BSTree::merge_siblings (Node *x, size_t i)
{
  Node *y = x->p[i];
  Node *z = x->p[i + 1];
  size_t j;
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

  for (j = i + 1; j < x->n; j++)
    {
      x->k[j - 1] = x->k[j];
      x->v[j - 1] = x->v[j];
    }

  for (j = i + 2; j <= x->n; j++)
    x->p[j - 1] = x->p[j];

  x->n--;

  delete z;
}

void
BSTree::delete_node (Node *x, int k)
{
  if (!x) return;

  size_t i = 0;
  while (i < x->n && k > x->k[i]) i++;

  if (x->leaf && i < x->n && k == x->k[i])
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
      if (y->n >= d)
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
      if (z->n >= d)
        {
          Node *w = z;
          while (!w->leaf) w = w->p[0];

          int succ_key = w->k[0];
          x->k[i] = succ_key;
          x->v[i] = w->v[0];

          delete_node (z, succ_key);

          goto done;
        }

      merge_siblings (x, i);

      delete_node (y, k);
    }
  else if (!x->leaf && (i >= x->n || k < x->k[i]))
    {
      bool out_of_bounds = i >= x->n;
      Node *y = x->p[i];
      Node *z = out_of_bounds ? x->p[i - 1] : x->p[i + 1];

      if (y->n <= d - 1 && z->n >= d)
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

  done: return;
}

void
BSTree::delete_item (int k)
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
BSTree::get_statistics () const
{
  size_t node_bytes = sizeof (Node) +
    sizeof (Node *) * (2 * d + 1) +
    sizeof (int) * (2 * d) +
    sizeof (Record) * (2 * d);
  Statistics s {};

  collect_node_stats (root, 1, s.n_nodes, s.n_keys, s.height);

  s.n_bytes = s.n_nodes * node_bytes;
  s.utilization = s.n_nodes == 0 ? 0.0 :
    static_cast<double> (s.n_keys) /
    static_cast<double> (s.n_nodes * (2 * d - 1));
  
  return s;
}

/* === Test and debug === */

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
BSTree::validate () const
{
  size_t leaf_depth = 0;
  return validate_node (root, true, d,
                        INT_MIN, INT_MAX,
                        1, leaf_depth);
}

void
BSTree::print_tree (FILE *out) const
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