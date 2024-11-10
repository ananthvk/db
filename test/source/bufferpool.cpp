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
        LRUCacheReplacer<frame_id_type> cache_replacer(number_of_frames);
        BufferPool pool(number_of_frames, storage, cache_replacer);

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
        LRUCacheReplacer<frame_id_type> cache_replacer(number_of_frames);
        BufferPool pool(number_of_frames, storage, cache_replacer);

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
        CHECK(pool.fetch_page(page1) == page1_buffer);
        CHECK(pool.fetch_page(page2) == page2_buffer);
        CHECK(pool.fetch_page(page3) == page3_buffer);

        SUBCASE("fetch page")
        {
            BufferPool pool2(number_of_frames, storage, cache_replacer);
            auto page1_buffer_2 = pool2.fetch_page(page1);
            auto page2_buffer_2 = pool2.fetch_page(page2);
            auto page3_buffer_2 = pool2.fetch_page(page3);
            CHECK(page1_buffer_2 != nullptr);
            CHECK(page2_buffer_2 != nullptr);
            CHECK(page3_buffer_2 != nullptr);
            CHECK(pool2.fetch_page(999) == nullptr);

            CHECK(page1_buffer_2[0] == 'A');
            CHECK(page1_buffer_2[1] == 'B');
            CHECK(page2_buffer_2[0] == 'C');
            CHECK(page2_buffer_2[1] == 'D');
            CHECK(page3_buffer_2[0] == 'E');
            CHECK(page3_buffer_2[1] == 'F');
        }
    }

    TEST_CASE("Invalid page id test cases")
    {
        page_size_type page_size = 128;
        int number_of_frames = 8;
        std::vector<uint8_t> buffer(page_size, 0);
        MemoryStorageBackend memory_backend(page_size);
        StorageBackend &storage = memory_backend;
        LRUCacheReplacer<frame_id_type> cache_replacer(number_of_frames);
        BufferPool pool(number_of_frames, storage, cache_replacer);
        pool.new_page();
        pool.new_page();
        pool.new_page();
        pool.delete_page(9999);
        pool.set_dirty(9999);
    }

    TEST_CASE("BufferPool cache replacement using LRU")
    {
        page_size_type page_size = 128;
        int number_of_frames = 2;
        std::vector<uint8_t> buffer(page_size, 0);
        MemoryStorageBackend memory_backend(page_size);
        StorageBackend &storage = memory_backend;
        LRUCacheReplacer<frame_id_type> cache_replacer(number_of_frames);
        BufferPool pool(number_of_frames, storage, cache_replacer);

        auto page1 = pool.new_page();
        auto page2 = pool.new_page();

        auto page1_buffer = pool.fetch_page(page1);
        auto page2_buffer = pool.fetch_page(page2);

        page1_buffer[0] = 'A';
        page2_buffer[0] = 'B';

        pool.set_dirty(page1);
        pool.set_dirty(page2);

        // Only two frames can be held in memory, so now page1 should get evicted
        auto page3 = pool.new_page();
        auto page3_buffer = pool.fetch_page(page3);

        CHECK(page3_buffer[0] == 0);
        CHECK(page1_buffer[0] == 0);
        CHECK(page2_buffer[0] == 'B');

        CHECK(storage.read_page(page1, buffer.data()));
        CHECK(buffer[0] == 'A');

        page3_buffer[0] = 'C';
        pool.set_dirty(page3);
        pool.flush_all();
        CHECK(storage.read_page(page3, buffer.data()));
        CHECK(buffer[0] == 'C');
    }

    TEST_CASE("BufferPool cache replacement using LRU, many pages")
    {
        page_size_type page_size = 128;
        int number_of_frames = 2;
        std::vector<uint8_t> buffer(page_size, 0);
        MemoryStorageBackend memory_backend(page_size);
        StorageBackend &storage = memory_backend;
        LRUCacheReplacer<frame_id_type> cache_replacer(number_of_frames);
        BufferPool pool(number_of_frames, storage, cache_replacer);

        auto page1 = pool.new_page();
        auto page2 = pool.new_page();
        auto page3 = pool.new_page();
        auto page4 = pool.new_page();
        auto page5 = pool.new_page();
        auto page6 = pool.new_page();

        auto buf1 = pool.fetch_page(page1);
        auto buf2 = pool.fetch_page(page2);

        buf1[0] = '1';
        buf2[0] = '2';

        pool.set_dirty(page1);
        pool.set_dirty(page2);

        buf1 = pool.fetch_page(page5);
        buf2 = pool.fetch_page(page6);

        buf1[0] = '5';
        buf2[0] = '6';

        pool.set_dirty(page5);
        pool.set_dirty(page6);

        buf1 = pool.fetch_page(page4);
        buf2 = pool.fetch_page(page3);

        buf1[0] = '4';
        buf2[0] = '3';

        pool.set_dirty(page4);
        pool.set_dirty(page3);

        pool.flush_all();

        CHECK(storage.read_page(page1, buffer.data()));
        CHECK(buffer[0] == '1');
        CHECK(storage.read_page(page2, buffer.data()));
        CHECK(buffer[0] == '2');
        CHECK(storage.read_page(page3, buffer.data()));
        CHECK(buffer[0] == '3');
        CHECK(storage.read_page(page4, buffer.data()));
        CHECK(buffer[0] == '4');
        CHECK(storage.read_page(page5, buffer.data()));
        CHECK(buffer[0] == '5');
        CHECK(storage.read_page(page6, buffer.data()));
        CHECK(buffer[0] == '6');

        buf1 = pool.fetch_page(page4);
        buf1[0] = '9';
        pool.set_dirty(page4);

        pool.fetch_page(999);
        pool.fetch_page(page6);
        pool.fetch_page(page3);

        CHECK(storage.read_page(page1, buffer.data()));
        CHECK(buffer[0] == '1');
        CHECK(storage.read_page(page2, buffer.data()));
        CHECK(buffer[0] == '2');
        CHECK(storage.read_page(page3, buffer.data()));
        CHECK(buffer[0] == '3');
        CHECK(storage.read_page(page4, buffer.data()));
        CHECK(buffer[0] == '9');
        CHECK(storage.read_page(page5, buffer.data()));
        CHECK(buffer[0] == '5');
        CHECK(storage.read_page(page6, buffer.data()));
        CHECK(buffer[0] == '6');
    }
}
