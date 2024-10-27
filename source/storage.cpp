#include <fcntl.h>
#include <memory>
#include <pinedb/config.h>
#include <pinedb/storage.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

using namespace pinedb;

DiskStorageBackend::DiskStorageBackend(const std::string &file_path, page_size_type page_sz)
    : file_path(file_path), page_sz(page_sz), current_page_id_counter(0)
{
    fd = open(file_path.c_str(), O_CREAT | O_DIRECT | O_RDWR | O_SYNC, 0644);
    if (fd == -1)
    {
        spdlog::error("Could not open DiskStorageBackend(\"{}\"): {}", file_path, strerror(errno));
        exit(1);
    }
    spdlog::info("Opened DiskStorageBackend(\"{}\")", file_path);
}

page_id_type DiskStorageBackend::create_new_page()
{
    off_t offset;
    if ((offset = lseek64(fd, 0, SEEK_END)) == -1)
    {
        spdlog::error("Could not create new page lseek: {}", strerror(errno));
        return -1;
    }
    current_page_id_counter = offset / page_sz;
    std::unique_ptr<char[]> buf(new char[page_sz]);
    memset(buf.get(), 0, page_sz);
    if (write(fd, buf.get(), page_sz) == -1)
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
    auto offset = page_id * page_sz;
    if (lseek64(fd, offset, SEEK_SET) == static_cast<loff_t>(-1))
    {
        // TODO: seek outside file still works
        spdlog::info("Could not read page {}: {}", page_id, strerror(errno));
        return false;
    }
    ssize_t bytes_read;
    if ((bytes_read = read(fd, buffer, page_sz)) == -1)
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
    auto offset = page_id * page_sz;
    if (lseek64(fd, offset, SEEK_SET) == static_cast<loff_t>(-1))
    {
        spdlog::info("Could not write page {}: {}", page_id, strerror(errno));
        return false;
    }
    ssize_t bytes_written;
    if ((bytes_written = write(fd, buffer, page_sz)) == -1)
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
    spdlog::info("Deleting page {}", page_id);
    // There is no free list management for now, so if a page is deleted
    // It is just zeroed out for now
    std::unique_ptr<uint8_t[]> buf(new uint8_t[page_sz]);
    memset(buf.get(), 0, page_sz);
    return write_page(page_id, buf.get());
}

page_size_type DiskStorageBackend::page_size() { return page_sz; }

bool DiskStorageBackend::close()
{
    if (fd == -1)
    {
        spdlog::warn("DiskStorageBackend(\"{}\") has already been closed", file_path);
        return true;
    }
    if (::close(fd) == -1)
    {
        spdlog::error("Error while closing DiskStorageBackend(\"{}\"): {}", file_path,
                      strerror(errno));
        return false;
    }
    fd = -1;
    spdlog::info("Closed DiskStorageBackend(\"{}\")", file_path);
    return true;
}

DiskStorageBackend::~DiskStorageBackend()
{
    if (fd == -1)
        return;
    DiskStorageBackend::close();
}