# Rocks-D private server

The patched Android client currently points to:

`http://192.168.8.59/pirate/public`

So the actual HTTP game server must run on the Mac at `192.168.8.59` (or the APK host must later be changed to a public deployment URL).

GitHub stores the source, APK and OBB; a normal GitHub repository/GitHub Pages cannot keep this dynamic game API running as an always-on HTTP server.

## Client protocol recovered from source

The client sends GET requests to `/pirate/public?action=...` and uses these actions during startup/login:

- `sGenerateUid`
- `sLogin`
- `sgLogin`
- `getServerListByVersion`
- `login`
- `register`
- `getServerList`
- `getServerTime`
- `getSetting`
- `getViewer`
- `pickName`
- `getLatestAppVersion`

It also sends headers such as `aid`, `serverCode`, and after game login `sid`.

## Important response format

The old client does **not** accept plain JSON. `source/netWork.lua` shows that successful HTTP responses are expected as:

1. first 32 bytes: MD5 signature
2. remaining bytes: gzip-compressed JSON

The JSON itself contains a numeric `code` field (normally `200`). The client recomputes the response MD5 before decoding JSON, so the server implementation must reproduce the game's signing algorithm exactly.

## Current next step

Recover/verify the MD5 signing behavior used by `Global.getMD5`, then implement the startup/SSO endpoints above. Do not fake plain-JSON endpoints because the client will reject them before the callbacks run.
