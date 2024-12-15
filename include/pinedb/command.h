#ifndef A_COMMAND_H
#define A_COMMAND_H
#include "page.h"

#include <fmt/format.h>

namespace pinedb
{
    class Command
    {
      public:
        virtual ~Command() = default;
        virtual void execute(const std::string &args) const = 0;
    };

    class CreateTableCommand : public Command
    {
      public:
        void execute(const std::string &args) const override;
    };

    class ListTableCommand : public Command
    {
      public:
        void execute(const std::string &args) const override;
    };

    class DescribeTableCommand : public Command
    {
      public:
        void execute(const std::string &args) const override;
    };

}; // namespace pinedb
#endif // A_COMMAND_H