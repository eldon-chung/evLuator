// #include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "AST.h"
#include "Environment.h"
#include "Lex.h"
#include "Parser.h"

std::string read_input(int fd) {
  constexpr size_t READ_CHUNK = 4096;
  std::vector<char> input;
  while (true) {
    size_t old_size = input.size();
    input.resize(old_size + READ_CHUNK);
    size_t num_read = read(fd, input.data() + old_size, READ_CHUNK);
    input.resize(old_size + num_read);
    if (num_read == 0) {
      break;
    }
  }

  return std::string{input.data()};
}

int main() {
  Environment env;

  std::string input = read_input(STDIN_FILENO);
  Lex luthor{input};
  // {
  //   for (auto const &token : luthor.m_tokens) {
  //     std::cout << "Token string[" << token.m_view << "]";
  //     if (token.m_type < 128) {
  //       std::cout << " type'" << token.m_type << "'";
  //     } else {
  //       std::cout << " type[" << (int)token.m_type << "]";
  //     }
  //     if (token.m_type == Token::Number) {
  //       std::cout << " number[" << token.m_num << "]";
  //     }
  //     std::cout << "\n";
  //   }
  // }
  Parser parser{&luthor};
  std::vector<std::unique_ptr<Statement>> all_da_tingz;
  while (!luthor.eof()) {
    all_da_tingz.push_back(parser.parse_statement());
  }
  for (const auto &stmt : all_da_tingz) {
    stmt->evaluate(env);
  }
}

// int main() {

// auto one = std::make_unique<Number>(1);
// auto two = std::make_unique<Number>(2);

// auto expr = std::make_unique<BinOp>(std::move(one), std::move(two),
// BinOp::Plus);

// std::cout << expr->evaluate() << std::endl;
// }