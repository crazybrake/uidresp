#include <algorithm>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

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
    std::set<std::string> muted;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty())
            continue;

        // SETADDR:<uid>
        if (line.rfind("SETADDR:", 0) == 0) {
            std::string uid = line.substr(8);
            auto it = std::find(uids.begin(), uids.end(), uid);
            if (it != uids.end()) {
                muted.insert(uid);
                // std::cerr << "[muted] " << uid << std::endl;
            } else {
                std::cerr << "[warn] tried to mute unknown uid: " << uid
                          << std::endl;
            }
            continue;
        }

        // RESETADDR:<uid>
        if (line.rfind("RESETADDR:", 0) == 0) {
            std::string uid = line.substr(10);
            auto it = muted.find(uid);
            if (it != muted.end()) {
                muted.erase(it);
                std::cerr << "[unmuted] " << uid << std::endl;
            } else {
                std::cerr
                    << "[warn] tried to unmute unknown or active uid: "
                    << uid << std::endl;
            }
            continue;
        }

        // RESETALL
        if (line == "RESETALL") {
            muted.clear();
            std::cerr << "[unmuted all]" << std::endl;
            continue;
        }

        // normal pattern matching
        std::vector<std::string> matched;
        for (const auto &uid : uids) {
            if (muted.count(uid))
                continue;
            if (responder.matches(line, uid))
                matched.push_back(uid);
        }

        if (matched.empty())
            continue;

        if (matched.size() == 1) {
            std::cout << matched[0] << std::endl;
        } else {
            std::string noise = responder.generateCollision(matched);
            if (matched[0].size() >= 2 && matched[0][0] == 'C' &&
                matched[0][1] == 'B') {
                // CB vendor returns empty line
                noise = "";
            } else {
                // all others return mix of symbols
                noise = responder.generateCollision(matched);
            }
            std::cout << noise << std::endl;
        }
        std::cout.flush();
    }

    return 0;
}
