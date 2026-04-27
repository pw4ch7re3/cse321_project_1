#ifndef LOADER_H
#define LOADER_H

#define MAX_RECORDS 100'000

struct Record {
    int student_id;
    char *name;
    bool gender;
    float gpa;
    float height;
    float weight;
};

Record *load(const char *);
void show_table(Record *);
void free_table(Record *);

#endif // LOADER_H