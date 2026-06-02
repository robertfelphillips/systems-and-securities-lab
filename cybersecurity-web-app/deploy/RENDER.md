# Render Free Deployment

Render is the simplest no-cost way to make this Flask app public for a portfolio demo.

## Setup

1. Push the repo to GitHub.
2. Sign in to Render.
3. Create a new Blueprint or Web Service from this repository.
4. Use `render.yaml` from the `cybersecurity-web-app` folder.
5. Deploy.

## Public Scanner Safety

The Render config sets:

```text
SCANNER_ALLOWED_HOSTS=127.0.0.1,localhost,::1
```

That means the public deployment cannot be used to scan arbitrary hosts. Local development can still scan normal approved targets if this variable is not set.

## Render Runtime

Render will run:

```bash
gunicorn -w 3 -b 0.0.0.0:$PORT app.app:app
```

The health check endpoint is:

```text
/health
```
