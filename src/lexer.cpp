#include "lexer.hpp"

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!this->eof())
    {
        char c = this->peek();

        if (this->is_whitespace(c))
        {
            this->advance();
            continue;
        }

        if (std::isdigit(c) || (c == '.' && std::isdigit(this->peek_next())))
        {
            tokens.push_back(this->make_number());
            continue;
        }

        if (std::isalpha(c) || c == '_')
        {
            tokens.push_back(this->make_indentifier_or_keyword());
            continue;
        }

        if (c == '"')
        {
            tokens.push_back(this->make_string());
            continue;
        }

        if (this->is_symbol(c))
        {
            tokens.push_back(this->make_symbol());
            continue;
        }

        this->error(std::string("Unexpected character"));
    }

    return tokens;
}

std::string Lexer::token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TokenType::Keyword:
        return "Keyword";
    case TokenType::Identifier:
        return "Identifier";
    case TokenType::Integer:
        return "Integer";
    case TokenType::Float:
        return "Float";
    case TokenType::String:
        return "String";
    case TokenType::Symbol:
        return "Symbol";
    default:
        return "Unknown";
    }
}

void Lexer::print_tokens(const std::vector<Token> &tokens)
{
    for (const auto &token : tokens)
    {
        std::cout << "[" << token_type_to_string(token.type) << "]"
                  << "\t\"" << token.value << "\""
                  << "\tat line " << token.line << ", column " << token.column
                  << std::endl;
    }
}

char Lexer::peek() const
{
    return this->pos < this->src.size() ? this->src[pos] : '\0';
}

char Lexer::peek_next() const
{
    if (this->pos + 1 >= this->src.size())
    {
        return '\0';
    }
    return this->src[this->pos + 1];
}

char Lexer::advance()
{
    char c = this->peek();
    ++this->pos;
    if (c == '\n')
    {
        ++this->line;
        this->col = 1;
    }
    else
    {
        ++this->col;
    }
    return c;
}

bool Lexer::eof() const
{
    return this->pos >= this->src.size();
}

bool Lexer::is_whitespace(char c) const
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::is_symbol(char c) const
{
    return this->symbols.find(c) != this->symbols.end();
}

Token Lexer::make_number()
{
    int start_col = this->col;
    std::string value;
    bool has_dot = false;

    // Optional leading dot e.g. '.5'
    if (this->peek() == '.')
    {
        has_dot = true;
        value += this->advance();
        if (!std::isdigit(this->peek()))
        {
            this->error("Expected digit after decimal point, but got: ");
        }
    }

    while (std::isdigit(this->peek()) || (!has_dot && this->peek() == '.'))
    {
        if (this->peek() == '.')
        {
            has_dot = true;
        }
        value += this->advance();
    }

    if (has_dot)
    {
        return {TokenType::Float, value, this->line, start_col};
    }
    else
    {
        return {TokenType::Integer, value, this->line, start_col};
    }
}

Token Lexer::make_indentifier_or_keyword()
{
    int start_col = this->col;

    std::string ident;

    while (std::isalnum(this->peek()) || this->peek() == '_')
    {
        ident += this->advance();
    }

    TokenType type = this->keywords.contains(ident) ? TokenType::Keyword : TokenType::Identifier;

    return Token{type, ident, this->line, start_col};
}

Token Lexer::make_string()
{
    int start_col = col;
    this->advance(); // skip opening "
    std::string value;
    while (this->peek() != '"' && !this->eof())
    {
        value += this->advance();
    }
    if (this->peek() == '"')
    {
        this->advance(); // skip closing "
    }
    else
    {
        this->error("Unterminated string literal");
    }

    return {TokenType::String, value, this->line, start_col};
}
