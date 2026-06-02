import ipaddress
import re
from collections import Counter


ALLOWED_LOG_EXTENSIONS = {".log", ".txt"}
MAX_LOG_UPLOAD_BYTES = 1_000_000
IP_PATTERN = re.compile(r"\b(?:\d{1,3}\.){3}\d{1,3}\b")
FAILED_LOGIN_PATTERN = re.compile(
    r"\b(failed password|authentication failure|invalid user|login failed|failed login)\b",
    re.IGNORECASE,
)
ERROR_PATTERN = re.compile(r"\b(error|exception|critical|fatal|denied)\b", re.IGNORECASE)
WARNING_PATTERN = re.compile(r"\b(warn|warning|timeout|refused|blocked)\b", re.IGNORECASE)
HTTP_STATUS_PATTERN = re.compile(r'"\s(?P<status>[1-5]\d{2})\s')
MAX_MATCHES = 25


def read_log_upload(uploaded_file) -> str:
    filename = uploaded_file.filename or ""
    if not _is_allowed_log_file(filename):
        raise ValueError("Upload a .log or .txt file.")

    contents = uploaded_file.read(MAX_LOG_UPLOAD_BYTES + 1)
    if len(contents) > MAX_LOG_UPLOAD_BYTES:
        raise ValueError("Uploaded log file must be 1 MB or smaller.")

    try:
        return contents.decode("utf-8-sig")
    except UnicodeDecodeError as exc:
        raise ValueError("Uploaded log file must be UTF-8 text.") from exc


def parse_logs(log_text: str) -> dict[str, object]:
    lines = [line.rstrip() for line in log_text.splitlines() if line.strip()]
    if not lines:
        raise ValueError("Paste log text to analyze.")

    ip_counter: Counter[str] = Counter()
    status_counter: Counter[str] = Counter()
    matches: list[dict[str, str | int]] = []
    failed_logins = 0
    errors = 0
    warnings = 0

    for line_number, line in enumerate(lines, start=1):
        valid_ips = _extract_valid_ips(line)
        ip_counter.update(valid_ips)

        status_match = HTTP_STATUS_PATTERN.search(line)
        if status_match:
            status_counter.update([status_match.group("status")])

        categories = []
        if FAILED_LOGIN_PATTERN.search(line):
            failed_logins += 1
            categories.append("failed-login")

        if ERROR_PATTERN.search(line):
            errors += 1
            categories.append("error")

        if WARNING_PATTERN.search(line):
            warnings += 1
            categories.append("warning")

        if categories and len(matches) < MAX_MATCHES:
            matches.append(
                {
                    "line_number": line_number,
                    "category": ", ".join(categories),
                    "line": line,
                }
            )

    unique_ips = sorted(ip_counter)
    top_ips = [
        {"ip": ip, "count": count, "scope": _ip_scope(ip)}
        for ip, count in ip_counter.most_common(10)
    ]
    http_statuses = [
        {"status": status, "count": count}
        for status, count in sorted(status_counter.items())
    ]

    return {
        "line_count": len(lines),
        "unique_ip_count": len(unique_ips),
        "failed_logins": failed_logins,
        "errors": errors,
        "warnings": warnings,
        "top_ips": top_ips,
        "http_statuses": http_statuses,
        "matches": matches,
        "match_limit": MAX_MATCHES,
    }


def _extract_valid_ips(line: str) -> list[str]:
    valid_ips = []
    for candidate in IP_PATTERN.findall(line):
        try:
            ipaddress.ip_address(candidate)
        except ValueError:
            continue

        valid_ips.append(candidate)

    return valid_ips


def _ip_scope(ip: str) -> str:
    parsed = ipaddress.ip_address(ip)
    if parsed.is_loopback:
        return "loopback"
    if parsed.is_private:
        return "private"
    if parsed.is_multicast:
        return "multicast"
    if parsed.is_reserved:
        return "reserved"

    return "public"


def _is_allowed_log_file(filename: str) -> bool:
    return any(filename.lower().endswith(extension) for extension in ALLOWED_LOG_EXTENSIONS)
