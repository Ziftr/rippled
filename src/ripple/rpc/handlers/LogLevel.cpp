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


namespace ripple {

Json::Value doLogLevel (RPC::Context& context)
{
    // log_level
    if (!context.params.isMember ("severity"))
    {
        // get log severities
        Json::Value ret (Json::objectValue);
        Json::Value lev (Json::objectValue);

        lev["base"] =
                Logs::toString(Logs::fromSeverity(deprecatedLogs().severity()));
        std::vector< std::pair<std::string, std::string> > logTable (
            deprecatedLogs().partition_severities());
        typedef std::map<std::string, std::string>::value_type stringPair;
        BOOST_FOREACH (const stringPair & it, logTable)
            lev[it.first] = it.second;

        ret["levels"] = lev;
        return ret;
    }

    LogSeverity const sv (
        Logs::fromString (context.params["severity"].asString ()));

    if (sv == lsINVALID)
        return rpcError (rpcINVALID_PARAMS);

    auto severity = Logs::toSeverity(sv);
    // log_level severity
    if (!context.params.isMember ("partition"))
    {
        // set base log severity
        deprecatedLogs().severity(severity);
        return Json::objectValue;
    }

    // log_level partition severity base?
    if (context.params.isMember ("partition"))
    {
        // set partition severity
        std::string partition (context.params["partition"].asString ());

        if (boost::iequals (partition, "base"))
            deprecatedLogs().severity (severity);
        else
            deprecatedLogs().get(partition).severity(severity);

        return Json::objectValue;
    }

    return rpcError (rpcINVALID_PARAMS);
}

} // ripple
