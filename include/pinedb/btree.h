#ifndef A_BTREE_H
#define A_BTREE_H

#include "bufferpool.h"

namespace pinedb
{
    template <typename Key, typename KeyCompare> class BTree
    {
        StorageBackend &storage;
        page_id_type root_page;

      public:
        using record_id_type = int64_t;

        // Do not forget to call create() or load() before using it
        BTree(StorageBackend &storage) : storage(storage), root_page(-1) {}

        // Creates a new B+ Tree, on the storage backend, and returns the page id for the root of
        // the BTree
        page_id_type create();

        // Loads a B+ Tree internal page from the storage backend
        int load(page_id_type page_id);

        // 0 means that the search failed
        record_id_type search(const Key &key) const;

        void insert(const Key &key, record_id_type rid);
    };
} // namespace pinedb
#endif // A_BTREE_H