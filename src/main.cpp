#include "main.hpp"

Adversary *adversary;

int main(int argc, char const *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: main <path to data dir>\n";
    return EXIT_SUCCESS;
  }
  adversary = new Adversary(argv[1]);
  char res[5];
  adversary->guess({'c', 'r', 'a', 'n', 'e'}, res);
  for (int i = 0; i < 5; i++)
  {
    std::cout << res[i];
  }
  std::cout << std::endl;
  return EXIT_SUCCESS;
}
