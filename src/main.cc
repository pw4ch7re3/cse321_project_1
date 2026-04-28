#include "include/loader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <filesystem>

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

  show_table (table);

  free_table (table);
}