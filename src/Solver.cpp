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
  temp = std::move(rvalue.temp);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = std::move(rvalue.total_weight);
  total_entropy = std::move(rvalue.total_entropy);
}

Solver &Solver::operator=(Solver &&rvalue) noexcept
{
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  temp = std::move(rvalue.temp);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = std::move(rvalue.total_weight);
  total_entropy = std::move(rvalue.total_entropy);
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
    ASSERT(words.back().weight, >, 0);
    total_weight += words.back().weight;
  }
  ASSERT(total_weight, >, 0);
  ASSERT(words.empty(), ==, false);
  calc_total_entropy();
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
  prev_guess = res->val;
  for (int i = 0; i < 5; i++)
  {
    guess[i] = prev_guess[i];
  }
}

void Solver::make_guess(char (&guess)[5], const char (&result)[5])
{
  ASSERT(prev_guess.size(), ==, 5);
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
  std::swap(words, temp);
  temp.clear();
  ASSERT(words.empty(), ==, false);
  calc_total_entropy();
  make_guess(guess);
}

double Solver::calc_expect(const Solver::Word &guess)
{
  double entropy = total_entropy;
  // 0=Gray, 1=Yellow, 2=Green
  // skip all greens
  char result[5]{0};
  for (int i = 1; i < SIZE_OF_RESULTS_SET; i++)
  {
    double p = 0;
    for (const auto &word : words)
    {
      if (word_fits_result(word.val, guess.val, result, {0, 1, 2}))
      {
        p += word.weight;
      }
    }
    if (p > 0)
    {
      p /= total_weight;
      entropy += p * std::log2(p);
    }

    result[0]++;
    for (int j = 0; j < 5 && result[j] > 2; j++)
    {
      result[j] = 0;
      result[j + 1]++;
    }
  }
  return guess.weight / total_weight + (1 - guess.weight / total_weight) * heuristic(entropy);
}

inline double Solver::heuristic(const double entropy)
{
  // a straight line between (0, 1) and (3.5, 11.5)
  // simple regression on assumption that 11.5 bits of entropy requires about 3.5 guesses
  return 0.217391304347826 * entropy + 1;
}

template <typename T, typename U, typename V>
inline bool Solver::word_fits_result(const T &word, const U &guessed, const V &result, const char (&code)[3])
{
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

void Solver::calc_total_entropy()
{
  total_entropy = 0;
  for (const auto &word : words)
  {
    double p = word.weight / total_weight;
    total_entropy -= p * std::log2(p);
  }
}

SolverParallel::SolverParallel(const std::string &data_path)
    : Solver(data_path)
{
  terminate_pool = false;
  int num_threads = std::max(1u, std::thread::hardware_concurrency());
  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; i++)
  {
    threads.push_back(std::thread([this]()
                                  { this->thread_start_routine(); }));
  }
}

SolverParallel::SolverParallel(SolverParallel &&rvalue) noexcept
{
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  temp = std::move(rvalue.temp);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = std::move(rvalue.total_weight);
  total_entropy = std::move(rvalue.total_entropy);
  job_stack = std::move(rvalue.job_stack);
  res_stack = std::move(rvalue.res_stack);
  terminate_pool = std::move(rvalue.terminate_pool);
  delete &rvalue;
  int num_threads = std::max(1u, std::thread::hardware_concurrency());
  for (int i = 0; i < num_threads; i++)
  {
    threads.push_back(std::thread([this]()
                                  { this->thread_start_routine(); }));
  }
}

SolverParallel &SolverParallel::operator=(SolverParallel &&rvalue) noexcept
{
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  temp = std::move(rvalue.temp);
  prev_guess = std::move(rvalue.prev_guess);
  total_weight = std::move(rvalue.total_weight);
  total_entropy = std::move(rvalue.total_entropy);
  job_stack = std::move(rvalue.job_stack);
  res_stack = std::move(rvalue.res_stack);
  terminate_pool = std::move(rvalue.terminate_pool);
  delete &rvalue;
  int num_threads = std::max(1u, std::thread::hardware_concurrency());
  for (int i = 0; i < num_threads; i++)
  {
    threads.push_back(std::thread([this]()
                                  { this->thread_start_routine(); }));
  }
  return *this;
}

SolverParallel::~SolverParallel()
{
  terminate_pool = true;
  job_cv.notify_all();
  for (std::thread &t : threads)
  {
    t.join();
  }
  threads.clear();
}

void SolverParallel::make_guess(char (&guess)[5])
{
  const Word *res = nullptr;
  double best = std::numeric_limits<double>::max();
  job_stack.reserve(words.size());
  res_stack.reserve(words.size());
  for (const auto &word : words)
  {
    add_job([this, &word]()
            {
              const double expect = this->calc_expect(word);
              {
                const std::unique_lock<std::mutex> lock(res_mutex);
                res_stack.push_back(std::make_pair(expect, &word));
              }
              res_cv.notify_all(); });
  }
  for (int i = 0; i < (int)words.size(); i++)
  {
    std::pair<const double, const Word *> *ret;
    {
      std::unique_lock<std::mutex> lock2(res_mutex);
      res_cv.wait(lock2, [this]()
                  { return res_stack.size(); });
      ret = &res_stack.back();
      res_stack.pop_back();
    }
    if (ret->first < best)
    {
      best = ret->first;
      res = ret->second;
    }
  }
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
  std::swap(words, temp);
  temp.clear();
  ASSERT(words.empty(), ==, false);
  calc_total_entropy();
  make_guess(guess);
}

inline void SolverParallel::add_job(std::function<void()> job)
{
  {
    const std::unique_lock<std::mutex> lock(job_mutex);
    job_stack.push_back(job);
  }
  job_cv.notify_one();
}

void SolverParallel::thread_start_routine()
{
  std::function<void()> job;
  while (true)
  {
    {
      std::unique_lock<std::mutex> lock(job_mutex);
      job_cv.wait(lock, [this]()
                  { return job_stack.size() || terminate_pool; });
      if (terminate_pool)
        return;
      job = job_stack.back();
      job_stack.pop_back();
    }
    job();
  }
};
