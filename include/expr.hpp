#pragma once
#include <string>
#include <memory>
#include <vector>

struct Expr
{
    int line = -1;
    virtual ~Expr() = default;
};

struct IntExpr : Expr
{
    long value;
    IntExpr(int value, int line) : value(value) { this->line = line; }
};

struct FloatExpr : Expr
{
    double value;
    FloatExpr(float value, int line) : value(value) { this->line = line; }
};

struct StringExpr : Expr
{
    std::string value;
    StringExpr(std::string value, int line) : value(std::move(value)) { this->line = line; }
};

struct BinaryExpr : Expr
{
    std::unique_ptr<Expr> lhs;
    std::string op;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(std::unique_ptr<Expr> lhs, std::string op, std::unique_ptr<Expr> rhs, int line)
        : lhs(std::move(lhs)), op(op), rhs(std::move(rhs))
    {
        this->line = line;
    }
};

struct IdentifierExpr : Expr
{
    std::string name;
    IdentifierExpr(std::string name, int line) : name(std::move(name)) { this->line = line; }
};

struct CallExpr : Expr
{
    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::string name, std::vector<std::unique_ptr<Expr>> arguments, int line)
        : name(std::move(name)), arguments(std::move(arguments)) { this->line = line; }
};