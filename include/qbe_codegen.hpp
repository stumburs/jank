// #pragma once
// #include <fstream>
// #include <sstream>
// #include "parser.hpp"
// #include <unordered_map>

// class QBECodegen
// {
//     std::ostringstream out;
//     int temp_count = 0;
//     std::unordered_map<std::string, std::string> locals;

// public:
//     std::string generate(const std::vector<std::unique_ptr<Expr>> &program);

// private:
//     std::string emit_expr(const Expr *expr);
//     std::string new_temp();
// };