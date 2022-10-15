#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <stdexcept>

#include "AST.h"
#include "Lex.h"

struct Parser {
  Lex *m_luthor;

  Parser(Lex *luthor) : m_luthor(luthor) {
  }

  std::unique_ptr<Statement> parse_statement() {
    // Handle statements that start with keywords
    switch (m_luthor->peek_type()) {
    case Token::Var: {
      m_luthor->consume();
      Token name_token = expect(Token::Name);
      std::string_view name_view{name_token.m_view};
      expect(Token::Equal1);

      auto parse_res = parse_expression();
      expect(Token::Semicolon);
      return std::make_unique<DeclarationStatement>(name_view,
                                                    std::move(parse_res));
    }
    default:
      break;
    }

    auto lhs = parse_expression();
    switch (m_luthor->peek_type()) {
    case Token::Semicolon:
      m_luthor->consume();
      return std::make_unique<ExpressionStatement>(std::move(lhs));
    case Token::Equal1: {
      m_luthor->consume();
      auto rhs = parse_expression();
      expect(Token::Semicolon);
      return std::make_unique<AssignmentStatement>(std::move(lhs),
                                                   std::move(rhs));
    }

    default:
      throw std::runtime_error("yeah man idk also");
    }
  }

  std::unique_ptr<Expression> parse_expression() {
    return parse_expression(0);
  }

  std::unique_ptr<Expression> parse_expression(int caller_precedence) {
    auto lhs = parse_atom();
    while (true) {
      Token peeked_token = m_luthor->peek_token();
      auto maybe_binop_type = token_to_binop_type(peeked_token.m_type);

      if (!maybe_binop_type.has_value()) {
        return lhs;
      }

      BinOp::Type binop_type = maybe_binop_type.value();
      int curr_precedence = binop_precedence(binop_type);
      if (curr_precedence <= caller_precedence) {
        return lhs;
      }

      m_luthor->consume();
      auto rhs = parse_expression(curr_precedence);
      auto bin_ast =
          std::make_unique<BinOp>(std::move(lhs), std::move(rhs), binop_type);
      lhs = std::move(bin_ast);
    }
  }

  std::unique_ptr<Expression> parse_atom() {
    Token front_token = m_luthor->consume();
    switch (front_token.m_type) {
    case Token::Name:
      return std::make_unique<Name>(std::string{front_token.m_view});
    case Token::Number:
      return std::make_unique<Number>(front_token.m_num);
    case Token::Lparen: {
      auto res_ast = parse_expression();
      expect(Token::Rparen);
      return res_ast;
    }

    default:
      throw std::runtime_error("expected wrong type when parsing");
    };
  }

  Token expect(Token::Type type) {
    Token front_token = m_luthor->consume();

    if (front_token.m_type != type) {
      throw std::runtime_error("expected wrong type when parsing");
    }
    return front_token;
  }

  static int binop_precedence(BinOp::Type type) {
    switch (type) {
    case BinOp::Plus:
      return 2;
    case BinOp::Minus:
      return 2;
    case BinOp::Times:
      return 3;
    case BinOp::Divide:
      return 3;
    case BinOp::Equality:
      return 1;
    default:
      throw std::runtime_error("current type has no precendence!");
    }
  }

  static std::optional<BinOp::Type> token_to_binop_type(Token::Type type) {
    switch (type) {
    case Token::Plus:
      return BinOp::Plus;
    case Token::Dash:
      return BinOp::Minus;
    case Token::Star:
      return BinOp::Times;
    case Token::Slash:
      return BinOp::Divide;
    case Token::Equal2:
      return BinOp::Equality;
    default:
      return std::nullopt;
    }
  }
};