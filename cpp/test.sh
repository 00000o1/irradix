#!/usr/bin/env bash

# Check if irradix_tool exists
if ! command -v ./irradix_tool &> /dev/null; then
    echo "irradix_tool not found. Make sure it is in the current directory."
    exit 1
fi

# Validate input
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_random_numbers>"
    exit 1
fi

NUM_COUNT=$1
MAX_BITS=64
TEMP_INPUT="input_nums.txt"
TEMP_ENCODED="encoded.bin"
TEMP_DECODED="decoded_nums.txt"

# Generate $NUM_COUNT random numbers with uniform bit-length distribution
generate_numbers() {
    echo "Generating $NUM_COUNT random numbers with uniform bit-length distribution..."
    > "$TEMP_INPUT"
    for ((i = 0; i < NUM_COUNT; i++)); do
        BITS=$((1 + RANDOM % MAX_BITS)) # Random bit length (1 to 64)
        NUM=$((RANDOM % (1 << BITS)))  # Random number with exactly $BITS bits
        echo -n "$NUM," >> "$TEMP_INPUT"
    done
    sed -i '' 's/,$/\n/' "$TEMP_INPUT" # Remove trailing comma (macOS-compatible `sed`)
}

# Run encode-decode test
test_encode_decode() {
    echo "Encoding numbers with l1encode..."
    ./irradix_tool l1encode "$(cat $TEMP_INPUT)" > "$TEMP_ENCODED"

    echo "Decoding numbers with l1decode..."
    ./irradix_tool l1decode "$TEMP_ENCODED" > "$TEMP_DECODED"

    echo "Verifying integrity..."
    if diff "$TEMP_INPUT" "$TEMP_DECODED" &> /dev/null; then
        echo "Test PASSED: Decoded numbers match the original input."
    else
        echo "Test FAILED: Decoded numbers do not match the original input."
    fi
}

# Clean up temporary files
cleanup() {
    rm -f "$TEMP_INPUT" "$TEMP_ENCODED" "$TEMP_DECODED"
}

# Main execution
generate_numbers
test_encode_decode
cleanup

