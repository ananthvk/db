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

    // Call the function to create a new page
    storageBackend.create_new_page();

    // Verify that the file exists and has the expected size
    CHECK(std::filesystem::exists(tempFilename));
    CHECK(std::filesystem::file_size(tempFilename) == pageSize);

    // Optionally check the content of the page
    std::ifstream file(tempFilename, std::ios::binary);
    std::vector<char> buffer(pageSize);
    file.read(buffer.data(), pageSize);

    // Check if the page is zeroed
    for (size_t i = 0; i < pageSize; ++i)
    {
        CHECK(buffer[i] == 0);
    }

    // Clean up the temporary file
    std::filesystem::remove(tempFilename);
}
