// #include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "Lex.h"

std::string read_input(int fd) {
  constexpr size_t READ_CHUNK = 4096;
  std::vector<char> input;
  while (true) {
    size_t old_size = input.size();
    input.resize(old_size + READ_CHUNK);
    size_t num_read = read(fd, input.data() + old_size, READ_CHUNK);
    input.resize(old_size + num_read);
    if (num_read < READ_CHUNK) {
      break;
    }
  }

  return std::string{input.data()};
}

int main() {
  std::string input = read_input(STDIN_FILENO);
  Lex luthor{input};
  for (auto const &token : luthor.m_tokens) {
    std::cout << "Token string[" << token.m_view << "]";
    if (token.m_type < 128) {
      std::cout << " type'" << token.m_type << "'";
    } else {
      std::cout << " type[" << (int)token.m_type << "]";
    }
    if (token.m_type == Token::Number) {
      std::cout << " number[" << token.m_num << "]";
    }
    std::cout << "\n";
  }
}