#include <pinedb/interpreter.h>

using namespace pinedb;

CommandInterpreter::CommandInterpreter(CommandRegistry &registry) : registry(registry) {}

void CommandInterpreter::executeLine(const std::string &line)
{
    // A command can have atmost two parts
    // i.e. [COMMAND] [SUBCOMMAND] or [COMMAND]
    try
    {
        auto commandIdx = line.find(' ');
        auto commandPrefix = line.substr(0, commandIdx);
        auto command = registry.getCommand(commandPrefix);
        if (!command)
        {
            // Check for first two words
            commandIdx = line.find(' ', commandIdx + 1);
            commandPrefix = line.substr(0, commandIdx);
            command = registry.getCommand(commandPrefix);
        }
        if (!command)
        {
            fmt::println("Unrecognized command");
            return;
        }
        std::string args;
        if (commandIdx != std::string::npos)
        {
            args = line.substr(commandIdx);
        }
        command->execute(args);
    }
    catch (std::exception &e)
    {
        fmt::println("Error while executing command: {}", e.what());
    }
}