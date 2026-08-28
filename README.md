# Rocks-D private-server patch

Target package: `com.domtokima.paddvn`

Changes in this bundle:
- `DEFAULT_HOST` -> `http://192.168.8.59/pirate/public`
- `SSO_DEFAULT_HOST` -> `http://192.168.8.59/pirate/public/`
- Auto Guest: if no saved UID exists, the client calls `SSOPlatform.GenerateUid()` automatically and then continues the existing SSO login flow.
- Modified Lua is re-encrypted back to `.op` using the game's original format.

## Files to keep in GitHub
Upload these folders/files:
- `source/`
- `patch/`
- `README.md`

Do NOT upload the signing key or keystore to GitHub.

## APK
`Rocks-D-private-server-UNSIGNED.apk` contains the patched `.op` files but is intentionally unsigned. Sign it with your own Android signing key before installing.

## OBB
Copy `main.3.com.domtokima.paddvn.obb` to:
`Android/obb/com.domtokima.paddvn/main.3.com.domtokima.paddvn.obb`

The OBB is unchanged from the original XAPK.
