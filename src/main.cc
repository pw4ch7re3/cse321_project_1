#include "include/b_tree.h"
#include "include/bp_tree.h"
#include "include/bs_tree.h"
#include "include/loader.h"
#include "include/statistics.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <numeric>
#include <random>
#include <string>
#include <vector>

struct QueryResult {
  size_t count;
  double avg_gpa;
  double avg_height;
};

struct ExperimentResult {
  const char *name;
  size_t d;
  double insert_ms;
  double search_us;
  double range_ms;
  double delete_ms;
  size_t splits;
  size_t merges;
  Statistics stats;
  bool valid_after_insert;
  bool valid_after_delete;
  QueryResult range;
};

static void print_result(const ExperimentResult &r);

template <typename F> static double measure_ms(F &&f) {
  auto begin = std::chrono::steady_clock::now();

  f();

  auto end = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::milli>(end - begin).count();
}

static bool eq(const Record &a, const Record &b) {
  return a.student_id == b.student_id && strcmp(a.name, b.name) == 0 &&
         a.gender == b.gender && a.gpa == b.gpa && a.height == b.height &&
         a.weight == b.weight;
}

static size_t count_records(Record *table) {
  size_t n = 0;
  while (n < MAX_RECORDS && table[n].name)
    n++;

  return n;
}

static std::vector<size_t> shuffled_rids(size_t n) {
  std::vector<size_t> rids(n);
  std::iota(rids.begin(), rids.end(), 0);

  std::mt19937 rng(321);
  std::shuffle(rids.begin(), rids.end(), rng);

  return rids;
}

static QueryResult range_from_records(const std::vector<Record *> &records) {
  QueryResult result{};
  double gpa = 0.0;
  double height = 0.0;

  for (Record *r : records) {
    if (!r->gender)
      continue;

    gpa += r->gpa;
    height += r->height;

    result.count++;
  }

  if (result.count > 0) {
    result.avg_gpa = gpa / result.count;
    result.avg_height = height / result.count;
  }

  return result;
}

static ExperimentResult run_btree(Record *table, size_t n,
                                  const std::vector<size_t> &order,
                                  const std::vector<size_t> &searches,
                                  const std::vector<size_t> &deletions,
                                  size_t d, int range_start, int range_end) {
  BTree tree(d);
  ExperimentResult result{};
  result.name = "B-tree";
  result.d = d;

  result.insert_ms = measure_ms([&] {
    for (size_t rid : order)
      tree.insert_item(table[rid].student_id, &table[rid]);
  });

  result.valid_after_insert = tree.validate();
  result.stats = tree.get_statistics();
  result.splits = tree.get_split_counter();
  result.merges = tree.get_merge_counter();

  size_t found = 0;
  double search_ms = measure_ms([&] {
    for (size_t rid : searches) {
      Record *record = tree.search_item(table[rid].student_id);
      if (record && eq(*record, table[rid]))
        found++;
    }
  });
  result.search_us =
      searches.empty() ? 0.0 : search_ms * 1000.0 / searches.size();

  std::vector<Record *> range_records;
  result.range_ms = measure_ms([&] {
    for (size_t rid = 0; rid < n; rid++)
      if (table[rid].student_id >= range_start &&
          table[rid].student_id <= range_end) {
        Record *record = tree.search_item(table[rid].student_id);

        if (record)
          range_records.push_back(record);
      }
  });
  result.range = range_from_records(range_records);

  result.delete_ms = measure_ms([&] {
    for (size_t rid : deletions)
      tree.delete_item(table[rid].student_id);
  });

  result.valid_after_delete = tree.validate();

  if (found != searches.size())
    result.valid_after_insert = false;

  return result;
}

