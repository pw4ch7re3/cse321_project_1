#include "../include/node.h"

Node *
allocate_node (int d)
{
  Node *x = new Node;
  x->p = new Node *[2 * d + 1] ();
  x->k = new int[2 * d] ();
  x->v = new Record[2 * d] ();
  return x;
}