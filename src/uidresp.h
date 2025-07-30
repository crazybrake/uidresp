#pragma once

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

/**
 * @class UidResponder
 * @brief Provides UID matching and random collision generation.
 *
 * Matches an input prefix/suffix against known UIDs and simulates
 * garbage collisions by combining parts of matching UIDs randomly.
 */
class UidResponder {
public:
  /**
   * @brief Checks if input partially matches a given UID.
   *
   * The input is considered a match if:
   * - the first two characters match the beginning of the UID
   * - the remaining characters match the end of the UID
   *
   * @param input Partial UID pattern from scan command.
   * @param uid   Full UID to compare against.
   * @return true if input matches the UID; false otherwise.
   */
  static bool matches(const std::string &input, const std::string &uid) {
    if (input.empty())
      return false;
    const size_t len = input.size();
    if (len > uid.size())
      return false;

    size_t left = std::min<size_t>(MATCH_LEFT, len);
    size_t right = len > MATCH_LEFT ? len - MATCH_LEFT : 0;

    for (size_t i = 0; i < left; ++i)
      if (input[i] != uid[i])
        return false;

    for (size_t i = 0; i < right; ++i)
      if (input[MATCH_LEFT + i] != uid[uid.size() - right + i])
        return false;

    return true;
  }

  /**
   * @brief Generates a pseudo-random collision string.
   *
   * For each character position up to maxLen, randomly selects a
   * character from all matching UIDs that have a character at that
   * position.
   *
   * Used to simulate bus collisions in response to ambiguous input.
   *
   * @param uids   List of matching UIDs.
   * @param maxLen Maximum length of the generated string.
   * @return Randomized string formed from characters in matched UIDs.
   */
  static std::string generateCollision(const std::vector<std::string> &uids,
                                       size_t maxLen = 19) {
    // emulate empty string only collision. will fix later
    return "";

    std::string result;
    std::mt19937 rng(std::random_device{}());

    for (size_t i = 0; i < maxLen; ++i) {
      std::vector<char> candidates;
      for (const auto &uid : uids) {
        if (i < uid.size())
          candidates.push_back(uid[i]);
      }

      if (candidates.empty())
        break;

      std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
      result.push_back(candidates[dist(rng)]);
    }

    return result;
  }

private:
  static constexpr size_t MATCH_LEFT = 2;
};
