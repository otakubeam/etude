#pragma once

#include <cstdlib>
#include <vector>

namespace vm {

class VmStack {
 public:
 private:
  std::vector<size_t> stack_;
};

}  // namespace vm
