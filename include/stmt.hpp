#pragma once
#include <string>
#include <memory>
#include <vector>
#include "expr.hpp"

struct Stmt
{
    virtual ~Stmt() = default;
};

struct LetStmt : Stmt
{
    std::string name;
    std::unique_ptr<Expr> value;
    LetStmt(std::string name, std::unique_ptr<Expr> value)
        : name(std::move(name)), value(std::move(value)) {}
};

struct ExprStmt : Stmt
{
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct ReturnStmt : Stmt
{
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};

struct BlockStmt : Stmt
{
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}
};

struct FunctionStmt : Stmt
{
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<BlockStmt> body;
    FunctionStmt(std::string name, std::vector<std::string> params, std::unique_ptr<BlockStmt> body)
        : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}
};