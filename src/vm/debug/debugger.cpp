#include <vm/debug/debugger.hpp>
#include <vm/interpreter.hpp>

namespace vm::debug {

////////////////////////////////////////////////////////////////////

bool Debugger::Step() {
  if (auto instr = memory_.program_text_->DIEs_.at(ip_.chunk_no)
                       .at(ip_.instr_no)
                       .dbg_instr) {
    printer_.AnnotateSlot(*instr);
  }

  auto instr = GetNextInstruction();
  fmt::print("Current instruction {}: {}\n", rt::FormatInstrRef(ip_),
             Disassembler::FormatInstruction(instr));

  static std::ofstream stream{"dots/raw"};
  stream << ToDot();

  try {
    RunFor(1);
  } catch (rt::PrimitiveValue ret) {
    return_ = ret;
    return false;
  }

  printer_.Print();

  return true;
}

////////////////////////////////////////////////////////////////////

rt::PrimitiveValue Debugger::StepToTheEnd() {
  while (Step())
    ;
  return return_.value();
}

////////////////////////////////////////////////////////////////////

std::string Debugger::ToDot() {
  HeapPrinter heap_printer{*this};

  return fmt::format(
      "digraph G {{                          \n"
      "rankdir=LR;                           \n"
      "node[fontname=\"mono\",shape=none];   \n"
      "{}                                    \n"
      "inst [label=\"{:<80}\"];              \n"
      "inst -> sp;                           \n"
      "{} }}                                \n",
      printer_.ToDot(), FormatCurrentInstruction(),
      heap_printer.FormatHeapPtrs());
}

auto Debugger::GetHeapPtrs() -> std::vector<uint32_t> {
  std::vector<uint32_t> heap_ptrs{0};
  rt::PrimitiveValue* it = (rt::PrimitiveValue*)memory_.GetStackArea();

  for (size_t i = 0; &it[i] <= &stack_.Top(); i++) {
    if (it[i].tag == rt::ValueTag::HeapRef && heap_ptrs.back() != i) {
      heap_ptrs.push_back(it[i].as_ref.to_data);
    }
  }

  heap_ptrs.erase(heap_ptrs.begin());

  return heap_ptrs;
}

////////////////////////////////////////////////////////////////////

std::string Debugger::HeapPrinter::FormatHeapPtrs() {
  auto heap_ptrs = this_debugger.GetHeapPtrs();

  for (auto ptr : heap_ptrs) {
    FormatStrucutre(ptr);
  }

  return fmt::to_string(buf);
}

////////////////////////////////////////////////////////////////////

void Debugger::HeapPrinter::FormatStrucutre(auto ptr) {
  auto [loc, size] = *this_debugger.memory_.LookupSize(ptr);

  fmt::memory_buffer struct_buf;

  for (size_t i = 0; i < size; i++) {
    const auto heap_mem = this_debugger.memory_.memory_;
    const auto value = ((rt::PrimitiveValue*)heap_mem)[loc + i];

    if (value.tag >= rt::ValueTag::HeapRef) {
      FormatSourceNode(loc, i);
      FormatDestinationNode(value);
    }

    fmt::format_to(std::back_inserter(struct_buf), "<td port='{}'>{}</td>",
                   loc + i, rt::FormatValue(value));
  }

  fmt::format_to(std::back_inserter(buf),
                 "heap_{} [label=<<table>\n"
                 "<tr>{}</tr>            \n"
                 "</table>>]             \n",
                 loc, fmt::to_string(struct_buf));
}

////////////////////////////////////////////////////////////////////

void Debugger::HeapPrinter::FormatSourceNode(auto loc, auto i) {
  fmt::format_to(std::back_inserter(buf), "heap_{}:{} -> ", loc, i + loc);
}

////////////////////////////////////////////////////////////////////

void Debugger::HeapPrinter::FormatDestinationNode(const auto& value) {
  switch (value.tag) {
    case rt::ValueTag::StackRef:
      fmt::format_to(std::back_inserter(buf), "stack{};", value.as_ref.to_data);
      break;

    case rt::ValueTag::HeapRef: {
      auto [struct_ref, _] =
          *this_debugger.memory_.LookupSize(value.as_ref.to_data);

      fmt::format_to(std::back_inserter(buf), "heap_{}:{};\n", struct_ref,
                     value.as_ref.to_data);
      break;
    }

    default:
      break;
  }
}

////////////////////////////////////////////////////////////////////

std::string Debugger::FormatCurrentInstruction() {
  auto instr = GetNextInstruction();
  return fmt::format("{}: {}", rt::FormatInstrRef(ip_),
                     Disassembler::FormatInstruction(instr));
}

////////////////////////////////////////////////////////////////////

}  // namespace vm::debug
