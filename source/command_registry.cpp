#include <pinedb/command_registry.h>

using namespace pinedb;

// Creator is a function which returns a command, it is kind of like a factory function
// The factory function takes care of instatiating the Command object, and returns it
void CommandRegistry::registerCommand(const std::string &commandPrefix,
                                      std::function<std::shared_ptr<Command>()> creator)
{
    commandMap[commandPrefix] = creator;
}

std::shared_ptr<Command> CommandRegistry::getCommand(const std::string &commandPrefix) const
{
    auto iter = commandMap.find(commandPrefix);
    if (iter == commandMap.end())
    {
        return nullptr;
    }
    return iter->second();
}