#ifndef HTML_H
#define HTML_H

// Serve this page as the main UI for the thermocouple.
// Uses simple JS to poll /readWeb_Thermo once per second.

const char html_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Thermocouple Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <style>
    :root {
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background: #111827;
      color: #e5e7eb;
    }

    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .card {
      background: #020617;
      border-radius: 16px;
      padding: 1.5rem;
      box-shadow: 0 20px 40px rgba(0,0,0,0.5);
      max-width: 420px;
      width: 100%;
      box-sizing: border-box;
      border: 1px solid #1f2937;
    }

    .title {
      font-size: 1.4rem;
      font-weight: 600;
      margin-bottom: 0.25rem;
    }

    .subtitle {
      font-size: 0.85rem;
      color: #9ca3af;
      margin-bottom: 1.5rem;
    }

    .grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 0.75rem;
      margin-bottom: 1rem;
    }

    .reading {
      border-radius: 14px;
      padding: 0.9rem 0.7rem;
      background: #0b1220;
      border: 1px solid #1d283a;
    }

    .label {
      font-size: 0.8rem;
      color: #9ca3af;
      margin-bottom: 0.25rem;
      text-transform: uppercase;
      letter-spacing: 0.04em;
    }

    .value {
      font-size: 1.6rem;
      font-weight: 600;
    }

    .value span.unit {
      font-size: 0.95rem;
      color: #9ca3af;
      margin-left: 0.25rem;
    }

    .status {
      font-size: 0.8rem;
      color: #9ca3af;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.5rem;
      margin-top: 0.5rem;
    }

    .pill {
      padding: 0.15rem 0.55rem;
      border-radius: 999px;
      font-size: 0.75rem;
      border: 1px solid #1d283a;
      background: #020617;
      display: inline-flex;
      align-items: center;
      gap: 0.3rem;
    }

    .dot {
      width: 7px;
      height: 7px;
      border-radius: 999px;
      background: #22c55e;
      box-shadow: 0 0 6px rgba(34,197,94,0.8);
    }

    .dot.offline {
      background: #ef4444;
      box-shadow: 0 0 6px rgba(239,68,68,0.8);
    }

    button.refresh-btn {
      margin-top: 0.5rem;
      width: 100%;
      padding: 0.55rem 0.75rem;
      border-radius: 999px;
      border: 1px solid #1d283a;
      background: #111827;
      color: #e5e7eb;
      font-size: 0.85rem;
      cursor: pointer;
    }

    button.refresh-btn:active {
      transform: scale(0.98);
    }

    @media (max-width: 480px) {
      .card {
        margin: 1rem;
      }
      .value {
        font-size: 1.4rem;
      }
    }
  </style>
</head>

<body>
  <div class="card">
    <div class="title">ESP32 Thermocouple</div>
    <div class="subtitle">Live readings from MAX6675</div>

    <div class="grid">
      <div class="reading">
        <div class="label">Celsius</div>
        <div class="value" id="tempC">--<span class="unit">°C</span></div>
      </div>
      <div class="reading">
        <div class="label">Fahrenheit</div>
        <div class="value" id="tempF">--<span class="unit">°F</span></div>
      </div>
    </div>

    <button class="refresh-btn" onclick="fetchThermo(true)">Refresh now</button>

    <div class="status">
      <div>
        <span id="statusText">Connecting…</span>
      </div>
      <div class="pill">
        <span class="dot" id="statusDot"></span>
        <span id="lastUpdate">—</span>
      </div>
    </div>
  </div>

<script>
  const tempCEl = document.getElementById('tempC');
  const tempFEl = document.getElementById('tempF');
  const statusTextEl = document.getElementById('statusText');
  const statusDotEl = document.getElementById('statusDot');
  const lastUpdateEl = document.getElementById('lastUpdate');

  function setOnlineState(online) {
    if (online) {
      statusDotEl.classList.remove('offline');
      statusTextEl.textContent = 'Connected';
    } else {
      statusDotEl.classList.add('offline');
      statusTextEl.textContent = 'Disconnected';
    }
  }

  function fetchThermo(manual) {
    fetch('/readWeb_Thermo')
      .then(res => res.text())
      .then(txt => {
        // The ESP32 sends: ["<C>","<F>"]
        let arr;
        try {
          arr = JSON.parse(txt);
        } catch (e) {
          console.error('Bad JSON from ESP32:', txt);
          throw e;
        }

        const c = arr[0];
        const f = arr[1];

        tempCEl.innerHTML = c + '<span class="unit">°C</span>';
        tempFEl.innerHTML = f + '<span class="unit">°F</span>';

        const now = new Date();
        lastUpdateEl.textContent = now.toLocaleTimeString();
        setOnlineState(true);

        if (manual) {
          statusTextEl.textContent = 'Updated manually';
          setTimeout(() => setOnlineState(true), 1500);
        }
      })
      .catch(err => {
        console.error(err);
        setOnlineState(false);
      });
  }

  // Poll every 1 second (matches your loop delay)
  setInterval(fetchThermo, 1000);
  // Try once on load
  fetchThermo(false);
</script>
</body>
</html>
)=====";

#endif
