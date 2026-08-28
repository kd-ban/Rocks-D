package com.domtokima.paddvn;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.TextView;

public final class MainActivity extends Activity {
    static {
        System.loadLibrary("game");
    }

    private static native String nativeStatus();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TextView view = new TextView(this);
        view.setGravity(Gravity.CENTER);
        view.setTextSize(22f);
        view.setText("Rocks-D ARM64\n" + nativeStatus() + "\nServer: http://192.168.8.59/pirate/public");
        setContentView(view);
    }
}
