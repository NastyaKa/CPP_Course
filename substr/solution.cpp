#include <array>
#include <cstdio>
#include <cstring>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::perror("Error. Expected <input file> and <word_to_search_for>");
    return -1;
  }

  FILE *file = std::fopen(argv[1], "r");
  if (file == nullptr) {
    std::perror("Error. fopen() failed");
    return -1;
  }

  char buf[4097];
  std::array<int, 260> prefix_value{};
  size_t search_string_length = strlen(argv[2]);
  size_t read_len, last_prefix_func_val = 0;
  for (size_t i = 1; i < search_string_length; i++) {
    prefix_value[i] = 0;
    int prev_pref = prefix_value[i - 1];
    while (prev_pref > 0 && argv[2][i] != argv[2][prev_pref]) {
      prev_pref = prefix_value[prev_pref - 1];
    }
    if (argv[2][i] == argv[2][prev_pref]) {
      prev_pref++;
    }
    prefix_value[i] = prev_pref;
  }
  prefix_value[search_string_length] = 0;

  do {
    read_len = std::fread(buf, 1, 4096, file);
    if (read_len != 4096) {
      if (std::ferror(file)) {
        std::perror("Error. fread() failed");
        if (std::fclose(file) == EOF) {
          std::perror("Error. fclose() failed");
          return -1;
        }
        return -1;
      } 
    }
    size_t max_value = 0;
    for (size_t i = 0; i < read_len; i++) {
      size_t prev_pref = last_prefix_func_val;
      while (prev_pref > 0 && buf[i] != argv[2][prev_pref]) {
        prev_pref = prefix_value[prev_pref - 1];
      }
      if (buf[i] == argv[2][prev_pref]) {
        prev_pref++;
      }
      last_prefix_func_val = prev_pref;
      max_value = (max_value > last_prefix_func_val) ? max_value : last_prefix_func_val;
    }
    if (max_value >= search_string_length) {
      std::fprintf(stdout, "Yes\n");
      if (std::fclose(file) == EOF) {
        std::perror("Error. fclose() failed");
        return -1;
      }
      return 0;
    }
  } while (read_len > 0);

  std::fprintf(stdout, "No");
  if (std::fclose(file) == EOF) {
    std::perror("Error. fclose() failed");
    return -1;
  }
  std::fprintf(stdout, "\n");
  return 0;
}