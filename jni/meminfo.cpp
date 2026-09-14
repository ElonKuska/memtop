// meminfo.cpp — JNI либка для MD3 memtop: память + процессы
// Пакет: com.example.md3empty.ProcNative
//   long[] getMem()       -> [totalKb, availKb, usedKb]
//   String[] getProcs(n)  -> "pid|name|pssKb|rssKb|threads" sorted by PSS desc
// Читает /proc напрямую (без root видит свои, с root — все).
// Позже добавим su-обёртку, пока так + fallback на тест в Java.

#include <jni.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

struct Proc {
    int pid = 0;
    std::string name;
    long pss = 0;
    long rss = 0;
    int threads = 1;
};

static long readField(const char* path, const char* prefix) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    char line[256];
    long val = -1;
    size_t plen = strlen(prefix);
    while (fgets(line, sizeof(line), f)) {
        if (!strncmp(line, prefix, plen)) {
            sscanf(line + plen, "%ld", &val);
            break;
        }
    }
    fclose(f);
    return val;
}

static bool readRollup(int pid, long& pss, long& rss) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/smaps_rollup", pid);
    FILE* f = fopen(path, "r");
    if (!f) return false;
    char line[256];
    pss = -1; rss = -1;
    while (fgets(line, sizeof(line), f)) {
        if (pss < 0 && !strncmp(line, "Pss:", 4)) sscanf(line + 4, "%ld", &pss);
        else if (rss < 0 && !strncmp(line, "Rss:", 4)) sscanf(line + 4, "%ld", &rss);
        if (pss >= 0 && rss >= 0) break;
    }
    fclose(f);
    return pss >= 0;
}

static void readStatus(int pid, long& rss, int& threads) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE* f = fopen(path, "r");
    if (!f) { rss = -1; threads = 1; return; }
    char line[256];
    rss = -1; threads = 1;
    while (fgets(line, sizeof(line), f)) {
        if (rss < 0 && !strncmp(line, "VmRSS:", 6)) sscanf(line + 6, "%ld", &rss);
        else if (threads == 1 && !strncmp(line, "Threads:", 8)) sscanf(line + 8, "%d", &threads);
        if (rss >= 0 && threads > 1) break;
    }
    fclose(f);
}

static std::string readName(int pid) {
    char path[64], buf[512] = {0};
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    FILE* f = fopen(path, "r");
    if (f) {
        size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
        if (n > 0 && buf[0] != '\0') {
            buf[n] = '\0';
            std::string s(buf);
            for (char& c : s) if (c == '|' || c == '\n') c = '/';
            return s;
        }
    }
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    f = fopen(path, "r");
    if (f) {
        if (fgets(buf, sizeof(buf), f)) {
            fclose(f);
            size_t len = strlen(buf);
            while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
            return std::string(buf);
        }
        fclose(f);
    }
    return "?";
}

extern "C" {

JNIEXPORT jlongArray JNICALL
Java_com_example_md3empty_ProcNative_getMem(JNIEnv* env, jclass) {
    long total = readField("/proc/meminfo", "MemTotal:");
    long avail = readField("/proc/meminfo", "MemAvailable:");
    if (total < 0) total = 0;
    if (avail < 0) avail = 0;
    long used = total > avail ? total - avail : 0;
    jlongArray out = env->NewLongArray(3);
    jlong vals[3] = {total, avail, used};
    env->SetLongArrayRegion(out, 0, 3, vals);
    return out;
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_md3empty_ProcNative_getProcs(JNIEnv* env, jclass, jint maxN) {
    std::vector<Proc> v;
    v.reserve(256);
    DIR* d = opendir("/proc");
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            if (e->d_type != DT_DIR) continue;
            int pid = atoi(e->d_name);
            if (pid <= 0) continue;
            long pss = -1, rssRoll = -1;
            bool hasPss = readRollup(pid, pss, rssRoll);
            long statusRss = -1;
            int threads = 1;
            readStatus(pid, statusRss, threads);
            long rss = hasPss ? rssRoll : statusRss;
            long finalPss = hasPss ? pss : statusRss;
            if (finalPss < 0) continue;
            std::string name = readName(pid);
            if (name.empty()) continue;
            Proc p;
            p.pid = pid; p.name = std::move(name);
            p.pss = finalPss; p.rss = rss >= 0 ? rss : finalPss;
            p.threads = threads;
            v.push_back(std::move(p));
        }
        closedir(d);
    }
    std::sort(v.begin(), v.end(), [](const Proc& a, const Proc& b) {
        return a.pss > b.pss;
    });
    int n = (int)v.size();
    int lim = maxN > 0 && maxN < n ? maxN : n;
    jclass strCls = env->FindClass("java/lang/String");
    jobjectArray arr = env->NewObjectArray(lim, strCls, nullptr);
    char tmp[600];
    for (int i = 0; i < lim; i++) {
        const Proc& p = v[i];
        snprintf(tmp, sizeof(tmp), "%d|%s|%ld|%ld|%d",
                 p.pid, p.name.c_str(), p.pss, p.rss, p.threads);
        jstring s = env->NewStringUTF(tmp);
        env->SetObjectArrayElement(arr, i, s);
        env->DeleteLocalRef(s);
    }
    return arr;
}

} // extern "C"
