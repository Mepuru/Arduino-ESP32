#pragma once

// 嵌入式 Web 页面 — 手机浏览器访问 ESP32 IP 时显示
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>BLE Door Lock</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0f0f1a;color:#fff;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:20px}
.card{background:#1a1a2e;border-radius:16px;padding:24px;width:100%;max-width:400px;margin-bottom:16px;box-shadow:0 4px 24px rgba(0,0,0,.3)}
h1{font-size:22px;font-weight:600;text-align:center;margin-bottom:8px;color:#e0e0ff}
.subtitle{text-align:center;font-size:13px;color:#888;margin-bottom:16px}
.status-item{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #2a2a3e;font-size:14px}
.status-item:last-child{border-bottom:none}
.label{color:#888}
.value{color:#e0e0ff;font-weight:500;text-align:right;max-width:60%;word-break:break-all}
.btn{display:block;width:100%;padding:18px;border:none;border-radius:12px;font-size:20px;font-weight:600;cursor:pointer;transition:all .2s;text-align:center;text-decoration:none}
.btn-unlock{background:linear-gradient(135deg,#00c853,#009624);color:#fff}
.btn-unlock:active{transform:scale(.97)}
.btn-unlock:disabled{background:#555;cursor:not-allowed;transform:none}
.btn-refresh{background:#2a2a3e;color:#aaa;font-size:14px;padding:12px;margin-top:8px}
.btn-refresh:active{background:#3a3a4e}
.result{text-align:center;padding:12px;border-radius:10px;font-size:16px;font-weight:500;margin-top:12px}
.result-ok{background:#00c85322;color:#00c853;border:1px solid #00c85344}
.result-fail{background:#ff174422;color:#ff1744;border:1px solid #ff174444}
.result-idle{background:#2a2a3e;color:#888}
.spinner{display:inline-block;width:20px;height:20px;border:3px solid #fff3;border-top-color:#fff;border-radius:50%;animation:spin .8s linear infinite;margin-right:8px;vertical-align:middle}
@keyframes spin{to{transform:rotate(360deg)}}
</style>
</head>
<body>
<div class="card">
  <h1>🔐 BLE Door Lock</h1>
  <p class="subtitle" id="devName">加载中...</p>
  <div id="status"></div>
</div>

<div class="card">
  <button class="btn btn-unlock" id="btnUnlock" onclick="doUnlock()">🔓 开锁</button>
  <button class="btn btn-refresh" onclick="refresh()">刷新状态</button>
  <div id="result" class="result result-idle">就绪</div>
</div>

<script>
async function refresh(){
  const r=await fetch('/api/status');
  const d=await r.json();
  document.getElementById('status').innerHTML=
    '<div class="status-item"><span class="label">设备</span><span class="value">'+d.device+'</span></div>'+
    '<div class="status-item"><span class="label">状态</span><span class="value">'+(d.locked?'🔒 锁定':'🔓 已开')+'</span></div>'+
    '<div class="status-item"><span class="label">WiFi</span><span class="value">'+d.wifi+'</span></div>'+
    '<div class="status-item"><span class="label">IP</span><span class="value">'+d.ip+'</span></div>';
  document.getElementById('devName').textContent=d.device||'无门锁';
}
let unlockPoll=null;
async function doUnlock(){
  const btn=document.getElementById('btnUnlock');
  btn.disabled=true;

  // 发送开锁请求 (阻塞 ~30 秒)
  const r=await fetch('/unlock');
  const d=await r.json();

  btn.disabled=false;
  refresh();
}

// 持续轮询状态 (进度/倒计时/回锁/闲置)
setInterval(async()=>{
  try{
    const r=await fetch('/api/status');
    const d=await r.json();
    const p=d.result||'';
    const res=document.getElementById('result');
    const btn=document.getElementById('btnUnlock');
    if(!p){
      res.className='result result-idle';
      res.innerHTML='就绪';
    }else if(p.startsWith('OK')){
      res.className='result result-ok';
      res.innerHTML='✅ '+p;
    }else if(p.startsWith('FAIL')||p.startsWith('ERR')){
      res.className='result result-fail';
      res.innerHTML='❌ '+p;
    }else{
      res.className='result result-idle';
      res.innerHTML='<span class="spinner"></span>'+p;
    }
    // 有操作进行时禁用按钮
    btn.disabled=!!p;
    refresh();
  }catch(e){}
},1000);
refresh();
</script>
<div class="card" style="text-align:center;padding:12px">
  <a href="/settings" style="color:#888;font-size:13px;text-decoration:none">⚙ 设置</a>
</div>
</body>
</html>
)rawliteral";

static const char SETTINGS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>Settings - BLE Lock</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0f0f1a;color:#fff;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:20px}
.card{background:#1a1a2e;border-radius:16px;padding:24px;width:100%;max-width:400px;margin-bottom:16px}
h1{font-size:22px;font-weight:600;margin-bottom:16px;color:#e0e0ff}
label{display:block;font-size:14px;color:#888;margin-bottom:6px}
textarea{width:100%;padding:12px;border:1px solid #3a3a4e;border-radius:10px;background:#0f0f1a;color:#e0e0ff;font-size:13px;font-family:monospace;resize:vertical;min-height:80px;box-sizing:border-box}
textarea:focus{outline:none;border-color:#00c853}
.btn{display:block;width:100%;padding:14px;border:none;border-radius:12px;font-size:16px;font-weight:600;cursor:pointer;margin-top:16px}
.btn-save{background:linear-gradient(135deg,#00c853,#009624);color:#fff}
.btn-back{background:#2a2a3e;color:#aaa;font-size:14px;padding:12px;margin-top:8px;text-align:center;text-decoration:none;display:block;border-radius:10px}
.result{padding:10px;border-radius:8px;font-size:14px;margin-top:12px;display:none}
.result-ok{display:block;background:#00c85322;color:#00c853;border:1px solid #00c85344}
.result-fail{display:block;background:#ff174422;color:#ff1744;border:1px solid #ff174444}
.spinner{display:inline-block;width:16px;height:16px;border:2px solid #fff3;border-top-color:#fff;border-radius:50%;animation:spin .8s linear infinite;margin-right:6px;vertical-align:middle}
@keyframes spin{to{transform:rotate(360deg)}}
</style>
</head>
<body>
<div class="card">
  <h1>⚙ 设置</h1>
  <label for="token">Token ID（抓包获取后粘贴到这里）</label>
  <textarea id="token" placeholder="粘贴 tokenId..."></textarea>
  <button class="btn btn-save" id="btnSave" onclick="saveToken()">💾 保存</button>
  <div id="result"></div>
  <a href="/" class="btn-back">← 返回</a>
</div>
<script>
async function saveToken(){
  const btn=document.getElementById('btnSave');
  const res=document.getElementById('result');
  const token=document.getElementById('token').value.trim();
  if(token.length<4){res.className='result result-fail';res.innerHTML='token 太短';return}
  btn.disabled=true;
  res.className='result result-idle';
  res.innerHTML='<span class="spinner"></span>保存中...';
  const r=await fetch('/api/save_token',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'token='+encodeURIComponent(token)});
  const d=await r.json();
  if(d.ok){
    res.className='result result-ok';
    res.innerHTML='✅ '+d.msg;
  }else{
    res.className='result result-fail';
    res.innerHTML='❌ '+d.msg;
  }
  btn.disabled=false;
}
// 加载当前 token（只显示后几位）
fetch('/api/status').then(r=>r.json()).then(d=>{
  if(d.token_preview) document.getElementById('token').placeholder='当前: ...'+d.token_preview;
});
</script>
</body>
</html>
)rawliteral";
