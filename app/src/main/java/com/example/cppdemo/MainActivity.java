package com.example.cppdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.example.cppdemo.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    // Used to load the 'cppdemo' library on application startup.
    static {
        System.loadLibrary("cppdemo");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Initialize Native Cpp Demo
        nativeInitialization();

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringHello());

        // Button 1: Print Hello
        binding.btnPrintHello.setOnClickListener(v -> {
            Log.i(TAG, "Hello from Java!");
            printGreetings();
        });

        // Button 2: Java Crash (uncaught exception)
        binding.btnJavaCrash.setOnClickListener(v -> {
            Log.w(TAG, "Will throw IllegalStateException");
            throw new IllegalStateException("Intentional Java crash!");
        });

        // Button 3: C++ Crash (abort)
        binding.btnCppAbort.setOnClickListener(v -> {
            Log.w(TAG, "Will call c++ abort()");
            callAbort();
        });

        // Button 4: C++ Crash (uncaught exception)
        binding.btnCppException.setOnClickListener(v -> {
            Log.w(TAG, "Will throw C++ exception");
            throwCppException();
        });

        // Button 5: Kotlin-style crash (simulated in Java)
        binding.btnKotlinCrash.setOnClickListener(v -> {
            Log.w(TAG, "Will throw Kotlin-style exception");
            Kotlin.throwKotlinStyleException();
        });
    }

    /**
     * A native method that is implemented by the 'cppdemo' native library,
     * which is packaged with this application.
     */
    public native void nativeInitialization();

    public native String stringHello();

    public native void printGreetings();

    public native void callAbort();

    public native void throwCppException();
}