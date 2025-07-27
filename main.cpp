#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>

#include "uidresp.h"

/**
 * @brief Simple UID responder tool.
 *
 * Reads UID patterns from stdin and prints:
 * - exact match if exactly one UID matches,
 * - a generated collision string if multiple match,
 * - nothing if no match.
 *
 * Matching is done using the rule:
 * - first two bytes of input must match UID start,
 * - remaining bytes must match UID end.
 *
 * Example:
 * ./uidtool 12341234 12349875976 12340870987076
 * → then type patterns interactively
 *
 * @param argc number of arguments
 * @param argv list of full UIDs to scan against
 * @return 0 on success, 1 on invalid usage
 */

int main(int argc, char **argv)
{
    UidResponder responder;

    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <uid1> <uid2> ...\n";
        return 1;
    }

    std::vector<std::string> uids(argv + 1, argv + argc);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty())
            continue;
        std::vector<std::string> matched;
        for (const auto &uid : uids) {
            if (responder.matches(line, uid))
                matched.push_back(uid);
        }

        if (matched.empty())
            continue;

        if (matched.size() == 1) {
            std::cout << matched[0] << std::endl;
        } else {
            std::string noise = responder.generateCollision(matched);
            std::cout << noise << std::endl;
        }
    }

    return 0;
}

