#!/usr/bin/env python3
"""Hash chain generator - creates a chain of 100 hashes from input text"""
import hashlib
import sys
import os

def create_hash_chain(text, chain_size=100):
    """
    Creates a hash chain of specified size from input text.
    
    Args:
        text: Input text string to start the chain
        chain_size: Number of times to hash (default 100)
    
    Returns:
        list: All hashes in the chain (as hex strings)
    """
    current = text.encode('utf-8')
    chain = []
    
    # Generate all hashes in the chain
    for i in range(chain_size):
        hash_hex = hashlib.sha256(current).hexdigest()
        chain.append(hash_hex)
        current = bytes.fromhex(hash_hex)  # Convert hex string to bytes, not UTF-8 encode
    
    return chain

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 hashchain.py <text>")
        sys.exit(1)
    
    input_text = sys.argv[1]
    chain = create_hash_chain(input_text, chain_size=100)
    
    # Output to text file
    output_file = os.path.join(os.path.dirname(__file__), 'hashchain_output.txt')
    with open(output_file, 'w') as f:
        for hash_value in chain:
            f.write(hash_value + '\n')
    
    anchor = chain[0]
    root = chain[-1]
    
    print(f"Input text: {input_text}")
    print(f"Anchor (first hash): {anchor}")
    print(f"Root (final hash after 100 iterations): {root}")
    print(f"All 100 hashes written to: {output_file}")

