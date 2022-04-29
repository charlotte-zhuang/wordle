#include "Solver.hpp"

Solver::Word::Word(const std::string &val_, const double weight_)
    : val(val_), weight(weight_)
{
}

Solver::Word::Word(Word &&rvalue) noexcept
    : val(std::move(rvalue.val)), weight(std::move(rvalue.weight))
{
}

Solver::Word &Solver::Word::operator=(Word &&rvalue) noexcept
{
  val = std::move(rvalue.val);
  weight = std::move(rvalue.weight);
  return *this;
}

char &Solver::Word::operator[](unsigned int i)
{
  return val[i];
}

const char &Solver::Word::operator[](const unsigned int i) const
{
  return val[i];
}

Solver::Solver(const std::string &data_path)
    : word_file_path(fs::path(data_path) / fs::path(WORD_WEIGHTS_FILE_NAME))
{
  reset();
}

Solver::Solver(Solver &&rvalue) noexcept
    : word_file_path(std::move(rvalue.word_file_path))
{
  words = std::move(rvalue.words);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = std::move(rvalue.total_weight);
}

Solver &Solver::operator=(Solver &&rvalue) noexcept
{
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = std::move(rvalue.total_weight);
  return *this;
}

void Solver::reset()
{
  std::ifstream word_file(word_file_path);
  std::string line;
  total_weight = 0;
  /*
    entropy = - SUM_{x in X) (weight_x / total_weight) * log2(weight_x / total_weight)
            = - SUM_{x in X} weight_x * (log2(weight_x) - log2(total_weight)) / total_weight
            = - SUM_{x in X} weight_x * log2(weight_x) / total_weight + SUM_{x in X} weight_x / total_weight * log2(total_weight)
            = - SUM_{x in X} weight_x * log2(weight_x) / total_weight + log2(total_weight)
    */
  while (std::getline(word_file, line))
  {
    if (line.empty())
      continue;
    words.push_back(std::move(Word(line.substr(0, 5), std::stod(line.substr(6)))));
    ASSERT(words.back().weight, >, 0);
    total_weight += words.back().weight;
  }
  ASSERT(total_weight, >, 0);
  ASSERT(words.empty(), ==, false);
}

void Solver::make_guess(char (&guess)[5])
{
  const Word *res = nullptr;
  double best = std::numeric_limits<double>::max();
  for (const auto &word : words)
  {
    double expect = calc_expect(word);
    if (expect < best)
    {
      best = expect;
      res = &word;
    }
  }
  ASSERT(res, !=, nullptr);
  ASSERT(best, >=, 1);
  prev_guess = res->val;
  for (int i = 0; i < 5; i++)
  {
    guess[i] = prev_guess[i];
  }
}

void Solver::make_guess(char (&guess)[5], const char (&result)[5])
{
  ASSERT(prev_guess.size(), ==, 5);
  std::vector<Word> temp;
  temp.reserve(words.size() / 2);
  // update from result
  total_weight = 0;
  for (auto &word : words)
  {
    if (word_fits_result(word.val, prev_guess, result, {'B', 'Y', 'G'}))
    {
      ASSERT(word.weight, >, 0);
      total_weight += word.weight;
      temp.push_back(std::move(word));
    }
  }
  ASSERT(total_weight, >, 0);
  words = std::move(temp);
  ASSERT(words.empty(), ==, false);
  make_guess(guess);
}

