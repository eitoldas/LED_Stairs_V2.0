#pragma once

#include <Arduino.h>

/*
 * The serial console page served at "/serial". It polls /api/logs once a
 * second and appends whatever the board has logged since the last poll.
 */

static const char SERIAL_PAGE[] PROGMEM = R"html(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>LED Stairs - Serial</title>
<style>
  :root { --bg: #f4f4f2; --card: #fff; --ink: #222; --muted: #777; --accent: #d88a1c; --line: #e2e2de; }
  @media (prefers-color-scheme: dark) {
    :root { --bg: #161616; --card: #222; --ink: #eee; --muted: #999; --line: #333; }
  }
  * { box-sizing: border-box; }
  html, body { height: 100%; }
  body { margin: 0; background: var(--bg); color: var(--ink); font: 15px/1.4 system-ui, sans-serif; }
  main { max-width: 960px; height: 100%; margin: 0 auto; padding: 16px; display: flex; flex-direction: column; }
  header { display: flex; align-items: center; gap: 12px; margin-bottom: 12px; flex-wrap: wrap; }
  h1 { font-size: 22px; margin: 0; flex: 1; }
  a { color: var(--accent); }
  button { padding: 6px 14px; border-radius: 6px; border: 1px solid var(--line); background: var(--card); color: var(--ink); font: inherit; cursor: pointer; }
  #state { color: var(--muted); font-size: 13px; }
  #console { flex: 1; overflow-y: auto; background: var(--card); border: 1px solid var(--line); border-radius: 10px;
             padding: 12px; font: 13px/1.45 ui-monospace, Consolas, monospace; white-space: pre-wrap; overflow-wrap: anywhere; }
  .missed { color: var(--accent); font-style: italic; }
</style>
</head>
<body>
<main>
  <header>
    <h1>Serial console</h1>
    <span id="state">Connecting&hellip;</span>
    <button id="pause">Pause</button>
    <button id="clear">Clear</button>
    <a href="/">Back</a>
  </header>
  <div id="console"></div>
</main>

<script>
const MAX_LINES = 2000;
const consoleBox = document.getElementById("console");
const state = document.getElementById("state");
const pauseButton = document.getElementById("pause");

let since = 0;
let paused = false;

function append(text, className) {
  const atBottom = consoleBox.scrollHeight - consoleBox.scrollTop - consoleBox.clientHeight < 20;
  const div = document.createElement("div");
  div.textContent = text;
  if (className) div.className = className;
  consoleBox.appendChild(div);
  while (consoleBox.childElementCount > MAX_LINES) consoleBox.firstElementChild.remove();
  if (atBottom) consoleBox.scrollTop = consoleBox.scrollHeight;
}

async function poll() {
  if (!paused) {
    try {
      const data = await (await fetch("/api/logs?since=" + since)).json();
      if (data.total < since) append("Board restarted", "missed");
      if (data.missed > 0) append("... " + data.missed + " lines missed ...", "missed");
      for (const line of data.lines) append(line);
      since = data.total;
      state.textContent = "Live";
    } catch (e) {
      state.textContent = "Board not responding";
    }
  }
  setTimeout(poll, 1000);
}

pauseButton.addEventListener("click", () => {
  paused = !paused;
  pauseButton.textContent = paused ? "Resume" : "Pause";
  state.textContent = paused ? "Paused" : "Live";
});

document.getElementById("clear").addEventListener("click", () => consoleBox.replaceChildren());

poll();
</script>
</body>
</html>
)html";
