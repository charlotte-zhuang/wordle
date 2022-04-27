#ifndef MATH_H
#define MATH_H
#include <math.h>
#endif

#ifndef STRING_H
#define STRING_H
#include <string>
#endif

#ifndef LIMITS_H
#define LIMITS_H
#include <limits>
#endif

#ifndef VECTOR_H
#define VECTOR_H
#include <vector>
#endif

#ifndef MEMORY_H
#define MEMORY_H
#include <memory>
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

#ifndef STRING_H
#define STRING_H
#include <string>
#endif

#ifndef FSTREAM_H
#define FSTREAM_H
#include <fstream>
#endif

#ifndef STDEXCEPT_H
#define STDEXCEPT_H
#include <stdexcept>
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

#ifdef _DEBUG
#define ASSERT(left, operator, right)                                                                                                                                                            \
  {                                                                                                                                                                                              \
    if (!((left) operator(right)))                                                                                                                                                               \
    {                                                                                                                                                                                            \
      std::cerr << "ASSERT FAILED: " << #left << #operator<< #right << " @ " << __FILE__ << " (" << __LINE__ << "). " << #left << "=" <<(left) << "; " << #right << "=" << (right) << std::endl; \
      throw "Assertion error";                                                                                                                                                                   \
    }                                                                                                                                                                                            \
  }
#else
#define ASSERT(left, operator, right) \
  {                                   \
  }
#endif

/**
 * @brief Wordle solver bot
 *
 */
class Solver
{
public:
  /**
   * @brief Construct a new Solver object
   *
   * @param data_path Path to data dir containing word_weights.txt
   */
  Solver(const std::string &data_path);

  /**
   * @brief Construct a new Solver object by moving
   *
   * @param rvalue
   */
  Solver(Solver &&rvalue) noexcept;

  Solver &operator=(Solver &&rvalue) noexcept;

  /**
   * @brief Reset the solver for a new game
   *
   */
  void reset();

  /**
   * @brief Make a guess
   *
   * @param guess Guess will be placed in here
   */
  void make_guess(char (&guess)[5]);

  /**
   * @brief Make a guess
   *
   * @param guess Guess will be placed in here
   * @param result Result from previous guess
   */
  void make_guess(char (&guess)[5], const char (&result)[5]);

private:
  class Word
  {
  public:
    Word(const std::string &val, const double weight);
    Word(Word &&rvalue) noexcept;
    Word &operator=(Word &&rvalue) noexcept;
    char &operator[](unsigned int i);
    const char &operator[](const unsigned int i) const;

    std::string val;
    double weight;
  };

  double calc_expect(const Word &guess);
  inline double heuristic(const double entropy);
  template <class T, class U, class V>
  inline bool word_fits_result(const T &word, const U &guessed, const V &result, const char (&code)[3]);
  inline void calc_total_entropy();

  fs::path word_file_path;
  std::vector<Word> words, temp;
  std::string prev_guess;
  double total_weight, total_entropy;
  bool used[5];
};
