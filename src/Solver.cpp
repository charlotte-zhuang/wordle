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
  while (std::getline(word_file, line))
  {
    if (line.empty())
      continue;
    words.push_back(std::move(Word(line.substr(0, 5), std::stod(line.substr(6)))));
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
  if (words.size() <= 1)
    return 1;
  ASSERT(total_weight, >, guess.weight);
  /*
  R = set of words matching a specific result
  r_weight = total weight of R
  entropy = - SUM_{w in R} w_weight * log2(w_weight) / r_weight + log2(r_weight)
  first = - SUM_{w in W} w_weight * log2(w_weight)
  second = r_weight
  */
  std::vector<std::pair<double, double>> results;
  results.reserve(SIZE_OF_RESULTS_SET);
  for (int i = 0; i < SIZE_OF_RESULTS_SET; i++)
  {
    results.push_back(std::make_pair(0, 0));
  }
  for (const auto &word : words)
  {

    const int i = word_to_result_index(word, guess);
    results[i].first -= word.weight * std::log2(word.weight);
    results[i].second += word.weight;
  }
  // skip all greens (SIZE_OF_RESULTS_SET - 1)
  results.pop_back();
  /*
  R = set of all results
  t = total weight of words without all greens
  mean_entropy = SUM_{r in R} r_weight * r_entropy / t
  */
  double mean_entropy = 0;
  for (const auto &result : results)
  {
    if (result.second > 0)
    {
      mean_entropy += result.first + std::log2(result.second) * result.second;
    }
  }
  mean_entropy /= total_weight - guess.weight;
  ASSERT(mean_entropy, >=, 0);
  ASSERT(std::isnan(mean_entropy), ==, false);
  return guess.weight / total_weight + (1 - guess.weight / total_weight) * heuristic(mean_entropy);
}

inline double Solver::heuristic(const double entropy)
{
  // a straight line between (0, 1) and (11.5, 3.5)
  // simple regression on assumption that 11.5 bits of entropy requires about 3.5 guesses
  return 0.217391304347826 * entropy + 1;
}

double Solver::get_entropy()
{
  /*
  W = set of possible words left
  t = total weight of possible words
  entropy = - SUM_{w in W} w_weight * log2(w_weight) / t + log2(t)
  */
  double entropy = 0;
  for (const auto &word : words)
  {
    if (word.weight > 0)
    {
      entropy -= word.weight * std::log2(word.weight);
    }
  }
  return entropy / total_weight + std::log2(total_weight);
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

template <typename T, typename U>
inline int Solver::word_to_result_index(const T &word, const U &guessed)
{
  bool word_used[5], guess_used[5];
  // 0=Gray, 1=Yellow, 2=Green
  int result = 0;
  for (int i = 0, radix = 1; i < 5; i++, radix *= 3)
  {
    if (guessed[i] == word[i])
    {
      result += 2 * radix;
      word_used[i] = true;
      guess_used[i] = true;
    }
    else
    {
      word_used[i] = false;
      guess_used[i] = false;
    }
  }
  for (int i = 0, radix = 1; i < 5; i++, radix *= 3)
  {
    if (!guess_used[i])
    {
      for (int j = 0; j < 5; j++)
      {
        if (!word_used[j] && guessed[i] == word[j])
        {
          result += radix;
          word_used[j] = true;
          break;
        }
      }
    }
  }
  return result;
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
  ASSERT(threads.size(), ==, thread_args.size());
  ASSERT(thread_args.size(), ==, thread_status.size());
  ASSERT(thread_ret.size(), ==, thread_args.size());
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
      for (size_t j = 0; j < thread_ret.size(); j++)
      {
        if (thread_status[j] || thread_ret[j].empty())
          continue;
        for (size_t k = 0; k < thread_ret[j].size(); k++)
        {
          ASSERT(std::isnan(thread_ret[j][k]), ==, false);
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
      ASSERT(i, <, (int)solver->thread_status.size());
      solver->pool_cv.wait(lock1, [&]()
                           { return solver->thread_status[i] || solver->terminate_pool; });
      if (solver->terminate_pool)
        return;
      ASSERT(i, <, (int)solver->thread_args.size());
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
