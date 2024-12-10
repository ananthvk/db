#include <pinedb/bufferpool.h>
#include <spdlog/spdlog.h>

using namespace pinedb;

bool BufferPool::evict()
{
    spdlog::info("Evicting a frame from the pool");
    auto opt = cache_replacer.evict();
    if (!opt.has_value())
    {
        spdlog::warn("BufferPool has run out of memory, cannot evict frame to make space for a "
                     "new page");
        return false;
    }

    spdlog::info("Evicting frame with id {}", opt.value());

    auto pageid = frame_to_page_map[opt.value()];
    if (dirty_frames[opt.value()])
    {
        spdlog::info("Frame {} is dirty, writing to storage", opt.value());
        bool status = storage_backend.write_page(pageid, get_buffer_ptr(opt.value()));
        if (!status)
        {
            spdlog::error("Error while writing frame {} data to storage", opt.value());
        }
        dirty_frames[opt.value()] = false;
    }
    page_to_frame_map.erase(pageid);
    frame_to_page_map.erase(opt.value());
    free_frames.push_back(opt.value());
    return true;
}

BufferPool::BufferPool(int number_of_frames, StorageBackend &storage_backend,
                       CacheReplacer<frame_id_type> &cache_replacer)
    : number_of_frames(number_of_frames),
      storage_backend(storage_backend),
      cache_replacer(cache_replacer),
      buffer(this->storage_backend.page_size() * number_of_frames, 0),
      dirty_frames(number_of_frames, false)
{
    free_frames.reserve(number_of_frames);
    for (auto i = 0; i < number_of_frames; ++i)
        free_frames.push_back(i);

    // spdlog::set_level(spdlog::level::off);
}

uint8_t *BufferPool::fetch_page(page_id_type pageid)
{
    spdlog::info("Fetching page {}", pageid);
    // The page was found in the pool
    auto iter = page_to_frame_map.find(pageid);
    if (iter != page_to_frame_map.end())
    {
        spdlog::info("Page {} found in cache, mapped to {}", pageid, iter->second);
        cache_replacer.access(iter->second);
        return get_buffer_ptr(iter->second);
    }

    // A page fault has occured, read the page from the disk
    // Find a free frame to hold the read out page
    if (free_frames.empty())
    {
        if (!evict())
            return nullptr;
    }
    auto frame_id = free_frames.back();
    free_frames.pop_back();

    spdlog::info("Page fault occured, reading page {} to frame {}", pageid, frame_id);
    bool status = storage_backend.read_page(pageid, get_buffer_ptr(frame_id));
    if (!status)
    {
        // The page could not be read
        spdlog::info("Could not read page {}, freeing frame {}", pageid, frame_id);
        free_frames.push_back(frame_id);
        return nullptr;
    }
    cache_replacer.access(frame_id);
    page_to_frame_map[pageid] = frame_id;
    frame_to_page_map[frame_id] = pageid;
    return get_buffer_ptr(frame_id);
}

bool BufferPool::delete_page(page_id_type pageid)
{
    // If the page is mapped to a frame, free the frame
    auto iter = page_to_frame_map.find(pageid);
    if (iter != page_to_frame_map.end())
    {
        frame_to_page_map.erase(iter->second);
        cache_replacer.reset(iter->second);
        free_frames.push_back(iter->second);
        dirty_frames[iter->second] = false;
        page_to_frame_map.erase(iter);
    }
    return storage_backend.delete_page(pageid);
}

bool BufferPool::flush_page(page_id_type pageid)
{
    auto iter = page_to_frame_map.find(pageid);
    if (iter == page_to_frame_map.end())
    {
        return false;
    }
    auto frameid = iter->second;
    // Check if the page is marked dirty
    if (dirty_frames[frameid])
    {
        dirty_frames[frameid] = false;
        cache_replacer.access(frameid);
        bool status = storage_backend.write_page(pageid, get_buffer_ptr(frameid));
        return status;
    }
    // Do nothing
    return true;
}

page_id_type BufferPool::new_page()
{
    spdlog::info("Creating new page");
    // Find a free frame to hold the new page
    if (free_frames.empty())
    {
        spdlog::info("No free frames for the new page, evicting a page");
        if (!evict())
            return -1;
    }
    auto frame_id = free_frames.back();
    free_frames.pop_back();

    auto pageid = storage_backend.create_new_page();
    spdlog::info("New page {} mapped to frame {}", pageid, frame_id);
    cache_replacer.access(frame_id);
    page_to_frame_map[pageid] = frame_id;
    frame_to_page_map[frame_id] = pageid;
    auto ptr = get_buffer_ptr(frame_id);
    // Zero fill the frame's location
    for (auto i = 0; i < storage_backend.page_size(); ++i)
        ptr[i] = 0;
    return pageid;
}

bool BufferPool::set_dirty(page_id_type pageid)
{
    auto iter = page_to_frame_map.find(pageid);
    if (iter == page_to_frame_map.end())
    {
        return false;
    }

    spdlog::info("Marking page {} (mapped to frame {}) as dirty", pageid, iter->second);
    dirty_frames[iter->second] = true;
    return true;
}

void BufferPool::flush_all()
{
    spdlog::info("Flushing all frames");
    for (const auto &page_frame : page_to_frame_map)
    {
        if (dirty_frames[page_frame.second])
        {
            spdlog::info("Flushing page {} mapped to {}", page_frame.first, page_frame.second);
            flush_page(page_frame.first);
        }
    }
}