#include <fmt/color.h>
#include <fmt/format.h>
#include <iostream>
#include <pinedb/bufferpool.h>
#include <pinedb/cachereplacer.h>
#include <pinedb/config.h>
#include <pinedb/page.h>
#include <pinedb/pinedb.h>
#include <pinedb/storage.h>
#include <pinedb/version.h>
#include <sstream>
#include <string>
#include <unordered_map>

auto main(int argc, char **argv) -> int
{
    if (argc != 2)
    {
        fmt::println(
            stderr,
            "Error: Database file not specified\n Usage: ./PineDBStandalone <db_file_name>");
        exit(1);
    }
    pinedb::DiskStorageBackend disk_storage_backend(argv[1], pinedb::config::PAGE_SIZE);
    pinedb::StorageBackend &storage = disk_storage_backend;
    pinedb::LRUCacheReplacer<pinedb::frame_id_type> cache_replacer(
        pinedb::config::NUMBER_OF_FRAMES);
    pinedb::BufferPool pool(pinedb::config::NUMBER_OF_FRAMES, storage, cache_replacer);

    fmt::println("Opening DB");

    // fmt::print(fmt::fg(fmt::color::green), "Database does not exit...Creating new db\n");
}