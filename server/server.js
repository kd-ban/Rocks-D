const http = require('http');
const zlib = require('zlib');
const crypto = require('crypto');

const PORT = Number(process.env.PORT || 80);
const HOST = process.env.HOST || '0.0.0.0';
const PUBLIC_URL = process.env.PUBLIC_URL || 'http://192.168.8.59/pirate/public';

// Private-server defaults. userdata.lua maps player.gold -> userdata.gold
// and player.silver -> userdata.berry.
const START_GOLD = 999999999;
const START_SILVER = 999999999;
const START_STRENGTH = 9999;
const START_ENERGY = 9999;

// Recovered from libgame.so -> Global::getMD5(std::string, int)
const MD5_SALT_NORMAL = '66fcc8ff4f3da51e6f62a9d98e288830';
const MD5_SALT_SSO = 'dd09232c3c1c3f3c376dd83ddc0eb58c';

const guests = new Map();
const sessions = new Map();
const players = new Map();

function md5(s) {
  return crypto.createHash('md5').update(Buffer.from(s, 'utf8')).digest('hex');
}
function gameMD5(text, mode = 0) {
  if (mode === 1) return md5(text + MD5_SALT_SSO);
  if (mode <= 1) return md5(text + MD5_SALT_NORMAL);
  return md5(text);
}
function makeUid(deviceId = '') {
  return `guest_${md5(`${deviceId}|${Date.now()}|${Math.random()}`).slice(0, 16)}`;
}
function makeSession(uid) {
  return `sid_${md5(`${uid}|${Date.now()}|${Math.random()}`).slice(0, 24)}`;
}
function serverList() {
  return {'1': {id:'1', url:PUBLIC_URL, serverName:'Rocks-D', status:1, sort:999}};
}
function parseParams(searchParams) {
  const raw = searchParams.get('params');
  if (!raw) return [];
  try { return JSON.parse(raw); } catch (_) { return []; }
}
function sendGameResponse(req, res, obj) {
  const json = JSON.stringify(obj);
  const sid = req.headers.sid || '';
  const signature = gameMD5(json + sid, 0);
  const zipped = zlib.gzipSync(Buffer.from(json, 'utf8'));
  const body = Buffer.concat([Buffer.from(signature, 'ascii'), zipped]);
  res.writeHead(200, {'Content-Type':'application/octet-stream','Content-Length':body.length,'Cache-Control':'no-store'});
  res.end(body);
}
function ok(info = {}) { return {code:200, info}; }

function makePlayer(uid, name) {
  return {
    id: Math.floor(Math.random()*9000000)+1000000,
    uid,
    name: name || `Guest-${uid.slice(-6)}`,
    level: 1,
    exp_now: 0,
    exp_all: 100,
    gold: START_GOLD,
    silver: START_SILVER,
    strength: START_STRENGTH,
    strength_time: Math.floor(Date.now()/1000),
    energy: START_ENERGY,
    energy_time: Math.floor(Date.now()/1000),
    flag: 0,
    form: {}, form_seven: {}, heros: {}, heroes_soul: {}, recruit: {},
    storys: {record:'', stageData:{}, pageData:{}, nextBatchTime:0},
    equips: {}, books: {}, items: {}, attrFix: {}, frags: {}, titles: {},
    login_succession_award: {}, vipScore: 0, vipItems: {}, vipCards: {},
    guideStep: 0, roster: {}, shakeData: {}, mails: {}, shadows: {}, shadowData: {},
    firstCashAward1: 0, firstCashAward2: 0, huntingTreasure: {}, equipShard: {},
    shareData: {}, monthCardData: {}
  };
}

function playerLoginInfo(player, sid) {
  return {
    sid,
    now: Math.floor(Date.now()/1000),
    newday: false,
    settingVersion: '1',
    bloodInfo: {},
    publicShareData: {},
    frontPage: {},
    player
  };
}

