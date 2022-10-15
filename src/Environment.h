#pragma once

#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Value.h"

struct Environment {
  static inline std::unordered_map<std::string, Value> name_primitives{
      {std::string{"print"}, Value{std::make_shared<PrintFunction>()}},
  };
  std::unordered_map<std::string, Value> m_name_to_value;

  Value operator[](std::string const &name) const {
    if (m_name_to_value.contains(name)) {
      return m_name_to_value.at(name);
    }

    if (name_primitives.contains(name)) {
      return name_primitives.at(name);
    }

    throw std::runtime_error("name not found!");
  }

  void declare(std::string const &name, Value value) {
    if (m_name_to_value.contains(name)) {
      throw std::runtime_error("you're redeclaring a name, goldfish.");
    }
    m_name_to_value.emplace(name, value);
  }

  void assign(std::string const &name, Value value) {
    if (!m_name_to_value.contains(name)) {
      throw std::runtime_error(
          "you're assigning to something undeclared, goldfish.");
    }
    m_name_to_value.at(name) = value;
  }

  bool is_declared(std::string const &name) {
    return m_name_to_value.contains(name);
  }
};

inline std::string Value::to_string() const {
  return std::visit(
      overloaded{
          [](size_t num) -> std::string { return std::to_string(num); },
          [](std::shared_ptr<Function> const &func_ptr) -> std::string {
            return func_ptr->to_string();
          },
      },
      m_value);
}