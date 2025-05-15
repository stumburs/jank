#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"
#include "qbe_codegen.hpp"

int main(int argc, const char *argv[])
{
    // for (int i = 0; i < argc; i++)
    // {
    //     std::cout << argv[i] << std::endl;
    // }

    if (argc < 2)
    {
        std::cerr << "No input file provided" << std::endl;
        std::exit(69);
    }

    auto input_file_path = argv[1];

    std::ifstream input(input_file_path);

    std::stringstream content;
    if (input.is_open())
    {
        content << input.rdbuf();
    }

    std::cout << content.str() << '\n';

    Lexer lexer("main.jank", content.str());
    auto tokens = lexer.tokenize();
    lexer.print_tokens(tokens);

    // Parser parser(tokens);
    // auto program = Parser(tokens).parse_program();

    // QBECodegen qbe_codegen;
    // std::string qbe = qbe_codegen.generate(program);

    // std::ofstream qbe_file("test.ssa");

    // qbe_file << qbe;

    // std::cout << qbe << std::endl;

    // std::cout << content.str() << std::endl;
}