static ExperimentResult run_bstree(Record *table, size_t n,
                                   const std::vector<size_t> &order,
                                   const std::vector<size_t> &searches,
                                   const std::vector<size_t> &deletions,
                                   size_t d, int range_start, int range_end) {
  BSTree tree(d);
  ExperimentResult result{};
  result.name = "B*-tree";
  result.d = d;

  result.insert_ms = measure_ms([&] {
    for (size_t rid : order)
      tree.insert_item(table[rid].student_id, &table[rid]);
  });

  result.valid_after_insert = tree.validate();
  result.stats = tree.get_statistics();
  result.splits = tree.get_split_counter();
  result.merges = tree.get_merge_counter();

  size_t found = 0;
  double search_ms = measure_ms([&] {
    for (size_t rid : searches) {
      Record *record = tree.search_item(table[rid].student_id);
      if (record && eq(*record, table[rid]))
        found++;
    }
  });
  result.search_us =
      searches.empty() ? 0.0 : search_ms * 1000.0 / searches.size();

  std::vector<Record *> range_records;
  result.range_ms = measure_ms([&] {
    for (size_t rid = 0; rid < n; rid++)
      if (table[rid].student_id >= range_start &&
          table[rid].student_id <= range_end) {
        Record *record = tree.search_item(table[rid].student_id);

        if (record)
          range_records.push_back(record);
      }
  });
  result.range = range_from_records(range_records);

  result.delete_ms = measure_ms([&] {
    for (size_t rid : deletions)
      tree.delete_item(table[rid].student_id);
  });

  result.valid_after_delete = tree.validate();

  if (found != searches.size())
    result.valid_after_insert = false;

  return result;
}

static ExperimentResult run_bptree(Record *table, size_t n,
                                   const std::vector<size_t> &order,
                                   const std::vector<size_t> &searches,
                                   const std::vector<size_t> &deletions,
                                   size_t d, int range_start, int range_end) {
  BPTree tree(d);
  ExperimentResult result{};
  result.name = "B+ tree";
  result.d = d;

  result.insert_ms = measure_ms([&] {
    for (size_t rid : order)
      tree.insert_item(table[rid].student_id, rid);
  });

  result.valid_after_insert = tree.validate();
  result.stats = tree.get_statistics();
  result.splits = tree.get_split_counter();
  result.merges = tree.get_merge_counter();

  size_t found = 0;
  double search_ms = measure_ms([&] {
    for (size_t rid : searches) {
      int found_rid = tree.search_item(table[rid].student_id);
      if (found_rid >= 0 && static_cast<size_t>(found_rid) < n &&
          eq(table[found_rid], table[rid]))
        found++;
    }
  });
  result.search_us =
      searches.empty() ? 0.0 : search_ms * 1000.0 / searches.size();

  std::vector<int> buffer(n + 1, -1);
  result.range_ms = measure_ms([&] {
    tree.range_query(range_start, range_end, buffer.data(), buffer.size());
  });

  std::vector<Record *> range_records;
  for (int rid : buffer) {
    if (rid < 0)
      break;
    if (static_cast<size_t>(rid) < n)
      range_records.push_back(&table[rid]);
  }
  result.range = range_from_records(range_records);

  result.delete_ms = measure_ms([&] {
    for (size_t rid : deletions)
      tree.delete_item(table[rid].student_id);
  });

  result.valid_after_delete = tree.validate();

  if (found != searches.size())
    result.valid_after_insert = false;

  return result;
}

static void run_sequential_insertion(Record *table, size_t n, size_t d,
                                     int range_start, int range_end) {
  printf("\n=== Additional Experiment 1: Sequential vs Random Insertion "
         "(d=%zu) ===\n",
         d);

  std::vector<size_t> random_order = shuffled_rids(n);

  std::vector<size_t> seq_order(n);
  std::iota(seq_order.begin(), seq_order.end(), 0);

  std::vector<size_t> empty_list;

  ExperimentResult rand_b = run_btree(table, n, random_order, empty_list,
                                      empty_list, d, range_start, range_end);
  rand_b.name = "B-tree(Rnd)";
  print_result(rand_b);

  ExperimentResult seq_b = run_btree(table, n, seq_order, empty_list,
                                     empty_list, d, range_start, range_end);
  seq_b.name = "B-tree(Seq)";
  print_result(seq_b);

  ExperimentResult rand_bs = run_bstree(table, n, random_order, empty_list,
                                        empty_list, d, range_start, range_end);
  rand_bs.name = "B*-tree(Rn)";
  print_result(rand_bs);

  ExperimentResult seq_bs = run_bstree(table, n, seq_order, empty_list,
                                       empty_list, d, range_start, range_end);
  seq_bs.name = "B*-tree(Sq)";
  print_result(seq_bs);

  ExperimentResult rand_bp = run_bptree(table, n, random_order, empty_list,
                                        empty_list, d, range_start, range_end);
  rand_bp.name = "B+tree(Rnd)";
  print_result(rand_bp);

  ExperimentResult seq_bp = run_bptree(table, n, seq_order, empty_list,
                                       empty_list, d, range_start, range_end);
  seq_bp.name = "B+tree(Seq)";
  print_result(seq_bp);
}

