#include "main.hpp"

inline void print_arr(const char *label, const char (&arr)[5])
{
  std::cout << label;
  for (int i = 0; i < 5; i++)
  {
    std::cout << arr[i];
  }
  std::cout << '\n';
}

template <typename T>
void play(Adversary adversary, T solver, const int n)
{
  char word[5], res[5];
  int guess_count = 0;
  std::vector<long long> stage_runtimes{0ll};
  chrono::_V2::system_clock::time_point start, stop;

  try
  {
    for (int i = 0; i < n; i++)
    {
      int j = 0;
      start = chrono::high_resolution_clock::now();
      solver.make_guess(word);
      stop = chrono::high_resolution_clock::now();
      stage_runtimes[0] += chrono::duration_cast<chrono::milliseconds>(stop - start).count();

      adversary.judge(word, res);

      while (std::count(std::begin(res), std::end(res), 'G') < 5)
      {
        j++;
        start = chrono::high_resolution_clock::now();
        solver.make_guess(word, res);
        stop = chrono::high_resolution_clock::now();
        if (j >= (int)stage_runtimes.size())
        {
          stage_runtimes.push_back(chrono::duration_cast<chrono::milliseconds>(stop - start).count());
        }
        else
        {
          stage_runtimes[j] += chrono::duration_cast<chrono::milliseconds>(stop - start).count();
        }

        adversary.judge(word, res);
      }
      guess_count += j + 1;
      adversary.new_word();
      solver.reset();
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "target: " << adversary.get_target_word() << '\n';
    print_arr("word: ", word);
    print_arr("res:  ", res);
    std::cerr << e.what();
    std::exit(EXIT_FAILURE);
  }

  std::printf("mean guess count: %.3f\n", (double)guess_count / n);
  std::cout << "guess mean runtimes (ms)\n";
  for (int i = 0; i < (int)stage_runtimes.size(); i++)
  {
    std::printf("%d: %'.3f\n", i + 1, (double)stage_runtimes[i] / n);
  }
  std::exit(EXIT_SUCCESS);
}

template <typename T>
void play(Adversary adversary, T solver)
{
  char word[5], res[5];
  int num_guesses = 1;
  chrono::_V2::system_clock::time_point start, stop;

  std::cout << "target: " << adversary.get_target_word() << '\n';

  std::cout << "GUESS 1" << std::endl;
  start = chrono::high_resolution_clock::now();
  solver.make_guess(word);
  stop = chrono::high_resolution_clock::now();
  std::printf("%'lld ms\n", chrono::duration_cast<chrono::milliseconds>(stop - start).count());
  print_arr("word: ", word);

  std::cout << "JUDGE 1" << std::endl;
  adversary.judge(word, res);
  print_arr("res:  ", res);

  while (std::count(std::begin(res), std::end(res), 'G') < 5)
  {
    num_guesses++;
    std::cout << "GUESS " << num_guesses << std::endl;
    start = chrono::high_resolution_clock::now();
    solver.make_guess(word, res);
    stop = chrono::high_resolution_clock::now();
    std::printf("%'lld ms\n", chrono::duration_cast<chrono::milliseconds>(stop - start).count());
    print_arr("word: ", word);

    std::cout << "JUDGE " << num_guesses << std::endl;
    adversary.judge(word, res);
    print_arr("res:  ", res);
  }
  std::exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
  std::setlocale(LC_NUMERIC, "");
  if (argc < 4)
  {
    std::cout << "Usage: main <path to data dir> <number of game iterations> <s || p>\n";
    return EXIT_SUCCESS;
  }

  int n = std::stoi(argv[2]);
  if (n <= 1)
  {
    if (argv[3][0] == 'p')
      play(Adversary(argv[1]), SolverParallel(argv[1]));
    else
      play(Adversary(argv[1]), Solver(argv[1]));
  }
  else
  {
    if (argv[3][0] == 'p')
      play(Adversary(argv[1]), SolverParallel(argv[1]), n);
    else
      play(Adversary(argv[1]), Solver(argv[1]), n);
  }

  return EXIT_SUCCESS;
}
