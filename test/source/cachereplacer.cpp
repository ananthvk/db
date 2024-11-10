#include <doctest/doctest.h>
#include <pinedb/cachereplacer.h>

using namespace pinedb;

TEST_SUITE("lrucachereplacer")
{
    TEST_CASE("LRU Cache creation")
    {
        LRUCacheReplacer<int> replacer(5);
        CHECK(replacer.evict() == std::nullopt);
        replacer.access(0);
        replacer.access(1);
        replacer.access(2);
        replacer.access(3);
        replacer.access(4);
    }
    TEST_CASE("LRU Cache eviction single element")
    {
        LRUCacheReplacer<int> replacer(1);
        CHECK(replacer.evict() == std::nullopt);
        replacer.access(1);
        auto opt = replacer.evict();
        CHECK(opt.value() == 1);
        opt = replacer.evict();
        CHECK(opt.has_value() == false);
    }
    TEST_CASE("LRU Cache eviction multiple elements")
    {
        LRUCacheReplacer<int> replacer(3);
        replacer.access(0);
        replacer.access(1);
        replacer.access(2);
        CHECK_THROWS(replacer.access(3));
        auto opt = replacer.evict();
        CHECK(opt.value() == 0);
        opt = replacer.evict();
        CHECK(opt.value() == 1);
        opt = replacer.evict();
        CHECK(opt.value() == 2);
        opt = replacer.evict();
        CHECK(opt.has_value() == false);

        replacer.access(0);
        replacer.access(1);
        replacer.access(2);
        opt = replacer.evict();
        CHECK(opt.value() == 0);
        opt = replacer.evict();
        CHECK(opt.value() == 1);
        opt = replacer.evict();
        CHECK(opt.value() == 2);
        opt = replacer.evict();
        CHECK(opt.has_value() == false);

        replacer.access(0);
        replacer.access(1);
        replacer.access(2);
        replacer.access(0);
        replacer.access(1);
        opt = replacer.evict();
        CHECK(opt.value() == 2);

        opt = replacer.evict();
        CHECK(opt.value() == 0);

        opt = replacer.evict();
        CHECK(opt.value() == 1);

        opt = replacer.evict();
        opt = replacer.evict();
        CHECK(opt.has_value() == false);
    }

    TEST_CASE("LRU Cache eviction and set_evictable")
    {
        LRUCacheReplacer<int> replacer(3);
        replacer.access(0);
        replacer.access(1);
        replacer.access(2);
        replacer.set_evictable(0, false);
        replacer.set_evictable(1, false);
        replacer.set_evictable(2, false);

        auto opt = replacer.evict();
        CHECK(opt.has_value() == false);

        replacer.access(0);
        replacer.access(1);
        replacer.access(2);

        opt = replacer.evict();
        CHECK(opt.value() == 0);
        opt = replacer.evict();
        CHECK(opt.value() == 1);
        opt = replacer.evict();
        CHECK(opt.value() == 2);

        replacer.access(0);
        replacer.access(1);
        replacer.access(2);
        replacer.set_evictable(0, false);
        replacer.set_evictable(1, false);
        opt = replacer.evict();
        CHECK(opt.value() == 2);

        replacer.set_evictable(0, true);
        replacer.set_evictable(1, true);
        opt = replacer.evict();
        CHECK(opt.value() == 0);
        opt = replacer.evict();
        CHECK(opt.value() == 1);
    }
}