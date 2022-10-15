#pragma once

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

struct Expression {
  virtual size_t evaluate() const = 0;
  virtual ~Expression();
};

struct Number : Expression {
  size_t m_number;
  size_t evaluate() const override {
    return m_number;
  }

  Number(size_t number) : m_number(number) {
  }
};

struct Name : Expression {
  std::string m_name;
  size_t evaluate() const override {
    throw std::runtime_error("wat do with name?");
  }

  Name(std::string name) : m_name(name) {
  }
};

struct BinOp : Expression {
  enum Type {
    Plus,
    Minus,
    Divide,
    Times,
    Equality,
  };

  std::unique_ptr<Expression> m_left_expression;
  std::unique_ptr<Expression> m_right_expression;
  Type m_type;

  BinOp(std::unique_ptr<Expression> &&left_expression,
        std::unique_ptr<Expression> &&right_expression, Type type)
      : m_left_expression(std::move(left_expression)),
        m_right_expression(std::move(right_expression)), m_type(type) {
  }

  size_t evaluate() const override {
    size_t left_op = m_left_expression->evaluate();
    size_t right_op = m_right_expression->evaluate();

    switch (m_type) {
    case Plus:
      return left_op + right_op;
    case Minus:
      return left_op - right_op;
    case Divide:
      return left_op / right_op;
    case Times:
      return left_op * right_op;
    case Equality:
      throw std::runtime_error("dont know how to evaluate equality yet");
    }
  }
};
