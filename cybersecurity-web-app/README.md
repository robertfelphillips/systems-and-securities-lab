# Cybersecurity Web App

A Flask-based cybersecurity tools dashboard with a retro terminal interface. This project lives inside `cybersecurity-web-app/` in the repository root.

## Features

- **Hash Generator**: generate SHA-256, SHA-512, SHA-1, and MD5 hashes.
- **Base64 Tool**: encode text to Base64 or decode valid Base64 back to text.
- **Controlled Port Scanner**: scan a limited port range after confirming permission.
- **Log Parser**: paste logs or upload `.log` / `.txt` files to extract IPs, HTTP status codes, failed login attempts, errors, warnings, and suspicious lines.
- **Production Health Check**: `/health` returns JSON for deployment monitoring.

## Tool Routes

```text
/              Hash generator
/base64        Base64 encoder/decoder
/port-scanner  Controlled port scanner
/log-parser    Log parser
/health        JSON health check
```

## Project Structure

```text
cybersecurity-web-app/
  app/
    app.py
    static/
      app.js
      styles.css
    templates/
    tools/
  deploy/
    DEPLOYMENT.md
    gunicorn.conf.py
    gunicorn.service
    nginx.conf
    nginx-wsl-local.conf
  samples/
  tests/
  requirements.txt
  render.yaml
  README.md
  .env.example
```

## Local Setup

Windows PowerShell:

```powershell
cd cybersecurity-web-app
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python -m app.app
```

Open:

```text
http://127.0.0.1:5000
```

## WSL / Gunicorn Test

Use this to confirm the app runs through Gunicorn before using Nginx:

```bash
cd /path/to/systems-and-securities-lab/cybersecurity-web-app
python3 -m venv .venv-linux
. .venv-linux/bin/activate
pip install -r requirements.txt
python -m unittest discover -s tests
gunicorn -c deploy/gunicorn.conf.py app.app:app
```

Open:

```text
http://127.0.0.1:8000
```

## WSL / Nginx Local Test

Install Nginx in WSL:

```bash
sudo apt update
sudo apt install -y nginx
```

Start Gunicorn in one WSL terminal:

```bash
cd /path/to/systems-and-securities-lab/cybersecurity-web-app
. .venv-linux/bin/activate
gunicorn -c deploy/gunicorn.conf.py app.app:app
```

In another WSL terminal, enable the local Nginx config:

```bash
cd /path/to/systems-and-securities-lab/cybersecurity-web-app
sudo cp deploy/nginx-wsl-local.conf /etc/nginx/sites-available/cybersecurity-web-app-wsl
sudo ln -s /etc/nginx/sites-available/cybersecurity-web-app-wsl /etc/nginx/sites-enabled/
sudo nginx -t
sudo service nginx reload
```

Open:

```text
http://127.0.0.1:8080
```

## Linux Server Deployment

The production path is:

```text
User -> Nginx -> Gunicorn -> Flask -> Tool logic
```

Full instructions are in:

```text
deploy/DEPLOYMENT.md
```

The included templates are:

```text
deploy/gunicorn.service
deploy/nginx.conf
```

## Manual Test Checklist

Hash generator:

```text
Input: hello
Algorithm: SHA-256
Expected: 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824
```

Base64:

```text
Encode hello -> aGVsbG8=
Decode aGVsbG8= -> hello
```

Port scanner:

```text
Host: 127.0.0.1
Start port: 1
End port: 100
Check the permission box before scanning.
```

Log parser:

```text
Use samples/sample-auth.log or samples/sample-web.log.
You can paste log text or upload a .log/.txt file up to 1 MB.
```

## Automated Tests

Run:

```powershell
.\.venv\Scripts\python -m unittest discover -s tests
```

Or in WSL:

```bash
python -m unittest discover -s tests
```

The tests cover:

- Hash generation
- Base64 encode/decode behavior
- Invalid Base64 handling
- Log parsing and file upload handling
- Port scanner validation and mocked port states
- Flask page routes
- `/health` response

## Safety Notes

Only scan systems you own or have explicit permission to test. The scanner is intentionally limited to 100 ports per scan and includes a permission confirmation step.

For public deployments, set:

```text
SCANNER_ALLOWED_HOSTS=127.0.0.1,localhost,::1
```

Leave it unset during trusted local development if you need to scan another approved host.

## Future Improvements

- Add screenshots or a short deployment demo.
- Add export buttons for scan and parser reports.
- Add authentication before exposing private/internal tools.
- Add rate limiting before heavier public use.