double Solver::calc_expect(const Solver::Word &guess)
{
  ASSERT(total_weight, >, 0);
  double expect_entropy = 0; // mean result entropy
  double all_results_total_weight = 0;
  // 0=Gray, 1=Yellow, 2=Green
  // skip all greens
  char result[5]{0};
  for (int i = 1; i < SIZE_OF_RESULTS_SET; i++)
  {
    /*
    entropy = - SUM_{x in X) (weight_x / total_weight) * log2(weight_x / total_weight)
            = - SUM_{x in X} weight_x * (log2(weight_x) - log2(total_weight)) / total_weight
            = - SUM_{x in X} weight_x * log2(weight_x) / total_weight + SUM_{x in X} weight_x / total_weight * log2(total_weight)
            = - SUM_{x in X} weight_x * log2(weight_x) / total_weight + log2(total_weight)
    */
    double result_total_weight = 0, result_entropy = 0;
    for (const auto &word : words)
    {
      if (word_fits_result(word.val, guess.val, result, {0, 1, 2}))
      {
        result_total_weight += word.weight;
        result_entropy -= word.weight * std::log2(word.weight);
      }
    }
    if (result_total_weight > 0)
    {
      all_results_total_weight += result_total_weight;
      expect_entropy += result_entropy + std::log2(result_total_weight) * result_total_weight;
    }

    result[0]++;
    for (int j = 0; j < 5 && result[j] > 2; j++)
    {
      result[j] = 0;
      result[j + 1]++;
    }
  }
  // check if all greens is the only possible result
  if (all_results_total_weight == 0)
    return 1;
  expect_entropy /= all_results_total_weight;
  ASSERT(expect_entropy, >=, 0);
  ASSERT(std::isnan(expect_entropy), ==, false);
  return guess.weight / total_weight + (1 - guess.weight / total_weight) * heuristic(expect_entropy);
}

inline double Solver::heuristic(const double entropy)
{
  // a straight line between (0, 1) and (11.5, 3.5)
  // simple regression on assumption that 11.5 bits of entropy requires about 3.5 guesses
  return 0.217391304347826 * entropy + 1;
}

template <typename T, typename U, typename V>
inline bool Solver::word_fits_result(const T &word, const U &guessed, const V &result, const char (&code)[3])
{
  bool used[5];
  // 0=Gray, 1=Yellow, 2=Green
  for (int i = 0; i < 5; i++)
  {
    if (result[i] == code[1] && guessed[i] == word[i])
    {
      return false;
    }
    else if (result[i] == code[2])
    {
      if (guessed[i] != word[i])
        return false;
      used[i] = true;
    }
    else
    {
      used[i] = false;
    }
  }
  for (int i = 0; i < 5; i++)
  {
    if (result[i] == code[0])
    {
      for (int j = 0; j < 5; j++)
      {
        if (!used[j] && guessed[i] == word[j])
          return false;
      }
    }
    else if (result[i] == code[1])
    {
      bool flag = true;
      for (int j = 0; j < 5; j++)
      {
        if (!used[j] && guessed[i] == word[j])
        {
          used[j] = true;
          flag = false;
          break;
        }
      }
      if (flag)
        return false;
    }
  }
  return true;
}

SolverParallel::SolverParallel(const std::string &data_path)
    : Solver(data_path)
{
  std::unique_lock<std::mutex> lock(pool_mutex);
  terminate_pool = false;
  int num_threads = std::max(1u, std::thread::hardware_concurrency());
  thread_done_count = 0;
  threads.reserve(num_threads);
  thread_status.reserve(num_threads);
  thread_args = std::vector<std::pair<int, int>>(num_threads);
  thread_ret = std::vector<std::vector<double>>(num_threads);
  for (int i = 0; i < num_threads; i++)
  {
    thread_status.push_back(false);
    threads.push_back(std::thread(thread_start_routine, this, i));
  }
}

SolverParallel::SolverParallel(SolverParallel &&rvalue) noexcept
{
  std::unique_lock<std::mutex> lock(pool_mutex);
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = rvalue.total_weight;
  terminate_pool = rvalue.terminate_pool;
  int num_threads = rvalue.threads.size();
  thread_args = std::move(rvalue.thread_args);
  thread_ret = std::move(rvalue.thread_ret);
  delete &rvalue;
  thread_done_count = 0;
  threads.reserve(num_threads);
  thread_status.reserve(num_threads);
  for (int i = 0; i < num_threads; i++)
  {
    thread_status.push_back(false);
    threads.push_back(std::thread(thread_start_routine, this, i));
  }
}

SolverParallel &SolverParallel::operator=(SolverParallel &&rvalue) noexcept
{
  std::unique_lock<std::mutex> lock(pool_mutex);
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = rvalue.total_weight;
  terminate_pool = rvalue.terminate_pool;
  int num_threads = rvalue.threads.size();
  thread_args = std::move(rvalue.thread_args);
  thread_ret = std::move(rvalue.thread_ret);
  delete &rvalue;
  thread_done_count = 0;
  threads.reserve(num_threads);
  thread_status.reserve(num_threads);
  for (int i = 0; i < num_threads; i++)
  {
    thread_status.push_back(false);
    threads.push_back(std::thread(thread_start_routine, this, i));
  }
  return *this;
}

