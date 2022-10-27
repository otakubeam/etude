#pragma once

#include <types/trait.hpp>
#include <queue>

namespace types::constraints {

class ConstraintSolver {
 public:
  void Solve() {
    // When do I stop if I have residual constraints?
    while (work_queue.size()) {
      auto it = std::move(work_queue.front());
      work_queue.pop();

      switch (it.tag) {
        case TraitTags::NUM:
        case TraitTags::ORD:
        case TraitTags::EQ:
        case TraitTags::DEREF:
        case TraitTags::CALLABLE:
        
      }
    }
  }

 private:
  using WorkQueue = std::queue<Trait>;

  WorkQueue work_queue;
};

}  // namespace types::constraints
