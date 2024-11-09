#include <pinedb/bufferpool.h>
#include <spdlog/spdlog.h>

using namespace pinedb;

BufferPool::BufferPool(int number_of_frames, StorageBackend &storage_backend)
    : number_of_frames(number_of_frames),
      storage_backend(storage_backend),
      buffer(this->storage_backend.page_size() * number_of_frames, 0),
      dirty_frames(number_of_frames, false)
{
    free_frames.reserve(number_of_frames);
    for (auto i = 0; i < number_of_frames; ++i)
        free_frames.push_back(i);
}

uint8_t *BufferPool::fetch_page(page_id_type pageid)
{
    spdlog::info("Fetching page {}", pageid);
    // The page was found in the pool
    auto iter = page_to_frame_map.find(pageid);
    if (iter != page_to_frame_map.end())
    {
        spdlog::info("Page {} found in cache", pageid);
        return get_buffer_ptr(iter->second);
    }

    // A page fault has occured, read the page from the disk
    // Find a free frame to hold the read out page
    if (free_frames.empty())
    {
        spdlog::info("No free frames left, trying to evict a page");
        // TODO: No free frames, use cache replacer here to evict a page
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
    page_to_frame_map[pageid] = frame_id;
    return get_buffer_ptr(frame_id);
}

bool BufferPool::delete_page(page_id_type pageid)
{
    // If the page is mapped to a frame, free the frame
    auto iter = page_to_frame_map.find(pageid);
    if (iter != page_to_frame_map.end())
    {
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
        bool status = storage_backend.write_page(pageid, get_buffer_ptr(frameid));
        return status;
    }
    // Do nothing
    return true;
}

page_id_type BufferPool::new_page()
{
    // Find a free frame to hold the new page
    if (free_frames.empty())
        // TODO: No free frames, use cache replacer here to evict a page
        return -1;
    auto frame_id = free_frames.back();
    free_frames.pop_back();

    auto pageid = storage_backend.create_new_page();
    page_to_frame_map[pageid] = frame_id;
    return pageid;
}

bool BufferPool::set_dirty(page_id_type pageid)
{
    auto iter = page_to_frame_map.find(pageid);
    if (iter == page_to_frame_map.end())
    {
        return false;
    }

    dirty_frames[iter->second] = true;
    return true;
}

void BufferPool::flush_all()
{
    for (const auto &page_frame : page_to_frame_map)
    {
        if (dirty_frames[page_frame.second])
        {
            flush_page(page_frame.first);
        }
    }
}