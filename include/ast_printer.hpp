#pragma once

#pragma once
#include <iostream>
#include <string>
#include "parser.hpp"

class ASTPrinter
{
    int indent = 0;

    void print_indent() const
    {
        for (int i = 0; i < indent; ++i)
            std::cout << "  ";
    }

public:
    void print(const Stmt *stmt)
    {
        if (!stmt)
            return;

        if (auto let = dynamic_cast<const LetStmt *>(stmt))
        {
            print_indent();
            std::cout << "LetStmt: " << let->name << " = ";
            print(let->value.get());
            std::cout << std::endl;
        }
        else if (auto exprStmt = dynamic_cast<const ExprStmt *>(stmt))
        {
            print_indent();
            std::cout << "ExprStmt:\n";
            ++indent;
            print(exprStmt->expr.get());
            --indent;
        }
        else if (auto ret = dynamic_cast<const ReturnStmt *>(stmt))
        {
            print_indent();
            std::cout << "ReturnStmt:\n";
            ++indent;
            print(ret->value.get());
            --indent;
        }
        else if (auto block = dynamic_cast<const BlockStmt *>(stmt))
        {
            print_indent();
            std::cout << "BlockStmt:\n";
            ++indent;
            for (const auto &s : block->statements)
                print(s.get());
            --indent;
        }
        else if (auto fn = dynamic_cast<const FunctionStmt *>(stmt))
        {
            print_indent();
            std::cout << "FunctionStmt: " << fn->name << "(";
            for (size_t i = 0; i < fn->params.size(); ++i)
            {
                std::cout << fn->params[i];
                if (i + 1 < fn->params.size())
                    std::cout << ", ";
            }
            std::cout << ")\n";
            ++indent;
            print(fn->body.get());
            --indent;
        }
        else
        {
            print_indent();
            std::cout << "Unknown Stmt\n";
        }
    }

    void print(const Expr *expr)
    {
        if (!expr)
            return;

        if (auto i = dynamic_cast<const IntExpr *>(expr))
        {
            print_indent();
            std::cout << "IntExpr: " << i->value << std::endl;
        }
        else if (auto f = dynamic_cast<const FloatExpr *>(expr))
        {
            print_indent();
            std::cout << "FloatExpr: " << f->value << std::endl;
        }
        else if (auto s = dynamic_cast<const StringExpr *>(expr))
        {
            print_indent();
            std::cout << "StringExpr: \"" << s->value << "\"" << std::endl;
        }
        else if (auto bin = dynamic_cast<const BinaryExpr *>(expr))
        {
            print_indent();
            std::cout << "BinaryExpr: " << bin->op << std::endl;
            ++indent;
            print(bin->lhs.get());
            print(bin->rhs.get());
            --indent;
        }
        else if (auto call = dynamic_cast<const CallExpr *>(expr))
        {
            print_indent();
            std::cout << "CallExpr: " << call->name << std::endl;
            ++indent;
            for (const auto &arg : call->arguments)
                print(arg.get());
            --indent;
        }
        else if (auto ident = dynamic_cast<const IdentifierExpr *>(expr))
        {
            print_indent();
            std::cout << "IdentifierExpr: " << ident->name << std::endl;
        }
        else
        {
            print_indent();
            std::cout << "Unknown Expr\n";
        }
    }
};