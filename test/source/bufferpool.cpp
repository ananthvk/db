#include <doctest/doctest.h>
#include <pinedb/bufferpool.h>

using namespace pinedb;

TEST_SUITE("bufferpool")
{
    TEST_CASE("BufferPool create,delete page")
    {
        page_size_type page_size = 4096;
        int number_of_frames = 128;
        MemoryStorageBackend memory_backend(page_size);
        StorageBackend &storage = memory_backend;
        BufferPool pool(number_of_frames, storage);

        auto pageid = pool.new_page();
        CHECK(pageid != -1);
        CHECK(pool.delete_page(pageid));
        // If the page is already deleted, this should return false
        CHECK(!pool.delete_page(pageid));

        // Deletion of non existent page should return false
        CHECK(!pool.delete_page(9999));
    }

    TEST_CASE("BufferPool fetch and flush page")
    {
        page_size_type page_size = 128;
        int number_of_frames = 8;
        std::vector<uint8_t> buffer(page_size, 0);
        MemoryStorageBackend memory_backend(page_size);
        StorageBackend &storage = memory_backend;
        BufferPool pool(number_of_frames, storage);

        // Create three pages
        auto page1 = pool.new_page();
        auto page2 = pool.new_page();
        auto page3 = pool.new_page();

        // Get the page buffers
        auto page1_buffer = pool.fetch_page(page1);
        auto page2_buffer = pool.fetch_page(page2);
        auto page3_buffer = pool.fetch_page(page3);

        page1_buffer[0] = 'A';
        page1_buffer[1] = 'B';
        page2_buffer[0] = 'C';
        page2_buffer[1] = 'D';
        page3_buffer[0] = 'E';
        page3_buffer[1] = 'F';

        SUBCASE("Writing page data, dirty flag")
        {
            // Check that writes are not carried out unless the dirty flag is set
            pool.flush_page(page1);
            storage.read_page(page1, buffer.data());
            CHECK(buffer[0] == 0);
            CHECK(buffer[1] == 0);
            pool.set_dirty(page1);
            pool.flush_page(page1);
            storage.read_page(page1, buffer.data());
            CHECK(buffer[0] == 'A');
            CHECK(buffer[1] == 'B');

            page1_buffer[0] = 'X';
            pool.set_dirty(page2);
            pool.set_dirty(page3);
            pool.flush_all();

            storage.read_page(page1, buffer.data());
            CHECK(buffer[0] == 'A');
            CHECK(buffer[1] == 'B');

            storage.read_page(page2, buffer.data());
            CHECK(buffer[0] == 'C');
            CHECK(buffer[1] == 'D');

            storage.read_page(page3, buffer.data());
            CHECK(buffer[0] == 'E');
            CHECK(buffer[1] == 'F');
        }
        pool.set_dirty(page1);
        pool.set_dirty(page2);
        pool.set_dirty(page3);
        pool.flush_all();

    }
}
