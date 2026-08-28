package com.domtokima.paddvn;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public final class MainActivity extends Activity {
    static { System.loadLibrary("game"); }
    private static native String nativeStatus();
    private static native String nativeRunLuaBytes(byte[] data, String chunkName);

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final TextView view = new TextView(this);
        view.setGravity(Gravity.CENTER);
        view.setTextSize(18f);
        setContentView(view);

        boolean sso = assetExists("original/lua/SSO/sso.op");
        boolean network = assetExists("original/lua/util/netWork.op");
        String lua = nativeStatus();
        String assetLua = runBootstrapSafely();

        view.setText("Rocks-D ARM64 Stage 4.2\n" + lua + "\n" + assetLua
                + "\nOriginal assets: " + ((sso && network) ? "LOADED" : "MISSING")
                + "\nServer: CHECKING...");

        new Thread(() -> {
            String server = checkServer();
            runOnUiThread(() -> view.setText("Rocks-D ARM64 Stage 4.2\n" + lua + "\n" + assetLua
                    + "\nOriginal assets: " + ((sso && network) ? "LOADED" : "MISSING")
                    + "\nServer: " + server));
        }).start();
    }

    private String runBootstrapSafely() {
        try {
            byte[] data = readAsset("stage4/bootstrap.lua");
            if (data == null || data.length == 0) return "Asset Lua: READ FAILED";
            return nativeRunLuaBytes(data, "stage4/bootstrap.lua");
        } catch (Throwable t) {
            return "Asset Lua: JAVA ERROR " + t.getClass().getSimpleName();
        }
    }

    private byte[] readAsset(String path) throws Exception {
        try (InputStream in = getAssets().open(path);
             ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            byte[] buffer = new byte[4096];
            int n;
            while ((n = in.read(buffer)) != -1) out.write(buffer, 0, n);
            return out.toByteArray();
        }
    }

    private boolean assetExists(String path) {
        try (InputStream in = getAssets().open(path)) { return in.read() >= -1; }
        catch (Exception ignored) { return false; }
    }

    private String checkServer() {
        HttpURLConnection connection = null;
        try {
            connection = (HttpURLConnection) new URL("http://192.168.8.59/health").openConnection();
            connection.setConnectTimeout(2500);
            connection.setReadTimeout(2500);
            int code = connection.getResponseCode();
            return code == 200 ? "ONLINE" : "HTTP " + code;
        } catch (Exception e) {
            return "OFFLINE";
        } finally {
            if (connection != null) connection.disconnect();
        }
    }
}
