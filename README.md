# Wordle Bot

Currently only plays hard mode and searches 1 guess into the future. More features to be included later.

[Link to paper](https://github.com/charlotte-zhuang/wordle/blob/cc285b15d9e629ae3ca6d31e23548e92fbdcda05/parallel-wordle-solver-in-cpp.pdf)

## Sources

The following data was taken from [3 Blue 1 Brown](https://github.com/3b1b/videos/tree/master/_2022/wordle):

1. `data/allowed_words.txt`
2. `data/freq_map.json`
3. `data/possible_words.txt`
4. Priors calculation logic for word weights in `src/util/convert.py`
5. Entropy to expected guesses function in `src/Solver.cpp`

## Compilation

Requires C++17 or C++20. Below are commands I use on MacOS to compile.

```sh
/usr/local/bin/g++-11 -std=gnu++17 -Og -I include -o bin/debug src/*.cpp -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wformat-overflow -Wformat-truncation -Wundef -fno-common -D_DEBUG
```

```sh
/usr/local/bin/g++-11 -std=gnu++17 -Ofast -I include -o bin/wordle src/*.cpp
```

## Execution

The debug executable (from complier settings above) includes assertions so that program state is more visible if an error happens.

```sh
bin/debug data <number of iterations> <p OR s>
```

The non-debug executable (from complier settings above) turns on all optimizations for best performance.

```sh
bin/wordle data <number of iterations> <p OR s>
```

Example command

```sh
bin/wordle data 10 p
```
