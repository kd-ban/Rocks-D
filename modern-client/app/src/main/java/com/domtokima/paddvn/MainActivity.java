package com.domtokima.paddvn;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class MainActivity extends Activity {
    @Override protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        setContentView(new GameView());
    }

    private final class GameView extends View {
        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);
        private final Paint text = new Paint(Paint.ANTI_ALIAS_FLAG);
        private Bitmap artwork;
        private String assetPath = "searching original game assets...";
        private volatile String serverState = "SERVER CHECKING";

        GameView() {
            super(MainActivity.this);
            setBackgroundColor(Color.BLACK);
            text.setColor(Color.WHITE);
            text.setTextSize(34f);
            loadOriginalArtwork();
            checkServerAsync();
        }

        private void loadOriginalArtwork() {
            try {
                List<String> candidates = new ArrayList<>();
                collectImages(getAssets(), "original/images", candidates, 0);
                Collections.sort(candidates);

                // Prefer larger/background-looking files when the names exist.
                String chosen = null;
                for (String path : candidates) {
                    String lower = path.toLowerCase();
                    if (lower.contains("login") || lower.contains("loading")
                            || lower.contains("background") || lower.contains("main")) {
                        chosen = path;
                        break;
                    }
                }
                if (chosen == null && !candidates.isEmpty()) chosen = candidates.get(0);

                if (chosen != null) {
                    try (InputStream in = getAssets().open(chosen)) {
                        artwork = BitmapFactory.decodeStream(in);
                    }
                    assetPath = chosen;
                } else {
                    assetPath = "NO PNG/JPG FOUND IN original/images";
                }
            } catch (Throwable t) {
                assetPath = "ASSET ERROR: " + t.getClass().getSimpleName();
            }
        }

        private void collectImages(AssetManager am, String dir, List<String> out, int depth) throws Exception {
            if (depth > 6) return;
            String[] names = am.list(dir);
            if (names == null) return;
            for (String name : names) {
                String path = dir.length() == 0 ? name : dir + "/" + name;
                String lower = name.toLowerCase();
                if (lower.endsWith(".png") || lower.endsWith(".jpg") || lower.endsWith(".jpeg")) {
                    out.add(path);
                } else if (!name.contains(".")) {
                    collectImages(am, path, out, depth + 1);
                }
            }
        }

        private void checkServerAsync() {
            new Thread(() -> {
                HttpURLConnection c = null;
                try {
                    c = (HttpURLConnection) new URL("http://192.168.8.59/health").openConnection();
                    c.setConnectTimeout(1800);
                    c.setReadTimeout(1800);
                    serverState = c.getResponseCode() == 200 ? "SERVER ONLINE" : "SERVER HTTP " + c.getResponseCode();
                } catch (Throwable t) {
                    serverState = "SERVER OFFLINE";
                } finally {
                    if (c != null) c.disconnect();
                }
                postInvalidate();
            }, "rocksd-server-check").start();
        }

        @Override protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            int w = getWidth();
            int h = getHeight();

            if (artwork != null && artwork.getWidth() > 0 && artwork.getHeight() > 0) {
                float srcRatio = (float) artwork.getWidth() / artwork.getHeight();
                float dstRatio = (float) w / Math.max(1, h);
                Rect dst;
                if (srcRatio > dstRatio) {
                    int dh = Math.round(w / srcRatio);
                    int top = (h - dh) / 2;
                    dst = new Rect(0, top, w, top + dh);
                } else {
                    int dw = Math.round(h * srcRatio);
                    int left = (w - dw) / 2;
                    dst = new Rect(left, 0, left + dw, h);
                }
                canvas.drawBitmap(artwork, null, dst, paint);
            }

            paint.setColor(0xAA000000);
            canvas.drawRect(0, Math.max(0, h - 150), w, h, paint);
            canvas.drawText("Rocks-D Stage 7.0 | " + serverState, 30, h - 92, text);
            text.setTextSize(23f);
            canvas.drawText("Original asset: " + assetPath, 30, h - 48, text);
            text.setTextSize(34f);
        }
    }
}
