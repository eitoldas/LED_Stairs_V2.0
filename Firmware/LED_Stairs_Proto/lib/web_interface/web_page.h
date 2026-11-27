#pragma once

#include <Arduino.h>

/*
 * The single page served at "/". Everything it shows comes from /api/status
 * and /api/settings, so the page itself never changes and lives in flash.
 */

static const char WEB_PAGE[] PROGMEM = R"html(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>LED Stairs</title>
<style>
  :root { --bg: #f4f4f2; --card: #fff; --ink: #222; --muted: #777; --accent: #d88a1c; --line: #e2e2de; }
  @media (prefers-color-scheme: dark) {
    :root { --bg: #161616; --card: #222; --ink: #eee; --muted: #999; --line: #333; }
  }
  * { box-sizing: border-box; }
  body { margin: 0; background: var(--bg); color: var(--ink); font: 15px/1.4 system-ui, sans-serif; }
  main { max-width: 640px; margin: 0 auto; padding: 16px; }
  h1 { font-size: 22px; margin: 8px 0 16px; }
  h2 { font-size: 15px; margin: 0 0 12px; color: var(--muted); text-transform: uppercase; letter-spacing: .05em; }
  section { background: var(--card); border: 1px solid var(--line); border-radius: 10px; padding: 16px; margin-bottom: 16px; }
  dl { display: grid; grid-template-columns: max-content 1fr; gap: 6px 16px; margin: 0; }
  dt { color: var(--muted); }
  dd { margin: 0; font-variant-numeric: tabular-nums; overflow-wrap: anywhere; }
  .badge { display: inline-block; padding: 1px 8px; border-radius: 999px; font-size: 13px; background: var(--line); }
  .on { background: var(--accent); color: #fff; }
  .warn { color: var(--accent); font-weight: 600; }
  label { display: grid; grid-template-columns: 1fr 140px; align-items: center; gap: 12px; margin-bottom: 10px; }
  input[type=number], input[type=time] { width: 100%; padding: 6px 8px; border: 1px solid var(--line); border-radius: 6px; background: var(--bg); color: var(--ink); font: inherit; }
  input[type=checkbox] { justify-self: start; width: 20px; height: 20px; }
  .buttons { display: flex; gap: 10px; margin-top: 16px; }
  button { padding: 8px 16px; border-radius: 6px; border: 1px solid var(--line); background: var(--bg); color: var(--ink); font: inherit; cursor: pointer; }
  button.primary { background: var(--accent); border-color: var(--accent); color: #fff; }
  #message { margin-top: 12px; min-height: 1.4em; }
  .error { color: #c0392b; }
</style>
</head>
<body>
<main>
  <h1>LED Stairs</h1>

  <section>
    <h2>Status</h2>
    <dl id="status"><dt>Loading</dt><dd>&hellip;</dd></dl>
  </section>

  <section>
    <h2>Settings</h2>
    <form id="settings">
      <label>Step delay (ms) <input type="number" name="stepDelayMs" min="50" max="5000" required></label>
      <label>Hold time (ms) <input type="number" name="holdDelayMs" min="0" max="60000" required></label>
      <label>Retrigger cooldown (ms) <input type="number" name="retriggerCooldownMs" min="0" max="60000" required></label>
      <label>Exit offset (ms) <input type="number" name="exitOffsetMs" min="0" max="30000" required></label>
      <label>Automatic daylight <input type="checkbox" name="automaticDaylight"></label>
      <label>Night start <input type="time" name="nightStart" required></label>
      <label>Night end <input type="time" name="nightEnd" required></label>
      <div class="buttons">
        <button type="submit" class="primary">Save</button>
        <button type="button" id="defaults">Restore defaults</button>
      </div>
      <div id="message"></div>
    </form>
  </section>
</main>

<script>
const form = document.getElementById("settings");
const message = document.getElementById("message");

function showMessage(text, isError) {
  message.textContent = text;
  message.className = isError ? "error" : "";
}

function uptime(seconds) {
  const d = Math.floor(seconds / 86400);
  const h = Math.floor(seconds % 86400 / 3600);
  const m = Math.floor(seconds % 3600 / 60);
  return (d ? d + "d " : "") + h + "h " + m + "m";
}

function row(label, value) {
  const dt = document.createElement("dt");
  const dd = document.createElement("dd");
  dt.textContent = label;
  if (value instanceof Node) dd.appendChild(value); else dd.textContent = value;
  return [dt, dd];
}

function badge(on, text) {
  const span = document.createElement("span");
  span.className = "badge" + (on ? " on" : "");
  span.textContent = text;
  return span;
}

async function loadStatus() {
  try {
    const s = await (await fetch("/api/status")).json();
    const rows = [
      row("Lights", badge(s.lightsAllowed, s.lightsAllowed ? "Allowed" : "Daytime, off")),
      row("Time", s.time || "Not synced"),
      row("Sunrise / sunset", s.sunrise ? s.sunrise + " / " + s.sunset : "Not fetched yet"),
      row("Current draw", s.amps + " A  (" + s.watts + " W)"),
      row("Firmware", s.firmware),
      row("Hardware", s.hardware),
      row("Built", s.built),
      row("Device ID", s.deviceId),
      row("Last reset", s.resetReason),
      row("Uptime", uptime(s.uptimeSeconds)),
      row("WiFi", s.wifiRssi + " dBm, " + s.ip),
    ];
    if (s.testBuild) {
      const warn = document.createElement("span");
      warn.className = "warn";
      warn.textContent = "Test build: night window ignored";
      rows.unshift(row("Build", warn));
    }
    document.getElementById("status").replaceChildren(...rows.flat());
  } catch (e) {
    document.getElementById("status").replaceChildren(...row("Status", "Board not responding"));
  }
}

function fillSettings(s) {
  for (const [key, value] of Object.entries(s)) {
    const input = form.elements[key];
    if (!input) continue;
    if (input.type === "checkbox") input.checked = value; else input.value = value;
  }
}

async function loadSettings() {
  fillSettings(await (await fetch("/api/settings")).json());
}

async function send(url, body) {
  const response = await fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: body ? JSON.stringify(body) : "",
  });
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || "Save failed");
  fillSettings(data);
  loadStatus();
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const body = {
    stepDelayMs: Number(form.elements.stepDelayMs.value),
    holdDelayMs: Number(form.elements.holdDelayMs.value),
    retriggerCooldownMs: Number(form.elements.retriggerCooldownMs.value),
    exitOffsetMs: Number(form.elements.exitOffsetMs.value),
    automaticDaylight: form.elements.automaticDaylight.checked,
    nightStart: form.elements.nightStart.value,
    nightEnd: form.elements.nightEnd.value,
  };
  try {
    await send("/api/settings", body);
    showMessage("Saved");
  } catch (e) {
    showMessage(e.message, true);
  }
});

document.getElementById("defaults").addEventListener("click", async () => {
  if (!confirm("Restore all settings to their defaults?")) return;
  try {
    await send("/api/settings/defaults");
    showMessage("Defaults restored");
  } catch (e) {
    showMessage(e.message, true);
  }
});

loadStatus();
loadSettings();
setInterval(loadStatus, 2000);
</script>
</body>
</html>
)html";
