#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <bitset>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <sstream>

namespace irradix {

// We'll use a simple 'double' for PHI now, to match your simpler approach.
// (If you need large numbers, consider 'long double' or a big-precision library.)
static long double PHI = (1.0L + std::sqrt(5.0L)) / 2.0L;

// Delimiters
static const std::string DELIMITER = "101";
static const std::string L1_DELIMITER = "1010101";

std::string irradix(uint64_t num) {
        std::vector<int> digits;
        double value = static_cast<double>(num);
        while (value > 1e-9) {
            double mod = std::fmod(value, PHI);
            int digit = static_cast<int>(std::floor(mod));
            digits.insert(digits.begin(), digit);
            value = (value - mod) / PHI;
        }

        std::ostringstream oss;
        for (int digit : digits) {
            oss << digit;
        }
        return oss.str();
    }

    uint64_t derradix(const std::string &rep) {
        double value = 0.0;
        for (char ch : rep) {
            value *= PHI;
            value = std::ceil(value + static_cast<int>(ch - '0'));
        }
        return static_cast<uint64_t>(std::ceil(value));
    }
// ---------------------------------------------------------------------
// encode(nums): mirror the Python approach
//   - For each num, transform to (num + 1)*2, then pass to irradix
//   - Append "0101" if it ends with "10"
//   - Join by "101"
//   - pad left with zeros to a multiple of 8 bits
//   - chunk into bytes
// ---------------------------------------------------------------------
inline std::vector<uint8_t> encode(const std::vector<uint64_t>& nums, bool bits=false) {
  std::string concatenated;
  // Build string of base-PHI reps joined by DELIMITER
  for (size_t i = 0; i < nums.size(); ++i) {
    uint64_t mappedNum = (nums[i] + 1) << 1;  // Python's (num + 1)*2
    std::string rep = irradix(mappedNum);

    // If rep ends with "10", append "0101"
    if (rep.size() >= 2 && rep.compare(rep.size() - 2, 2, "10") == 0) {
      rep += "0101";
    }

    if (i > 0) {
      concatenated += DELIMITER;
    }
    concatenated += rep;
  }

  // If bits == true, you might want to return raw ASCII bits,
  // but let's keep returning 8-bit-chunked data for consistency:
  // i.e., we always do the left-pad and chunking.
  size_t padLen = (8 - (concatenated.size() % 8)) % 8;
  // Left-pad with zeros:
  std::string padded(padLen, '0');
  padded += concatenated;

  std::vector<uint8_t> result;
  result.reserve((padded.size() + 7) / 8);
  for (size_t i = 0; i < padded.size(); i += 8) {
    // Use bitset to parse 8 bits
    std::bitset<8> bitsVal(padded.substr(i, 8));
    result.push_back(static_cast<uint8_t>(bitsVal.to_ulong()));
  }
  return result;
}

// ---------------------------------------------------------------------
// decode(chunks):
//   - Convert each byte to an 8-bit string
//   - Remove leading zeros
//   - Split on "101"
//   - For each piece, call derradix and do ((val // 2) - 1) to invert the encode
// ---------------------------------------------------------------------
inline std::vector<uint64_t> decode(const std::vector<uint8_t>& chunks, bool bits=false) {
  // Reconstruct the bit string
  std::string reconstructed;
  reconstructed.reserve(chunks.size() * 8);

  for (auto byte : chunks) {
    // Use bitset for standard tooling
    std::bitset<8> bitsVal(byte);
    reconstructed += bitsVal.to_string();
  }

  // For bits==false, remove leading zeros
  // (But watch out for the corner case of all zeros => find_first_not_of returns npos)
  size_t firstOne = reconstructed.find_first_not_of('0');
  if (firstOne == std::string::npos) {
    // Means everything is '0'
    // That would decode to an empty set if we wanted, or [0]. We'll just treat it as empty
    reconstructed.clear();
  } else {
    reconstructed.erase(0, firstOne);
  }

  // Split on DELIMITER
  std::vector<uint64_t> numbers;
  std::vector<std::string> parts;
  size_t start = 0;
  while (start < reconstructed.size()) {
    size_t pos = reconstructed.find(DELIMITER, start);
    std::string part;
    if (pos == std::string::npos) {
      part = reconstructed.substr(start);
      start = reconstructed.size();
    } else {
      part = reconstructed.substr(start, pos - start);
      start = pos + DELIMITER.size();
    }

    parts.push_back(part);
  }

  for( auto i = 0; i < parts.size(); i++ ) {
    auto part = parts[i];
    if ( i < parts.size() - 1 && parts[i+1].empty() ) {
      part = part.substr(0, part.size()-1);
      i++;
    }
    numbers.push_back((derradix(part)>>1)-1);
  }

  return numbers;
}

// ---------------------------------------------------------------------
// l1encode(nums):
//   - We'll do the "lengths" by calling irradix() on each (similar to your approach)
//   - Then we encode those lengths in a bit-based manner
//   - Then append L1_DELIMITER + the raw bits of each num
//   - Finally chunk into 8-bit pieces
// ---------------------------------------------------------------------
inline std::vector<uint8_t> l1encode(const std::vector<uint64_t>& nums) {
  // Step 1: gather lengths and build the big "numsBits"
  std::string numsBits;
  numsBits.reserve(nums.size() * 10); // just an estimate

  std::vector<uint64_t> lengths;
  lengths.reserve(nums.size());

  // We'll do the same logic as encode: (num+1)*2 => irradix => store size
  // But your alternative does it differently, so let's keep it simpler:
  // We'll do a direct base-PHI rep, measure size of that rep, store it in 'lengths', then append rep to numsBits
  for (auto n : nums) {
    uint64_t mappedNum = (n + 1ULL) * 2ULL;
    std::string rep = irradix(mappedNum);
    lengths.push_back(rep.size());
    numsBits += rep;
  }

  // Step 2: encode lengths with bits=true
  std::vector<uint8_t> encodedLengthsBits = encode(lengths, true);

  // Convert that chunk data back into a bitstring
  // (like your approach does with std::ostringstream etc.)
  std::string lengthsBits;
  lengthsBits.reserve(encodedLengthsBits.size() * 8);
  for (auto b : encodedLengthsBits) {
    std::bitset<8> bs(b);
    lengthsBits += bs.to_string();
  }

  // Step 3: Append L1_DELIMITER + numsBits
  std::string fullBitSequence = lengthsBits + L1_DELIMITER + numsBits;

  // Step 4: Pad to 8 bits
  size_t pad = (8 - (fullBitSequence.size() % 8)) % 8;
  fullBitSequence.append(pad, '0');

  // Step 5: Convert to bytes
  std::vector<uint8_t> finalBytes;
  finalBytes.reserve((fullBitSequence.size() + 7) / 8);
  for (size_t i = 0; i < fullBitSequence.size(); i += 8) {
    std::bitset<8> chunkBits(fullBitSequence.substr(i, 8));
    finalBytes.push_back(static_cast<uint8_t>(chunkBits.to_ulong()));
  }
  return finalBytes;
}

// ---------------------------------------------------------------------
// l1decode(chunks):
//   - convert chunks to a bitstring
//   - split at L1_DELIMITER => lengthsBits + numsBits
//   - decode lengths by passing lengthsBits to decode(..., true)
//   - parse that many bits for each num
//   - call derradix => (val / 2) - 1
// ---------------------------------------------------------------------
inline std::vector<uint64_t> l1decode(const std::vector<uint8_t>& chunks) {
  // Step 1: convert each byte to an 8-bit string
  std::string fullSequence;
  fullSequence.reserve(chunks.size() * 8);
  for (auto b : chunks) {
    std::bitset<8> bs(b);
    fullSequence += bs.to_string();
  }

  // Step 2: find L1_DELIMITER
  size_t pos = fullSequence.find(L1_DELIMITER);
  if (pos == std::string::npos) {
    throw std::runtime_error("Missing L1_DELIMITER in l1decode!");
  }
  std::string lengthsBits = fullSequence.substr(0, pos);
  std::string numsBits = fullSequence.substr(pos + L1_DELIMITER.size());

  // Step 3: parse lengthsBits as “bits = true”
  // Convert lengthsBits into a vector<uint8_t>
  std::vector<uint8_t> lengthChunks;
  lengthChunks.reserve((lengthsBits.size() + 7) / 8);
  for (size_t i = 0; i < lengthsBits.size(); i += 8) {
    std::bitset<8> chunkBits(lengthsBits.substr(i, 8));
    lengthChunks.push_back(static_cast<uint8_t>(chunkBits.to_ulong()));
  }
  std::vector<uint64_t> lengths = decode(lengthChunks, true); // returns a vector<uint64_t>

  // Step 4: reconstruct the numbers from numsBits
  std::vector<uint64_t> results;
  results.reserve(lengths.size());

  size_t offset = 0;
  for (auto len : lengths) {
    // If we run out of bits, error out
    if (offset + len > numsBits.size()) {
      throw std::runtime_error("Ran out of bits in l1decode!");
    }
    std::string part = numsBits.substr(offset, len);
    offset += len;

    // Convert from base-PHI => val
    uint64_t val = derradix(part);
    // Undo the ((n+1)*2)
    val = (val / 2ULL) - 1ULL;
    results.push_back(val);
  }
  return results;
}

} // namespace irradix

