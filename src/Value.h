#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct Function;

struct Value {
  std::variant<size_t, std::shared_ptr<Function>> m_value;

  Value(size_t num) : m_value(num) {
  }
  Value(std::shared_ptr<Function> &&func) : m_value(std::move(func)) {
  }

  std::string to_string() const;
};

struct Function {

  virtual inline ~Function() {
  }

  virtual std::optional<Value> call(std::vector<Value> const &) = 0;
  virtual std::string to_string() const = 0;
};

struct PrintFunction : Function {
  std::optional<Value> call(std::vector<Value> const &params) override {
    for (Value const &v : params) {
      std::cout << v.to_string() << std::endl;
    }

    return std::nullopt;
  }
  std::string to_string() const override {
    return {"Primitive function [Print]"};
  }
};