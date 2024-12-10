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
    // Check if the database exists, i.e. the first page is a catalog page
    if (!pool.fetch_page(0))
    {
        fmt::print(fmt::fg(fmt::color::green), "Database does not exit...Creating new db\n");
        auto page_id = pool.new_page();
        if (page_id != 0)
        {
            fmt::print(fmt::fg(fmt::color::red),
                       "Error while creating database, first page does not have id 0\n");
            throw std::logic_error("Error while creating database");
        }

        pinedb::CatalogPage catalog;
        uint8_t *buffer_ptr = pool.fetch_page(page_id);
        pool.pin_page(page_id);
        catalog.save(buffer_ptr);
        pool.set_dirty(page_id);
        pool.flush_page(page_id);
        pool.unpin_page(page_id);
    }

    // Simple command processor
    std::string line;
    fmt::println("PineDB version {} terminal", PINEDB_VERSION);
    fmt::println("Type 'exit' to exit the terminal");
    while (fmt::print(fmt::fg(fmt::color::green), "> "), std::getline(std::cin, line))
    {

        if (line == "exit")
        {
            fmt::print(fmt::fg(fmt::color::yellow), "Exiting...\n");
            pool.flush_all();
            storage.close();
            exit(0);
        }
        // For simplicity, assume all commands are separated by spaces, and the first word gives the
        // type of command
        if (line.empty())
            continue;
        std::stringstream ss(line);
        std::string command;
        ss >> command;
        if (command == "create")
        {
            std::string subcommand;
            ss >> subcommand;
            if (subcommand == "table")
            {
                std::string table_name;
                while (ss >> table_name)
                {
                    if (table_name.empty())
                    {
                        fmt::print(fmt::fg(fmt::color::red),
                                   "Invalid table name: table name cannot be empty\n");
                        continue;
                    }
                    if (table_name.size() > 128)
                    {
                        fmt::print(fmt::fg(fmt::color::red),
                                   "Invalid table name: table name cannot be longer than 128 "
                                   "characters\n");
                        continue;
                    }

                    pinedb::page_id_type initial_page_id = 0;
                    pinedb::page_id_type current_page_id = initial_page_id;
                    while (1)
                    {
                        // Page id 0 is always the first catalog page
                        auto buffer = pool.fetch_page(current_page_id);
                        pinedb::CatalogPage catalog;
                        pool.pin_page(current_page_id);
                        catalog.load(buffer);
                        if (catalog.size() == catalog.MAX_RECORDS)
                        {
                            // We cannot add any more records to the current catalog page
                            // Go to the next one
                            if (catalog.next_catalog_id == 0)
                            {
                                // There are no more catalog pages after this one
                                auto new_page_id = pool.new_page();
                                catalog.next_catalog_id = new_page_id;
                                catalog.save(buffer);
                                pool.set_dirty(current_page_id);
                                pool.flush_page(current_page_id);
                            }
                            current_page_id = catalog.next_catalog_id;
                        }
                        else
                        {
                            catalog[table_name] = 1;
                            catalog.save(buffer);
                            pool.set_dirty(current_page_id);
                            pool.flush_page(current_page_id);
                            break;
                        }
                        pool.unpin_page(current_page_id);
                    }

                    fmt::print(fmt::fg(fmt::color::green), "Created table {}\n", table_name);
                }
            }
            else
            {
                fmt::print(fmt::fg(fmt::color::red), "Invalid command after 'create'\n");
                continue;
            }
        }
        else if (command == "list")
        {
            std::string subcommand;
            ss >> subcommand;
            if (subcommand == "table")
            {
                pinedb::page_id_type current_page_id = 0;
                do
                {
                    auto buffer = pool.fetch_page(current_page_id);
                    pinedb::CatalogPage catalog;
                    catalog.load(buffer);
                    for (const auto &kv : catalog)
                    {
                        fmt::println("{:<10} | {:<10}", kv.first, kv.second);
                    }
                    fmt::println("There are a total of {} table(s) in page {}", catalog.size(),
                                 current_page_id);
                    current_page_id = catalog.next_catalog_id;
                } while (current_page_id != 0);
            }

            else
            {
                fmt::print(fmt::fg(fmt::color::red), "Invalid command after 'list'\n");
                continue;
            }
        }
        else if (command == "flush")
        {
            pool.flush_all();
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "Invalid command\n");
            continue;
        }
    }
    return 0;
}
