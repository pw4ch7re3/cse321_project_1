#include "../include/statistics.h"

void
collect_node_stats (Node *x, size_t depth,
                    size_t &nodes, size_t &keys,
                    size_t &height)
{
  if (!x) return;

  nodes++;
  keys += x->n;

  if (depth > height) height = depth;

  if (!x->leaf)
    for (size_t i = 0; i <= x->n; i++)
      collect_node_stats (x->p[i], depth + 1, nodes, keys, height);
}

void
collect_list_stats (List *x, size_t depth,
                    size_t &nodes, size_t &leaves,
                    size_t &keys, size_t &height)
{
  if (!x) return;

  nodes++;
  
  if (x->leaf) leaves++;

  keys += x->n;
  
  if (depth > height) height = depth;

  if (!x->leaf)
    for (size_t i = 0; i <= x->n; i++)
      collect_list_stats (x->p[i], depth + 1, nodes, leaves, keys, height);
}