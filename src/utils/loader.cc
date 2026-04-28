#include "../include/loader.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>

static void fixed_width_sprint (const char *, int);
static void show_record (int, Record &);

Record *
load_table (const char *pathname)
{
  Record *table = new Record[MAX_RECORDS];

  std::ifstream file (pathname);
  if (!file.is_open ())
    {
      fprintf (
        stderr,
        "Failed to open %s: %s\n",
        pathname,
        strerror (errno)
      );
      return nullptr;
    }

  std::string line;
  std::getline (file, line);

  int rid = 0;
  while (std::getline (file, line) && rid < MAX_RECORDS) {
    std::stringstream ss (line);
    std::string cell;
    Record &r = table[rid++];

    if (std::getline (ss, cell, ',')) r.student_id = std::stoi (cell);
    if (std::getline (ss, cell, ',')) r.name = strdup (cell.c_str ());
    if (std::getline (ss, cell, ',')) r.gender = cell == "Male";
    if (std::getline (ss, cell, ',')) r.gpa = std::stof (cell);
    if (std::getline (ss, cell, ',')) r.height = std::stof (cell);
    if (std::getline (ss, cell, ',')) r.weight = std::stof (cell);
  }

  return table;
}

static void
fixed_width_sprint (const char *s, int w)
{
  int n_bytes = 0;
  int visual_width = 0;

  while (s[n_bytes]) {
    if ((s[n_bytes] & 0xc0) != 0x80)
      visual_width++;
    n_bytes++;
  }

  printf ("%-*s", w + n_bytes - visual_width, s);
}

static void
show_record (int rid, Record &record)
{
  printf ("%-5d | %-10d | ", rid, record.student_id);
  fixed_width_sprint (record.name, 18);
  printf (
    " | %-6s | %-4.2f | %-6.1f | %-6.1f\n",
    record.gender ? "Male" : "Female",
    record.gpa,
    record.height,
    record.weight
  );
}

void
show_table (Record *table) {
  printf (
    "%-5s | %-10s | %-18s | %-6s | %-4s | %-6s | %-6s\n",
    "     ",
    "Student ID",
    "Name",
    "Gender",
    "GPA",
    "Height",
    "Weight"
  );
  printf ("-------------------------------------");
  printf ("------------------------------------\n");

  for (int i = 0; i < MAX_RECORDS; i++)
    show_record (i, table[i]);
}

void
free_table (Record *table) {
  for (int i = 0; i < MAX_RECORDS; i++)
    if (table[i].name) free (table[i].name);
  delete[] table;
}