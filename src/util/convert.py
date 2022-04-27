import os
import json
import math

FREQ_MAP = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'data', 'freq_map.json'))
OUTPUT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'data', 'word_weights.txt'))

def sigmoid(x):
  return 1 / (1 + math.exp(-x))

def calc_frequency_based_priors(n_common=2048, width_under_sigmoid=8):
  with open(FREQ_MAP, 'r') as freq_file, open(OUTPUT, 'w') as dest:
    freq_map = json.load(freq_file)
    words = list(freq_map.keys())
    words.sort(key=lambda w: freq_map[w])
    n = len(words)
    left = width_under_sigmoid * n_common / n - width_under_sigmoid
    for i, word in enumerate(words):
      x = left + i * width_under_sigmoid / (n - 1)
      dest.write(f"{word} {max(1e-7, sigmoid(x))}\n")

if __name__ == '__main__':
  calc_frequency_based_priors()
