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
  virtual Value evaluate(Environment const &) const = 0;
  virtual ~Expression();
  virtual bool is_name() const {
    return false;
  }
};

struct Number : Expression {
  Value m_number;
  Value evaluate(Environment const &) const override {
    return m_number;
  }

  Number(size_t number) : m_number(number) {
  }
};

struct Name : Expression {
  std::string m_name;
  Value evaluate(Environment const &env) const override {
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

  Value evaluate(Environment const &env) const override {
    Value left_op = m_left_expression->evaluate(env);
    Value right_op = m_right_expression->evaluate(env);

    if (!std::holds_alternative<size_t>(left_op.m_value)) {
      throw std::runtime_error("left operand is not a size_t!");
    }

    if (!std::holds_alternative<size_t>(right_op.m_value)) {
      throw std::runtime_error("right operand is not a size_t!");
    }

    switch (m_type) {
    case Plus:
      return std::get<size_t>(left_op.m_value) +
             std::get<size_t>(right_op.m_value);
    case Minus:
      return std::get<size_t>(left_op.m_value) -
             std::get<size_t>(right_op.m_value);
    case Divide:
      return std::get<size_t>(left_op.m_value) /
             std::get<size_t>(right_op.m_value);
    case Times:
      return std::get<size_t>(left_op.m_value) *
             std::get<size_t>(right_op.m_value);
    case Equality:
      throw std::runtime_error("dont know how to evaluate equality yet");
    }
  }
};

struct Call : Expression {
  std::unique_ptr<Expression> m_function_expression;
  std::vector<std::unique_ptr<Expression>> m_arguments;

  Call(std::unique_ptr<Expression> &&function_expression,
       std::vector<std::unique_ptr<Expression>> &&arguments)
      : m_function_expression(std::move(function_expression)),
        m_arguments(std::move(arguments)) {
  }

  Value evaluate(Environment const &env) const override {
    Value maybe_func = m_function_expression->evaluate(env);
    if (!std::holds_alternative<std::shared_ptr<Function>>(maybe_func.m_value)) {
      throw std::runtime_error("name does not refer to a function!");
    }

    Value arg_val = m_arguments.at(0)->evaluate(env);
    std::optional<Value> maybe_ret_val =
        std::get<std::shared_ptr<Function>>(maybe_func.m_value)
            ->call(std::vector<Value>{arg_val});

    if (!maybe_ret_val.has_value()) {
      throw std::runtime_error("no return value received");
    }

    return maybe_ret_val.value();
  }

  void evaluate_as_statement(Environment const &env) const {
    Value maybe_func = m_function_expression->evaluate(env);
    if (!std::holds_alternative<std::shared_ptr<Function>>(maybe_func.m_value)) {
      throw std::runtime_error("name does not refer to a function!");
    }

    Value arg_val = m_arguments.at(0)->evaluate(env);
    std::get<std::shared_ptr<Function>>(maybe_func.m_value)
        ->call(std::vector<Value>{arg_val});
  }
};

struct ExpressionStatement : Statement {
  std::unique_ptr<Expression> m_expression;

  ExpressionStatement(std::unique_ptr<Expression> &&expression)
      : m_expression(std::move(expression)) {
  }

  void evaluate(Environment &env) const override {
    Call *expression_as_call = dynamic_cast<Call *>(m_expression.get());
    if (expression_as_call == nullptr) {
      // expression was not a function call
      Value val = m_expression->evaluate(env);
      // std::cerr << val.to_string() << std::endl;
    } else {
      expression_as_call->evaluate_as_statement(env);
    }
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
    Value res = m_expression->evaluate(env);
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
    env.assign(name, m_expression->evaluate(env));
  }
};