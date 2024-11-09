#ifndef PINEDB_BUFFERPOOL_H
#define PINEDB_BUFFERPOOL_H
#include "storage.h"

#include <map>
#include <memory>
#include <vector>

// Class which implements a buffer pool manager
// Uses storage backend to read and write pages, this class also implements
// caching and frame to page mappings
namespace pinedb
{
    class BufferPool
    {
        int number_of_frames;
        StorageBackend &storage_backend;
        std::vector<uint8_t> buffer;
        std::map<page_id_type, frame_id_type> page_to_frame_map;
        // TODO: Simple free frame management, implex more complex schemes such as bitmap later
        std::vector<frame_id_type> free_frames;
        std::vector<bool> dirty_frames;

        // Returns the pointer in the buffer corresponding to the frame
        inline auto get_buffer_ptr(frame_id_type frameid)
        {
            return buffer.data() + (frameid * storage_backend.page_size());
        }

      public:
        BufferPool(int number_of_frames, StorageBackend &storage_backend);

        /**
         * Fetches the page with the given page id
         * @return nullptr if the page is not found, otherwise a `uint8_t*` pointing to the page
         * data
         */
        uint8_t *fetch_page(page_id_type pageid);
        /**
         * Deletes the page with the given page id
         */
        bool delete_page(page_id_type pageid);
        /**
         * Flushes a page (i.e. writes it to the disk irrespective of whether it is dirty or not)
         */
        bool flush_page(page_id_type pageid);
        /**
         * Creates a new page, adds it to the buffer pool and returns the page id
         */
        page_id_type new_page();
        /**
         * Pins the page with the given page id, which ensures that the page
         * is not evicted before being unpinned.
         */
        bool pin_page(page_id_type pageid);
        /**
         * Unpins the page, so that the associated frame can be reused, i.e.
         * the page can be unloaded.
         */
        bool unpin_page(page_id_type pageid);
        
        /**
         * Sets the page as dirty, i.e. data has been modified, this has to be called
         * by all functions which have modified the page data.
         * @return true if the page is in the buffer pool, otherwise false
         */
        bool set_dirty(page_id_type pageid);
        
        /**
         * Flushes all dirty pages to the disk
         */
        void flush_all();
    };
}; // namespace pinedb
#endif // PINEDB_BUFFERPOOL_H