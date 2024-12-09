#include <doctest/doctest.h>
#include <pinedb/btree.h>
#include <pinedb/bufferpool.h>
#include <pinedb/page.h>

using namespace pinedb;

TEST_SUITE("page")
{
    TEST_CASE("Catalog page tests storing of table name")
    {
        CatalogPage catalog, catalog1, catalog2;
        uint8_t buffer[4096];
        uint8_t buffer1[4096];

        catalog["table1"] = 1312;
        catalog["thisisanothertablename"] = 146;
        catalog["t3"] = 64;
        catalog["x"] = 5123332;
        catalog[std::string(126, 'x')] = 322341;
        catalog[std::string(127, 'y')] = 322341;
        catalog[std::string(128, 'z')] = 322341;
        catalog.save(buffer);

        catalog1.load(buffer);
        CHECK_EQ(catalog1["table1"], 1312);
        CHECK_EQ(catalog1["thisisanothertablename"], 146);
        CHECK_EQ(catalog1["t3"], 64);
        CHECK_EQ(catalog1["x"], 5123332);
        CHECK_EQ(catalog[std::string(126, 'x')] , 322341);
        CHECK_EQ(catalog[std::string(127, 'y')] , 322341);
        CHECK_EQ(catalog[std::string(128, 'z')] , 322341);
        
        catalog1.save(buffer1);
        catalog2.load(buffer1);

        CHECK_EQ(catalog1, catalog2);
        CHECK_EQ(catalog, catalog1);
    }
}