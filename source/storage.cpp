#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <memory>
#include <pinedb/common.h>
#include <pinedb/config.h>
#include <pinedb/storage.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

using namespace pinedb;

DiskStorageBackend::DiskStorageBackend(const std::string &file_path, page_size_type page_sz)
    : file_path(file_path), page_sz(page_sz), current_page_id_counter(0)
{
#ifdef _WIN32
    if (!file_handle.open(file_path.c_str(), _O_CREAT | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE))
#elif defined(__APPLE__)
    if (!file_handle.open(file_path.c_str(), O_CREAT | O_RDWR, 0644))
#else
    if (!file_handle.open(file_path.c_str(), O_CREAT | O_DIRECT | O_RDWR | O_SYNC, 0644))
#endif
    {
        spdlog::error("Could not open DiskStorageBackend(\"{}\"): {}", file_path, strerror(errno));
        exit(1);
    }
    off_t offset;
    if ((offset = file_handle.seek(0, SEEK_END)) == -1)
    {
        spdlog::error("Could not seek to end of file to determine number of pages {}",
                      strerror(errno));
        exit(1);
    }
    current_page_id_counter = offset / page_sz;

    spdlog::info("Opened DiskStorageBackend(\"{}\")", file_path);
}

page_id_type DiskStorageBackend::create_new_page()
{
    off_t offset;
    if ((offset = file_handle.seek(0, SEEK_END)) == -1)
    {
        spdlog::error("Could not create new page lseek: {}", strerror(errno));
        return -1;
    }
    current_page_id_counter = offset / page_sz;
    std::unique_ptr<char[]> buf(new char[page_sz]);
    memset(buf.get(), 0, page_sz);
    if (file_handle.write(buf.get(), page_sz) == -1)
    {
        spdlog::error("Could not create new page write: {}", strerror(errno));
        return -1;
    }
    spdlog::info("Created page {}", current_page_id_counter);
    return current_page_id_counter;
}

bool DiskStorageBackend::read_page(page_id_type page_id, uint8_t *buffer)
{
    // Pages are stored contiguously in the file, so the n th page is
    // at location n * page_sz

    // Check if the page id is valid
    if (page_id > current_page_id_counter)
        return false;

    int64_t offset = static_cast<int64_t>(page_id) * page_sz;
    if (file_handle.seek(offset, SEEK_SET) == -1)
    {
        spdlog::info("Could not read page {}: {}", page_id, strerror(errno));
        return false;
    }
    ssize_t bytes_read;
    if ((bytes_read = file_handle.read(buffer, page_sz)) == -1)
    {
        spdlog::info("Could not read page {}: {}", page_id, strerror(errno));
        return false;
    }
    if (bytes_read != page_sz)
    {
        spdlog::warn("When reading page {}, only {} bytes were read", page_id, bytes_read);
        return false;
    }
    spdlog::info("Read page {} [{:n} ...] at 0x{:x}", page_id,
                 spdlog::to_hex(buffer, buffer + config::PAGE_LOG_BYTES), offset);

    return true;
}

bool DiskStorageBackend::write_page(page_id_type page_id, uint8_t *buffer)
{
    // Overwrites the page at offset with the new data
    int64_t offset = static_cast<int64_t>(page_id) * page_sz;
    if (file_handle.seek(offset, SEEK_SET) == -1)
    {
        spdlog::info("Could not write page {}: {}", page_id, strerror(errno));
        return false;
    }
    ssize_t bytes_written;
    if ((bytes_written = file_handle.write(buffer, page_sz)) == -1)
    {
        spdlog::info("Could not write page {}: {}", page_id, strerror(errno));
        return false;
    }
    if (bytes_written != page_sz)
    {
        spdlog::warn("When writing page {}, only {} bytes were written", page_id, bytes_written);
        return false;
    }
    spdlog::info("Wrote page {} [{:n} ...] at 0x{:x}", page_id,
                 spdlog::to_hex(buffer, buffer + config::PAGE_LOG_BYTES), offset);
    return true;
}

bool DiskStorageBackend::delete_page(page_id_type page_id)
{
    spdlog::info("Deleting page [{}]", page_id);
    // There is no free list management for now, so if a page is deleted
    // It is just zeroed out for now, free page management to be implemented by
    // another class
    std::unique_ptr<uint8_t[]> buf(new uint8_t[page_sz]);
    memset(buf.get(), 0, page_sz);
    return write_page(page_id, buf.get());
}

page_size_type DiskStorageBackend::page_size() { return page_sz; }

bool DiskStorageBackend::close()
{
    if (file_handle.closed())
    {
        spdlog::warn("DiskStorageBackend(\"{}\") has already been closed", file_path);
        return true;
    }
    if (!file_handle.close())
    {
        spdlog::error("Error while closing DiskStorageBackend(\"{}\"): {}", file_path,
                      strerror(errno));
        return false;
    }
    spdlog::info("Closed DiskStorageBackend(\"{}\")", file_path);
    return true;
}

DiskStorageBackend::~DiskStorageBackend()
{
    if (file_handle.closed())
        return;
    DiskStorageBackend::close();
}

MemoryStorageBackend::MemoryStorageBackend(page_size_type page_sz)
    : page_sz(page_sz), current_page_id_counter(0)
{
}

page_id_type MemoryStorageBackend::create_new_page()
{
    pages[++current_page_id_counter] = std::vector<uint8_t>(page_sz, 0);
    return current_page_id_counter;
}

bool MemoryStorageBackend::read_page(page_id_type page_id, uint8_t *buffer)
{
    auto iter = pages.find(page_id);
    if (iter == pages.end())
        return false;
    std::copy(iter->second.begin(), iter->second.end(), buffer);
    return true;
}

bool MemoryStorageBackend::write_page(page_id_type page_id, uint8_t *buffer)
{
    auto iter = pages.find(page_id);
    if (iter == pages.end())
        return false;
    std::copy(buffer, buffer + page_sz, iter->second.begin());
    return true;
}

bool MemoryStorageBackend::delete_page(page_id_type page_id)
{
    auto iter = pages.find(page_id);
    if (iter == pages.end())
        return false;
    pages.erase(iter);
    return true;
}

page_size_type MemoryStorageBackend::page_size() { return page_sz; }

bool MemoryStorageBackend::close()
{
    pages.clear();
    return true;
}

MemoryStorageBackend::~MemoryStorageBackend() { close(); }