//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef RIPPLE_RADMAP_BASICFULLBELOWCACHE_H_INCLUDED
#define RIPPLE_RADMAP_BASICFULLBELOWCACHE_H_INCLUDED

#include <ripple/basics/KeyCache.h>
#include <ripple/app/main/Tuning.h>
#include <beast/insight/Collector.h>

namespace ripple {

/** Remembers which tree keys have all descendants resident.
    This optimizes the process of acquiring a complete tree.
*/
template <class Key>
class BasicFullBelowCache
{
private:
    typedef KeyCache <Key> CacheType;

public:
    typedef Key key_type;
    typedef typename CacheType::size_type size_type;
    typedef typename CacheType::clock_type clock_type;

    /** Construct the cache.

        @param name A label for diagnostics and stats reporting.
        @param collector The collector to use for reporting stats.
        @param targetSize The cache target size.
        @param targetExpirationSeconds The expiration time for items.
    */
    BasicFullBelowCache (std::string const& name, clock_type& clock,
        beast::insight::Collector::ptr const& collector =
            beast::insight::NullCollector::New (),
        std::size_t target_size = defaultCacheTargetSize,
        std::size_t expiration_seconds = defaultCacheExpirationSeconds)
        : m_cache (name, clock, collector, target_size,
            expiration_seconds)
    {
    }

    /** Return the clock associated with the cache. */
    clock_type& clock()
    {
        return m_cache.clock ();
    }

    /** Return the number of elements in the cache.
        Thread safety:
            Safe to call from any thread.
    */
    size_type size () const
    {
        return m_cache.size ();
    }

    /** Remove expired cache items.
        Thread safety:
            Safe to call from any thread.
    */
    void sweep ()
    {
        m_cache.sweep ();
    }

    /** Refresh the last access time of an item, if it exists.
        Thread safety:
            Safe to call from any thread.
        @param key The key to refresh.
        @return `true` If the key exists.
    */
    bool touch_if_exists (key_type const& key)
    {
        return m_cache.touch_if_exists (key);
    }

    /** Insert a key into the cache.
        If the key already exists, the last access time will still
        be refreshed.
        Thread safety:
            Safe to call from any thread.
        @param key The key to insert.
    */
    void insert (key_type const& key)
    {
        m_cache.insert (key);
    }

private:
    KeyCache <Key> m_cache;
};

}

#endif
