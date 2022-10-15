#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

struct Environment {
  std::unordered_map<std::string, size_t> m_name_to_value;

  size_t &operator[](std::string const &name) {
    return m_name_to_value.at(name);
  }

  size_t operator[](std::string const &name) const {
    return m_name_to_value.at(name);
  }

  void declare(std::string const &name, size_t value) {
    if (m_name_to_value.contains(name)) {
      throw std::runtime_error("you're redeclaring a name, goldfish.");
    }
    m_name_to_value[name] = value;
  }

  bool is_declared(std::string const &name) {
    return m_name_to_value.contains(name);
  }
};
