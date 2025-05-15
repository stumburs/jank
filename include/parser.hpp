#pragma once
#include <memory>
#include "lexer.hpp"

class Parser
{
public:
    Parser(std::vector<Token> tokens) : tokens(std::move(tokens))
    {
    }

private:
    const std::vector<Token> tokens;
};