function handleAction(req, res, url) {
  const action = url.searchParams.get('action') || '';
  const params = parseParams(url.searchParams);
  console.log(new Date().toISOString(), action, {params, aid:req.headers.aid||'', serverCode:req.headers.servercode||'1', sid:req.headers.sid||''});

  switch (action) {
    case 'sGenerateUid': {
      const uid = makeUid(String(params[0] || 'android'));
      const userName = `Guest-${uid.slice(-6)}`;
      guests.set(uid,{uid,userName,createdAt:Date.now()});
      players.set(uid, makePlayer(uid,userName));
      return sendGameResponse(req,res,ok({uid,userName}));
    }
    case 'sLogin':
    case 'sLoginNew': {
      const uid = String(params[0] || makeUid('fallback'));
      if (!guests.has(uid)) guests.set(uid,{uid,userName:`Guest-${uid.slice(-6)}`});
      if (!players.has(uid)) players.set(uid,makePlayer(uid,guests.get(uid).userName));
      const session = makeSession(uid);
      sessions.set(session,uid);
      return sendGameResponse(req,res,ok({uid,session,userName:guests.get(uid).userName,server_list:serverList(),serverCode:{'0':'1'}}));
    }
    case 'getServerListByVersion':
    case 'public_getServerList':
    case 'getServerList':
      return sendGameResponse(req,res,ok({server_list:serverList(),serverCode:{'0':'1'}}));
    case 'getServerTime':
      return sendGameResponse(req,res,ok({time:Math.floor(Date.now()/1000)}));
    case 'getLatestAppVersion':
    case 'public_getVersionInfo':
      return sendGameResponse(req,res,ok({version:'1.0.1',force:0}));
    case 'login':
    case 'testlogin': {
      const nested = Array.isArray(params[0]) ? params[0] : [];
      const uid = String(nested[0] || sessions.get(req.headers.sid||'') || '');
      if (!uid || !players.has(uid)) return sendGameResponse(req,res,{code:1207,info:uid||'new_user'});
      const sid = req.headers.sid || makeSession(uid);
      return sendGameResponse(req,res,ok(playerLoginInfo(players.get(uid),sid)));
    }
    case 'register':
    case 'pickName': {
      const uid = sessions.get(req.headers.sid||'') || String((Array.isArray(params[0]) ? params[0][0] : '') || '');
      if (uid) {
        if (!players.has(uid)) players.set(uid,makePlayer(uid));
        const p = players.get(uid);
        const candidate = Array.isArray(params) ? params.flat(Infinity).find(x => typeof x === 'string' && x.length > 0 && x !== uid) : null;
        if (candidate) p.name = candidate;
        p.gold = START_GOLD;
        p.silver = START_SILVER;
        return sendGameResponse(req,res,ok({player:p,gold:p.gold,silver:p.silver}));
      }
      return sendGameResponse(req,res,ok({gold:START_GOLD,silver:START_SILVER}));
    }
    case 'getViewer':
      return sendGameResponse(req,res,ok({}));
    case 'getSetting':
      return sendGameResponse(req,res,ok({version:'1'}));
    default:
      return sendGameResponse(req,res,{code:9999,info:`Unsupported action: ${action}`});
  }
}

const server = http.createServer((req,res)=>{
  const url = new URL(req.url,`http://${req.headers.host||'localhost'}`);
  if (url.pathname === '/health') {
    res.writeHead(200,{'Content-Type':'application/json'});
    return res.end(JSON.stringify({ok:true,service:'Rocks-D',publicUrl:PUBLIC_URL,startGold:START_GOLD,startSilver:START_SILVER}));
  }
  if (['/pirate/public','/pirate/public/','/public','/public/'].includes(url.pathname)) return handleAction(req,res,url);
  res.writeHead(404,{'Content-Type':'text/plain; charset=utf-8'});
  res.end('Rocks-D private server\n');
});
server.listen(PORT,HOST,()=>{
  console.log(`Rocks-D server listening on http://${HOST}:${PORT}`);
  console.log(`Client endpoint: ${PUBLIC_URL}`);
  console.log(`Starting currency: gold=${START_GOLD}, silver=${START_SILVER}`);
});
