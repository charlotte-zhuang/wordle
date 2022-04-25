#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#define WORD_FILE_NAME "test_words.txt"

namespace fs = std::filesystem;

/**
 * @brief Wordle game runner
 *
 */
class Adversary
{
public:
  /**
   * @brief Construct a new Adversary object and start a game.
   *
   * @param data_path Path to data dir containing test_words.txt
   */
  Adversary(const std::string &data_path);
  /**
   * @brief Start a new game and choose a new word.
   *
   */
  void new_word();
  /**
   * @brief Make a guess
   *
   * @param word Guess word (all lowercase)
   * @param result 'G' for green/correct, 'Y' for yellow, 'B' for gray/wrong
   */
  void guess(const char (&word)[5], char (&result)[5]);

private:
  const fs::path word_file_path;
  int num_words;
  std::string word;
  bool used[5];
};
