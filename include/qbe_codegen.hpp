#pragma once
#include <fstream>
#include <sstream>
#include "parser.hpp"
#include <unordered_map>

class QBECodegen
{
    std::ostream &out;
    int temp_count = 0;
    int label_count = 0;
    std::unordered_map<std::string, std::string> locals;
    std::unordered_map<std::string, std::string> globals;

public:
    QBECodegen(std::ostream &out) : out(out) {}

    std::string gen_temp()
    {
        return "%" + std::to_string(temp_count++);
    }

    std::string gen_label(const std::string &base = "L")
    {
        return base + std::to_string(label_count++);
    }

    // Emit
    void emit_global_let(const std::string &name)
    {
        std::string label = "@" + name;
        globals[name] = label;
        out << label << " = data l 0\n"; // default to int for now
    }

    void emit_start()
    {
        out << "\n@start = data l 0\n"; // QBE requires data for @start, but it's also the entry

        out << "\n$start = function l () {\n";
        out << "entry0:\n";
        out << "\t%0 = call $main()\n";
        out << "\tret %0\n";
        out << "}\n";
    }

    // Emit a function
    void emit_function(const FunctionStmt *fn)
    {
        out << "\n$" << fn->name << " = function l () {\n"; // TODO: adjust return/param types
        out << gen_label("entry") << ":\n";

        locals.clear();
        for (size_t i = 0; i < fn->params.size(); ++i)
        {
            std::string param_reg = "%" + fn->params[i];
            locals[fn->params[i]] = param_reg;
        }

        for (const auto &stmt : fn->body->statements)
        {
            emit_stmt(stmt.get());
        }

        out << "\tret 0\n"; // Make sure to ret something, adjust as needed
        out << "}\n";
    }

    void emit_stmt(const Stmt *stmt)
    {
        if (auto let = dynamic_cast<const LetStmt *>(stmt))
        {
            std::string value_reg = emit_expr(let->value.get());

            // Local or global
            if (globals.count(let->name))
            {
                out << "\tstore " << value_reg << ", " << globals[let->name] << "\n";
            }
            else
            {
                std::string reg = gen_temp();
                locals[let->name] = reg;
                out << "\t" << reg << " = copy " << value_reg << "\n";
            }
        }
        else if (auto exprstmt = dynamic_cast<const ExprStmt *>(stmt))
        {
            emit_expr(exprstmt->expr.get()); // ignore result
        }
    }

