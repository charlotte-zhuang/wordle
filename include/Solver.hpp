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

#ifndef THREAD_H
#define THREAD_H
#include <thread>
#endif

#ifndef MUTEX_H
#define MUTEX_H
#include <mutex>
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
   * @brief Solver move constructor
   *
   * @param rvalue
   */
  Solver(Solver &&rvalue) noexcept;

  /**
   * @brief Destroy the Solver object
   *
   */
  ~Solver() = default;

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

  /**
   * @brief Get the entropy in the current game.
   * 
   * @return double 0 ≤ entropy, probably less than 16
   */
  double get_entropy();

protected:
  Solver() = default;

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
  static inline double heuristic(const double entropy);
  template <typename T, typename U, typename V>
  static inline bool word_fits_result(const T &word, const U &guessed, const V &result, const char (&code)[3]);
  template <typename T, typename U>
  static inline int word_to_result_index(const T &word, const U &guessed);

  fs::path word_file_path;
  std::vector<Word> words;
  std::string prev_guess;
  double total_weight;
};

/**
 * @brief Wordle solver bot with parallelization
 *
 */
class SolverParallel : public Solver
{
public:
  /**
   * @brief Construct a new Solver Parallel object
   *
   * @param data_path Path to data dir containing word_weights.txt
   */
  SolverParallel(const std::string &data_path);

  /**
   * @brief Solver Parallel move constructor
   *
   * @param rvalue
   */
  SolverParallel(SolverParallel &&rvalue) noexcept;

  SolverParallel &operator=(SolverParallel &&rvalue) noexcept;

  /**
   * @brief Destroy the Solver Parallel object
   *
   */
  ~SolverParallel();

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
  static void thread_start_routine(SolverParallel *solver, const int i);

  std::vector<std::thread> threads;
  std::vector<std::pair<int, int>> thread_args;
  std::vector<std::vector<double>> thread_ret;
  std::vector<bool> thread_status;
  std::mutex pool_mutex, master_mutex;
  std::condition_variable pool_cv, master_cv;
  int thread_done_count;
  bool terminate_pool;
};
