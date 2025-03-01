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

#include <ripple/basics/CheckLibraryVersions.h>
#include <ripple/basics/Sustain.h>
#include <ripple/basics/ThreadName.h>
#include <ripple/core/ConfigSections.h>
#include <ripple/crypto/RandomNumbers.h>
#include <ripple/net/RPCCall.h>
#include <ripple/resource/Fees.h>
#include <ripple/protocol/BuildInfo.h>
#include <beast/unit_test.h>
#include <beast/streams/debug_ostream.h>

#if defined(BEAST_LINUX) || defined(BEAST_MAC) || defined(BEAST_BSD)
#include <sys/resource.h>
#endif

namespace po = boost::program_options;

namespace ripple {

void setupServer ()
{
#ifdef RLIMIT_NOFILE
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
    {
         if (rl.rlim_cur != rl.rlim_max)
         {
             rl.rlim_cur = rl.rlim_max;
             setrlimit(RLIMIT_NOFILE, &rl);
         }
    }
#endif

    getApp().setup ();
}

void startServer ()
{
    //
    // Execute start up rpc commands.
    //
    if (getConfig ().RPC_STARTUP.isArray ())
    {
        for (int i = 0; i != getConfig ().RPC_STARTUP.size (); ++i)
        {
            Json::Value const& jvCommand    = getConfig ().RPC_STARTUP[i];

            if (!getConfig ().QUIET)
                std::cerr << "Startup RPC: " << jvCommand << std::endl;

            RPCHandler  rhHandler (getApp().getOPs ());

            Resource::Charge loadType = Resource::feeReferenceRPC;
            Json::Value jvResult    = rhHandler.doCommand (jvCommand, Role::ADMIN, loadType);

            if (!getConfig ().QUIET)
                std::cerr << "Result: " << jvResult << std::endl;
        }
    }

    getApp().run ();                 // Blocks till we get a stop RPC.
}

void printHelp (const po::options_description& desc)
{
    std::cerr
        << SYSTEM_NAME "d [options] <command> <params>\n"
        << desc << std::endl
        << "Commands: \n"
           "     account_info <account>|<seed>|<pass_phrase>|<key> [<ledger>] [strict]\n"
           "     account_lines <account> <account>|\"\" [<ledger>]\n"
           "     account_offers <account>|<account_public_key> [<ledger>]\n"
           "     account_tx accountID [ledger_min [ledger_max [limit [offset]]]] [binary] [count] [descending]\n"
           "     book_offers <taker_pays> <taker_gets> [<taker [<ledger> [<limit> [<proof> [<marker>]]]]]\n"
           "     connect <ip> [<port>]\n"
           "     consensus_info\n"
           "     get_counts\n"
           "     json <method> <json>\n"
           "     ledger [<id>|current|closed|validated] [full]\n"
           "     ledger_accept\n"
           "     ledger_closed\n"
           "     ledger_current\n"
           "     ledger_request <ledger>\n"
           "     ledger_header <ledger>\n"
           "     logrotate \n"
           "     peers\n"
           "     proof_create [<difficulty>] [<secret>]\n"
           "     proof_solve <token>\n"
           "     proof_verify <token> <solution> [<difficulty>] [<secret>]\n"
           "     random\n"
           "     ripple ...\n"
           "     ripple_path_find <json> [<ledger>]\n"
           "     server_info\n"
           "     stop\n"
           "     tx <id>\n"
           "     unl_add <domain>|<public> [<comment>]\n"
           "     unl_delete <domain>|<public_key>\n"
           "     unl_list\n"
           "     unl_load\n"
           "     unl_network\n"
           "     unl_reset\n"
           "     validation_create [<seed>|<pass_phrase>|<key>]\n"
           "     validation_seed [<seed>|<pass_phrase>|<key>]\n"
           "     wallet_accounts <seed>\n"
           "     wallet_add <regular_seed> <paying_account> <master_seed> [<initial_funds>] [<account_annotation>]\n"
           "     wallet_claim <master_seed> <regular_seed> [<source_tag>] [<account_annotation>]\n"
           "     wallet_propose [<passphrase>]\n"
           "     wallet_seed [<seed>|<passphrase>|<passkey>]\n";
}

//------------------------------------------------------------------------------

static
void
setupConfigForUnitTests (Config* config)
{
    config->nodeDatabase = parseDelimitedKeyValueString ("type=memory");
    config->ephemeralNodeDatabase = beast::StringPairArray ();
    config->importNodeDatabase = beast::StringPairArray ();
}

static
int
runUnitTests(std::string const& pattern, std::string const& argument)
{
    // Config needs to be set up before creating Application
    setupConfigForUnitTests (&getConfig ());
    // VFALCO TODO Remove dependence on constructing Application object
    std::unique_ptr <Application> app (make_Application (deprecatedLogs()));
    using namespace beast::unit_test;
    beast::debug_ostream stream;
    reporter r (stream);
    r.arg(argument);
    bool const failed (r.run_each_if (
        global_suites(), match_auto (pattern)));
    if (failed)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------

int run (int argc, char** argv)
{
    // Make sure that we have the right OpenSSL and Boost libraries.
    version::checkLibraryVersions();

    FatalErrorReporter reporter;

    using namespace std;

    setCallingThreadName ("main");
    int iResult = 0;
    po::variables_map vm;

    std::string importText;
    {
        importText += "Import an existing node database (specified in the [";
        importText += ConfigSection::importNodeDatabase ();
        importText += "] configuration file section) into the current ";
        importText += "node database (specified in the [";
        importText += ConfigSection::nodeDatabase ();
        importText += "] configuration file section).";
    }

    // VFALCO TODO Replace boost program options with something from Beast.
    //
    // Set up option parsing.
    //
    po::options_description desc ("General Options");
    desc.add_options ()
    ("help,h", "Display this message.")
    ("conf", po::value<std::string> (), "Specify the configuration file.")
    ("rpc", "Perform rpc command (default).")
    ("rpc_ip", po::value <std::string> (), "Specify the IP address for RPC command. Format: <ip-address>[':'<port-number>]")
    ("rpc_port", po::value <int> (), "Specify the port number for RPC command.")
    ("standalone,a", "Run with no peers.")
    ("unittest,u", po::value <std::string> ()->implicit_value (""), "Perform unit tests.")
    ("unittest-arg", po::value <std::string> ()->implicit_value (""), "Supplies argument to unit tests.")
    ("parameters", po::value< vector<string> > (), "Specify comma separated parameters.")
    ("quiet,q", "Reduce diagnotics.")
    ("quorum", po::value <int> (), "Set the validation quorum.")
    ("verbose,v", "Verbose logging.")
    ("load", "Load the current ledger from the local DB.")
    ("replay","Replay a ledger close.")
    ("ledger", po::value<std::string> (), "Load the specified ledger and start from .")
    ("ledgerfile", po::value<std::string> (), "Load the specified ledger file.")
    ("start", "Start from a fresh Ledger.")
    ("net", "Get the initial ledger from the network.")
    ("fg", "Run in the foreground.")
    ("import", importText.c_str ())
    ("version", "Display the build version.")
    ;

    // Interpret positional arguments as --parameters.
    po::positional_options_description p;
    p.add ("parameters", -1);

    if (! RandomNumbers::getInstance ().initialize ())
    {
        std::cerr << "Unable to add system entropy" << std::endl;
        iResult = 2;
    }

    if (!iResult)
    {
        // Parse options, if no error.
        try
        {
            po::store (po::command_line_parser (argc, argv)
                .options (desc)               // Parse options.
                .positional (p)               // Remainder as --parameters.
                .run (),
                vm);
            po::notify (vm);                  // Invoke option notify functions.
        }
        catch (...)
        {
            iResult = 1;
        }
    }

    if (!iResult && vm.count ("help"))
    {
        iResult = 1;
    }

    if (vm.count ("version"))
    {
        std::cout << "rippled version " <<
            BuildInfo::getVersionString () << std::endl;
        return 0;
    }

    // Use a watchdog process unless we're invoking a stand alone type of mode
    //
    if (HaveSustain ()
        && !iResult
        && !vm.count ("parameters")
        && !vm.count ("fg")
        && !vm.count ("standalone")
        && !vm.count ("unittest"))
    {
        std::string logMe = DoSustain (getConfig ().getDebugLogFile ().string());

        if (!logMe.empty ())
            std::cerr << logMe;
    }

    if (vm.count ("quiet"))
    {
        deprecatedLogs().severity(beast::Journal::kFatal);
    }
    else if (vm.count ("verbose"))
    {
        deprecatedLogs().severity(beast::Journal::kTrace);
    }
    else
    {
        deprecatedLogs().severity(beast::Journal::kInfo);
    }

    // Run the unit tests if requested.
    // The unit tests will exit the application with an appropriate return code.
    //
    if (vm.count ("unittest"))
    {
        std::string argument;

        if (vm.count("unittest-arg"))
            argument = vm["unittest-arg"].as<std::string>();

        return runUnitTests(vm["unittest"].as<std::string>(), argument);
    }

    if (!iResult)
    {
        auto configFile = vm.count ("conf") ?
                vm["conf"].as<std::string> () : std::string();

        // config file, quiet flag.
        getConfig ().setup (configFile, bool (vm.count ("quiet")));

        if (vm.count ("standalone"))
        {
            getConfig ().RUN_STANDALONE = true;
            getConfig ().LEDGER_HISTORY = 0;
        }
    }

    if (vm.count ("start")) getConfig ().START_UP = Config::FRESH;

    // Handle a one-time import option
    //
    if (vm.count ("import"))
    {
        getConfig ().doImport = true;
    }

    if (vm.count ("ledger"))
    {
        getConfig ().START_LEDGER = vm["ledger"].as<std::string> ();
        if (vm.count("replay"))
            getConfig ().START_UP = Config::REPLAY;
        else
            getConfig ().START_UP = Config::LOAD;
    }
    else if (vm.count ("ledgerfile"))
    {
        getConfig ().START_LEDGER = vm["ledgerfile"].as<std::string> ();
        getConfig ().START_UP = Config::LOAD_FILE;
    }
    else if (vm.count ("load"))
    {
        getConfig ().START_UP = Config::LOAD;
    }
    else if (vm.count ("net"))
    {
        getConfig ().START_UP = Config::NETWORK;

        if (getConfig ().VALIDATION_QUORUM < 2)
            getConfig ().VALIDATION_QUORUM = 2;
    }

    if (iResult == 0)
    {
        // These overrides must happen after the config file is loaded.

        // Override the RPC destination IP address
        //
        if (vm.count ("rpc_ip"))
        {
            // VFALCO TODO This is currently broken
            //getConfig ().setRpcIpAndOptionalPort (vm ["rpc_ip"].as <std::string> ());
            //getConfig().overwrite("rpc", "ip", vm["rpc_ip"].as<std::string>());
        }

        // Override the RPC destination port number
        //
        if (vm.count ("rpc_port"))
        {
            // VFALCO TODO This should be a short.
            // VFALCO TODO This is currently broken
            //getConfig ().setRpcPort (vm ["rpc_port"].as <int> ());
            //getConfig().overwrite("rpc", "port", vm["rpc_port"].as<std::string>());
        }

        if (vm.count ("quorum"))
        {
            getConfig ().VALIDATION_QUORUM = vm["quorum"].as <int> ();

            if (getConfig ().VALIDATION_QUORUM < 0)
                iResult = 1;
        }
    }

    if (iResult == 0)
    {
        if (!vm.count ("parameters"))
        {
            // No arguments. Run server.
            std::unique_ptr <Application> app (make_Application (deprecatedLogs()));
            setupServer ();
            startServer ();
        }
        else
        {
            // Have a RPC command.
            setCallingThreadName ("rpc");
            std::vector<std::string> vCmd   = vm["parameters"].as<std::vector<std::string> > ();

            iResult = RPCCall::fromCommandLine (vCmd);
        }
    }

    if (1 == iResult && !vm.count ("quiet"))
        printHelp (desc);

    return iResult;
}

}
