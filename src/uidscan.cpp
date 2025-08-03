/**
 * @file uidscan.cpp
 * @brief UID scanning utility that probes devices using partial pattern
 * matching.
 *
 * Sends reversed UID patterns with a prefix and reads back responses. The
 * search goes deeper in case of collisions, trying to find all unique UIDs
 * on the line.
 *
 * Usage:
 * - Sends reversed string (with prefix) to stdout.
 * - Reads lines from stdin with optional prefix removal.
 * - Detects UID collisions and recursively refines pattern.
 * - Confirms UID by repeating pattern.
 * - Mutes confirmed UIDs using SETADDR command.
 *
 * Expected to be used with a compatible responder (like `uidresp`).
 */

#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <vector>

std::string reverse_string(const std::string &line)
{
    return std::string(line.rbegin(), line.rend());
}

// return "" if timeout, "!" if collision
std::string read_line(const std::string &pfx, int timeout_ms = 5)
{
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, timeout_ms);
    if (ret > 0 && (pfd.revents & POLLIN)) {
        std::string line;
        if (std::getline(std::cin, line)) {
            if (!line.empty() && line.size() == 19) {
                return std::string(line.begin() + 2,
                    line.end()); // return
                                 // string w/o
                                 // prefix
            } else {
                return "!"; // collision
            }
        }
    }
    return ""; // timeout or error
}

void send(const std::string &line)
{
    std::cout << line << std::endl;
    std::cout.flush();
}

std::string send_and_recv(const std::string &line, const std::string &pfx)
{
    send(pfx + reverse_string(line));

    std::string s = read_line(pfx);
    if (s != "!") {
        return reverse_string(s);
    }
    return s;
}

void mute(const std::string &uid, const std::string &pfx)
{
    send("SETADDR:" + pfx + reverse_string(uid));
}

constexpr int MAXLEN = 17;
const std::string charset = "0123456789"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "-_";
//
//
int main(int argc, char **argv)
{
    send("RESETALL");
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<prefix> [prefix ..."
                  << std::endl;
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        std::string pfx = argv[i];
        std::set<std::string> found_uids;
        std::vector<std::string> stack;
        std::vector<std::string> collision_stack;

        stack.push_back("");

        while (!stack.empty()) {
            std::string s = stack.back();
            stack.pop_back();

            for (char c : charset) {
                if (s.size() >= MAXLEN)
                    continue;

                std::string next = s + c;

                std::string resp = send_and_recv(next, pfx);

                if (resp.empty()) {
                    continue; // timeout
                }

                if (resp == "!") {
                    // collision
                    std::cerr
                        << "COLLISION: " << pfx + reverse_string(next)
                        << std::endl;
                    stack.push_back(next);
                    collision_stack.push_back(next);
                    continue;
                }

                // check
                std::string confirm = send_and_recv(resp, pfx);

                if (confirm.empty()) {
                    // not confirmed -> collision
                    std::cerr
                        << "[implied collision] no confirmation for: "
                        << pfx + reverse_string(resp) << std::endl;
                    stack.push_back(next);
                    collision_stack.push_back(next);
                    continue;
                }

                if (confirm == resp) {
                    // подтверждённый UID
                    if (!found_uids.count(resp)) {
                        found_uids.insert(resp);
                        std::cerr
                            << "FOUND: " << pfx + reverse_string(resp)
                            << std::endl;
                        mute(resp, pfx);
                    }

                    stack.push_back(next);

                    if (!collision_stack.empty()) {
                        stack.push_back(collision_stack.back());
                        collision_stack.pop_back();
                    }
                } else {
                    // garbage
                    std::cerr << "[warn] confirmation mismatch: "
                              << pfx + reverse_string(resp) << " vs "
                              << pfx + reverse_string(confirm)
                              << std::endl;
                    stack.push_back(next);
                }
            }
        }

        send("RESETALL");
        std::cerr << "\n== search complete ==\n";
        std::cerr << "total uids found: " << found_uids.size() << "\n"
                  << std::endl;
    }
    return 0;
}
