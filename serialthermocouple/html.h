#ifndef HTML_H
#define HTML_H

// ESP32-hosted UI for the thermocouple, with a Begin/Stop
// recording button and a simple line chart of °C vs time.

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
      max-width: 520px;
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
      margin-bottom: 1.0rem;
    }

    .grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 0.75rem;
      margin-bottom: 0.75rem;
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

    .status-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.5rem;
      font-size: 0.8rem;
      color: #9ca3af;
      margin-top: 0.5rem;
      margin-bottom: 0.75rem;
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

    button.record-btn {
      width: 100%;
      padding: 0.6rem 0.75rem;
      border-radius: 999px;
      border: 1px solid #1d283a;
      background: #16a34a;
      color: #e5e7eb;
      font-size: 0.9rem;
      cursor: pointer;
      margin-bottom: 0.75rem;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 0.4rem;
    }

    button.record-btn.stopped {
      background: #111827;
    }

    button.record-btn:active {
      transform: scale(0.98);
    }

    .dot-small {
      width: 8px;
      height: 8px;
      border-radius: 999px;
      background: #f97316;
      box-shadow: 0 0 8px rgba(249,115,22,0.9);
    }

    canvas {
      width: 100%;
      height: 220px;
      border-radius: 12px;
      background: #020617;
      border: 1px solid #1f2937;
      display: block;
    }

    .legend {
      display: flex;
      justify-content: flex-start;
      align-items: center;
      gap: 0.5rem;
      margin-top: 0.45rem;
      font-size: 0.8rem;
      color: #9ca3af;
    }

    .legend-swatch {
      width: 16px;
      height: 3px;
      border-radius: 999px;
      background: #38bdf8;
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
    <div class="subtitle">Live °C vs time (MAX6675)</div>

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

    <button class="record-btn stopped" id="recordBtn" onclick="toggleRecording()">
      <span class="dot-small" id="recordDot"></span>
      <span id="recordLabel">Begin recording</span>
    </button>

    <div class="status-row">
      <div id="statusText">Idle</div>
      <div class="pill">
        <span class="dot" id="statusDot"></span>
        <span id="lastUpdate">—</span>
      </div>
    </div>

    <canvas id="tempChart"></canvas>
    <div class="legend">
      <div class="legend-swatch"></div>
      <span>°C vs time since recording started (s)</span>
    </div>
  </div>

<script>
  const tempCEl = document.getElementById('tempC');
  const tempFEl = document.getElementById('tempF');
  const statusTextEl = document.getElementById('statusText');
  const statusDotEl = document.getElementById('statusDot');
  const lastUpdateEl = document.getElementById('lastUpdate');

  const recordBtn = document.getElementById('recordBtn');
  const recordLabelEl = document.getElementById('recordLabel');
  const recordDotEl = document.getElementById('recordDot');

  const canvas = document.getElementById('tempChart');
  const ctx = canvas.getContext('2d');

  let recording = false;
  let pollTimer = null;
  let startTime = null;

  // {t: seconds since start, c: temperature in °C}
  let samples = [];
  const maxSamples = 600; // e.g., 10 minutes at 1 Hz

  function setOnlineState(online) {
    if (online) {
      statusDotEl.classList.remove('offline');
    } else {
      statusDotEl.classList.add('offline');
    }
  }

  function toggleRecording() {
    if (!recording) {
      // Start recording
      recording = true;
      samples = [];
      startTime = Date.now();
      recordBtn.classList.remove('stopped');
      recordLabelEl.textContent = 'Stop recording';
      recordDotEl.style.background = '#f97316';
      recordDotEl.style.boxShadow = '0 0 8px rgba(249,115,22,0.9)';
      statusTextEl.textContent = 'Recording…';
      lastUpdateEl.textContent = '—';

      // Start polling at 1 Hz
      fetchAndRecord(); // immediate first sample
      pollTimer = setInterval(fetchAndRecord, 1000);
    } else {
      // Stop recording
      recording = false;
      if (pollTimer) {
        clearInterval(pollTimer);
        pollTimer = null;
      }
      recordBtn.classList.add('stopped');
      recordLabelEl.textContent = 'Begin recording';
      recordDotEl.style.background = '#4b5563';
      recordDotEl.style.boxShadow = 'none';
      statusTextEl.textContent = 'Idle (last run: ' + samples.length + ' pts)';
    }
  }

  function fetchAndRecord() {
    if (!recording) return;

    fetch('/readWeb_Thermo')
      .then(res => res.text())
      .then(txt => {
        let arr;
        try {
          arr = JSON.parse(txt);
        } catch (e) {
          console.error('Bad JSON from ESP32:', txt);
          throw e;
        }

        const cStr = arr[0];
        const fStr = arr[1];
        const cVal = parseFloat(cStr);

        tempCEl.innerHTML = cStr + '<span class="unit">°C</span>';
        tempFEl.innerHTML = fStr + '<span class="unit">°F</span>';

        const now = Date.now();
        const elapsedSec = (now - startTime) / 1000.0;

        samples.push({ t: elapsedSec, c: cVal });
        if (samples.length > maxSamples) {
          samples.shift();
        }

        const timeLabel = new Date().toLocaleTimeString();
        lastUpdateEl.textContent = timeLabel;
        statusTextEl.textContent = 'Recording… (' + samples.length + ' pts)';
        setOnlineState(true);

        drawChart();
      })
      .catch(err => {
        console.error(err);
        setOnlineState(false);
        statusTextEl.textContent = 'Error: could not read from ESP32';
      });
  }

  function resizeCanvasToDisplaySize() {
    const ratio = window.devicePixelRatio || 1;
    const displayWidth  = canvas.clientWidth;
    const displayHeight = canvas.clientHeight;

    if (canvas.width !== displayWidth * ratio ||
        canvas.height !== displayHeight * ratio) {
      canvas.width  = displayWidth * ratio;
      canvas.height = displayHeight * ratio;
      ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
    }
  }

  function drawChart() {
    resizeCanvasToDisplaySize();

    const width  = canvas.clientWidth;
    const height = canvas.clientHeight;

    ctx.clearRect(0, 0, width, height);

    if (samples.length < 1) {
      // draw placeholder text
      ctx.fillStyle = '#6b7280';
      ctx.font = '12px system-ui, sans-serif';
      ctx.textAlign = 'center';
      ctx.fillText('Begin recording to see live data', width / 2, height / 2);
      return;
    }

    // Determine bounds
    let minT = samples[0].t;
    let maxT = samples[samples.length - 1].t;
    let minC = samples[0].c;
    let maxC = samples[0].c;

    for (let i = 0; i < samples.length; i++) {
      const s = samples[i];
      if (s.t < minT) minT = s.t;
      if (s.t > maxT) maxT = s.t;
      if (s.c < minC) minC = s.c;
      if (s.c > maxC) maxC = s.c;
    }

    // Avoid zero-span axes
    if (maxT === minT) {
      maxT = minT + 1;
    }
    if (maxC === minC) {
      maxC = minC + 1;
    }

    // Add some padding to y range
    const yPad = (maxC - minC) * 0.1 || 1;
    minC -= yPad;
    maxC += yPad;

    // Chart margins
    const marginLeft   = 40;
    const marginRight  = 10;
    const marginTop    = 10;
    const marginBottom = 24;

    const plotWidth  = width  - marginLeft - marginRight;
    const plotHeight = height - marginTop  - marginBottom;

    // Helpers to map data -> pixel
    const xScale = plotWidth / (maxT - minT);
    const yScale = plotHeight / (maxC - minC);

    function xPixel(t) {
      return marginLeft + (t - minT) * xScale;
    }

    function yPixel(c) {
      return marginTop + plotHeight - (c - minC) * yScale;
    }

    // Draw axes
    ctx.strokeStyle = '#1f2933';
    ctx.lineWidth = 1;
    ctx.beginPath();
    // X axis
    ctx.moveTo(marginLeft, marginTop + plotHeight);
    ctx.lineTo(marginLeft + plotWidth, marginTop + plotHeight);
    // Y axis
    ctx.moveTo(marginLeft, marginTop);
    ctx.lineTo(marginLeft, marginTop + plotHeight);
    ctx.stroke();

    // Ticks & labels (simple 4 ticks)
    ctx.fillStyle = '#6b7280';
    ctx.font = '10px system-ui, sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'top';

    const xTicks = 4;
    for (let i = 0; i <= xTicks; i++) {
      const frac = i / xTicks;
      const tVal = minT + frac * (maxT - minT);
      const x = marginLeft + frac * plotWidth;
      const yAxis = marginTop + plotHeight;

      ctx.beginPath();
      ctx.moveTo(x, yAxis);
      ctx.lineTo(x, yAxis + 3);
      ctx.strokeStyle = '#1f2933';
      ctx.stroke();

      ctx.fillText(tVal.toFixed(0), x, yAxis + 5);
    }

    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    const yTicks = 4;
    for (let i = 0; i <= yTicks; i++) {
      const frac = i / yTicks;
      const cVal = minC + frac * (maxC - minC);
      const y = marginTop + plotHeight - frac * plotHeight;

      ctx.beginPath();
      ctx.moveTo(marginLeft - 3, y);
      ctx.lineTo(marginLeft, y);
      ctx.strokeStyle = '#1f2933';
      ctx.stroke();

      ctx.fillText(cVal.toFixed(1), marginLeft - 5, y);
    }

    // Draw line
    ctx.strokeStyle = '#38bdf8';
    ctx.lineWidth = 2;
    ctx.beginPath();
    for (let i = 0; i < samples.length; i++) {
      const s = samples[i];
      const x = xPixel(s.t);
      const y = yPixel(s.c);
      if (i === 0) {
        ctx.moveTo(x, y);
      } else {
        ctx.lineTo(x, y);
      }
    }
    ctx.stroke();

    // Optional: small circle at last point
    const last = samples[samples.length - 1];
    const lx = xPixel(last.t);
    const ly = yPixel(last.c);
    ctx.fillStyle = '#38bdf8';
    ctx.beginPath();
    ctx.arc(lx, ly, 3, 0, Math.PI * 2);
    ctx.fill();
  }

  // On resize, re-draw the chart
  window.addEventListener('resize', drawChart);

  // Initial state
  setOnlineState(true);
  drawChart(); // draws the "Begin recording" placeholder
</script>
</body>
</html>
)=====";

#endif
