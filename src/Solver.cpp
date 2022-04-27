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
}

Solver &Solver::operator=(Solver &&rvalue) noexcept
{
  word_file_path = std::move(rvalue.word_file_path);
  words = std::move(rvalue.words);
  temp = std::move(rvalue.temp);
  prev_guess = std::move(rvalue.prev_guess);
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

inline void Solver::calc_total_entropy()
{
  total_entropy = 0;
  for (const auto &word : words)
  {
    double p = word.weight / total_weight;
    total_entropy -= p * std::log2(p);
  }
}
