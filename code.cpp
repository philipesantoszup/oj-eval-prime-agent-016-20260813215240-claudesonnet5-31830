
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <vector>
#include <climits>

using namespace std;

static const int IDXLEN = 65; // 64 chars + null terminator

struct Key {
    char idx[IDXLEN];
    int val;
};

struct KeyCmp {
    bool operator()(const Key& a, const Key& b) const {
        int c = strcmp(a.idx, b.idx);
        if (c != 0) return c < 0;
        return a.val < b.val;
    }
};

static set<Key, KeyCmp> db;

// ---------- Persistence ----------
static const char* DB_FILE = "bpt_database.dat";

void loadDB() {
    FILE* f = fopen(DB_FILE, "rb");
    if (!f) return;
    unsigned int cnt = 0;
    if (fread(&cnt, sizeof(cnt), 1, f) != 1) { fclose(f); return; }
    for (unsigned int i = 0; i < cnt; i++) {
        Key k;
        memset(k.idx, 0, IDXLEN);
        unsigned char klen = 0;
        if (fread(&klen, 1, 1, f) != 1) break;
        if (klen > 0) { if (fread(k.idx, 1, klen, f) != klen) break; }
        k.idx[klen] = '\0';
        int val;
        if (fread(&val, sizeof(val), 1, f) != 1) break;
        k.val = val;
        db.insert(k);
    }
    fclose(f);
}

void saveDB() {
    FILE* f = fopen(DB_FILE, "wb");
    if (!f) return;
    unsigned int cnt = (unsigned int)db.size();
    fwrite(&cnt, sizeof(cnt), 1, f);
    for (const auto& k : db) {
        unsigned char klen = (unsigned char)strlen(k.idx);
        fwrite(&klen, 1, 1, f);
        if (klen > 0) fwrite(k.idx, 1, klen, f);
        fwrite(&k.val, sizeof(k.val), 1, f);
    }
    fclose(f);
}

// ---------- Fast IO (streaming, fixed buffer) ----------
static const size_t IOBUFSZ = 1 << 16; // 64KB
static char ioBuf[IOBUFSZ];
static size_t ioLen = 0, ioPos = 0;

static inline int getch() {
    if (ioPos >= ioLen) {
        ioLen = fread(ioBuf, 1, IOBUFSZ, stdin);
        ioPos = 0;
        if (ioLen == 0) return -1;
    }
    return (unsigned char)ioBuf[ioPos++];
}

static inline void skipSpaces(int& c) {
    while (c == ' ' || c == '\n' || c == '\r' || c == '\t') c = getch();
}

static inline bool readToken(char* out, int maxlen, int& c) {
    skipSpaces(c);
    if (c == -1) return false;
    int len = 0;
    while (c != -1 && c != ' ' && c != '\n' && c != '\r' && c != '\t') {
        if (len < maxlen) out[len++] = (char)c;
        c = getch();
    }
    out[len] = '\0';
    return true;
}

static inline bool readInt(int& val, int& c) {
    skipSpaces(c);
    if (c == -1) return false;
    bool neg = false;
    if (c == '-') { neg = true; c = getch(); }
    long long v = 0;
    while (c >= '0' && c <= '9') {
        v = v * 10 + (c - '0');
        c = getch();
    }
    val = (int)(neg ? -v : v);
    return true;
}

// ---------- Output buffering ----------
static const size_t OUTBUFSZ = 1 << 16; // 64KB
static char outBuf[OUTBUFSZ];
static size_t outPos = 0;

static inline void flushOut() {
    if (outPos > 0) {
        fwrite(outBuf, 1, outPos, stdout);
        outPos = 0;
    }
}

static inline void putch(char c) {
    if (outPos >= OUTBUFSZ) flushOut();
    outBuf[outPos++] = c;
}

static inline void appendInt(int v) {
    char buf[12];
    int len = 0;
    unsigned int uv;
    if (v < 0) { putch('-'); uv = (unsigned int)(-(long long)v); }
    else uv = (unsigned int)v;
    if (uv == 0) { putch('0'); return; }
    while (uv > 0) { buf[len++] = '0' + (uv % 10); uv /= 10; }
    while (len > 0) putch(buf[--len]);
}

int main() {
    loadDB();

    int c = getch();

    int n;
    if (!readInt(n, c)) n = 0;

    char cmd[16];
    char key[128];

    for (int i = 0; i < n; i++) {
        if (!readToken(cmd, 15, c)) break;
        if (cmd[0] == 'i') { // insert
            readToken(key, 127, c);
            int val;
            readInt(val, c);
            Key k;
            memset(k.idx, 0, IDXLEN);
            strncpy(k.idx, key, IDXLEN - 1);
            k.val = val;
            db.insert(k);
        } else if (cmd[0] == 'd') { // delete
            readToken(key, 127, c);
            int val;
            readInt(val, c);
            Key k;
            memset(k.idx, 0, IDXLEN);
            strncpy(k.idx, key, IDXLEN - 1);
            k.val = val;
            db.erase(k);
        } else { // find
            readToken(key, 127, c);
            Key lo;
            memset(lo.idx, 0, IDXLEN);
            strncpy(lo.idx, key, IDXLEN - 1);
            lo.val = INT_MIN;

            auto it = db.lower_bound(lo);
            bool any = false;
            while (it != db.end() && strcmp(it->idx, key) == 0) {
                if (any) putch(' ');
                appendInt(it->val);
                any = true;
                ++it;
            }
            if (!any) {
                putch('n'); putch('u'); putch('l'); putch('l');
            }
            putch('\n');
        }
    }

    flushOut();

    saveDB();

    return 0;
}
