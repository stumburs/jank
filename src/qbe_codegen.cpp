// #include "qbe_codegen.hpp"
// #include "parser.hpp"

// std::string QBECodegen::generate(const std::vector<std::unique_ptr<Expr>> &program)
// {
//     out << "export function w $main() {\n";
//     out << "@start\n";

//     for (const auto &expr : program)
//     {
//         if (auto let = dynamic_cast<LetExpr *>(expr.get()))
//         {
//             std::string value_temp = emit_expr(let->value.get());
//             locals[let->name] = value_temp;

//             out << "    %" << let->name << " = copy " << value_temp << '\n';
//         }
//         else
//         {
//             emit_expr(expr.get());
//         }
//     }

//     out << "    ret 0\n";
//     out << "}\n";

//     return out.str();
// }

// std::string QBECodegen::emit_expr(const Expr *expr)
// {
//     if (const auto *e = dynamic_cast<const IntegerExpr *>(expr))
//     {
//         return std::to_string(e->value);
//     }

//     if (const auto *e = dynamic_cast<const FloatExpr *>(expr))
//     {
//         throw std::runtime_error("Floats not yet supported in codegen");
//     }

//     if (const auto *e = dynamic_cast<const VariableExpr *>(expr))
//     {
//         auto it = locals.find(e->name);
//         if (it == locals.end())
//         {
//             throw std::runtime_error("Undefined variable: " + e->name);
//         }
//         return it->second;
//     }

//     if (const auto *e = dynamic_cast<const BinaryExpr *>(expr))
//     {
//         std::string lhs = emit_expr(e->left.get());
//         std::string rhs = emit_expr(e->right.get());
//         std::string tmp = new_temp();

//         std::string op;
//         if (e->op == "+")
//         {
//             op = "add";
//         }
//         else if (e->op == "-")
//         {
//             op = "sub";
//         }
//         else if (e->op == "*")
//         {
//             op = "mul";
//         }
//         else if (e->op == "/")
//         {
//             op = "div";
//         }
//         else
//         {
//             throw std::runtime_error("Unsupported binary op: " + e->op);
//         }

//         out << "    " << tmp << " = " << op << " w " << lhs << ", " << rhs << '\n';
//         return tmp;
//     }

//     throw std::runtime_error("Unknown expression");
// }

// std::string QBECodegen::new_temp()
// {
//     return "%" + std::to_string(temp_count++);
// }
