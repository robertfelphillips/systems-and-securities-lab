# Systems and Security Lab

A growing repository for operating systems, Linux, networking, and cybersecurity practice projects. The goal is to keep hands-on exercises organized in one place with clear project writeups and room for new labs.

## Lab folders

- `c-shell/` — shell scripting and systems tooling lab placeholder for Bash and shell-focused exercises
- `cybersecurity-web-app/` — Flask-based cybersecurity tools web application
- `siem-lab/` — starter lab for SIEM and log analysis projects
- `custom-linux-shell/` — a Unix-style shell written in C with piping, redirection, background jobs, built-in commands, and environment expansion
- `linux-practice/` — Linux command notes, exercises, and terminal practice logs
- `networking-labs/` — networking experiments such as Wireshark captures and Nmap scans
- `log-analysis/` — future log parsing and system investigation work
- `scripts/` — future automation scripts in Bash or Python

## cyber-security web app

The `cybersecurity-web-app/` folder contains a Python Flask web application with tools for hashing, Base64 encoding/decoding, log parsing, and a controlled port scanner. It is intended as a portfolio-ready security toolkit and supports local development and deployment via Gunicorn/Nginx.

## Getting started with the web app

From the repository root:

```powershell
cd cybersecurity-web-app
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python -m app.app
```

Then open:

```text
http://127.0.0.1:5000
```

## SIEM Lab

The `siem-lab/` folder is reserved for Security Information and Event Management work, including log ingestion, detection rules, alerting workflows, and analyst investigation notes.

## Notes

This repo is intended to remain on the `main` branch for local organization. No commits or pushes will be made until you review the changes.
