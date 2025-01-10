#ifndef IRRADIX_HPP
#define IRRADIX_HPP

#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <stdexcept>
#include <iomanip>

namespace irradix {

    const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
    const std::string DELIMITER = "1010101";

    std::string irradixEncode(uint64_t num) {
        if (num == 0) return "0";

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

    uint64_t irradixDecode(const std::string &rep) {
        double value = 0.0;
        for (char ch : rep) {
            value *= PHI;
            value += static_cast<int>(ch - '0');
        }
        return static_cast<uint64_t>(std::round(value));
    }

    std::vector<uint8_t> l1encode(const std::vector<uint64_t>& nums) {
        std::ostringstream lengthBits, dataBits;

        // Encode lengths and data
        for (uint64_t num : nums) {
            auto binaryRep = std::bitset<64>(num).to_string();
            binaryRep.erase(0, binaryRep.find_first_not_of('0'));
            lengthBits << std::bitset<8>(binaryRep.size()).to_string();
            dataBits << binaryRep;
        }

        // Combine length and data bits with delimiter
        std::string combinedBits = lengthBits.str() + DELIMITER + dataBits.str();

        // Convert to bytes
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < combinedBits.size(); i += 8) {
            std::string byteStr = combinedBits.substr(i, 8);
            while (byteStr.size() < 8) byteStr += "0"; // Pad to byte boundary
            bytes.push_back(static_cast<uint8_t>(std::bitset<8>(byteStr).to_ulong()));
        }
        return bytes;
    }

    std::vector<uint64_t> l1decode(const std::vector<uint8_t>& bytes) {
        // Reconstruct binary string
        std::ostringstream binaryStream;
        for (uint8_t byte : bytes) {
            binaryStream << std::bitset<8>(byte);
        }

        std::string binary = binaryStream.str();
        auto delimPos = binary.find(DELIMITER);
        if (delimPos == std::string::npos) {
            throw std::runtime_error("Invalid encoded data: missing delimiter");
        }

        // Split into lengths and data
        std::string lengthBits = binary.substr(0, delimPos);
        std::string dataBits = binary.substr(delimPos + DELIMITER.size());

        // Decode lengths
        std::vector<size_t> lengths;
        for (size_t i = 0; i < lengthBits.size(); i += 8) {
            lengths.push_back(std::bitset<8>(lengthBits.substr(i, 8)).to_ulong());
        }

        // Decode data using lengths
        std::vector<uint64_t> decoded;
        size_t pos = 0;
        for (size_t len : lengths) {
            if (pos + len > dataBits.size()) {
                throw std::runtime_error("Data bits do not match lengths");
            }
            std::string numBits = dataBits.substr(pos, len);
            decoded.push_back(std::bitset<64>(numBits).to_ulong());
            pos += len;
        }

        return decoded;
    }

} // namespace irradix

#endif // IRRADIX_HPP

