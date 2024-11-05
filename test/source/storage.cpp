#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <pinedb/pinedb.h>
#include <pinedb/storage.h>
#include <vector>
using namespace pinedb;

TEST_CASE("DiskStorageBackend creates a new page")
{
    std::string tempFilename = "temp_test_file.dat";
    size_t pageSize = 4096; // Example page size

    // Create an instance of DiskStorageBackend
    DiskStorageBackend storageBackend(tempFilename, pageSize);

    // Verify that the file exists and has the expected size
    CHECK(std::filesystem::exists(tempFilename));
    storageBackend.create_new_page();
    CHECK(std::filesystem::file_size(tempFilename) == pageSize);

    // Check the contents of the page
    std::ifstream file(tempFilename, std::ios::binary);
    std::vector<char> buffer(pageSize);
    file.read(buffer.data(), pageSize);

    // Check if the page is zeroed
    for (size_t i = 0; i < pageSize; ++i)
    {
        CHECK(buffer[i] == 0);
    }
    storageBackend.create_new_page();
    file.close();

    std::filesystem::remove(tempFilename);
}

TEST_CASE("DiskStorageBackend creates multiple new page")
{
    std::string tempFilename = "temp_test_file.dat";
    size_t pageSize = 4096; // Example page size

    // Create an instance of DiskStorageBackend
    DiskStorageBackend storageBackend(tempFilename, pageSize);

    // Call the function to create a new page
    storageBackend.create_new_page();
    storageBackend.create_new_page();
    storageBackend.create_new_page();

    CHECK(storageBackend.page_size() == pageSize);

    // Verify that the file exists and has the expected size
    CHECK(std::filesystem::exists(tempFilename));
    CHECK(std::filesystem::file_size(tempFilename) == pageSize * 3);

    std::ifstream file(tempFilename, std::ios::binary);
    std::vector<char> buffer(pageSize * 3);
    file.read(buffer.data(), pageSize * 3);

    // Check if the page is zeroed
    for (size_t i = 0; i < pageSize * 3; ++i)
    {
        CHECK(buffer[i] == 0);
    }
    storageBackend.close();
    file.close();
    std::filesystem::remove(tempFilename);
}

TEST_CASE("DiskStorageBackend read and write page data")
{
    std::string tempFilename = "temp_test_file.dat";
    size_t pageSize = 4096;

    DiskStorageBackend storageBackend(tempFilename, pageSize);

    auto page_id = storageBackend.create_new_page();

    std::vector<uint8_t> buffer(pageSize);
    // Create some random test data
    for (int i = 0; i < static_cast<int>(pageSize); i++)
    {
        buffer[i] = i % 256;
    }
    // First write the page
    CHECK(storageBackend.write_page(page_id, buffer.data()));
    std::fill(buffer.begin(), buffer.end(), 0);
    CHECK(storageBackend.read_page(page_id, buffer.data()));
    CHECK(storageBackend.close());
    std::filesystem::remove(tempFilename);
}

TEST_CASE("DiskStorageBackend read and write multiple pages")
{
    std::string tempFilename = "temp_test_file.dat";
    size_t pageSize = 4096;

    DiskStorageBackend storageBackend(tempFilename, pageSize);

    auto page_id = storageBackend.create_new_page();
    auto page_id2 = storageBackend.create_new_page();

    std::vector<uint8_t> buffer(pageSize);
    // Create some random test data
    for (int i = 0; i < static_cast<int>(pageSize); i++)
    {
        buffer[i] = i % 256;
    }
    // Write the page data
    CHECK(storageBackend.write_page(page_id, buffer.data()));
    for (int i = 0; i < static_cast<int>(pageSize); i++)
    {
        buffer[i] = (i * 3) % 256;
    }
    CHECK(storageBackend.write_page(page_id2, buffer.data()));
    storageBackend.close();

    std::fill(buffer.begin(), buffer.end(), 0);
    DiskStorageBackend storageBackendNew(tempFilename, pageSize);

    CHECK(storageBackendNew.read_page(page_id, buffer.data()));
    for (int i = 0; i < static_cast<int>(pageSize); i++)
    {
        CHECK(buffer[i] == (i % 256));
    }
    CHECK(storageBackendNew.read_page(page_id2, buffer.data()));
    for (int i = 0; i < static_cast<int>(pageSize); i++)
    {
        CHECK(buffer[i] == (i * 3) % 256);
    }
    CHECK(storageBackendNew.close());
    std::filesystem::remove(tempFilename);
}
