#include "include/loader.h"
#include "include/b_tree.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <filesystem>

static void test_btree (Record *);

int
main ()
{
  std::filesystem::path cwd = std::filesystem::current_path ();
  std::filesystem::path path;
  std::string pathname;
  
  while (cwd.string () != "/")
    {
      path = cwd / "data" / "student.csv";
      if (std::filesystem::exists (path)) break;
      cwd = cwd.parent_path();
    }
  pathname = path.string ();

  Record *table = load_table (pathname.c_str ());
  if (!table)
    {
      fprintf (
        stderr,
        "Failed to load %s: %s\n",
        pathname.c_str (),
        strerror (errno)
      );
      exit (EXIT_FAILURE);
    }

#if 0
  show_table (table);
#endif

#if 1
  test_btree (table);
#endif

  free_table (table);
}

static void
test_btree (Record *table)
{
  BTree *btree = new BTree (3);
  size_t n_rows = 10;
  int k = 0;
  Record *record;

  for (size_t i = 0; i < n_rows; i++)
    {
      btree->insert_item (table[i].student_id, &table[i]);
      record = btree->search_item (table[i].student_id);
      if (record) printf ("Student %d inserted\n", table[i].student_id);

      if (k == 0) k = table[i].student_id;
    }

  btree->delete_item (k);
  record = btree->search_item (k);
  if (!record) printf ("Student %d deleted\n", k);

  delete btree;
}