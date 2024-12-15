#include <pinedb/command.h>
using namespace pinedb;

void CreateTableCommand::execute(const std::string &args) const { fmt::println("Executed create table command!"); }

void ListTableCommand::execute(const std::string &args) const { fmt::println("Executed list table command!"); }

void DescribeTableCommand::execute(const std::string &args) const { fmt::println("Executed describe table command!"); }
