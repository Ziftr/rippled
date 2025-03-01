//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012-2014 Ripple Labs Inc.

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

#include <ripple/core/LoadFeeTrack.h>
#include <ripple/server/Role.h>

namespace ripple {

// ledger [id|index|current|closed] [full]
// {
//    ledger: 'current' | 'closed' | <uint256> | <number>,  // optional
//    full: true | false    // optional, defaults to false.
// }
Json::Value doLedger (RPC::Context& context)
{
    if (!context.params.isMember ("ledger")
        && !context.params.isMember ("ledger_hash")
        && !context.params.isMember ("ledger_index"))
    {
        Json::Value ret (Json::objectValue), current (Json::objectValue),
                closed (Json::objectValue);

        getApp().getLedgerMaster ().getCurrentLedger ()->addJson (current, 0);
        getApp().getLedgerMaster ().getClosedLedger ()->addJson (closed, 0);

        ret["open"] = current;
        ret["closed"] = closed;

        return ret;
    }

    Ledger::pointer     lpLedger;
    Json::Value jvResult = RPC::lookupLedger (
        context.params, lpLedger, context.netOps);

    if (!lpLedger)
        return jvResult;

    bool bFull = context.params.isMember ("full")
            && context.params["full"].asBool ();
    bool bTransactions = context.params.isMember ("transactions")
            && context.params["transactions"].asBool ();
    bool bAccounts = context.params.isMember ("accounts")
            && context.params["accounts"].asBool ();
    bool bExpand = context.params.isMember ("expand")
            && context.params["expand"].asBool ();
    int     iOptions        = (bFull ? LEDGER_JSON_FULL : 0)
                              | (bExpand ? LEDGER_JSON_EXPAND : 0)
                              | (bTransactions ? LEDGER_JSON_DUMP_TXRP : 0)
                              | (bAccounts ? LEDGER_JSON_DUMP_STATE : 0);

    if (bFull || bAccounts)
    {

        if (context.role != Role::ADMIN)
        {
            // Until some sane way to get full ledgers has been implemented,
            // disallow retrieving all state nodes.
            return rpcError (rpcNO_PERMISSION);
        }

        if (getApp().getFeeTrack().isLoadedLocal() &&
            context.role != Role::ADMIN)
        {
            WriteLog (lsDEBUG, Peer) << "Too busy to give full ledger";
            return rpcError(rpcTOO_BUSY);
        }
        context.loadType = Resource::feeHighBurdenRPC;
    }


    Json::Value ret (Json::objectValue);
    lpLedger->addJson (ret, iOptions);

    return ret;
}

} // ripple
