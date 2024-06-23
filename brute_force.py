import hashlib

# Simulated data: hashed passwords using SHA-256, with and without salts
hashed_passwords = {
    'sha256_no_salt': 'example_sha256_hash_no_salt',
    'sha256_with_salt': ('example_sha256_hash_with_salt', 'example_salt'),
}

# Dictionary of common passwords the attacker wants to try
common_passwords = ["password", "123456", "admin", "letmein", "qwerty"]

# Hash functions the attacker wants to try
hash_functions = {
    'sha256': hashlib.sha256,
    'sha512': hashlib.sha512
}

def simulate_attack(hashed_passwords, passwords_list, hash_funcs):
    """
    Attempt to find the passwords corresponding to the given hashes.
    
    :param hashed_passwords: A dictionary of hashes to find passwords for.
    :param passwords_list: A list of common passwords to try.
    :param hash_funcs: A dictionary of hash functions to use.
    """
    cracked_passwords = {}

    for hash_type, hash_data in hashed_passwords.items():
        target_hash, salt = hash_data if isinstance(hash_data, tuple) else (hash_data, None)
        hash_func = hash_funcs[hash_type.split('_')[0]]
        
        for password in passwords_list:
            # If there's a salt, prepend or append it to the password before hashing
            if salt:
                password = f"{salt}{password}{salt}"
            
            # Calculate the hash of the current password
            guess_hash = hash_func(password.encode()).hexdigest()
            
            if guess_hash == target_hash:
                cracked_passwords[hash_type] = password
                break  # Stop searching if we've found the password

    return cracked_passwords

# Simulating the attack
cracked_passwords = simulate_attack(hashed_passwords, common_passwords, hash_functions)

if cracked_passwords:
    for hash_type, password in cracked_passwords.items():
        print(f"Cracked {hash_type}: {password}")
else:
    print("No matches found using the dictionary.")

