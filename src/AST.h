#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

#include "Environment.h"

struct Statement {
  virtual void evaluate(Environment &) const = 0;
  virtual ~Statement();
};

struct Expression {
  virtual size_t evaluate(Environment const &) const = 0;
  virtual ~Expression();
  virtual bool is_name() const {
    return false;
  }
};

struct Number : Expression {
  size_t m_number;
  size_t evaluate(Environment const &) const override {
    return m_number;
  }

  Number(size_t number) : m_number(number) {
  }
};

struct Name : Expression {
  std::string m_name;
  size_t evaluate(Environment const &env) const override {
    return env[m_name];
  }

  Name(std::string name) : m_name(name) {
  }

  bool is_name() const override {
    return true;
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

  size_t evaluate(Environment const &env) const override {
    size_t left_op = m_left_expression->evaluate(env);
    size_t right_op = m_right_expression->evaluate(env);

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

struct ExpressionStatement : Statement {
  std::unique_ptr<Expression> m_expression;

  ExpressionStatement(std::unique_ptr<Expression> &&expression)
      : m_expression(std::move(expression)) {
  }

  void evaluate(Environment &env) const override {
    size_t res = m_expression->evaluate(env);
    std::cerr << res << std::endl;
  }
};

struct DeclarationStatement : Statement {
  std::string m_name;
  std::unique_ptr<Expression> m_expression;

  DeclarationStatement(std::string_view name,
                       std::unique_ptr<Expression> &&expression)
      : m_name(name), m_expression(std::move(expression)) {
  }

  void evaluate(Environment &env) const override {
    size_t res = m_expression->evaluate(env);
    env.declare(m_name, res);
  }
};

struct AssignmentStatement : Statement {
  std::unique_ptr<Expression> m_lhs;
  std::unique_ptr<Expression> m_expression;

  AssignmentStatement(std::unique_ptr<Expression> &&lhs,
                      std::unique_ptr<Expression> &&expression)
      : m_lhs(std::move(lhs)), m_expression(std::move(expression)) {
  }

  void evaluate(Environment &env) const override {
    Name *name_ptr = dynamic_cast<Name *>(m_lhs.get());
    if (name_ptr == nullptr) {
      throw std::runtime_error("lhs is not name!");
    }
    std::string name{name_ptr->m_name};
    if (!env.is_declared(name)) {
      throw std::runtime_error("name not declared!");
    }
    env[name] = m_expression->evaluate(env);
  }
};