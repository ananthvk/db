#ifndef PINEDB_CACHEREPLACER_H
#define PINEDB_CACHEREPLACER_H
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace pinedb
{
    template <typename T> class CacheReplacer
    {
      public:
        /**
         * Records an acccess to an object identified by `id`
         * @param id id of the object which was accessed
         */
        virtual void access(T id) = 0;

        /**
         * Uses a policy to choose which object to evict, it takes into consideration all
         * the accesses and chooses the object which can be evicted
         * @return An optional which contains the value of the object to be evicted
         */
        virtual std::optional<T> evict() = 0;

        /**
         * Resets all access information of an object `id`, this function should
         * be called after the object is evicted
         * @param id Id of the object whose access is to be cleared
         */
        virtual void reset(T id) = 0;

        /**
         * Sets the object as evictable or not evictable
         * @param id Id of the object
         * @param evictable true if the object can be evicted, false otherwise
         */
        virtual void set_evictable(T id, bool evictable) = 0;

        virtual ~CacheReplacer() {}
    };

    template <typename T> class LRUCacheReplacer : public CacheReplacer<T>
    {
      private:
        // Maximum number of objects in the cache
        int maxsize;
        // Elements at the front are the oldest, and elements
        // at the rear are the newest
        std::list<T> queue;
        std::unordered_map<T, typename std::list<T>::iterator> mp;
        std::unordered_map<T, bool> evictable;

      public:
        LRUCacheReplacer(int maxsize) : maxsize(maxsize) {}

        void access(T id)
        {
            auto iter = mp.find(id);
            // If the element is in the cache, move it to the end
            if (iter != mp.end())
            {
                queue.erase(iter->second);
            }
            queue.push_back(id);
            mp[id] = --queue.end();
            evictable[id] = true;
            // TODO: What if the cache size exceeds the max size?
            // It doesn't happen in this application because there are a fixed number of frames
            if (static_cast<int>(queue.size()) > maxsize)
            {
                queue.pop_back();
                mp.erase(id);
                evictable.erase(id);
                throw std::logic_error("Invalid use of LRU");
            }
        }

        std::optional<T> evict()
        {
            // There are no objects in the cache
            if (queue.empty())
                return std::nullopt;

            T id = queue.front();
            while (!evictable[id])
            {
                // Find an evictable item
                queue.pop_front();
                mp.erase(id);
                evictable.erase(id);
                // There are no more elements, i.e. all elements are marked
                // non evictable
                if (queue.empty())
                    return std::nullopt;
                id = queue.front();
            }

            queue.pop_front();
            mp.erase(id);
            evictable.erase(id);

            return id;
        }

        void reset(T id)
        {
            auto iter = mp.find(id);
            if (iter != mp.end())
            {
                queue.erase(iter->second);
                evictable.erase(id);
                mp.erase(iter);
            }
        }

        void set_evictable(T id, bool is_evictable)
        {
            if (is_evictable && mp.find(id) == mp.end())
            {
                access(id);
                return;
            }
            evictable[id] = is_evictable;
        }

        ~LRUCacheReplacer() {}
    };

} // namespace pinedb

#endif // PINEDB_BUFFERPOOL_H
