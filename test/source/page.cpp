#include <doctest/doctest.h>
#include <pinedb/btree.h>
#include <pinedb/bufferpool.h>
#include <pinedb/page.h>

using namespace pinedb;

TEST_SUITE("page")
{
    TEST_CASE("Page header tests")
    {
        PageHeader header;
        header.set_page_id(3);
        header.set_page_type(0x54);
        uint8_t buffer[4096];
        CHECK_EQ(header.write(buffer), buffer + 16);

        PageHeader header2;
        CHECK_EQ(header2.read(buffer), buffer + 16);
        CHECK_EQ(header2.get_page_id(), 3);
        CHECK_EQ(header.get_page_type(), 0x54);
    }
    TEST_CASE("Table metadata tests")
    {
        uint8_t buffer[4096] = {0};
        TableMetadataPage meta(buffer + 16);
        meta.set_column_format("iiSSdf");
        meta.set_table_name("Test table name");
        meta.set_column_name(0, "first column name");
        meta.set_column_name(1, "second column name");
        meta.set_column_name(31, "column_d");
        meta.set_table_page_id(3132);

        TableMetadataPage meta2(buffer + 16);
        CHECK_EQ(meta2.get_column_format(), "iiSSdf");
        CHECK_EQ(meta2.get_table_name(), "Test table name");
        CHECK_EQ(meta2.get_table_page_id(), 3132);
        CHECK_EQ(meta.get_column_name(0), "first column name");
        CHECK_EQ(meta.get_column_name(1), "second column name");
        CHECK_EQ(meta.get_column_name(31), "column_d");
    }
}