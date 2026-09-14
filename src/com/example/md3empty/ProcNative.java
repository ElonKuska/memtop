package com.example.md3empty;

public class ProcNative {
    private static boolean loaded = false;

    static {
        try {
            System.loadLibrary("memtop");
            loaded = true;
        } catch (Throwable t) {
            loaded = false;
        }
    }

    public static boolean isLoaded() { return loaded; }

    public static native long[] getMem();
    public static native String[] getProcs(int max);

    public static long[] safeMem() {
        if (!loaded) return null;
        try { return getMem(); } catch (Throwable t) { return null; }
    }

    public static String[] safeProcs(int max) {
        if (!loaded) return null;
        try { return getProcs(max); } catch (Throwable t) { return null; }
    }
}
