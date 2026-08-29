package com.domtokima.paddvn;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

public final class MainActivity extends Activity {
    private static final String PREFS = "rocksd_stage62";
    private static final String KEY_CHECKPOINT = "checkpoint";
    private static final String CRASH_FILE = "last_crash.txt";

    private boolean nativeLoaded = false;
    private String nativeLoadError = "not attempted";
    private TextView view;

    private static native String nativeStatus();
    private static native String nativeRunLuaBytes(byte[] data);

    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        view = new TextView(this);
        view.setGravity(Gravity.CENTER);
        view.setTextSize(16f);
        view.setPadding(24, 24, 24, 24);
        view.setTextIsSelectable(true);
        setContentView(view);

        installJavaCrashRecorder();

        String previousCheckpoint = prefs().getString(KEY_CHECKPOINT, "");
        if (previousCheckpoint != null && !previousCheckpoint.isEmpty()) {
            String saved = readCrashFile();
            String message = "Rocks-D ARM64 Stage 6.2\n\nCRASH CHECKPOINT FOUND\n"
                    + "The previous run stopped during:\n" + previousCheckpoint
                    + (saved.isEmpty() ? "" : "\n\nSaved Java error:\n" + saved)
                    + "\n\nTake a screenshot of this screen and send it.";
            view.setText(message);
            return;
        }

        deleteCrashFile();

        if (!loadNativeLibrary()) return;

        boolean sso = assetExists("original/lua/SSO/sso.op");
        boolean network = assetExists("original/lua/util/netWork.op");
        boolean conf = assetExists("decrypted/conf.lua");

        String runtime = runNativeStatus();
        if (runtime == null) return;

        String bootstrap = executeLua("stage4/bootstrap.lua", "Bootstrap");
        if (bootstrap == null) return;

        String originalConf;
        if (conf) {
            originalConf = executeLua("decrypted/conf.lua", "Original conf");
            if (originalConf == null) return;
        } else {
            originalConf = "Original conf: MISSING";
        }

        clearCheckpoint();

        String base = "Rocks-D ARM64 Stage 6.2\nNative library: LOADED\n" + runtime + "\n" + bootstrap + "\n" + originalConf
                + "\nOriginal assets: " + ((sso && network) ? "LOADED" : "MISSING");
        view.setText(base + "\nServer: CHECKING...");

        new Thread(() -> {
            String server = checkServer();
            runOnUiThread(() -> view.setText(base + "\nServer: " + server));
        }).start();
    }

    private boolean loadNativeLibrary() {
        setCheckpoint("1/4 System.loadLibrary(game)");
        try {
            System.loadLibrary("game");
            nativeLoaded = true;
            nativeLoadError = "none";
            clearCheckpoint();
            return true;
        } catch (Throwable t) {
            nativeLoaded = false;
            nativeLoadError = describe(t);
            saveJavaError("System.loadLibrary(game)", t);
            view.setText("Rocks-D ARM64 Stage 6.2\nNative library: FAILED\n" + nativeLoadError
                    + "\n\nThis error was saved inside the app.");
            return false;
        }
    }

    private String runNativeStatus() {
        if (!nativeLoaded) return "Native status: SKIPPED";
        setCheckpoint("2/4 nativeStatus()");
        try {
            String result = nativeStatus();
            clearCheckpoint();
            return result;
        } catch (Throwable t) {
            saveJavaError("nativeStatus()", t);
            view.setText("Rocks-D ARM64 Stage 6.2\nNative status: ERROR\n" + describe(t));
            return null;
        }
    }

    private String executeLua(String path, String label) {
        if (!nativeLoaded) return label + ": SKIPPED / native library unavailable";

        String checkpoint = label.startsWith("Bootstrap")
                ? "3/4 nativeRunLuaBytes(stage4/bootstrap.lua)"
                : "4/4 nativeRunLuaBytes(decrypted/conf.lua)";
        setCheckpoint(checkpoint);

        try {
            byte[] data = readAsset(path);
            if (data.length == 0) {
                clearCheckpoint();
                return label + ": EMPTY";
            }
            String result = nativeRunLuaBytes(data);
            clearCheckpoint();
            return label + ": " + result;
        } catch (Throwable t) {
            saveJavaError(checkpoint, t);
            view.setText("Rocks-D ARM64 Stage 6.2\n" + label + ": ERROR\n" + describe(t));
            return null;
        }
    }

    private void installJavaCrashRecorder() {
        final Thread.UncaughtExceptionHandler previous = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler((thread, throwable) -> {
            try {
                String checkpoint = prefs().getString(KEY_CHECKPOINT, "unknown");
                saveJavaError("Uncaught exception at " + checkpoint, throwable);
            } catch (Throwable ignored) {
            }
            if (previous != null) previous.uncaughtException(thread, throwable);
        });
    }

    private SharedPreferences prefs() {
        return getSharedPreferences(PREFS, MODE_PRIVATE);
    }

    private void setCheckpoint(String checkpoint) {
        prefs().edit().putString(KEY_CHECKPOINT, checkpoint).commit();
    }

    private void clearCheckpoint() {
        prefs().edit().remove(KEY_CHECKPOINT).commit();
    }

    private void saveJavaError(String where, Throwable t) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        pw.println(where);
        t.printStackTrace(pw);
        pw.flush();
        byte[] bytes = sw.toString().getBytes(StandardCharsets.UTF_8);
        try (FileOutputStream out = openFileOutput(CRASH_FILE, MODE_PRIVATE)) {
            out.write(bytes);
        } catch (Exception ignored) {
        }
    }

    private String readCrashFile() {
        File file = new File(getFilesDir(), CRASH_FILE);
        if (!file.exists()) return "";
        try (FileInputStream in = new FileInputStream(file); ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            byte[] buffer = new byte[2048];
            int n;
            while ((n = in.read(buffer)) != -1) out.write(buffer, 0, n);
            return new String(out.toByteArray(), StandardCharsets.UTF_8);
        } catch (Exception ignored) {
            return "";
        }
    }

    private void deleteCrashFile() {
        try {
            new File(getFilesDir(), CRASH_FILE).delete();
        } catch (Throwable ignored) {
        }
    }

    private String describe(Throwable t) {
        return t.getClass().getSimpleName() + ": " + String.valueOf(t.getMessage());
    }

    private byte[] readAsset(String path) throws Exception {
        try (InputStream in = getAssets().open(path); ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            byte[] buffer = new byte[4096];
            int n;
            while ((n = in.read(buffer)) != -1) out.write(buffer, 0, n);
            return out.toByteArray();
        }
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
            connection = (HttpURLConnection) new URL("http://192.168.8.59/health").openConnection();
            connection.setConnectTimeout(2500);
            connection.setReadTimeout(2500);
            int code = connection.getResponseCode();
            return code == 200 ? "ONLINE" : "HTTP " + code;
        } catch (Exception e) {
            return "OFFLINE / " + e.getClass().getSimpleName();
        } finally {
            if (connection != null) connection.disconnect();
        }
    }
}
