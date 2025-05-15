#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>

enum class TokenType
{
    // Basic types
    Integer,
    Float,
    Identifier,
    // Keywords
    String,
    Symbol,
    // Other
    Keyword,
};

struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer
{
public:
    Lexer(const std::string &file_name, const std::string &input) : file_name(file_name), src(input), pos(0), line(1), col(1) {}

    std::vector<Token> tokenize();

    static std::string token_type_to_string(TokenType type);

    void print_tokens(const std::vector<Token> &tokens);

private:
    std::string file_name;
    std::string src;
    std::size_t pos;
    int line, col;

    const std::unordered_set<std::string> keywords = {
        "let", "print", "if", "else", "while", "fn", "return"};

    const std::unordered_set<char> symbols = {
        '=', '+', '-', '*', '/', '(', ')', '{', '}', ';', ','};

    char peek() const;

    char peek_next() const;

    char advance();

    bool eof() const;

    bool is_whitespace(char c) const;

    bool is_symbol(char c) const;

    Token make_number();

    Token make_indentifier_or_keyword();

    Token make_string();

    Token make_symbol()
    {
        int start_col = col;
        char c = this->advance();
        return {TokenType::Symbol, std::string(1, c), this->line, start_col};
    }

    void error(const std::string &message) const
    {
        char c = this->peek();
        std::cerr << "[LEXER] " << this->file_name << ":" << this->line << ":" << this->col
                  << ": " << message
                  << " '" << c << "' (ASCII: " << static_cast<int>(static_cast<unsigned char>(c)) << ")"
                  << std::endl;
        std::exit(69);
    }
};
