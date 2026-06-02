# Gunicorn and Nginx Deployment

These files assume the app lives at:

```text
/var/www/cybersecurity-web-app
```

## Server Setup

```bash
sudo apt update
sudo apt install -y python3 python3-venv nginx
cd /var/www/cybersecurity-web-app
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
```

## WSL Local Production Check

From Windows, open a WSL terminal and run:

```bash
cd /mnt/c/Users/rober/OneDrive/Documents/Playground/systems-and-securities-lab/cybersecurity-web-app
python3 -m venv .venv-linux
. .venv-linux/bin/activate
pip install -r requirements.txt
python -m unittest discover -s tests
gunicorn -c deploy/gunicorn.conf.py app.app:app
```

Then open:

```text
http://127.0.0.1:8000
```

This confirms the app runs through Gunicorn before moving to Nginx.

## Gunicorn

Copy the service file:

```bash
sudo cp deploy/gunicorn.service /etc/systemd/system/cybersecurity-web-app.service
sudo systemctl daemon-reload
sudo systemctl enable cybersecurity-web-app
sudo systemctl start cybersecurity-web-app
sudo systemctl status cybersecurity-web-app
```

## Nginx

Edit `deploy/nginx.conf` and replace `your-domain.example`.

Then copy and enable it:

```bash
sudo cp deploy/nginx.conf /etc/nginx/sites-available/cybersecurity-web-app
sudo ln -s /etc/nginx/sites-available/cybersecurity-web-app /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

## WSL Nginx Local Test

Nginx was not installed in the tested WSL environment. Install it with:

```bash
sudo apt update
sudo apt install -y nginx
```

Then, with Gunicorn running in another WSL terminal:

```bash
cd /mnt/c/Users/rober/OneDrive/Documents/Playground/systems-and-securities-lab/cybersecurity-web-app
. .venv-linux/bin/activate
gunicorn -c deploy/gunicorn.conf.py app.app:app
```

Copy the WSL-local Nginx config:

```bash
sudo cp deploy/nginx-wsl-local.conf /etc/nginx/sites-available/cybersecurity-web-app-wsl
sudo ln -s /etc/nginx/sites-available/cybersecurity-web-app-wsl /etc/nginx/sites-enabled/
sudo nginx -t
sudo service nginx reload
```

Open:

```text
http://127.0.0.1:8080
```

## Production Check

```bash
curl -I http://127.0.0.1:8000
curl http://127.0.0.1:8000/health
curl -I http://your-domain.example
curl http://your-domain.example/health
```

The health route should return:

```json
{"status":"ok"}
```
