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
    private static native String nativeRunLuaBytes(byte[] data);

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final TextView view = new TextView(this);
        view.setGravity(Gravity.CENTER);
        view.setTextSize(16f);
        setContentView(view);

        boolean sso = assetExists("original/lua/SSO/sso.op");
        boolean network = assetExists("original/lua/util/netWork.op");
        boolean conf = assetExists("decrypted/conf.lua");
        String runtime = nativeStatus();
        String bootstrap = executeLua("stage4/bootstrap.lua", "Bootstrap");
        String originalConf = conf ? executeLua("decrypted/conf.lua", "Original conf") : "Original conf: MISSING";

        String base = "Rocks-D ARM64 Stage 5\n" + runtime + "\n" + bootstrap + "\n" + originalConf
                + "\nOriginal assets: " + ((sso && network) ? "LOADED" : "MISSING");
        view.setText(base + "\nServer: CHECKING...");

        new Thread(() -> {
            String server = checkServer();
            runOnUiThread(() -> view.setText(base + "\nServer: " + server));
        }).start();
    }

    private String executeLua(String path, String label) {
        try {
            byte[] data = readAsset(path);
            if (data.length == 0) return label + ": EMPTY";
            return label + ": " + nativeRunLuaBytes(data);
        } catch (Throwable t) {
            return label + ": JAVA ERROR / " + t.getClass().getSimpleName();
        }
    }

    private byte[] readAsset(String path) throws Exception {
        try (InputStream in = getAssets().open(path); ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            byte[] buffer = new byte[4096]; int n;
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
            connection.setConnectTimeout(2500); connection.setReadTimeout(2500);
            return connection.getResponseCode() == 200 ? "ONLINE" : "ERROR";
        } catch (Exception e) { return "OFFLINE"; }
        finally { if (connection != null) connection.disconnect(); }
    }
}
