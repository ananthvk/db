#ifndef A_INTERPRETER_H
#define A_INTERPRETER_H
#include "command_registry.h"

namespace pinedb
{
    class CommandInterpreter
    {
        CommandRegistry &registry;

      public:
        CommandInterpreter(CommandRegistry &registry);
        
        void executeLine(const std::string& line);
    };
}; // namespace pinedb
#endif // A_INTERPRETER_H