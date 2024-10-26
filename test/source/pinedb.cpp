#include <doctest/doctest.h>
#include <pinedb/pinedb.h>
#include <pinedb/version.h>
#include <string>

TEST_CASE("PineDB version")
{
    static_assert(std::string_view(PINEDB_VERSION) == std::string_view("1.0"));
    CHECK(std::string(PINEDB_VERSION) == std::string("1.0"));
}
