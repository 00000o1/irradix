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
MAX_BITS=12
TEMP_INPUT="input_nums.txt"
TEMP_ENCODED="encoded.bin"
TEMP_DECODED="decoded_nums.txt"

# Generate $NUM_COUNT random numbers with uniform bit-length distribution
generate_numbers() {
    echo "Generating $NUM_COUNT random numbers with uniform bit-length distribution..."
    python3 - <<EOF > "$TEMP_INPUT"
import random

def random_number(bits):
    return random.randint(0, (1 << bits) - 1)

with open("$TEMP_INPUT", "w") as f:
    f.write(",".join(str(random_number(random.randint(1, $MAX_BITS))) for _ in range($NUM_COUNT)) + "\\n")
EOF
}

# Run encode-decode test
test_encode_decode() {
    echo "Encoding numbers with l1encode..."
    cat "$TEMP_INPUT" | ./irradix_tool l1encode > "$TEMP_ENCODED"

    echo "Decoding numbers with l1decode..."
    ./irradix_tool l1decode "$TEMP_ENCODED" > "$TEMP_DECODED"

    echo "Verifying integrity..."
    if diff "$TEMP_INPUT" "$TEMP_DECODED" &> /dev/null; then
        echo "Test PASSED: Decoded numbers match the original input."
        cleanup
    else
        echo "Test FAILED: Decoded numbers do not match the original input."
        exit 1
    fi
}

# Clean up temporary files
cleanup() {
    rm -f "$TEMP_INPUT" "$TEMP_ENCODED" "$TEMP_DECODED"
}

cleanup
# Main execution
generate_numbers
test_encode_decode

