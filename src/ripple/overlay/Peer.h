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

#ifndef RIPPLE_OVERLAY_PEER_H_INCLUDED
#define RIPPLE_OVERLAY_PEER_H_INCLUDED

#include <ripple/overlay/Message.h>

#include <ripple/unity/json.h>
#include <ripple/types/base_uint.h>
#include <ripple/protocol/RippleAddress.h>

#include <beast/net/IPEndpoint.h>

namespace ripple {

namespace Resource {
class Charge;
}

/** Represents a peer connection in the overlay. */
class Peer
{
public:
    typedef std::shared_ptr <Peer> ptr;

    /** Uniquely identifies a particular connection of a peer. */
    typedef std::uint32_t ShortId;

    //
    // Network
    //

    virtual void send (Message::pointer const& m) = 0;
    virtual beast::IP::Endpoint getRemoteAddress() const = 0;

    /** Adjust this peer's load balance based on the type of load imposed. */
    virtual void charge (Resource::Charge const& fee) = 0;

    //
    // Identity
    //

    virtual ShortId getShortId () const = 0;
    virtual RippleAddress const& getNodePublic () const = 0;
    virtual Json::Value json () = 0;
    // VFALCO TODO Replace both with
    //             boost::optional<std::string> const& cluster_id();
    //
    virtual bool isInCluster () const = 0;
    virtual std::string const& getClusterNodeName() const = 0;

    //
    // Ledger
    //

    virtual uint256 const& getClosedLedgerHash () const = 0;
    virtual bool hasLedger (uint256 const& hash, std::uint32_t seq) const = 0;
    virtual void ledgerRange (std::uint32_t& minSeq, std::uint32_t& maxSeq) const = 0;
    virtual bool hasTxSet (uint256 const& hash) const = 0;
    virtual void cycleStatus () = 0;
    virtual bool supportsVersion (int version) = 0;
    virtual bool hasRange (std::uint32_t uMin, std::uint32_t uMax) = 0;
};

}

#endif
