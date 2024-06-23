import os
from hashlib import sha256
from random import Random

from Crypto import Random
from Crypto.Cipher import AES


def aes_encrypt(iv, key, plain):
    return AES.new(key, AES.MODE_CBC, iv).encrypt(plain)


def aes_decrypt(iv, key, cipher):
    return AES.new(key, AES.MODE_CBC, iv).decrypt(cipher)


def calculate_hash(string):
    return sha256(string.encode('utf-8')).hexdigest()


def generate_aes_key():
    return os.urandom(32)


def generate_iv():
    return Random.new().read(AES.block_size)


