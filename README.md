# cse321_project_1

## Preliminaries

- OS: Ubuntu 24.04.4 LTS
- Arch: x86-64
- Lang: C++23

## Install

If you don't have g++ compiler, run the following commands:

```sh
sudo apt update
sudo apt install build-essential
```

CWD stands for current working directory.

Clone this repository in your CWD:

```sh
git clone https://github.com/pw4ch7re3/cse321_project_1.git
```

Then, you can access the directory, `<CWD>/cse321_project_1/`. Let's name the directory as `<REPO>`.

## Build

Be ready to build by running the following commands:

```sh
cd <REPO>
```

Use `make` command to build.

```sh
make
```

Or you can use other commands

- `make clean` cleans build and target directories.
- `make rebuild` builds after cleaning.

## Run

In `<REPO>` directory, run `./target/main` to get experiment results.

# Example: Experiment Results

- \# of records = 100000
- \# of point queries = 10000
- \# of deletions = 2000
- range = [202000000, 202100000]

d=3|insert (ms)|search (us)|range (ms)|delete (ms)|\# of nodes|height|util|memory (KiB)
:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:
B-tree|24.00|0.211|2.24|0.46|31283|9|63.9%|5132.4
B*-tree|25.40|0.214|2.84|0.43|25484|8|78.5%|4181.0
B+-tree|33.55|0.270|0.12|0.56|38494|9|66.8%|5480.6

d=5|insert (ms)|search (us)|range (ms)|delete (ms)|\# of nodes|height|util|memory (KiB)
:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:
B-tree|24.87|0.212|1.69|0.34|16653|6|66.7%|4033.1
B*-tree|20.61|0.226|2.60|0.47|13727|6|80.9%|3324.5
B+-tree|20.90|0.228|0.10|0.41|18560|6|69.3%|3805.0

d=10|insert (ms)|search (us)|range (ms)|delete (ms)|\# of nodes|height|util|memory (KiB)
:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:
B-tree|19.94|0.270|4.57|0.40|7690|5|68.4%|3364.4
B*-tree|19.85|0.297|1.51|0.35|6424|5|81.9%|2810.5
B+-tree|23.17|0.239|0.03|0.29|8085|5|70.0%|2922.9

(avg_gpa=3.28, avg_height=174.1)
