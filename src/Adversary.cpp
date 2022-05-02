#include "Adversary.hpp"

Adversary::Adversary(const std::string &data_path)
    : word_file_path(fs::path(data_path) / fs::path(TEST_WORDS_FILE_NAME))
{
  std::ifstream word_file(word_file_path);
  num_words = std::count(
      std::istreambuf_iterator<char>(word_file),
      std::istreambuf_iterator<char>(),
      '\n');
  srand(time(NULL));
  new_word();
}

Adversary::Adversary(Adversary &&rvalue) noexcept
    : word_file_path(std::move(rvalue.word_file_path))
{
  num_words = rvalue.num_words;
  target_word = std::move(rvalue.target_word);
}

Adversary &Adversary::operator=(Adversary &&rvalue) noexcept
{
  word_file_path = std::move(rvalue.word_file_path);
  num_words = rvalue.num_words;
  target_word = std::move(rvalue.target_word);
  return *this;
}

void Adversary::new_word()
{
  std::ifstream word_file(word_file_path);
  const int r = rand() % num_words;
  for (int i = 0; i <= r; i++)
  {
    std::getline(word_file, target_word);
  }
}

void Adversary::judge(const char (&guess)[5], char (&result)[5])
{
  for (int i = 0; i < 5; i++)
  {
    if (guess[i] == target_word[i])
    {
      result[i] = 'G';
      used[i] = true;
    }
    else
    {
      result[i] = 'B';
      used[i] = false;
    }
  }
  for (int i = 0; i < 5; i++)
  {
    if (result[i] == 'B')
    {
      for (int j = 0; j < 5; j++)
      {
        if (!used[j] && guess[i] == target_word[j])
        {
          result[i] = 'Y';
          used[j] = true;
          break;
        }
      }
    }
  }
}

std::string Adversary::get_target_word()
{
  return target_word;
}
