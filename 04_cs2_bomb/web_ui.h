#pragma once

// ============ Web 控制页面 ============

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>C4 Bomb</title>
<style>
  *{margin:0;padding:0;box-sizing:border-box;-webkit-tap-highlight-color:transparent;}
  body{
    background:#090a0b;
    color:#ccc;
    font-family:'Courier New',monospace;
    display:flex;
    justify-content:center;
    align-items:center;
    min-height:100vh;min-height:100dvh;
  }
  .card{
    background:#111214;
    border:1px solid #2a2c2e;
    border-radius:10px;
    padding:22px 20px 18px;
    width:340px;max-width:96vw;
    text-align:center;
  }
  .title{
    font-size:20px;letter-spacing:8px;
    color:#aaa;margin-bottom:14px;
  }

  /* Status */
  .status-row{
    display:flex;align-items:center;justify-content:center;
    gap:8px;margin-bottom:8px;
    font-size:14px;letter-spacing:2px;text-transform:uppercase;
  }
  .dot{
    width:8px;height:8px;border-radius:50%;display:inline-block;
    transition:background .3s;
  }
  .dot-idle{background:#555;}
  .dot-armed{background:#ff5a2a;animation:pulse .8s infinite;}
  .dot-exploded{background:#e02020;}
  .dot-defused{background:#2ecc71;}
  @keyframes pulse{0%,100%{opacity:.1;}50%{opacity:1;}}

  /* Timer */
  .timer{
    font-size:60px;font-weight:bold;
    color:#ff5a2a;margin:6px 0 10px;
    letter-spacing:2px;font-family:'Courier New',monospace;line-height:1;
  }
  .timer-defused{color:#2ecc71;}
  .timer-idle{color:#555;}

  /* Progress bar */
  .bar-bg{
    width:100%;height:6px;
    background:#1e1f21;border-radius:3px;
    overflow:hidden;margin-bottom:14px;
  }
  .bar-fill{height:100%;border-radius:3px;transition:width 1s linear;}

  /* Mode tabs */
  .mode-tabs{
    display:flex;gap:0;margin-bottom:14px;
    border:1px solid #2a2c2e;border-radius:6px;overflow:hidden;
  }
  .tab{
    flex:1;padding:8px 0;border:none;background:transparent;
    color:#555;font-size:13px;font-family:'Courier New',monospace;
    letter-spacing:3px;cursor:pointer;text-transform:uppercase;
    transition:all .2s;
  }
  .tab:not(.tab-active):hover{color:#888;}
  .tab-active{background:#ff5a2a;color:#fff;}
  .tab-active.defuse-tab{background:#2980b9;}

  /* Keypad */
  .mode-label{
    font-size:11px;color:#555;letter-spacing:4px;
    text-transform:uppercase;margin-bottom:10px;
  }
  .mode-label span{color:#ff5a2a;}
  .mode-label.defuse span{color:#2980b9;}
  .keypad{max-width:260px;margin:0 auto;}
  .pin-row{
    display:flex;justify-content:center;gap:10px;
    margin-bottom:16px;min-height:40px;
  }
  .pin-slot{
    width:32px;height:40px;
    background:#0a0a0a;
    border:1px solid #2a2c2e;border-radius:4px;
    display:flex;align-items:center;justify-content:center;
    font-size:20px;color:#ff5a2a;
    transition:border-color .2s,box-shadow .2s;
  }
  .pin-slot.filled{
    border-color:#ff5a2a;
    box-shadow:0 0 6px rgba(255,90,42,.25);
  }
  .pin-slot.defuse-mode{border-color:#2980b9;}
  .pin-slot.defuse-mode.filled{
    border-color:#2980b9;
    box-shadow:0 0 6px rgba(41,128,185,.25);
  }
  .key-grid{
    display:grid;grid-template-columns:repeat(3,1fr);gap:8px;
  }
  .key{
    aspect-ratio:1.5;border:none;border-radius:5px;
    background:#1a1b1d;color:#ddd;font-size:20px;
    font-family:'Courier New',monospace;font-weight:bold;
    cursor:pointer;border:1px solid #2a2c2e;
    transition:background .1s,transform .1s;
    display:flex;align-items:center;justify-content:center;
    -webkit-user-select:none;user-select:none;touch-action:manipulation;
  }
  .key:active{background:#2a2c2e;transform:scale(.94);}
  .key:disabled{opacity:.2;cursor:default;transform:none;}
  .key:disabled:active{transform:none;background:#1a1b1d;}
  .key-zero{grid-column:2;}
  .key-del{font-size:12px;letter-spacing:1px;color:#777;aspect-ratio:auto;}
  .key-ok{
    background:#2a1a12;border-color:#ff5a2a;color:#ff5a2a;aspect-ratio:auto;
  }
  .key-ok:active{background:#3a2a1a;}
  .key-ok.defuse-mode{
    background:#12202a;border-color:#2980b9;color:#2980b9;
  }
  .key-ok.defuse-mode:active{background:#1a3040;}

  /* ===== Game ===== */
  .game-prompt{
    font-size:16px;font-weight:bold;letter-spacing:2px;
    margin-bottom:12px;min-height:24px;
  }
  .game-wires{
    display:grid;grid-template-columns:1fr 1fr;gap:10px;
    margin-bottom:14px;
  }
  .wire-btn{
    padding:22px 8px;border:none;border-radius:8px;
    cursor:pointer;font-size:14px;font-weight:bold;
    font-family:'Courier New',monospace;
    color:#fff;text-shadow:0 1px 3px rgba(0,0,0,.6);
    letter-spacing:2px;text-transform:uppercase;
    transition:transform .1s,opacity .15s;
    touch-action:manipulation;
    position:relative;overflow:hidden;
  }
  .wire-btn::after{
    content:'';position:absolute;inset:0;
    background:linear-gradient(180deg,rgba(255,255,255,.12) 0%,transparent 50%);
  }
  .wire-btn:active{transform:scale(.92);}
  .wire-btn:disabled{opacity:.25;cursor:default;transform:none;}
  .wire-btn:disabled:active{transform:none;}
  .wire-red{background:#c0392b;}
  .wire-blue{background:#2471a3;}
  .wire-green{background:#1e8449;}
  .wire-yellow{background:#b7950b;color:#222;text-shadow:0 1px 2px rgba(255,255,255,.3);}
  .wire-white{background:#bdc3c7;color:#222;text-shadow:0 1px 2px rgba(255,255,255,.3);}
  .wire-purple{background:#7d3c98;}

  /* Round timer */
  .round-timer{
    display:flex;align-items:center;gap:8px;
    margin-bottom:10px;
  }
  .rt-bar-bg{
    flex:1;height:6px;background:#1e1f21;
    border-radius:3px;overflow:hidden;
  }
  .rt-bar-fill{
    height:100%;border-radius:3px;
    background:#ff5a2a;width:100%;
    transition:width .1s linear;
  }
  .rt-bar-fill.urgent{background:#e74c3c;}
  .rt-text{
    font-size:15px;font-weight:bold;color:#aaa;
    min-width:28px;text-align:right;font-variant-numeric:tabular-nums;
  }
  .rt-text.urgent{color:#e74c3c;}

  .game-score{
    font-size:15px;color:#aaa;letter-spacing:2px;
    margin-bottom:10px;
  }
  .game-score span{color:#ff5a2a;font-size:18px;}
  .game-score.defuse-score span{color:#2980b9;}

  .game-msg{
    padding:8px;border-radius:5px;
    font-size:13px;font-weight:bold;letter-spacing:2px;
    margin-bottom:10px;min-height:34px;
    background:transparent;color:transparent;
    transition:background .2s,color .2s;
  }
  .game-msg-correct{background:#0a1f0a;color:#2ecc71;}
  .game-msg-wrong{background:#1f0a0a;color:#e74c3c;}

  .game-back{
    width:100%;padding:9px;margin-top:4px;
    background:transparent;border:1px solid #2a2c2e;border-radius:5px;
    color:#555;font-size:12px;font-family:'Courier New',monospace;
    letter-spacing:2px;cursor:pointer;text-transform:uppercase;
    transition:color .2s,border-color .2s;
  }
  .game-back:hover{color:#888;border-color:#3a3c3e;}
  .game-back:active{background:#1a1b1d;}

  /* Result (exploded/defused) */
  .msg{
    margin:14px 0 10px;padding:10px;border-radius:5px;
    font-size:14px;font-weight:bold;letter-spacing:3px;min-height:20px;
  }
  .msg-ok{background:#0a1f0a;color:#2ecc71;border:1px solid #1a3a1a;}
  .msg-err{background:#1f0a0a;color:#e74c3c;border:1px solid #3a1a1a;}

  /* Reset */
  .btn-reset{
    display:block;width:100%;padding:10px;margin-top:10px;
    background:#1a1b1d;border:1px solid #2a2c2e;border-radius:5px;
    color:#777;font-size:13px;font-family:'Courier New',monospace;
    letter-spacing:3px;text-transform:uppercase;cursor:pointer;
    transition:background .2s,color .2s;
  }
  .btn-reset:hover{background:#222;color:#aaa;}
  .btn-reset:active{background:#2a2c2e;}

  /* Language toggle */
  .lang-toggle{
    font-size:11px;color:#555;cursor:pointer;
    letter-spacing:1px;-webkit-user-select:none;user-select:none;
    margin-bottom:6px;
  }
  .lang-toggle span{padding:2px 4px;transition:color .2s;}
  .lang-toggle .lang-active{color:#aaa;}
  .lang-toggle .lang-sep{color:#333;cursor:default;}

  .footer{
    margin-top:14px;font-size:10px;color:#333;letter-spacing:1px;
  }

  .hidden{display:none !important;}
</style>
</head>
<body>

<div class="card">
  <div class="lang-toggle" onclick="toggleLang()">
    <span id="l-en" class="lang-active">EN</span>
    <span class="lang-sep">|</span>
    <span id="l-zh">中文</span>
  </div>

  <div class="title">C4 BOMB</div>

  <!-- Status row -->
  <div class="status-row">
    <span id="dot" class="dot dot-idle"></span>
    <span id="status-text">IDLE</span>
  </div>

  <!-- Timer -->
  <div id="timer" class="timer timer-idle">--:--</div>

  <!-- Progress bar -->
  <div class="bar-bg">
    <div id="bar-fill" class="bar-fill" style="width:0%;background:#555;"></div>
  </div>

  <!-- Result -->
  <div id="result-msg" class="msg hidden"></div>

  <!-- ===== Mode tabs (ARMED only) ===== -->
  <div id="mode-tabs" class="mode-tabs hidden">
    <button class="tab tab-active" id="tab-pin" onclick="switchMode('pin')">PIN</button>
    <button class="tab" id="tab-game" onclick="switchMode('game')">GAME</button>
  </div>

  <!-- ===== PIN mode ===== -->
  <div id="pin-mode">
    <div id="mode-label" class="mode-label">ARM CODE <span>PLANT</span></div>

    <div class="keypad">
      <div class="pin-row" id="pin-row">
        <div class="pin-slot" id="pin-0"></div>
        <div class="pin-slot" id="pin-1"></div>
        <div class="pin-slot" id="pin-2"></div>
        <div class="pin-slot" id="pin-3"></div>
        <div class="pin-slot" id="pin-4"></div>
        <div class="pin-slot" id="pin-5"></div>
        <div class="pin-slot" id="pin-6"></div>
      </div>
      <div class="key-grid">
        <button class="key" data-d="1" onclick="pressDigit(1)">1</button>
        <button class="key" data-d="2" onclick="pressDigit(2)">2</button>
        <button class="key" data-d="3" onclick="pressDigit(3)">3</button>
        <button class="key" data-d="4" onclick="pressDigit(4)">4</button>
        <button class="key" data-d="5" onclick="pressDigit(5)">5</button>
        <button class="key" data-d="6" onclick="pressDigit(6)">6</button>
        <button class="key" data-d="7" onclick="pressDigit(7)">7</button>
        <button class="key" data-d="8" onclick="pressDigit(8)">8</button>
        <button class="key" data-d="9" onclick="pressDigit(9)">9</button>
        <button class="key key-del" id="key-del" onclick="pressDel()">DEL</button>
        <button class="key key-zero" data-d="0" onclick="pressDigit(0)">0</button>
        <button class="key key-ok" id="key-ok" onclick="pressOk()">OK</button>
      </div>
    </div>
  </div>

  <!-- ===== Game mode ===== -->
  <div id="game-mode" class="hidden">
    <div id="game-prompt" class="game-prompt">Cut the RED wire!</div>

    <!-- Round timer -->
    <div class="round-timer">
      <div class="rt-bar-bg">
        <div id="rt-bar" class="rt-bar-fill"></div>
      </div>
      <div id="rt-text" class="rt-text">8s</div>
    </div>

    <div class="game-wires" id="game-wires">
      <button class="wire-btn wire-red"    onclick="cutWire('red')"    id="w-red"></button>
      <button class="wire-btn wire-blue"   onclick="cutWire('blue')"   id="w-blue"></button>
      <button class="wire-btn wire-green"  onclick="cutWire('green')"  id="w-green"></button>
      <button class="wire-btn wire-yellow" onclick="cutWire('yellow')" id="w-yellow"></button>
      <button class="wire-btn wire-white"  onclick="cutWire('white')"  id="w-white"></button>
      <button class="wire-btn wire-purple" onclick="cutWire('purple')" id="w-purple"></button>
    </div>

    <div class="game-score" id="game-score">Score: <span>0</span> / 5</div>
    <div id="game-msg" class="game-msg"></div>

    <button class="game-back" onclick="abortGame()">BACK TO PIN</button>
  </div>

  <!-- ===== Reset ===== -->
  <button class="btn-reset hidden" id="btn-reset" onclick="doReset()">RESTART</button>

  <div class="footer" id="footer"></div>
</div>

<script>
// ===== State =====
let currentState = 'IDLE';
let lastState = '';
let pinDigits = [];
const MAX_PIN = 7;
let actionMode = 'plant';
let submitting = false;
let defuseMode = 'pin';  // 'pin' or 'game'

// ===== Language =====
let lang = 'en';

const L = {
  en: {
    score: 'Score: {0} / {1}',
    cutPrompt: 'Cut the {0} wire!',
    correct: 'Correct!',
    wrong: 'Wrong wire!',
    tooSlow: 'Too slow!',
    bombDefused: 'Bomb defused!',
    backToPin: 'BACK TO PIN',
    restart: 'RESTART',
    armLabel: 'ARM CODE <span>PLANT</span>',
    disarmLabel: 'DISARM CODE <span>DEFUSE</span>',
    pinTab: 'PIN', gameTab: 'GAME',
    idleMsg: 'Open browser to control',
    exploded: 'EXPLODED', defused: 'DEFUSED',
  },
  zh: {
    score: '分数: {0} / {1}',
    cutPrompt: '请剪{0}色的线!',
    correct: '正确!',
    wrong: '剪错了!',
    tooSlow: '太慢了!',
    bombDefused: '炸弹已拆除!',
    backToPin: '返回密码',
    restart: '重新开始',
    armLabel: '下包密码 <span>PLANT</span>',
    disarmLabel: '拆包密码 <span>DEFUSE</span>',
    pinTab: '密码', gameTab: '游戏',
    idleMsg: '打开浏览器控制',
    exploded: '已爆炸', defused: '已拆除',
  }
};

const COLOR_LABELS = {
  en: { red:'RED', blue:'BLU', green:'GRN', yellow:'YEL', white:'WHT', purple:'PUR' },
  zh: { red:'红', blue:'蓝', green:'绿', yellow:'黄', white:'白', purple:'紫' },
};

function toggleLang(){
  lang = (lang === 'en') ? 'zh' : 'en';
  qs('#l-en').classList.toggle('lang-active', lang === 'en');
  qs('#l-zh').classList.toggle('lang-active', lang === 'zh');
  updateStaticText();
  pollStatus();
  if(defuseMode === 'game') startGame();
}

function updateStaticText(){
  const tabs = qsa('.tab');
  if(tabs.length >= 2){
    tabs[0].textContent = txt('pinTab');
    tabs[1].textContent = txt('gameTab');
  }
  qs('#btn-reset').textContent = txt('restart');
  const backBtn = qs('.game-back');
  if(backBtn) backBtn.textContent = txt('backToPin');
}

function txt(key, ...args){
  let s = L[lang][key] || L['en'][key] || key;
  args.forEach((a,i) => { s = s.replace('{'+i+'}', a); });
  return s;
}

function qs(s){return document.querySelector(s);}
function qsa(s){return document.querySelectorAll(s);}

// ===== PIN entry =====
function updatePinDisplay(){
  for(let i=0;i<MAX_PIN;i++){
    const slot = qs('#pin-'+i);
    slot.textContent = i < pinDigits.length ? '\u25CF' : '';
    slot.classList.toggle('filled', i < pinDigits.length);
  }
  qs('#key-del').disabled = pinDigits.length === 0;
}

function pressDigit(d){
  if(submitting) return;
  if(pinDigits.length >= MAX_PIN) return;
  pinDigits.push(d);
  updatePinDisplay();
}

function pressDel(){
  if(submitting) return;
  if(pinDigits.length === 0) return;
  pinDigits.pop();
  updatePinDisplay();
}

function pressOk(){
  if(submitting) return;
  if(pinDigits.length !== MAX_PIN) return;

  submitting = true;
  const pw = pinDigits.join('');
  pinDigits = [];
  updatePinDisplay();

  const url = actionMode === 'plant' ? '/api/plant' : '/api/defuse';
  const form = new URLSearchParams();
  form.append('password', pw);

  fetch(url, {method:'POST', body:form})
    .then(r => r.json())
    .then(data => {
      if(!data.ok){
        const pinRow = qs('#pin-row');
        pinRow.style.animation = 'none';
        void pinRow.offsetWidth;
        pinRow.style.animation = 'shake .35s ease';
        setTimeout(()=>{pinRow.style.animation='';},400);
      }
      submitting = false;
      updatePinDisplay();
      pollStatus();
    })
    .catch(()=>{submitting=false;});
}

// ===== Mode switching =====
function switchMode(mode){
  defuseMode = mode;
  qs('#pin-mode').classList.toggle('hidden', mode !== 'pin');
  qs('#game-mode').classList.toggle('hidden', mode !== 'game');
  qs('#tab-pin').classList.toggle('tab-active', mode === 'pin');
  qs('#tab-game').classList.toggle('tab-active', mode === 'game');

  if(mode === 'game'){
    startGame();
  } else {
    abortGame();
  }
}

// ===== Game Timer =====
let roundTimer = null;
let roundTimeLimit = 8;
let roundTimeStart = 0;
let timerAnimFrame = null;

function startRoundTimer(limit){
  cancelRoundTimer();
  roundTimeLimit = limit;
  roundTimeStart = Date.now();

  const bar = qs('#rt-bar');
  const text = qs('#rt-text');
  bar.classList.remove('urgent');
  text.classList.remove('urgent');
  bar.style.width = '100%';
  text.textContent = limit + 's';

  // Timeout
  roundTimer = setTimeout(() => {
    onRoundTimeout();
  }, limit * 1000);

  // Animation loop
  function anim(){
    const elapsed = (Date.now() - roundTimeStart) / 1000;
    const left = Math.max(0, roundTimeLimit - elapsed);
    const pct = (left / roundTimeLimit) * 100;
    bar.style.width = pct + '%';
    text.textContent = Math.ceil(left) + 's';

    if(left < 3){
      bar.classList.add('urgent');
      text.classList.add('urgent');
    }

    if(left > 0){
      timerAnimFrame = requestAnimationFrame(anim);
    }
  }
  timerAnimFrame = requestAnimationFrame(anim);
}

function cancelRoundTimer(){
  if(roundTimer){ clearTimeout(roundTimer); roundTimer = null; }
  if(timerAnimFrame){ cancelAnimationFrame(timerAnimFrame); timerAnimFrame = null; }
}

function onRoundTimeout(){
  const bar = qs('#rt-bar');
  bar.style.width = '0%';
  // Send timeout as wrong answer
  cutWire('timeout');
}

// ===== Game =====
function startGame(){
  fetch('/api/game/start', {method:'POST'})
    .then(r => r.json())
    .then(data => {
      if(!data.ok) return;
      setGameRound(data);
    });
}

function cutWire(color){
  const btn = qs('#w-'+color);
  if(btn && btn.disabled) return;
  if(btn) btn.disabled = true;

  cancelRoundTimer();

  const form = new URLSearchParams();
  form.append('color', color);

  fetch('/api/game/cut', {method:'POST', body:form})
    .then(r => r.json())
    .then(data => {
      // Re-enable all wires
      qsa('.wire-btn').forEach(b => { if(b) b.disabled = false; });

      if(data.state === 'DEFUSED' || data.state === 'EXPLODED'){
        cancelRoundTimer();
        pollStatus();
        return;
      }

      // Show feedback message (translated, using data.correct boolean)
      const msgEl = qs('#game-msg');
      msgEl.classList.remove('game-msg-correct','game-msg-wrong');
      const isCorrect = data.correct === true;
      if(data.msg){
        // Map server msg to translated text
        const key = isCorrect ? 'correct' : (data.msg === 'Too slow!' ? 'tooSlow' : 'wrong');
        msgEl.textContent = txt(key);
        msgEl.classList.add(isCorrect ? 'game-msg-correct' : 'game-msg-wrong');
        clearTimeout(window._gameMsgTimer);
        window._gameMsgTimer = setTimeout(() => {
          msgEl.textContent = '';
          msgEl.classList.remove('game-msg-correct','game-msg-wrong');
        }, 500);
      } else {
        msgEl.textContent = '';
      }

      // Next round (with timer)
      setGameRound(data);
    })
    .catch(() => {
      qsa('.wire-btn').forEach(b => { if(b) b.disabled = false; });
    });
}

const WIRE_HEX = {
  red:'#e74c3c', blue:'#3498db', green:'#2ecc71',
  yellow:'#f1c40f', white:'#bdc3c7', purple:'#9b59b6'
};

function setGameRound(data){
  const colors = data.colors || [];
  const target = data.target_color || '';
  const labels = COLOR_LABELS[lang];

  // Reset prompt visibility
  const promptEl = qs('#game-prompt');
  promptEl.style.opacity = '1';
  clearTimeout(window._promptTimer);

  // Prompt: "Cut the RED wire!" with RED shown in a RANDOM color
  const targetLabel = labels[target] || target.toUpperCase();
  const otherColors = colors.filter(c => c !== target);
  const randomColor = otherColors[Math.floor(Math.random() * otherColors.length)];
  const randomHex = WIRE_HEX[randomColor];
  const prompt = qs('#game-prompt');
  prompt.innerHTML = txt('cutPrompt',
    '<span style="color:' + randomHex + '">' + targetLabel + '</span>');

  // No highlighting — all wires look the same
  qsa('.wire-btn').forEach(b => { b.style.opacity = '1'; });

  // Score
  const scoreEl = qs('#game-score');
  const scoreVal = data.score || 0;
  const targetVal = data.target || 7;
  scoreEl.innerHTML = txt('score', '<span>'+scoreVal+'</span>', targetVal);

  // Start round timer (faster: 7→6→5→4→3→3→3)
  const round = scoreVal + 1;
  const timeLimit = Math.max(3, 8 - round);
  startRoundTimer(timeLimit);

  // Prompt hides after 2.5s — test your memory
  clearTimeout(window._promptTimer);
  window._promptTimer = setTimeout(() => {
    const p = qs('#game-prompt');
    if(p && !p.textContent.startsWith('??')){
      p.textContent = '??';
      p.style.opacity = '0.3';
    }
  }, 2500);
}

function abortGame(){
  cancelRoundTimer();
  clearTimeout(window._promptTimer);
  fetch('/api/game/abort', {method:'POST'}).catch(()=>{});
  qs('#game-msg').textContent = '';
  qs('#game-msg').classList.remove('game-msg-correct','game-msg-wrong');
  qsa('.wire-btn').forEach(b => { if(b) b.disabled = false; });
}

// ===== Reset =====
function doReset(){
  pinDigits = [];
  submitting = false;
  updatePinDisplay();
  // 清除终态锁，让 pollStatus 能重新请求
  let prevState = currentState;
  currentState = '';
  fetch('/api/reset', {method:'POST'})
    .then(() => {
      timerSync.running = false;
      pollStatus();
    })
    .catch(() => { currentState = prevState; });
}

// ===== UI Update =====
function enableKeypad(en){
  qsa('.key[data-d]').forEach(b => b.disabled = !en);
  qs('#key-del').disabled = !en || pinDigits.length===0;
  qs('#key-ok').disabled = !en;
}

function updateUI(data){
  const stateChanged = data.state !== lastState;
  lastState = data.state;
  currentState = data.state;
  updateStaticText();

  const dot = qs('#dot');
  const statusText = qs('#status-text');
  const timer = qs('#timer');
  const barFill = qs('#bar-fill');
  const modeTabs = qs('#mode-tabs');
  const pinMode = qs('#pin-mode');
  const gameMode = qs('#game-mode');
  const modeLabel = qs('#mode-label');
  const resetBtn = qs('#btn-reset');
  const resultMsg = qs('#result-msg');
  const okBtn = qs('#key-ok');
  const pinSlots = qsa('.pin-slot');
  const tabs = qsa('.tab');

  // Reset all
  resultMsg.classList.add('hidden');
  pinMode.classList.remove('hidden');
  gameMode.classList.add('hidden');
  modeTabs.classList.add('hidden');
  resetBtn.classList.add('hidden');
  timer.classList.remove('timer-defused','timer-idle');
  okBtn.classList.remove('defuse-mode');
  pinSlots.forEach(s => s.classList.remove('defuse-mode'));
  tabs.forEach(t => t.classList.remove('defuse-tab'));

  if(data.state === 'IDLE'){
    dot.className = 'dot dot-idle';
    statusText.textContent = 'IDLE';
    timer.textContent = '--:--';
    timer.classList.add('timer-idle');
    barFill.style.width = '0%';
    barFill.style.background = '#555';
    modeLabel.innerHTML = txt('armLabel');
    modeLabel.classList.remove('defuse');
    actionMode = 'plant';
    enableKeypad(true);
    if(stateChanged && pinDigits.length) { pinDigits=[]; updatePinDisplay(); }
    defuseMode = 'pin';

  } else if(data.state === 'ARMED'){
    dot.className = 'dot dot-armed';
    statusText.textContent = 'ARMED';
    modeTabs.classList.remove('hidden');

    // Set label/tab based on current mode
    if(defuseMode === 'game'){
      pinMode.classList.add('hidden');
      gameMode.classList.remove('hidden');
      qs('#tab-pin').classList.remove('tab-active');
      qs('#tab-game').classList.add('tab-active');
    } else {
      modeLabel.innerHTML = txt('disarmLabel');
      modeLabel.classList.add('defuse');
      okBtn.classList.add('defuse-mode');
      pinSlots.forEach(s => s.classList.add('defuse-mode'));
      qs('#tab-pin').classList.add('tab-active');
      qs('#tab-game').classList.remove('tab-active');
      tabs.forEach(t => t.classList.add('defuse-tab'));
      actionMode = 'defuse';
      enableKeypad(true);
      if(stateChanged && pinDigits.length) { pinDigits=[]; updatePinDisplay(); }
    }
    resetBtn.classList.remove('hidden');

  } else if(data.state === 'EXPLODED'){
    dot.className = 'dot dot-exploded';
    statusText.textContent = 'EXPLODED';
    pinMode.classList.add('hidden');
    gameMode.classList.add('hidden');
    resetBtn.classList.remove('hidden');
    resultMsg.classList.remove('hidden');
    resultMsg.className = 'msg msg-err';
    resultMsg.textContent = txt('exploded');

  } else if(data.state === 'DEFUSED'){
    dot.className = 'dot dot-defused';
    statusText.textContent = 'DEFUSED';
    pinMode.classList.add('hidden');
    gameMode.classList.add('hidden');
    resetBtn.classList.remove('hidden');
    resultMsg.classList.remove('hidden');
    resultMsg.className = 'msg msg-ok';
    resultMsg.textContent = txt('defused');
  }
}

// ===== Timer sync: local interpolation =====
let timerSync = { time: 0, remaining: 0, total: 45, running: false };

function onTimerSync(remaining, total){
  // Guard: ignore sync if local estimate is close (avoid backward jitter)
  if(timerSync.running){
    const elapsed = (Date.now() - timerSync.time) / 1000;
    const local = timerSync.remaining - elapsed;
    if(Math.abs(local - remaining) < 1.5) return;
  }

  timerSync.time = Date.now();
  timerSync.remaining = remaining;
  timerSync.total = total;
  timerSync.running = true;
}

function getTimerDisplay(){
  if(!timerSync.running) return { remaining: 0, total: 45, idle: true };
  const elapsed = (Date.now() - timerSync.time) / 1000;
  const remaining = Math.max(0, timerSync.remaining - elapsed);
  return { remaining, total: timerSync.total, idle: false };
}

function updateTimerDisplay(){
  // Don't update in terminal states — frozen by updateUI
  if(currentState === 'DEFUSED' || currentState === 'EXPLODED') return;

  const { remaining, total, idle } = getTimerDisplay();
  if(idle) return;

  const disp = Math.round(remaining);

  const timer = qs('#timer');
  const barFill = qs('#bar-fill');

  let m = Math.floor(disp / 60);
  let s = disp % 60;
  timer.textContent = m + ':' + (s < 10 ? '0' : '') + s;

  let pct = total > 0 ? (remaining / total) * 100 : 0;
  barFill.style.width = Math.max(0, pct) + '%';
  if(pct > 66) barFill.style.background = '#e0e0e0';
  else if(pct > 33) barFill.style.background = '#f1c40f';
  else if(pct > 11) barFill.style.background = '#e67e22';
  else barFill.style.background = '#e74c3c';
}

function pollStatus(){
  // Terminal states: stop polling until user resets
  if(currentState === 'DEFUSED' || currentState === 'EXPLODED') return;

  fetch('/api/status')
    .then(r => r.json())
    .then(data => {
      updateUI(data);
      if(data.state === 'ARMED'){
        onTimerSync(data.remaining, data.total);
      } else {
        timerSync.running = false;
      }
    })
    .catch(()=>{});
}

// ===== Inject shake animation =====
const shakeStyle = document.createElement('style');
shakeStyle.textContent = '@keyframes shake{0%,100%{transform:translateX(0)}20%{transform:translateX(-8px)}40%{transform:translateX(8px)}60%{transform:translateX(-6px)}80%{transform:translateX(6px)}}';
document.head.appendChild(shakeStyle);

// ===== Start =====
setInterval(pollStatus, 500);
setInterval(updateTimerDisplay, 200);  // smooth local countdown
pollStatus();
</script>
</body>
</html>
)rawliteral";
