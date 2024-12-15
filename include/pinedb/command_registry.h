#ifndef A_COMMAND_REGISTRY_HPP
#define A_COMMAND_REGISTRY_HPP

#include "command.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace pinedb
{

    class CommandRegistry
    {
        std::unordered_map<std::string, std::function<std::shared_ptr<Command>()>> commandMap;

      public:
        void registerCommand(const std::string &commandPrefix,
                             std::function<std::shared_ptr<Command>()> creator);
        std::shared_ptr<Command> getCommand(const std::string &commandPrefix) const;
    };
} // namespace pinedb

#endif // COMMAND_REGISTRY_HPP
