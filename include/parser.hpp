#pragma once
#include <memory>
#include "lexer.hpp"
#include "expr.hpp"
#include "stmt.hpp"

class Parser
{
    std::string file_name;
    const std::vector<Token> tokens;
    std::size_t pos = 0;

public:
    Parser(std::string file_name, std::vector<Token> tokens) : file_name(std::move(file_name)), tokens(std::move(tokens)) {}

private:
    // Get current token and advance to next
    const Token &advance()
    {
        if (!this->is_at_end())
        {
            return this->tokens[this->pos++];
        }

        return this->tokens.back();
    }

    // Get current token
    const Token &peek() const
    {
        return this->tokens[this->pos];
    }

    // Check if all tokens have been used up
    bool is_at_end() const
    {
        return this->pos >= this->tokens.size();
    }

    // Get previous token
    const Token &previous() const
    {
        return this->tokens[this->pos - 1];
    }

    // Check if token is of given type and value
    bool check(TokenType type, const std::string &val = "") const
    {
        if (this->is_at_end())
        {
            return false;
        }

        return this->peek().type == type && (val.empty() || this->peek().value == val);
    }

    // Check current token and move on to next
    bool match(TokenType type, const std::string &val = "")
    {
        if (this->check(type, val))
        {
            this->advance();
            return true;
        }
        return false;
    }

    // Check if current token is of given type and value.
    // Advance if true.
    // Error out if false.
    const Token &consume(TokenType type, const std::string &val, const std::string &msg)
    {
        if (this->check(type, val))
        {
            return this->advance();
        }
        this->error(msg + " at line " + std::to_string(this->peek().line));
    }

    // Same as previously.
    // Check if current token is of given type only.
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
        if (op == "*" || op == "/")
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

        this->error("Unexpected token in expression: " + peek().value);
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
            std::cerr << "[PARSER] " << this->file_name << ":" << tok.line << ":" << tok.column
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