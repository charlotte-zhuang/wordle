#ifndef CTIME_H
#define CTIME_H
#include <ctime>
#endif

#ifndef STRING_H
#define STRING_H
#include <string>
#endif

#ifndef FSTREAM_H
#define FSTREAM_H
#include <fstream>
#endif

#ifndef IOSTREAM_H
#define IOSTREAM_H
#include <iostream>
#endif

#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <algorithm>
#endif

#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <filesystem>
namespace fs = std::filesystem;
#endif

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <constants.hpp>
#endif

/**
 * @brief Wordle game runner
 *
 */
class Adversary
{
public:
  /**
   * @brief Construct a new Adversary object and start a game
   *
   * @param data_path Path to data dir containing test_words.txt
   */
  Adversary(const std::string &data_path);

  /**
   * @brief Construct a new Adversary object by moving
   *
   * @param rvalue
   */
  Adversary(Adversary &&rvalue) noexcept;

  Adversary &operator=(Adversary &&rvalue) noexcept;

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
  void judge(const char (&guess)[5], char (&result)[5]);

  /**
   * @brief Get the target word
   * 
   * @return std::string Length 5, all lowercase
   */
  std::string get_target_word();

private:
  fs::path word_file_path;
  int num_words;
  std::string target_word;
  bool used[5];
};
