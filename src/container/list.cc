#include "../include/list.h"

List *
allocate_list (List *next, int d, bool leaf)
{
  List *x = new List;
  x->leaf = leaf;
  x->p = new List *[2 * d + 1] ();
  x->k = new int[2 * d] ();
  
  if (leaf)
    x->v = new Record *[2 * d] ();
  
  x->next = next;
  
  return x;
}