#pragma once

#include <vm/rt/primitive.hpp>

#include <ast/statements.hpp>

#include <optional>
#include <vector>

namespace vm::codegen {

class FrameTranslator {
 public:
  FrameTranslator() {
    // Dummy value in the first slot of fp
    layout_.push_back(Slot{});
  }

  struct Slot {
    std::string name;
    size_t size = 1;
    size_t depth = 0;
  };

  FrameTranslator(FunDeclStatement* decl) {
    for (int i = decl->formals_.size() - 1; i >= 0; i--) {
      layout_.emplace_back(Slot{
          .name = decl->formals_[i].ident.GetName(),
          .depth = 0,
      });
    }

    layout_.emplace_back(Slot{
        .name = "+ip@" + std::to_string(current_depth_),
        .depth = 0,
    });

    fp_ = layout_.size();

    layout_.emplace_back(Slot{
        .name = "+fp@" + std::to_string(current_depth_),
        .depth = 0,
    });
  }

  // Returns offset from the fp
  // (positive if in the current frame)
  // (nagative if from the past frames)
  std::optional<int> Lookup(std::string name) {
    // Check args
    for (size_t i = 0; i < fp_; i++) {
      auto& stack_name = layout_.at(i).name;
      if (stack_name == name) {
        return {i - fp_};
      }
    }

    // Check locals
    for (size_t i = fp_; i < layout_.size(); i++) {
      auto& stack_name = layout_.at(i).name;
      if (stack_name == name) {
        return {i - fp_};
      }
    }

    return std::nullopt;
  }

  void AddLocal(std::string name, size_t size = 1) {
    layout_.emplace_back(Slot{
        .name = name,
        .size = size,
        .depth = current_depth_,
    });

    // Lazy but works
    while (--size > 0) {
      PushAnonValue();
    }
  }

  void CreateNewScope() {
    current_depth_ += 1;
  }

  size_t TeardownInnermostScope() {
    size_t popped = 0;
    while (layout_.back().depth == current_depth_) {
      layout_.pop_back();
      popped += 1;
    }
    return popped;
  }

  void PushAnonValue() {
    layout_.emplace_back(Slot{.name = "", .depth = current_depth_});
  }

  void SetNextSize(size_t size) {
    next_local_size_ = size;
  }

  size_t GetNextSize() {
    return std::exchange(next_local_size_, 1);
  }

 private:
  std::vector<Slot> layout_;
  size_t current_depth_ = 0;
  size_t fp_ = 0;

  // Detail
  size_t next_local_size_ = 1;
};

}  // namespace vm::codegen