    std::string emit_expr(const Expr *expr)
    {
        if (auto intlit = dynamic_cast<const IntExpr *>(expr))
        {
            std::string reg = gen_temp();
            out << "\t" << reg << " =l const " << intlit->value << "\n";
            return reg;
        }

        if (auto floatlit = dynamic_cast<const FloatExpr *>(expr))
        {
            std::string reg = gen_temp();
            out << "\t" << reg << " =d const " << floatlit->value << "\n";
            return reg;
        }

        if (auto stringlit = dynamic_cast<const StringExpr *>(expr))
        {
            std::string reg = gen_temp();
            // Assuming you have a way to handle string constants in QBE
            out << "\t" << reg << " =s const \"" << stringlit->value << "\"\n";
            return reg;
        }

        if (auto ident = dynamic_cast<const IdentifierExpr *>(expr))
        {
            if (locals.count(ident->name))
            {
                return locals[ident->name];
            }
            else if (globals.count(ident->name))
            {
                std::string reg = gen_temp();
                out << "\t" << reg << " =l load " << globals[ident->name] << "\n";
                return reg;
            }
            throw std::runtime_error("Undefined variable: " + ident->name);
        }

        if (auto bin = dynamic_cast<const BinaryExpr *>(expr))
        {
            std::string lhs = emit_expr(bin->lhs.get());
            std::string rhs = emit_expr(bin->rhs.get());
            std::string result = gen_temp();

            if (bin->op == "+")
                out << "\t" << result << " =l add " << lhs << ", " << rhs << "\n";
            else if (bin->op == "-")
                out << "\t" << result << " =l sub " << lhs << ", " << rhs << "\n";
            else if (bin->op == "*")
                out << "\t" << result << " =l mul " << lhs << ", " << rhs << "\n";
            else if (bin->op == "/")
                out << "\t" << result << " =l divs " << lhs << ", " << rhs << "\n";
            else
                throw std::runtime_error("Unsupported binary operator: " + bin->op);

            return result;
        }

        if (auto call = dynamic_cast<const CallExpr *>(expr))
        {
            if (call->name == "println")
            {
                std::string format_str;
                std::vector<std::string> arg_regs;

                for (size_t i = 0; i < call->arguments.size(); ++i)
                {
                    const Expr *arg = call->arguments[i].get();

                    // Determine format specifier based on expression type
                    if (dynamic_cast<const IntExpr *>(arg))
                        format_str += "%d";
                    else if (dynamic_cast<const FloatExpr *>(arg))
                        format_str += "%f";
                    else if (dynamic_cast<const StringExpr *>(arg))
                        format_str += "%s";
                    else if (dynamic_cast<const IdentifierExpr *>(arg))
                    {
                        // You might need symbol table/type info to decide format here.
                        // For now, assume integer:
                        format_str += "%d";
                    }
                    else
                    {
                        format_str += "%s"; // fallback as string
                    }

                    if (i < call->arguments.size() - 1)
                        format_str += " "; // space between arguments
                }
                format_str += "\\n"; // newline at the end

                // Emit the format string as a global constant
                std::string fmt_reg = gen_temp();
                out << "\t" << fmt_reg << " =s const \"" << format_str << "\"\n";

                // Emit argument registers
                for (const auto &arg : call->arguments)
                {
                    arg_regs.push_back(emit_expr(arg.get()));
                }

                // Call printf: assume signature like int printf(const char*, ...)
                out << "\tcall $printf(s " << fmt_reg;
                for (const auto &reg : arg_regs)
                {
                    out << ", l " << reg; // use 'l' for integer arguments, adjust if float/string
                }
                out << ")\n";

                return ""; // println returns void
            }

            // Normal function call
            std::vector<std::string> arg_regs;
            for (const auto &arg : call->arguments)
            {
                arg_regs.push_back(emit_expr(arg.get()));
            }

            std::string result = gen_temp();

            out << "\t" << result << " = call $" << call->name << "(";
            for (size_t i = 0; i < arg_regs.size(); ++i)
            {
                if (i > 0)
                    out << ", ";
                out << "l " << arg_regs[i];
            }
            out << ")\n";

            return result;
        }

        error(expr, "Unknown expression in codegen");
    }

    // void emit_program(const std::vector<std::unique_ptr<Stmt>> &stmts)
    // {
    //     for (const auto &stmt : stmts)
    //     {
    //         if (auto let = dynamic_cast<const LetStmt *>(stmt.get()))
    //         {
    //             emit_global_let(let->name); // just declares for now
    //         }
    //     }

    //     for (const auto &stmt : stmts)
    //     {
    //         if (auto fn = dynamic_cast<const FunctionStmt *>(stmt.get()))
    //         {
    //             emit_function(fn);
    //         }
    //     }
    // }

    void emit_program(const std::vector<std::unique_ptr<Stmt>> &stmts)
    {
        // 1) Emit globals
        for (const auto &stmt : stmts)
        {
            if (auto let = dynamic_cast<const LetStmt *>(stmt.get()))
            {
                emit_global_let(let->name); // just declares for now
            }
        }

        // 2) Emit functions and check for main
        bool found_main = false;
        for (const auto &stmt : stmts)
        {
            if (auto fn = dynamic_cast<const FunctionStmt *>(stmt.get()))
            {
                if (fn->name == "main")
                    found_main = true;
                emit_function(fn);
            }
        }

        if (!found_main)
            throw std::runtime_error("Mandatory function 'main' not found.");

        // 3) Emit the real program entry point that calls main
        out << "\n@start = data l 0\n"; // define the entry label data

        out << "\n$start = function l () {\n";
        out << "entry0:\n";
        out << "\t%0 = call $main()\n";
        out << "\tret %0\n";
        out << "}\n";
    }

    [[noreturn]] void error(const Expr *expr, const std::string &message) const
    {
        // Assuming Expr has token info (line, col), or you pass line info separately
        std::cerr << "[CODEGEN] Line " << expr->line << ": " << message << "\n";
        std::exit(69);
    }
};