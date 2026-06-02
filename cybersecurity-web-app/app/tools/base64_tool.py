import base64
import binascii


def encode_base64(text: str) -> str:
    encoded = base64.b64encode(text.encode("utf-8"))
    return encoded.decode("ascii")


def decode_base64(text: str) -> str:
    try:
        decoded = base64.b64decode(text.encode("ascii"), validate=True)
    except (binascii.Error, UnicodeEncodeError) as exc:
        raise ValueError("Input is not valid Base64 text.") from exc

    try:
        return decoded.decode("utf-8")
    except UnicodeDecodeError:
        return "Decoded bytes are not valid UTF-8 text.\n\nHex output:\n" + decoded.hex()
