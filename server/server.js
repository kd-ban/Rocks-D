const http = require('http');
const zlib = require('zlib');
const crypto = require('crypto');

const PORT = Number(process.env.PORT || 80);
const HOST = process.env.HOST || '0.0.0.0';
const PUBLIC_URL = process.env.PUBLIC_URL || 'http://192.168.8.59/pirate/public';

// Recovered from libgame.so -> Global::getMD5(std::string, int)
const MD5_SALT_NORMAL = '66fcc8ff4f3da51e6f62a9d98e288830';
const MD5_SALT_SSO = 'dd09232c3c1c3f3c376dd83ddc0eb58c';

const guests = new Map();
const sessions = new Map();

function md5(s) {
  return crypto.createHash('md5').update(Buffer.from(s, 'utf8')).digest('hex');
}

function gameMD5(text, mode = 0) {
  // Native behavior:
  // mode == 1 -> append SSO salt
  // mode <= 1 and != 1 -> append normal salt
  // mode > 1 -> plain MD5
  if (mode === 1) return md5(text + MD5_SALT_SSO);
  if (mode <= 1) return md5(text + MD5_SALT_NORMAL);
  return md5(text);
}

function makeUid(deviceId = '') {
  const seed = `${deviceId}|${Date.now()}|${Math.random()}`;
  return `guest_${md5(seed).slice(0, 16)}`;
}

function makeSession(uid) {
  return `sid_${md5(`${uid}|${Date.now()}|${Math.random()}`).slice(0, 24)}`;
}

function serverList() {
  return {
    '1': {
      id: '1',
      url: PUBLIC_URL,
      serverName: 'Rocks-D',
      status: 1,
      sort: 999
    }
  };
}

function parseParams(searchParams) {
  const raw = searchParams.get('params');
  if (!raw) return [];
  try { return JSON.parse(raw); } catch (_) { return []; }
}

function sendGameResponse(req, res, obj) {
  const json = JSON.stringify(obj);

  // netWork.lua appends userdata.sessionId before Global.getMD5()
  // for authenticated game responses. The client sends the same value as sid header.
  const sid = req.headers.sid || '';
  const signature = gameMD5(json + sid, 0);
  const zipped = zlib.gzipSync(Buffer.from(json, 'utf8'));
  const body = Buffer.concat([Buffer.from(signature, 'ascii'), zipped]);

  res.writeHead(200, {
    'Content-Type': 'application/octet-stream',
    'Content-Length': body.length,
    'Cache-Control': 'no-store'
  });
  res.end(body);
}

function ok(info = {}) {
  return { code: 200, info };
}

function handleAction(req, res, url) {
  const action = url.searchParams.get('action') || '';
  const params = parseParams(url.searchParams);
  const aid = req.headers.aid || '';
  const serverCode = req.headers.servercode || req.headers.serverCode || '1';

  console.log(new Date().toISOString(), action, { params, aid, serverCode, sid: req.headers.sid || '' });

  switch (action) {
    case 'sGenerateUid': {
      const deviceId = String(params[0] || 'android');
      const uid = makeUid(deviceId);
      const userName = `Guest-${uid.slice(-6)}`;
      guests.set(uid, { uid, userName, createdAt: Date.now() });
      return sendGameResponse(req, res, ok({ uid, userName }));
    }

    case 'sLogin':
    case 'sLoginNew': {
      const uid = String(params[0] || makeUid('fallback'));
      if (!guests.has(uid)) guests.set(uid, { uid, userName: `Guest-${uid.slice(-6)}` });
      const session = makeSession(uid);
      sessions.set(session, uid);
      return sendGameResponse(req, res, ok({
        uid,
        session,
        userName: guests.get(uid).userName,
        server_list: serverList(),
        serverCode: { '0': '1' }
      }));
    }

    case 'getServerListByVersion':
    case 'public_getServerList':
    case 'getServerList': {
      return sendGameResponse(req, res, ok({
        server_list: serverList(),
        serverCode: { '0': '1' }
      }));
    }

    case 'getServerTime': {
      return sendGameResponse(req, res, ok({ time: Math.floor(Date.now() / 1000) }));
    }

    case 'getLatestAppVersion':
    case 'public_getVersionInfo': {
      return sendGameResponse(req, res, ok({ version: '1.0.1', force: 0 }));
    }

    case 'login':
    case 'testlogin': {
      // This proves the network/login protocol is working. Full world/player data
      // still needs to be reconstructed before entering gameplay safely.
      const uid = Array.isArray(params[0]) ? String(params[0][0] || '') : '';
      return sendGameResponse(req, res, {
        code: 1207,
        info: uid || 'new_user'
      });
    }

    case 'register':
    case 'pickName':
    case 'getViewer':
    case 'getSetting': {
      return sendGameResponse(req, res, ok({}));
    }

    default:
      return sendGameResponse(req, res, { code: 9999, info: `Unsupported action: ${action}` });
  }
}

const server = http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);

  if (url.pathname === '/health') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    return res.end(JSON.stringify({ ok: true, service: 'Rocks-D', publicUrl: PUBLIC_URL }));
  }

  // Client target: http://192.168.8.59/pirate/public?action=...
  if (url.pathname === '/pirate/public' || url.pathname === '/pirate/public/' || url.pathname === '/public' || url.pathname === '/public/') {
    return handleAction(req, res, url);
  }

  res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
  res.end('Rocks-D private server\n');
});

server.listen(PORT, HOST, () => {
  console.log(`Rocks-D server listening on http://${HOST}:${PORT}`);
  console.log(`Client endpoint: ${PUBLIC_URL}`);
});
