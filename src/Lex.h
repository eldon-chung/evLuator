#pragma once

#include <cassert>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

struct Token {
  enum Type : uint8_t {
    Star = '*',
    Plus = '+',
    Equal1 = '=',
    Slash = '/',
    Dash = '-',
    Bang = '!',
    Que = '?',
    Dot = '.',
    Number = 128,
    Name,
    Equal2,
    Invalid = 200,
    EoF,
  };
  std::string_view m_view;
  Type m_type;
  size_t m_num{};
};

struct Lex {
  std::string const &m_text;
  std::vector<Token> m_tokens;

  static inline std::regex name_regex{R"([[:alpha:]_][[:alnum:]_]*)"};
  static inline std::regex num_regex{R"([[:digit:]]+)"};

  Lex(std::string const &text) : m_text(text), m_tokens() {
    std::string_view curr_view{text};

    auto consume_whitespace = [&]() {
      size_t non_ws_pos = curr_view.find_first_not_of(" \r\n");
      if (non_ws_pos != std::string::npos) {
        curr_view.remove_prefix(non_ws_pos);
      } else {
        curr_view = "";
      }
    };

    auto match_name = [&]() {
      std::cmatch res;
      auto matched = std::regex_search(curr_view.begin(), curr_view.end(), res, name_regex,
                                       std::regex_constants::match_continuous);
      assert(matched);
      std::string_view name{res[0].first, res[0].second};
      return Token{name, Token::Name};
    };

    auto match_num = [&]() {
      size_t num;
      auto [ptr, ec] = std::from_chars(curr_view.begin(), curr_view.end(), num);

      return Token{std::string_view{curr_view.data(), ptr}, Token::Number, num};
    };

    auto match_single_char = [&]() {
      std::string_view one_curr_view{curr_view.data(), curr_view.data() + 1};
      switch (curr_view[0]) {
      case '+':
        return Token{one_curr_view, Token::Plus};
        break;
      case '-':
        return Token{one_curr_view, Token::Dash};
        break;
      case '/':
        return Token{one_curr_view, Token::Slash};
        break;
      case '*':
        return Token{one_curr_view, Token::Star};
        break;
      default:
        return Token{"", Token::Invalid};
      }
    };

    auto insert_token = [&](Token token) {
      curr_view.remove_prefix(token.m_view.length());
      m_tokens.push_back(std::move(token));
    };

    while (true) {
      consume_whitespace();
      if (curr_view.empty()) {
        m_tokens.push_back(Token{curr_view, Token::EoF});
        break;
      } else if (std::isalpha(curr_view[0]) || curr_view[0] == '_') {
        insert_token(match_name());
      } else if (std::isdigit(curr_view[0])) {
        insert_token(match_num());
      } else {
        Token token = match_single_char();
        if (token.m_type == Token::Invalid) {
          throw std::runtime_error("you done goofed");
        } else {
          assert(token.m_type != Token::Invalid);
          insert_token(std::move(token));
        }
      }
    }
  }
};