SolverParallel::~SolverParallel()
{
  {
    std::unique_lock<std::mutex> lock(pool_mutex);
    terminate_pool = true;
  }
  pool_cv.notify_all();
  for (auto &t : threads)
  {
    t.join();
  }
  threads.clear();
  thread_args.clear();
  thread_ret.clear();
  thread_status.clear();
}

void SolverParallel::make_guess(char (&guess)[5])
{
  const Word *res = nullptr;
  double best = std::numeric_limits<double>::max();
  int words_per_thread = (words.size() + threads.size() - 1) / threads.size();
  {
    std::unique_lock<std::mutex> lock1(pool_mutex);
    for (size_t i = 0; i < thread_args.size(); i++)
    {
      thread_args[i] = std::make_pair(std::min(i * words_per_thread, words.size()), std::min((i + 1) * words_per_thread, words.size()));
      thread_status[i] = true;
    }
  }
  pool_cv.notify_all();
  for (size_t i = 0; i < threads.size(); i++)
  {
    {
      std::unique_lock<std::mutex> lock_master(master_mutex);
      master_cv.wait(lock_master, [this]()
                     { return thread_done_count > 0; });
      thread_done_count--;
    }
    {
      std::unique_lock<std::mutex> lock2(pool_mutex);
      ASSERT(thread_ret.size(), ==, 8);
      for (size_t j = 0; j < thread_ret.size(); j++)
      {
        if (thread_status[j] || thread_ret[j].empty())
          continue;
        for (size_t k = 0; k < thread_ret[j].size(); k++)
        {
          if (thread_ret[j][k] < best)
          {
            best = thread_ret[j][k];
            res = &words[k + thread_args[j].first];
          }
        }
        thread_ret[j].clear();
        break;
      }
    }
  }
  ASSERT(best, >=, 1);
  ASSERT(res, !=, nullptr);
  prev_guess = res->val;
  for (int i = 0; i < 5; i++)
  {
    guess[i] = prev_guess[i];
  }
}

void SolverParallel::make_guess(char (&guess)[5], const char (&result)[5])
{
  ASSERT(prev_guess.size(), ==, 5);
  // update from result
  std::vector<Word> temp;
  temp.reserve(words.size() / 2);
  total_weight = 0;
  for (auto &word : words)
  {
    if (word_fits_result(word.val, prev_guess, result, {'B', 'Y', 'G'}))
    {
      ASSERT(word.weight, >, 0);
      total_weight += word.weight;
      temp.push_back(std::move(word));
    }
  }
  ASSERT(total_weight, >, 0);
  words = std::move(temp);
  ASSERT(words.empty(), ==, false);
  make_guess(guess);
}

void SolverParallel::thread_start_routine(SolverParallel *solver, const int i)
{
  int j, k;
  while (true)
  {
    {
      std::unique_lock<std::mutex> lock1(solver->pool_mutex);
      ASSERT((int)solver->thread_status.size(), >, i);
      solver->pool_cv.wait(lock1, [&]()
                           { return solver->thread_status[i] || solver->terminate_pool; });
      if (solver->terminate_pool)
        return;
      ASSERT((int)solver->thread_args.size(), >, i);
      j = solver->thread_args[i].first;
      k = solver->thread_args[i].second;
    }
    ASSERT(j, <=, k);
    std::vector<double> ret;
    ret.reserve(k - j);
    for (; j < k; j++)
    {
      ASSERT(j, <, (int)solver->words.size());
      ret.push_back(solver->calc_expect(solver->words[j]));
    }
    {
      std::unique_lock<std::mutex> lock2(solver->pool_mutex);
      solver->thread_ret[i] = std::move(ret);
      solver->thread_status[i] = false;
    }
    {
      std::unique_lock<std::mutex> lock_master(solver->master_mutex);
      solver->thread_done_count++;
    }
    solver->master_cv.notify_all();
  }
};
