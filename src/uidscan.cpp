/**
 * @file uidscan.cpp
 * @brief UID scanning utility that probes devices using partial pattern matching.
 *
 * Sends reversed UID patterns with a prefix and reads back responses. The search
 * goes deeper in case of collisions, trying to find all unique UIDs on the line.
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

std::string reverse(const std::string &line, const std::string &pfx) {
  std::string s(line.rbegin(), line.rend());
  return pfx + s;
}

void send_pattern(const std::string &line, const std::string &pfx) {
  std::cout << reverse(line, pfx) << std::endl;
  std::cout.flush();
}

void send(const std::string &line) {
  std::cout << line << std::endl;
  std::cout.flush();
}

std::string read_line(std::string &pfx, int timeout_ms = 5) {
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, timeout_ms);
  if (ret > 0 && (pfd.revents & POLLIN)) {
    std::string line;
    if (std::getline(std::cin, line)) {
      if (!line.empty()) {
        if (line.compare(0, pfx.size(), pfx) == 0) {
          std::string uid = line.substr(pfx.size());
          std::reverse(uid.begin(), uid.end());
          return uid;
        }
      } else {
        return "!";
      }
    }
  }
  return "";
}

void mute_uid(const std::string &uid, const std::string &pfx) {
  send("SETADDR:" + reverse(uid, pfx));
}

constexpr int MAXLEN = 17;
const std::string charset =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "-_";

int main() {
  std::string pfx = "CB";
  std::set<std::string> found_uids;
  std::set<std::string> tried;
  std::queue<std::string> q;

  q.push("");

  while (!q.empty()) {
    std::string s = q.front();
    q.pop();

    for (char c : charset) {
      if (s.size() >= MAXLEN)
        continue;

      std::string next = s + c;
      if (tried.count(next))
        continue;
      tried.insert(next);

      send_pattern(next, pfx);

      std::vector<std::string> response;

      while (true) {
        std::string line = read_line(pfx);
        if (line.empty())
          break;
        response.push_back(line);
      }

      if (response.empty()) {
        continue;
      }
      if (response[0] == "!") {
        std::cerr << "COLLISION: " << reverse(next, pfx) << std::endl;
        q.push(next);  // collision. go deeper
        continue;
      }

      if (response.size() == 1) {
        const std::string &uid_candidate = response[0];

        // repeat attempt
        send_pattern(next, pfx);
        std::vector<std::string> confirm;
        while (true) {
          std::string line = read_line(pfx);
          if (line.empty())
            break;
          confirm.push_back(line);
        }

        if (confirm.size() == 1 && confirm[0] == uid_candidate) {
          if (!found_uids.count(uid_candidate)) {
            found_uids.insert(uid_candidate);
            std::cerr << "[+] UID confirmed: " << reverse(uid_candidate, pfx)
                      << std::endl;
            mute_uid(uid_candidate, pfx);
          }
          // go back to collision s: it's possible that there are more
          q.push(s);
        } else {
          // it's collision, go deeper
          q.push(next);
        }

      } else {
        // collision, go deeper
        q.push(next);
      }
    }
  }

  std::cerr << "\n== search complete ==\n";
  std::cerr << "total uids found: " << found_uids.size() << std::endl;
  return 0;
}

