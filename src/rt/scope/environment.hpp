#pragma once

#include <rt/base_object.hpp>

#include <unordered_map>
#include <optional>
#include <string>

class Environment {
 public:
  using Name = std::string;

  static Environment MakeGlobal() {
    auto gl = Environment{nullptr};
    return gl;
  }

  ////////////////////////////////////////////////////////////////////

  struct ScopeGuard {
    ScopeGuard(Environment** current_scope) {
      auto new_env = new Environment{*current_scope};
      saved_current_scope = current_scope;
      *current_scope = new_env;
    }

    // WARNING: here might be bugs, be careful

    ~ScopeGuard() {
      auto to_delete = *saved_current_scope;
      *saved_current_scope = (*saved_current_scope)->parent_scope_;
      delete to_delete;
    }

    Environment** saved_current_scope;
  };

  ////////////////////////////////////////////////////////////////////

  std::optional<SBObject> Get(Name name) {
    if (auto mb_iter = FindInternal(name)) {
      return {mb_iter.value()->second};
    } else {
      return std::nullopt;
    }
  }

  // For example: name = 3;
  bool TryAssign(Name name, SBObject val) {
    if (auto mb_iter = FindInternal(name)) {
      mb_iter.value()->second = val;
      return true;
    }

    return false;
  }

  // For example: var new_name = 1;
  void Declare(Name name, SBObject val) {
    state_.insert_or_assign(name, val);
  }

  ////////////////////////////////////////////////////////////////////

 private:
  Environment(Environment* parent) : parent_scope_{parent} {
  }

  auto FindInternal(Name name)
      -> std::optional<std::unordered_map<Name, SBObject>::iterator> {
    auto iter = state_.find(name);

    if (iter != state_.end()) {
      return {iter};
    }

    return parent_scope_ ? parent_scope_->FindInternal(std::move(name))
                         : std::nullopt;
  }

 private:
  Environment* parent_scope_;

  std::unordered_map<Name, SBObject> state_{};
};
