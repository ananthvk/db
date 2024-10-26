#ifndef PINEDB_STORAGE_H
#define PINEDB_STORAGE_H

#include "common.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace pinedb
{
    /**
     * This is an interface which represents a storage backend, this class is used to create,
     * read and write pages to a physical storage.
     */
    class StorageBackend
    {
      public:
        /**
         * This method creates a new page in the physical storage
         * @return If successful, `page_id` of the created page, otherwise `-1`
         */
        virtual page_id_type create_new_page() = 0;

        /**
         * This method reads a page from the storage and copies its contents to the passed buffer
         * @param page_id id of the page to be read
         * @param buffer A buffer of size atleast equal to `page_size`, which is used to hold the
         * page data
         * @return true if the read was successful, false otherwise
         */
        virtual bool read_page(page_id_type page_id, uint8_t *buffer) = 0;

        /**
         * This method writes a page to the storage and copies the contents of the buffer
         * @param page_id id of the page to be written
         * @param buffer A buffer of size atleast equal to `page_size`, which is used to hold the
         * page data
         * @return true if the write was successful, false otherwise
         */
        virtual bool write_page(page_id_type page_id, uint8_t *buffer) = 0;

        /**
         * This method deletes a page, and frees the storage associated with the page
         * @param page_id id of the page to be deleted
         * @return true if the delete was successful, false otherwise
         */
        virtual bool delete_page(page_id_type page_id) = 0;

        /**
         * @return The page size of this storage backend
         */
        virtual page_size_type page_size() = 0;

        /**
         * @brief Closes the storage backend
         */
        virtual void close() = 0;

        // Virtual destructor
        virtual ~StorageBackend() {}
    };

    class DiskStorageBackend : public StorageBackend
    {
      private:
        std::string file_path;
        page_size_type page_sz;
        int fd;
        int current_page_id_counter;

      public:
        DiskStorageBackend(const std::string &file_path, page_size_type page_sz);
        page_id_type create_new_page();
        bool read_page(page_id_type page_id, uint8_t *buffer);
        bool write_page(page_id_type page_id, uint8_t *buffer);
        bool delete_page(page_id_type page_id);
        page_size_type page_size();
        void close();
        ~DiskStorageBackend();
    };
} // namespace pinedb
#endif // PINEDB_STORAGE_H