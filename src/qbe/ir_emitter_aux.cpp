#include <qbe/ir_emitter.hpp>
#include <qbe/qbe_types.hpp>

#include <qbe/gen_match.hpp>
#include <qbe/gen_addr.hpp>
#include <qbe/gen_at.hpp>

#include <span>

namespace qbeex {
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::GenAddress(Expression* what, Value out) {
//   GenAddr gen_addr(*this, out);
//   what->Accept(&gen_addr);
// };
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::GenAtAddress(Expression* what, Value where) {
//   if (measure_.IsZST(what->GetType())) {
//     return;
//   }
//   class GenAt gen_addr(*this, where);
//   what->Accept(&gen_addr);
// };
// 
// ////////////////////////////////////////////////////////////////////
// 
// auto IrEmitter::SizeAlign(types::Type* ty) -> std::pair<uint8_t, size_t> {
//   return std::pair(GetTypeSize(ty), measure_.MeasureAlignment(ty));
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// auto IrEmitter::SizeAlign(Expression* node) -> std::pair<uint8_t, size_t> {
//   auto ty = node->GetType();
//   return SizeAlign(ty);
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::EmitType(types::Type* ty) {
//   if (ty->tag != types::TypeTag::TY_APP &&
//       ty->tag != types::TypeTag::TY_STRUCT) {
//     return;
//   }
// 
//   auto storage = types::TypeStorage(ty);
// 
//   if (storage->tag <= types::TypeTag::TY_PTR) {
//     return;
//   }
// 
//   switch (storage->tag) {
//     case types::TypeTag::TY_STRUCT: {
//       auto& members = storage->as_struct.first;
// 
//       for (auto& mem : members) {
//         EmitType(mem.ty);
//       }
// 
//       fmt::print("type :{} = {{ ", Mangle(*ty));
// 
//       for (auto& mem : members) {
//         fmt::print("{} {}, ", ToQbeType(mem.ty), 1);
//       }
// 
//       fmt::print("}}\n");
//       break;
//     }
// 
//     case types::TypeTag::TY_SUM: {
//       auto& members = storage->as_sum.first;
// 
//       for (auto& mem : members) {
//         EmitType(mem.ty);
//       }
// 
//       fmt::print("type :{} = {{ w 1, ", Mangle(*ty));
// 
//       auto size = measure_.MeasureSum(storage);
//       fmt::print(" w {} ", size / 4 - 1);
// 
//       fmt::print("}}\n");
//       break;
//     }
// 
//     default:
//       std::abort();
//   }
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::EmitTypes(std::vector<types::Type*> types) {
//   for (auto ty : types) {
//     EmitType(ty);
//   }
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// IrEmitter::~IrEmitter() {
//   EmitTestArray();
//   EmitStringLiterals();
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::EmitStringLiterals() {
//   for (size_t i = 0; i < string_literals_.size(); i++) {
//     fmt::print("data $strdata.{} = {{ b \"{}\", b 0 }}", i,
//                string_literals_[i]);
//     fmt::print("\n");
//   }
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::EmitTestArray() {
//   fmt::print("export data $et_test_array = {{ ");
// 
//   for (size_t i = 0; i < test_functions_.size(); i++) {
//     fmt::print("l ${}, ", test_functions_[i]);
//   }
// 
//   fmt::print("l 0 }}\n");
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// char IrEmitter::GetStoreSuf(size_t align) {
//   switch (align) {
//     case 1:
//       return 'b';
//     case 4:
//       return 'w';
//     case 8:
//       return 'l';
//     default:
//       std::abort();
//   }
// }
// 
// std::string_view IrEmitter::GetLoadSuf(size_t align) {
//   switch (align) {
//     case 1:
//       return "ub";
//     case 4:
//       return "w";
//     case 8:
//       return "l";
//     default:
//       std::abort();
//   }
// }
// 
// std::string_view IrEmitter::LoadResult(size_t align) {
//   switch (align) {
//     case 1:
//     case 4:
//       return "w";
//     case 8:
//       return "l";
//     default:
//       std::abort();
//   }
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::Copy(size_t align, size_t size, Value src, Value dst) {
//   auto str_suf = GetStoreSuf(align);
//   auto ld_suf = GetLoadSuf(align);
//   auto ld_res = LoadResult(align);
// 
//   auto temp = GenTemporary();
//   auto src_ptr = GenTemporary();
//   auto dst_ptr = GenTemporary();
// 
//   fmt::print("  {} = l copy {}\n", src_ptr.Emit(), src.Emit());
//   fmt::print("  {} = l copy {}\n", dst_ptr.Emit(), dst.Emit());
// 
//   for (size_t copied = 0; copied < size; copied += align) {
//     fmt::print("# Copying \n");
//     fmt::print("  {} = {} load{} {}\n", temp.Emit(), ld_res, ld_suf,
//                src_ptr.Emit());
//     fmt::print("  store{} {}, {}  \n", str_suf, temp.Emit(), dst_ptr.Emit());
// 
//     fmt::print("  {} =l add {}, {}\n", src_ptr.Emit(), src_ptr.Emit(), align);
//     fmt::print("  {} =l add {}, {}\n", dst_ptr.Emit(), dst_ptr.Emit(), align);
//   }
// }
// 
// void IrEmitter::Copy2(Expression* node, Value src, Value dst) {
//   auto loadsuf = LoadSuf(node->GetType());
//   auto [align, size] = SizeAlign(node);
// 
//   auto str_suf = GetStoreSuf(align);
//   auto ld_suf = GetLoadSuf(align);
//   auto ld_res = LoadResult(align);
// 
//   auto temp = GenTemporary();
//   auto src_ptr = GenTemporary();
//   auto dst_ptr = GenTemporary();
// 
//   fmt::print("  {} = l copy {}\n", src_ptr.Emit(), src.Emit());
//   fmt::print("  {} = l copy {}\n", dst_ptr.Emit(), dst.Emit());
// 
//   for (size_t copied = 0; copied < size; copied += align) {
//     fmt::print("# Copying \n");
//     fmt::print("  {} = {} load{} {}\n", temp.Emit(), ld_res, ld_suf,
//                src_ptr.Emit());
//     fmt::print("  store{} {}, {}  \n", str_suf, temp.Emit(), dst_ptr.Emit());
// 
//     fmt::print("  {} =l add {}, {}\n", src_ptr.Emit(), src_ptr.Emit(), align);
//     fmt::print("  {} =l add {}, {}\n", dst_ptr.Emit(), dst_ptr.Emit(), align);
//   }
// }
// 
// uint8_t IrEmitter::GetTypeSize(types::Type* t) {
//   return measure_.MeasureSize(t);
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::CallPrintf(IntrinsicCall* node) {
//   auto fmt = Eval(node->arguments_[0]);
// 
//   std::deque<Value> values;
// 
//   for (auto& a : std::span(node->arguments_).subspan(1)) {
//     values.push_back(Eval(a));
//   }
// 
//   fmt::print("  call $printf (l {}, ..., ", fmt.Emit());
// 
//   for (auto& a : std::span(node->arguments_).subspan(1)) {
//     auto value = std::move(values.front());
//     fmt::print("{} {}, ", ToQbeType(a->GetType()), value.Emit());
//     values.pop_front();
//   }
// 
//   fmt::print(")\n");
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::CheckAssertion(Expression* cond) {
//   auto true_id = id_ += 1;
//   auto false_id = id_ += 1;
//   auto join_id = id_ += 1;
// 
//   auto condition = Eval(cond);
// 
//   fmt::print("#if-start\n");
//   fmt::print("  jnz {}, @true.{}, @false.{}\n", condition.Emit(), true_id,
//              false_id);
// 
//   fmt::print("@true.{}          \n", true_id);
//   // Do nothing
//   fmt::print("  jmp @join.{}    \n", join_id);
// 
//   fmt::print("@false.{}         \n", false_id);
//   CallAbort(cond);
// 
//   fmt::print("@join.{}          \n", join_id);
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// void IrEmitter::CallAbort(Expression* cond) {
//   error_msg_storage_.push_back(
//       fmt::format("Abort at {}\\n", cond->GetLocation().Format()));
// 
//   string_literals_.push_back(error_msg_storage_.back());
// 
//   fmt::print("  call $printf (l $strdata.{}, ..., ) \n",
//              string_literals_.size() - 1);
//   fmt::print("  call $abort ()  \n");
// }
// 
// ////////////////////////////////////////////////////////////////////////////////
// 
// Value IrEmitter::GenParam() {
//   return {.tag = Value::PARAM, .id = id_ += 1};
// }
// 
// Value IrEmitter::GenTemporary() {
//   return {.tag = Value::TEMPORARY, .id = id_ += 1};
// }
// 
// Value IrEmitter::GenConstInt(int value) {
//   return {.tag = Value::CONST_INT, .value = value};
// }
// 
// Value IrEmitter::GenGlobal(std::string_view name) {
//   return {.tag = Value::GLOBAL, .name = std::string{name}};
// }
// 
// ////////////////////////////////////////////////////////////////////////////////

}  // namespace qbe
