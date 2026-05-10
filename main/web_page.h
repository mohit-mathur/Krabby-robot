/**
 * Embedded Web Page - Auto-generated
 * Responsive HTML/CSS/JS control interface
 */

#ifndef WEB_PAGE_H
#define WEB_PAGE_H

static const char WEB_PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Quadruped Bot</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,system-ui,sans-serif;background:#1a1a2e;color:#eee;
  max-width:480px;margin:0 auto;padding:12px;touch-action:manipulation;user-select:none}
h1{text-align:center;font-size:1.3em;padding:10px 0;color:#e94560}
.section{background:#16213e;border-radius:12px;padding:14px;margin:10px 0}
.section-title{font-size:.85em;color:#888;text-transform:uppercase;letter-spacing:1px;margin-bottom:10px}

/* D-Pad */
.dpad{display:grid;grid-template-columns:60px 60px 60px;grid-template-rows:60px 60px 60px;
  gap:4px;justify-content:center;margin:0 auto}
.dpad-btn{background:#0f3460;border:none;border-radius:10px;color:#eee;font-size:1.4em;
  display:flex;align-items:center;justify-content:center;cursor:pointer;
  transition:background .1s}
.dpad-btn:active{background:#e94560}
.dpad-center{background:#1a1a2e;border-radius:50%}
.dpad-stop{background:#533;font-size:.7em;font-weight:700;color:#e94560}
.dpad-stop:active{background:#e94560;color:#fff}

/* Buttons grid */
.btn-grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px}
.btn{background:#0f3460;border:none;border-radius:10px;color:#eee;padding:14px 8px;
  font-size:.85em;cursor:pointer;transition:background .1s;font-weight:500}
.btn:active{background:#e94560}
.btn-danger{background:#5c1a1a}
.btn-danger:active{background:#e94560}

/* Sliders */
.slider-row{display:flex;align-items:center;gap:10px;margin:8px 0}
.slider-label{font-size:.8em;color:#aaa;min-width:50px}
.slider-val{font-size:.8em;color:#e94560;min-width:30px;text-align:right}
input[type=range]{flex:1;accent-color:#e94560;height:6px}

/* Tilt pad */
.tilt-pad{width:160px;height:160px;background:#0f3460;border-radius:50%;
  margin:0 auto;position:relative;touch-action:none}
.tilt-dot{width:20px;height:20px;background:#e94560;border-radius:50%;
  position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);
  pointer-events:none;transition:left .05s,top .05s}

/* Mood buttons */
.mood-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:6px}
.mood-btn{background:#0f3460;border:none;border-radius:8px;padding:10px 4px;
  font-size:1.2em;cursor:pointer;transition:background .1s}
.mood-btn:active{background:#e94560}

/* Status */
.status{font-size:.75em;color:#666;text-align:center;padding:8px}
</style>
</head>
<body>

<h1>&#x1F916; Quadruped Bot</h1>

<!-- Movement D-Pad -->
<div class="section">
  <div class="section-title">Movement</div>
  <div class="dpad">
    <div></div>
    <button class="dpad-btn" ontouchstart="move('forward')" ontouchend="move('stop')"
            onmousedown="move('forward')" onmouseup="move('stop')">&#9650;</button>
    <div></div>
    <button class="dpad-btn" ontouchstart="turn('left')" ontouchend="move('stop')"
            onmousedown="turn('left')" onmouseup="move('stop')">&#9664;</button>
    <button class="dpad-btn dpad-stop" onclick="estop()">STOP</button>
    <button class="dpad-btn" ontouchstart="turn('right')" ontouchend="move('stop')"
            onmousedown="turn('right')" onmouseup="move('stop')">&#9654;</button>
    <div></div>
    <button class="dpad-btn" ontouchstart="move('backward')" ontouchend="move('stop')"
            onmousedown="move('backward')" onmouseup="move('stop')">&#9660;</button>
    <div></div>
  </div>
  <div class="slider-row">
    <span class="slider-label">Speed</span>
    <input type="range" id="speed" min="0" max="100" value="50" oninput="updateSpeed()">
    <span class="slider-val" id="speed-val">50%</span>
  </div>
</div>

<!-- Preset Poses -->
<div class="section">
  <div class="section-title">Poses</div>
  <div class="btn-grid">
    <button class="btn" onclick="pose('stand')">Stand</button>
    <button class="btn" onclick="pose('sit')">Sit</button>
    <button class="btn" onclick="pose('lay')">Lay Down</button>
    <button class="btn" onclick="pose('wave')">Wave</button>
    <button class="btn" onclick="pose('pushup')">Push-up</button>
    <button class="btn" onclick="pose('dance')">Dance</button>
  </div>
</div>

<!-- Body Tilt -->
<div class="section">
  <div class="section-title">Body Tilt</div>
  <div style="display:flex;align-items:center;justify-content:space-around">
    <div>
      <div class="tilt-pad" id="tilt-pad">
        <div class="tilt-dot" id="tilt-dot"></div>
      </div>
      <div style="text-align:center;margin-top:6px">
        <span class="slider-val" id="tilt-val">P:0° R:0°</span>
      </div>
    </div>
    <div style="width:120px">
      <div class="slider-row" style="flex-direction:column;align-items:stretch">
        <span class="slider-label" style="text-align:center">Height</span>
        <input type="range" id="height" min="20" max="48" value="48"
               orient="vertical" style="writing-mode:bt-lr;height:120px;width:30px;margin:0 auto"
               oninput="updateHeight()">
        <span class="slider-val" style="text-align:center" id="height-val">48mm</span>
      </div>
    </div>
  </div>
</div>

<!-- Mood -->
<div class="section">
  <div class="section-title">Mood</div>
  <div class="mood-grid">
    <button class="mood-btn" onclick="mood('happy')">&#128522;</button>
    <button class="mood-btn" onclick="mood('sleepy')">&#128564;</button>
    <button class="mood-btn" onclick="mood('angry')">&#128545;</button>
    <button class="mood-btn" onclick="mood('surprised')">&#128562;</button>
    <button class="mood-btn" onclick="mood('sad')">&#128546;</button>
    <button class="mood-btn" onclick="mood('love')">&#128525;</button>
    <button class="mood-btn" onclick="mood('idle')">&#128528;</button>
  </div>
</div>

<!-- E-Stop -->
<div class="section">
  <button class="btn btn-danger" style="width:100%;padding:18px;font-size:1.1em"
          onclick="estop()">&#x26A0; EMERGENCY STOP</button>
</div>

<div class="status" id="status">Connecting...</div>

<script>
const API = '';  // Same origin
let speed = 0.5;

function api(endpoint, data) {
  fetch(API + '/api/' + endpoint, {
    method: data ? 'POST' : 'GET',
    headers: data ? {'Content-Type': 'application/json'} : {},
    body: data ? JSON.stringify(data) : null
  })
  .then(r => r.json())
  .then(d => { document.getElementById('status').textContent = d.message || 'OK'; })
  .catch(e => { document.getElementById('status').textContent = 'Error: ' + e; });
}

function move(dir) {
  if (dir === 'stop') { api('move', {direction:'stop',speed:0}); }
  else { api('move', {direction:dir, speed:speed}); }
}

function turn(dir) { api('turn', {direction:dir, speed:speed}); }
function pose(p) { api('pose', {pose:p}); }
function mood(m) { api('mood', {mood:m}); }
function estop() { api('estop', {}); }

function updateSpeed() {
  let v = document.getElementById('speed').value;
  speed = v / 100;
  document.getElementById('speed-val').textContent = v + '%';
}

function updateHeight() {
  let v = document.getElementById('height').value;
  document.getElementById('height-val').textContent = v + 'mm';
  api('height', {height: parseFloat(v)});
}

// Tilt pad touch handling
const pad = document.getElementById('tilt-pad');
const dot = document.getElementById('tilt-dot');
let tiltActive = false;

function handleTilt(e) {
  e.preventDefault();
  const rect = pad.getBoundingClientRect();
  const touch = e.touches ? e.touches[0] : e;
  let x = (touch.clientX - rect.left) / rect.width * 2 - 1;  // -1 to 1
  let y = (touch.clientY - rect.top) / rect.height * 2 - 1;
  x = Math.max(-1, Math.min(1, x));
  y = Math.max(-1, Math.min(1, y));
  // Clamp to circle
  const dist = Math.sqrt(x*x + y*y);
  if (dist > 1) { x /= dist; y /= dist; }
  dot.style.left = ((x+1)/2*100) + '%';
  dot.style.top = ((y+1)/2*100) + '%';
  const roll = Math.round(x * 15);
  const pitch = Math.round(y * 15);
  document.getElementById('tilt-val').textContent = 'P:'+pitch+'° R:'+roll+'°';
  api('tilt', {pitch: pitch, roll: roll});
}

function resetTilt() {
  tiltActive = false;
  dot.style.left = '50%';
  dot.style.top = '50%';
  document.getElementById('tilt-val').textContent = 'P:0° R:0°';
  api('tilt', {pitch:0, roll:0});
}

pad.addEventListener('mousedown', (e) => { tiltActive=true; handleTilt(e); });
pad.addEventListener('mousemove', (e) => { if(tiltActive) handleTilt(e); });
pad.addEventListener('mouseup', resetTilt);
pad.addEventListener('mouseleave', () => { if(tiltActive) resetTilt(); });
pad.addEventListener('touchstart', (e) => { tiltActive=true; handleTilt(e); });
pad.addEventListener('touchmove', (e) => { if(tiltActive) handleTilt(e); });
pad.addEventListener('touchend', resetTilt);

// Poll status
setInterval(() => {
  fetch(API + '/api/status').then(r=>r.json()).then(d => {
    document.getElementById('status').textContent = 'Connected | ' + JSON.stringify(d);
  }).catch(() => {
    document.getElementById('status').textContent = 'Disconnected';
  });
}, 2000);
</script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
