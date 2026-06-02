const forms = document.querySelectorAll("[data-running-text]");

for (const form of forms) {
  form.addEventListener("submit", () => {
    const status = document.querySelector("[data-status-target]");
    const button = form.querySelector("button[type='submit']");

    if (status) {
      status.textContent = form.dataset.runningText || "working...";
      status.dataset.state = "running";
    }

    if (button) {
      button.dataset.running = "true";
    }
  });
}
