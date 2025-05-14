#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, const char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << argv[i] << std::endl;
    }

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

    std::cout << content.str() << std::endl;
}