package com.example.md3empty;

import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.view.View;
import android.view.animation.DecelerateInterpolator;

public class AnimHelper {
    public static void juicy(View v) {
        v.setOnTouchListener((view, e) -> {
            int action = e.getAction();
            if (action == android.view.MotionEvent.ACTION_DOWN) {
                press(view, 0.93f, 120);
            } else if (action == android.view.MotionEvent.ACTION_UP
                    || action == android.view.MotionEvent.ACTION_CANCEL) {
                press(view, 1f, 200);
            }
            return false;
        });
    }

    private static void press(View v, float scale, long dur) {
        ObjectAnimator sx = ObjectAnimator.ofFloat(v, "scaleX", scale);
        ObjectAnimator sy = ObjectAnimator.ofFloat(v, "scaleY", scale);
        sx.setDuration(dur);
        sy.setDuration(dur);
        sx.setInterpolator(new DecelerateInterpolator());
        sy.setInterpolator(new DecelerateInterpolator());
        AnimatorSet set = new AnimatorSet();
        set.playTogether(sx, sy);
        set.start();
    }

    public static void popIn(View v, long delay) {
        v.setScaleX(0.85f);
        v.setScaleY(0.85f);
        v.setAlpha(0f);
        v.animate().scaleX(1f).scaleY(1f).alpha(1f)
                .setStartDelay(delay).setDuration(300)
                .setInterpolator(new DecelerateInterpolator()).start();
    }

    public static void fadeLog(final android.widget.TextView log, final String line) {
        log.append(line + "\n");
        log.setAlpha(0f);
        log.animate().alpha(1f).setDuration(250).start();
    }
}
