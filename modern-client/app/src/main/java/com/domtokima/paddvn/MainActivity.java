package com.domtokima.paddvn;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.TextView;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public final class MainActivity extends Activity {
    private static final String SERVER = "http://192.168.8.59/pirate/public";

    static {
        System.loadLibrary("game");
    }

    private static native String nativeStatus();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final TextView view = new TextView(this);
        view.setGravity(Gravity.CENTER);
        view.setTextSize(20f);
        setContentView(view);

        boolean sso = assetExists("original/lua/SSO/sso.op");
        boolean network = assetExists("original/lua/util/netWork.op");
        view.setText("Rocks-D ARM64 Stage 2\n" + nativeStatus()
                + "\nOriginal assets: " + ((sso && network) ? "LOADED" : "MISSING")
                + "\nServer: CHECKING...");

        new Thread(() -> {
            String server = checkServer();
            runOnUiThread(() -> view.setText("Rocks-D ARM64 Stage 2\n" + nativeStatus()
                    + "\nOriginal assets: " + ((sso && network) ? "LOADED" : "MISSING")
                    + "\nServer: " + server));
        }).start();
    }

    private boolean assetExists(String path) {
        try (InputStream in = getAssets().open(path)) {
            return in.read() >= -1;
        } catch (Exception ignored) {
            return false;
        }
    }

    private String checkServer() {
        HttpURLConnection connection = null;
        try {
            URL url = new URL("http://192.168.8.59/health");
            connection = (HttpURLConnection) url.openConnection();
            connection.setConnectTimeout(2500);
            connection.setReadTimeout(2500);
            return connection.getResponseCode() == 200 ? "ONLINE" : "HTTP " + connection.getResponseCode();
        } catch (Exception e) {
            return "OFFLINE";
        } finally {
            if (connection != null) connection.disconnect();
        }
    }
}