static void run_massive_deletion(Record *table, size_t n, size_t d,
                                 int range_start, int range_end) {
  printf("\n=== Additional Experiment 2: Massive Deletion (80%%) Impact "
         "(d=%zu) ===\n",
         d);

  std::vector<size_t> order = shuffled_rids(n);
  std::vector<size_t> empty_searches;

  size_t massive_del_count = static_cast<size_t>(n * 0.8);
  std::vector<size_t> massive_deletions(order.begin(),
                                        order.begin() + massive_del_count);

  printf("Target: Insert %zu records, then Delete %zu records.\n", n,
         massive_del_count);

  ExperimentResult res_b =
      run_btree(table, n, order, empty_searches, massive_deletions, d,
                range_start, range_end);
  print_result(res_b);

  ExperimentResult res_bs =
      run_bstree(table, n, order, empty_searches, massive_deletions, d,
                 range_start, range_end);
  print_result(res_bs);

  ExperimentResult res_bp =
      run_bptree(table, n, order, empty_searches, massive_deletions, d,
                 range_start, range_end);
  print_result(res_bp);
}

static void print_result(const ExperimentResult &r) {
  printf("%-7s d=%-2zu insert=%8.2f ms search=%7.3f us "
         "range=%7.2f ms delete=%7.2f ms nodes=%6zu height=%2zu "
         "util=%5.1f%% splits=%6zu merges=%6zu memory=%8.1f KiB valid=%s/%s "
         "range_count=%5zu avg_gpa=%4.2f avg_height=%5.1f\n",
         r.name, r.d, r.insert_ms, r.search_us, r.range_ms, r.delete_ms,
         r.stats.n_nodes, r.stats.height, r.stats.utilization * 100.0, r.splits,
         r.merges, r.stats.n_bytes / 1024.0,
         r.valid_after_insert ? "yes" : "no",
         r.valid_after_delete ? "yes" : "no", r.range.count, r.range.avg_gpa,
         r.range.avg_height);
}

int main() {
  std::filesystem::path cwd = std::filesystem::current_path();
  std::filesystem::path path;

  while (cwd.string() != "/") {
    path = cwd / "data" / "student.csv";
    if (std::filesystem::exists(path))
      break;
    cwd = cwd.parent_path();
  }

  std::string pathname = path.string();
  Record *table = load_table(pathname.c_str());
  if (!table) {
    fprintf(stderr, "Failed to load %s: %s\n", pathname.c_str(),
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  size_t n = count_records(table);
  std::vector<size_t> order = shuffled_rids(n);
  std::vector<size_t> searches(order.begin(),
                               order.begin() + std::min<size_t>(10000, n));
  std::vector<size_t> deletions(order.begin(),
                                order.begin() + std::min<size_t>(2000, n));

  int range_start = 202000000;
  int range_end = 202100000;

  printf("records=%zu point_queries=%zu deletions=%zu range=[%d,%d]\n", n,
         searches.size(), deletions.size(), range_start, range_end);

  for (size_t d : {3u, 5u, 10u}) {
    print_result(run_btree(table, n, order, searches, deletions, d, range_start,
                           range_end));
    print_result(run_bstree(table, n, order, searches, deletions, d,
                            range_start, range_end));
    print_result(run_bptree(table, n, order, searches, deletions, d,
                            range_start, range_end));
    printf("\n");
  }

  size_t target_d = 5;
  run_sequential_insertion(table, n, target_d, range_start, range_end);
  run_massive_deletion(table, n, target_d, range_start, range_end);

  free_table(table);
}