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