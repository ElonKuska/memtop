package com.example.md3empty;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;

public class ThemeManager {
    private static final String PREFS = "md3_prefs";
    public static final String MODE_SYSTEM = "system";
    public static final String MODE_LIGHT = "light";
    public static final String MODE_DARK = "dark";

    public static SharedPreferences prefs(Context c) {
        return c.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }

    public static String getMode(Context c) {
        return prefs(c).getString("theme_mode", MODE_SYSTEM);
    }

    public static void setMode(Context c, String mode) {
        prefs(c).edit().putString("theme_mode", mode).apply();
    }

    public static String getPalette(Context c) {
        return prefs(c).getString("palette", "espresso");
    }

    public static void setPalette(Context c, String palette) {
        prefs(c).edit().putString("palette", palette).apply();
    }

    public static boolean isDark(Context c) {
        String mode = getMode(c);
        if (MODE_DARK.equals(mode)) return true;
        if (MODE_LIGHT.equals(mode)) return false;
        int night = c.getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK;
        return night == Configuration.UI_MODE_NIGHT_YES;
    }

    public static int getThemeRes(Context c) {
        String pal = getPalette(c);
        boolean dark = isDark(c);
        if ("indigo".equals(pal)) return dark ? R.style.Theme_MD3Empty_Indigo_Dark : R.style.Theme_MD3Empty_Indigo_Light;
        if ("forest".equals(pal)) return dark ? R.style.Theme_MD3Empty_Forest_Dark : R.style.Theme_MD3Empty_Forest_Light;
        if ("sakura".equals(pal)) return dark ? R.style.Theme_MD3Empty_Sakura_Dark : R.style.Theme_MD3Empty_Sakura_Light;
        if ("ocean".equals(pal)) return dark ? R.style.Theme_MD3Empty_Ocean_Dark : R.style.Theme_MD3Empty_Ocean_Light;
        if ("sunset".equals(pal)) return dark ? R.style.Theme_MD3Empty_Sunset_Dark : R.style.Theme_MD3Empty_Sunset_Light;
        if ("grape".equals(pal)) return dark ? R.style.Theme_MD3Empty_Grape_Dark : R.style.Theme_MD3Empty_Grape_Light;
        if ("mono".equals(pal)) return dark ? R.style.Theme_MD3Empty_Mono_Dark : R.style.Theme_MD3Empty_Mono_Light;
        if ("lime".equals(pal)) return dark ? R.style.Theme_MD3Empty_Lime_Dark : R.style.Theme_MD3Empty_Lime_Light;
        return dark ? R.style.Theme_MD3Empty_Espresso_Dark : R.style.Theme_MD3Empty_Espresso_Light;
    }

    public static void apply(Activity a) {
        a.setTheme(getThemeRes(a));
    }
}
