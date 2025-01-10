#include "irradix.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>

void printUsage(const std::string& programName) {
    std::cout << "Usage:\n"
              << "  " << programName << " encode <number>\n"
              << "  " << programName << " decode <number>\n"
              << "  " << programName << " l1encode <num1,num2,...> [-vv]\n"
              << "  " << programName << " l1decode <filename>\n"
              << "  " << programName << " l1encode (reads stdin if no input)\n"
              << "  " << programName << " l1decode (reads stdin if no file)\n";
}

void calculateStats(const std::vector<uint64_t>& nums, size_t encodedSize) {
    uint64_t maxNum = *std::max_element(nums.begin(), nums.end());
    size_t baselineSize = nums.size();

    if (maxNum <= 255) {
        baselineSize *= sizeof(uint8_t);
    } else if (maxNum <= 65535) {
        baselineSize *= sizeof(uint16_t);
    } else if (maxNum <= 4294967295) {
        baselineSize *= sizeof(uint32_t);
    } else {
        baselineSize *= sizeof(uint64_t);
    }

    size_t theoreticalSize = 0;
    for (uint64_t num : nums) {
        size_t bitLength = static_cast<size_t>(std::log2(num) + 1);
        theoreticalSize += bitLength + static_cast<size_t>(std::log2(bitLength) + 1);
    }
    theoreticalSize = (theoreticalSize + 7) / 8;

    double compactionPercent = 100.0 * encodedSize / baselineSize;
    double expansionPercent = 100.0 * encodedSize / theoreticalSize;

    std::cerr << "Baseline encoding size (bytes): " << baselineSize << "\n";
    std::cerr << "Theoretical minimum size (bytes): " << theoreticalSize << "\n";
    std::cerr << "Irradix encoding size (bytes): " << encodedSize << "\n";
    std::cerr << "Versus baseline encoding: " << compactionPercent << "%\n";
    std::cerr << "Versus theoretical limit: " << expansionPercent << "%\n";
}

std::string readFromStdin() {
    std::ostringstream oss;
    std::string line;
    while (std::getline(std::cin, line)) {
        oss << line;
    }
    return oss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    std::string input = (argc > 2) ? argv[2] : "";
    bool verbose = (argc > 3 && std::string(argv[3]) == "-vv");

    try {
        if (command == "encode") {
            uint64_t num = std::stoull(input);
            std::cout << "Irradix Encoding: " << irradix::irradix(num) << "\n";

        } else if (command == "decode") {
            std::string rep = input;
            std::cout << "Irradix Decoding: " << irradix::derradix(input) << "\n";

        } else if (command == "l1encode") {
            std::vector<uint64_t> nums;
            if (input.empty()) {
                std::string stdinInput = readFromStdin();
                std::istringstream iss(stdinInput);
                std::string token;
                while (std::getline(iss, token, ',')) {
                    nums.push_back(std::stoull(token));
                }
            } else {
                std::istringstream iss(input);
                std::string token;
                while (std::getline(iss, token, ',')) {
                    nums.push_back(std::stoull(token));
                }
            }

            auto bytes = irradix::encode(nums);
            std::cout.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());

            if (verbose) {
                calculateStats(nums, bytes.size());
            }

        } else if (command == "l1decode") {
            std::vector<uint8_t> bytes;

            if (input.empty()) {
                std::string stdinInput = readFromStdin();
                bytes = std::vector<uint8_t>(stdinInput.begin(), stdinInput.end());
            } else {
                std::ifstream file(input, std::ios::binary);
                if (!file) {
                    throw std::runtime_error("Could not open file: " + input);
                }
                bytes = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                             std::istreambuf_iterator<char>());
            }

            auto nums = irradix::decode(bytes);
            for (size_t i = 0; i < nums.size(); ++i) {
                if (i > 0) std::cout << ",";
                std::cout << nums[i];
            }
            std::cout << "\n";

        } else {
            printUsage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

