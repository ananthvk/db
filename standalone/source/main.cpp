#include <fmt/color.h>
#include <fmt/format.h>
#include <iostream>
#include <pinedb/bufferpool.h>
#include <pinedb/cachereplacer.h>
#include <pinedb/command.h>
#include <pinedb/command_registry.h>
#include <pinedb/config.h>
#include <pinedb/interpreter.h>
#include <pinedb/page.h>
#include <pinedb/pinedb.h>
#include <pinedb/storage.h>
#include <pinedb/version.h>
#include <sstream>
#include <string>
#include <unordered_map>

bool should_exit = false;

class ExitCommand : public pinedb::Command
{
  public:
    void execute([[maybe_unused]] const std::string &args) const override { should_exit = true; }
};

auto main(int argc, char **argv) -> int

{
    std::unique_ptr<pinedb::StorageBackend> storage;
    if (argc != 2)
    {
        fmt::println("Database file not specified, opening in memory database");
        storage = std::make_unique<pinedb::MemoryStorageBackend>(pinedb::config::PAGE_SIZE);
    }
    else
    {
        fmt::println("Opening DB");
        storage = std::make_unique<pinedb::DiskStorageBackend>(argv[1], pinedb::config::PAGE_SIZE);
    }

    pinedb::LRUCacheReplacer<pinedb::frame_id_type> cache_replacer(
        pinedb::config::NUMBER_OF_FRAMES);
    pinedb::BufferPool pool(pinedb::config::NUMBER_OF_FRAMES, *storage.get(), cache_replacer);

    pinedb::CommandRegistry registry;
    pinedb::CommandInterpreter interpreter(registry);

    registry.registerCommand("create table",
                             []() { return std::make_shared<pinedb::CreateTableCommand>(); });
    registry.registerCommand("list tables",
                             []() { return std::make_shared<pinedb::ListTableCommand>(); });
    registry.registerCommand("describe",
                             []() { return std::make_shared<pinedb::DescribeTableCommand>(); });
    registry.registerCommand("exit", []() { return std::make_shared<ExitCommand>(); });

    std::string line;
    fmt::println("PineDB version {} terminal", PINEDB_VERSION);
    fmt::println("Type 'exit' to exit the terminal");
    while (!should_exit
           && (fmt::print(fmt::fg(fmt::color::green), "> "), std::getline(std::cin, line)))
    {
        if (line.empty())
            continue;
        interpreter.executeLine(line);
    }

    return 0;
}
