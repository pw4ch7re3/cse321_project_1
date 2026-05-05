#include "../include/list.h"

List *
allocate_list (int d, bool leaf, List *next)
{
  List *x = new List;
  x->leaf = leaf;
  x->p = new List *[2 * d + 1] ();
  x->k = new int[2 * d] ();
  
  if (leaf)
    x->rid = new int[2 * d] ();
  
  x->next = next;
  
  return x;
}