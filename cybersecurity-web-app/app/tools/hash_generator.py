import hashlib


SUPPORTED_ALGORITHMS = {
    "sha256": "SHA-256",
    "sha512": "SHA-512",
    "sha1": "SHA-1",
    "md5": "MD5",
}


def generate_hash(text: str, algorithm: str = "sha256") -> str:
    if algorithm not in SUPPORTED_ALGORITHMS:
        raise ValueError("Unsupported hash algorithm.")

    hasher = hashlib.new(algorithm)
    hasher.update(text.encode("utf-8"))
    return hasher.hexdigest()
