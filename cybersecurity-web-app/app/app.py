from flask import Flask, render_template, request
from werkzeug.exceptions import RequestEntityTooLarge

from app.tools.base64_tool import decode_base64, encode_base64
from app.tools.hash_generator import SUPPORTED_ALGORITHMS, generate_hash
from app.tools.log_parser import MAX_LOG_UPLOAD_BYTES, parse_logs, read_log_upload
from app.tools.port_scanner import MAX_PORT_RANGE, scan_ports


app = Flask(__name__)
app.config["MAX_CONTENT_LENGTH"] = MAX_LOG_UPLOAD_BYTES


@app.errorhandler(RequestEntityTooLarge)
def handle_file_too_large(_):
    return (
        render_template(
            "log_parser.html",
            log_text="",
            uploaded_filename=None,
            report=None,
            error="Uploaded log file must be 1 MB or smaller.",
        ),
        413,
    )


@app.route("/health")
def health_check():
    return {"status": "ok"}


@app.route("/", methods=["GET", "POST"])
def index():
    digest = None
    error = None
    input_text = ""
    algorithm = "sha256"

    if request.method == "POST":
        input_text = request.form.get("input_text", "")
        algorithm = request.form.get("algorithm", "sha256")

        try:
            digest = generate_hash(input_text, algorithm)
        except ValueError as exc:
            error = str(exc)

    return render_template(
        "index.html",
        algorithms=SUPPORTED_ALGORITHMS,
        selected_algorithm=algorithm,
        input_text=input_text,
        digest=digest,
        error=error,
    )


@app.route("/base64", methods=["GET", "POST"])
def base64_tool():
    output_text = None
    error = None
    input_text = ""
    action = "encode"

    if request.method == "POST":
        input_text = request.form.get("input_text", "")
        action = request.form.get("action", "encode")

        try:
            if action == "encode":
                output_text = encode_base64(input_text)
            elif action == "decode":
                output_text = decode_base64(input_text)
            else:
                error = "Choose encode or decode."
        except ValueError as exc:
            error = str(exc)

    return render_template(
        "base64.html",
        input_text=input_text,
        output_text=output_text,
        selected_action=action,
        error=error,
    )


@app.route("/log-parser", methods=["GET", "POST"])
def log_parser():
    log_text = ""
    uploaded_filename = None
    report = None
    error = None

    if request.method == "POST":
        uploaded_file = request.files.get("log_file")

        try:
            if uploaded_file and uploaded_file.filename:
                uploaded_filename = uploaded_file.filename
                log_text = read_log_upload(uploaded_file)
            else:
                log_text = request.form.get("log_text", "")

            report = parse_logs(log_text)
        except ValueError as exc:
            error = str(exc)

    return render_template(
        "log_parser.html",
        log_text=log_text,
        uploaded_filename=uploaded_filename,
        report=report,
        error=error,
    )


@app.route("/port-scanner", methods=["GET", "POST"])
def port_scanner():
    host = "127.0.0.1"
    start_port = "1"
    end_port = "100"
    permission_ack = False
    results = None
    open_results = []
    scan_summary = None
    error = None

    if request.method == "POST":
        host = request.form.get("host", "127.0.0.1").strip()
        start_port = request.form.get("start_port", "1").strip()
        end_port = request.form.get("end_port", "100").strip()
        permission_ack = request.form.get("permission_ack") == "yes"

        if not permission_ack:
            error = "Confirm you have permission to scan this host."
        else:
            try:
                results = scan_ports(host, start_port, end_port)
                open_results = [
                    result
                    for result in results
                    if result["status"] == "open"
                ]
                scan_summary = {
                    "host": host,
                    "range": f"{start_port}-{end_port}",
                    "total_count": len(results),
                    "open_count": len(open_results),
                    "other_count": len(results) - len(open_results),
                }
            except ValueError as exc:
                error = str(exc)

    return render_template(
        "port_scanner.html",
        host=host,
        start_port=start_port,
        end_port=end_port,
        max_port_range=MAX_PORT_RANGE,
        permission_ack=permission_ack,
        results=results,
        open_results=open_results,
        scan_summary=scan_summary,
        error=error,
    )


if __name__ == "__main__":
    app.run(debug=True)
