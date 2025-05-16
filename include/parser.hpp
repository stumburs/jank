#pragma once
#include <memory>
#include "lexer.hpp"

struct Expr
{
    int line = -1;
    virtual ~Expr() = default;
};

struct IntExpr : Expr
{
    int value;
    IntExpr(int value, int line) : value(value) { this->line = line; }
};

struct FloatExpr : Expr
{
    float value;
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

// Statements
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

class Parser
{
    const std::vector<Token> tokens;
    std::size_t pos = 0;

public:
    Parser(std::vector<Token> tokens) : tokens(std::move(tokens))
    {
    }

private:
    const Token &advance()
    {
        if (!this->is_at_end())
        {
            return this->tokens[this->pos++];
        }

        return this->tokens.back();
    }

    const Token &peek() const
    {
        return this->tokens[this->pos];
    }

    bool is_at_end() const
    {
        return this->pos >= this->tokens.size();
    }

    const Token &previous() const
    {
        return this->tokens[this->pos - 1];
    }

    bool check(TokenType type, const std::string &val = "") const
    {
        if (this->is_at_end())
        {
            return false;
        }

        return this->peek().type == type && (val.empty() || this->peek().value == val);
    }

    bool match(TokenType type, const std::string &val = "")
    {
        if (this->check(type, val))
        {
            this->advance();
            return true;
        }
        return false;
    }

    const Token &consume(TokenType type, const std::string &val, const std::string &msg)
    {
        if (this->check(type, val))
        {
            return this->advance();
        }
        this->error(msg + " at line " + std::to_string(this->peek().line));
    }

    const Token &consume(TokenType type, const std::string &msg)
    {
        if (this->check(type))
        {
            return this->advance();
        }
        this->error(msg + " at line " + std::to_string(this->peek().line));
    }

    int get_precedence(const std::string &op)
    {
        if (op == "+" || op == "-")
        {
            return 10;
        }
        if (op == "+" || op == "-")
        {
            return 20;
        }
        return 0;
    }

    std::unique_ptr<Expr> parse_expression(int precedence = 0)
    {
        auto left = this->parse_nud(); // Null denotation

        while (!this->is_at_end() && this->peek().type == TokenType::Symbol && this->get_precedence(this->peek().value) > precedence)
        {
            std::string op = this->advance().value;
            left = this->parse_led(std::move(left), op); // Left denotation
        }

        return left;
    }

    std::unique_ptr<Expr> parse_nud()
    {
        if (match(TokenType::Integer))
        {
            return std::make_unique<IntExpr>(std::stoi(previous().value), previous().line);
        }
        if (match(TokenType::Float))
        {
            return std::make_unique<FloatExpr>(std::stof(previous().value), previous().line);
        }
        if (match(TokenType::String))
        {
            return std::make_unique<StringExpr>(previous().value, previous().line);
        }
        if (match(TokenType::Identifier))
        {
            std::string name = previous().value;

            // Function call
            if (match(TokenType::Symbol, "("))
            {
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::Symbol, ")"))
                {
                    do
                    {
                        args.push_back(parse_expression());
                    } while (match(TokenType::Symbol, ","));
                }
                consume(TokenType::Symbol, ")", "Expected ')' after function arguments");
                return std::make_unique<CallExpr>(name, std::move(args), previous().line);
            }

            return std::make_unique<IdentifierExpr>(name, previous().line);
        }
        if (match(TokenType::Symbol, "("))
        {
            auto expr = parse_expression();
            consume(TokenType::Symbol, ")", "Expected ')'");
            return expr;
        }

        throw std::runtime_error("Unexpected token in expression: " + peek().value);
    }

    std::unique_ptr<Expr> parse_led(std::unique_ptr<Expr> left, const std::string &op)
    {
        int precedence = get_precedence(op);
        auto right = parse_expression(precedence);
        return std::make_unique<BinaryExpr>(std::move(left), op, std::move(right), previous().line);
    }

    std::unique_ptr<Stmt> parse_function()
    {
        std::string name = consume(TokenType::Identifier, "Expected function name").value;
        consume(TokenType::Symbol, "(", "Expected '(' after function name");

        std::vector<std::string> params;
        if (!check(TokenType::Symbol, ")"))
        {
            do
            {
                params.push_back(consume(TokenType::Identifier, "Expected parameter name").value);
            } while (match(TokenType::Symbol, ","));
        }
        consume(TokenType::Symbol, ")", "Expected ')' after parameters");

        auto body = parse_block();
        return std::make_unique<FunctionStmt>(name, std::move(params), std::move(body));
    }

    std::unique_ptr<Stmt> parse_let()
    {
        auto name = consume(TokenType::Identifier, "Expected variable name").value;
        consume(TokenType::Symbol, "=", "Expected '=' after variable name");
        auto init = parse_expression();
        consume(TokenType::Symbol, ";", "Expected ';' after variable declaration");
        return std::make_unique<LetStmt>(name, std::move(init));
    }

    std::unique_ptr<Stmt> parse_declaration()
    {
        if (match(TokenType::Keyword, "let"))
            return this->parse_let();
        if (match(TokenType::Keyword, "fn"))
            return this->parse_function();
        return this->parse_statement();
    }

    std::unique_ptr<Stmt> parse_statement()
    {
        // if (match(TokenType::Keyword, "return"))
        // {
        //     auto value = parse_expression();
        //     consume(TokenType::Symbol, ";", "Expected ';' after return value");
        //     return std::make_unique<ReturnStmt>(std::move(value));
        // }
        // return parse_expression_statement();
        if (match(TokenType::Keyword, "return"))
        {
            std::unique_ptr<Expr> value = nullptr;
            if (!check(TokenType::Symbol, ";"))
            {
                value = parse_expression();
            }
            consume(TokenType::Symbol, ";", "Expected ';' after return statement");
            return std::make_unique<ReturnStmt>(std::move(value));
        }
        return parse_expression_statement();
    }

    std::unique_ptr<Stmt> parse_expression_statement()
    {
        auto expr = parse_expression();
        consume(TokenType::Symbol, ";", "Expected ';' after expression");
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    std::unique_ptr<BlockStmt> parse_block()
    {
        consume(TokenType::Symbol, "{", "Expected '{' to start block");
        std::vector<std::unique_ptr<Stmt>> statements;

        while (!check(TokenType::Symbol, "}") && !is_at_end())
        {
            statements.push_back(parse_declaration());
        }

        consume(TokenType::Symbol, "}", "Expected '}' after block");
        return std::make_unique<BlockStmt>(std::move(statements));
    }

    [[noreturn]] void error(const std::string &message) const
    {
        if (is_at_end())
        {
            std::cerr << "[PARSER] Unexpected end of input: " << message << "\n";
        }
        else
        {
            const auto &tok = peek();
            std::cerr << "[PARSER] " << "<FILE_NAME>" << ":" << tok.line << ":" << tok.column
                      << ": " << message
                      << " near token '" << tok.value << "'\n";
        }
        std::exit(69);
    }

public:
    std::vector<std::unique_ptr<Stmt>> parse_program()
    {
        std::vector<std::unique_ptr<Stmt>> statements;

        while (!is_at_end())
        {
            statements.push_back(this->parse_declaration());
        }

        return statements;
    }
};