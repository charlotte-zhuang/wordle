# Wordle Bot

## Sources

The following data was taken from [3 Blue 1 Brown](https://github.com/3b1b/videos/tree/master/_2022/wordle):

1. `data/allowed_words.txt`
2. `data/freq_map.json`
3. `data/possible_words.txt`
4. Priors calculation logic for word weights in `src/util/convert.py`

## Compilation

```sh
/usr/local/bin/g++-11 -I include -o bin/test src/main.cpp src/Adversary.cpp -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wformat-overflow -Wformat-truncation -Wundef -fno-common

/usr/local/bin/g++-11 -I include -o bin/wordle src/main.cpp src/Adversary.cpp -Ofast
```
