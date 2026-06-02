import os
import socket
from concurrent.futures import ThreadPoolExecutor, as_completed


MAX_PORT_RANGE = 100
CONNECT_TIMEOUT_SECONDS = 0.35
MAX_WORKERS = 50


def scan_ports(host: str, start_port: str, end_port: str) -> list[dict[str, str | int]]:
    clean_host = host.strip()
    if not clean_host:
        raise ValueError("Enter a host to scan.")

    resolved_addresses = _resolve_host(clean_host)
    _enforce_allowed_host(clean_host, resolved_addresses)

    start = _parse_port(start_port, "Start port")
    end = _parse_port(end_port, "End port")

    if end < start:
        raise ValueError("End port must be greater than or equal to start port.")

    port_count = end - start + 1
    if port_count > MAX_PORT_RANGE:
        raise ValueError(f"Scan range is limited to {MAX_PORT_RANGE} ports at a time.")

    ports = range(start, end + 1)
    results: list[dict[str, str | int]] = []

    with ThreadPoolExecutor(max_workers=min(MAX_WORKERS, port_count)) as executor:
        future_to_port = {
            executor.submit(_scan_port, clean_host, port): port
            for port in ports
        }

        for future in as_completed(future_to_port):
            results.append(future.result())

    return sorted(results, key=lambda item: int(item["port"]))


def _resolve_host(host: str):
    try:
        return socket.getaddrinfo(host, None)
    except socket.gaierror as exc:
        raise ValueError("Host could not be resolved.") from exc


def _enforce_allowed_host(host: str, resolved_addresses) -> None:
    allowed_hosts = _configured_allowed_hosts()
    if not allowed_hosts:
        return

    candidates = {host.lower()}
    for resolved in resolved_addresses:
        try:
            candidates.add(str(resolved[4][0]).lower())
        except (IndexError, TypeError):
            continue

    if candidates.isdisjoint(allowed_hosts):
        allowed_display = ", ".join(sorted(allowed_hosts))
        raise ValueError(f"This deployment only allows scans for: {allowed_display}.")


def _configured_allowed_hosts() -> set[str]:
    raw_hosts = os.environ.get("SCANNER_ALLOWED_HOSTS", "")
    return {
        host.strip().lower()
        for host in raw_hosts.split(",")
        if host.strip()
    }


def _parse_port(value: str, label: str) -> int:
    try:
        port = int(value)
    except ValueError as exc:
        raise ValueError(f"{label} must be a number.") from exc

    if port < 1 or port > 65535:
        raise ValueError(f"{label} must be between 1 and 65535.")

    return port


def _scan_port(host: str, port: int) -> dict[str, str | int]:
    try:
        with socket.create_connection((host, port), timeout=CONNECT_TIMEOUT_SECONDS):
            return {"port": port, "status": "open", "detail": "Connection accepted"}
    except TimeoutError:
        return {"port": port, "status": "timeout", "detail": "No response before timeout"}
    except ConnectionRefusedError:
        return {"port": port, "status": "closed", "detail": "Connection refused"}
    except OSError as exc:
        if "timed out" in str(exc).lower():
            return {"port": port, "status": "timeout", "detail": "No response before timeout"}

        return {"port": port, "status": "closed", "detail": "No connection"}
