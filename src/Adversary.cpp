#include "Adversary.hpp"

Adversary::Adversary(const std::string& data_path)
: word_file_path(fs::path(data_path) / fs::path(WORD_FILE_NAME))
{
  std::ifstream word_file(word_file_path);
  num_words = std::count(
    std::istreambuf_iterator<char>(word_file),
    std::istreambuf_iterator<char>(),
    '\n'
  );
  srand(time(NULL));
  new_word();
}

void Adversary::new_word()
{
  std::ifstream word_file(word_file_path);
  const int r = rand() % num_words;
  for (int i = 0; i <= r; i++)
  {
    std::getline(word_file, word);
  }
}


void Adversary::guess(const char (&guess)[5], char (&result)[5])
{
  for (int i = 0; i < 5; i++)
  {
    if (guess[i] == word[i])
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
        if (!used[j] && guess[i] == word[j])
        {
          result[i] = 'Y';
          used[j] = true;
        }
      }
    }
  }
}
