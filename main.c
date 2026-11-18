/*
 * 情緒飯店 Emotion Hotel（現代高畫質旗艦版）
 * 包含：四按鈕主頁、獨立語言選單、自適應三首 BGM、月曆管家分析、
 *       12項需求、Room Service、退房回顧、乾淨清晰加粗字型引擎
 */
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#define SCREEN_W 1280
#define SCREEN_H 720
#define MAX_RECORDS 200
#define TEXT 256
#define NOTE 768
#define DATA_FILE "emotion_hotel.txt"
#define FONT_BASE_SIZE 64
#define MAX_CP 8192
#define NUM_TRACKS 3
#define NUM_PARTICLES 42

/* ---------- 資料結構 ---------- */
typedef struct {
    int emotion;
    int intensity;
    int needs;
    int completed;
    int checked_out;
    int after;
    char date[32];
    /* text[0]事件 [1]原因 [2]身體感受 [3]日記 [4]給情緒的話 */
    char text[5][TEXT];
    char notes[5][NOTE];
    char review[TEXT];
} Record;

static Record records[MAX_RECORDS];
static int count = 0;
static int writable = 1;

static Record work;          /* 目前編輯中的紀錄 */
static int work_idx = -1;    /* -1 表示新入住 */

static const char *names_zh[6] = { "悲傷", "快樂", "焦慮", "生氣", "平靜", "疲憊" };
static const char *names_en[6] = { "Sad", "Happy", "Anxious", "Angry", "Calm", "Tired" };

static const Color ecolor[6] = {
    { 100, 149, 237, 255 }, { 255, 204, 0, 255 },  { 255, 140, 0, 255 },
    { 235, 77, 75, 255 },   { 46, 204, 113, 255 }, { 155, 89, 182, 255 }
};

static const char *welcome[6] = {
    "讓雨聲先陪你待一下。",
    "讓這份光多停留一會。",
    "不用一次整理完所有事情。",
    "先替這股力量找一個安全的位置。",
    "不需要追求更平靜，只要感覺現在。",
    "讓身體和心都少用一點力。"
};

static const char *fields[5] = {
    "今天發生了什麼？",
    "你覺得這個情緒為什麼來了？",
    "身體現在有什麼感覺？",
    "自由日記",
    "想對這位情緒客人說的話"
};

static const char *needs_list[12] = {
    "被理解", "被陪伴", "一個人待著", "哭一下",
    "休息", "找人說話", "解決問題", "運動",
    "寫下來", "聽音樂", "睡一覺", "我也不知道"
};

static const char *services[6][5] = {
    { "給自己一點空間", "聽一首歌", "抱住自己", "喝一杯溫水", "允許自己哭一下" },
    { "留下一張快樂小卡", "記住今天的美好", "留一句話給未來的自己", NULL, NULL },
    { "呼吸一下", "把擔心分類", "寫下現在能做的一件事", "暫時放下手機", NULL },
    { "放下這張紙", "寫下想說的話", "呼吸一下", "暫時離開刺激來源", NULL },
    { "呼吸一下", "喝茶", "感受周圍五件事", "安靜坐一下", NULL },
    { "休息", "補水", "關閉部分通知", "睡前準備", NULL }
};
static const int service_count[6] = { 5, 3, 4, 4, 4, 4 };

static const char *L_FEEDBACK[6] = {
    "這個月「悲傷」常來作客。雨水洗刷過的地方總會萌發新芽，記得給自己多一點擁抱與允許。",
    "這個月充滿了「快樂」的金色光芒！將這份明亮收藏在心底，作為未來前行的力量。",
    "這個月「焦慮」有些頻繁。不用急著一次解決所有難題，一步一步來，深呼吸，你做得很好了。",
    "這個月感受到了不少「生氣」與起伏。這代表你重視自己的邊界，先找個安全的位置好好安置它。",
    "這個月主旋律是「平靜」。能與日常平和相處是一種溫柔的力量，願你繼續安住在這份從容中。",
    "這個月累積了不少「疲憊」。你的身心正在提醒你放慢腳步，今晚放下所有包袱，好好睡個飽覺吧。"
};

/* 預載基礎字型池 */
static const char *UI_POOL = "、。一上下不並中主也了事五人什今介他以件份休伴位住你來依保修個值做停備傳傷允先光免入內全共其具再分列到刺前力功動勾卡原取受句只可台同吸周呼和哪哭唯問啟喝單嘗回因圍圖在坐型執填增多大天失好字存安完客容宿寫對小少工己已平序店度廳式張強形待後徑得心快息悲情想感態慮憊成我或房所手找把抱按接控換擇擔改放敗整數文料新方既日時暫更替最會有未束杯查格框案構樂標模機檔檢欄次歌歡此步氣水求決沒沿法消清游源準溫激為無焦版狀現理生用由留略畫疲發白的目直相看睡知示程空第筆算管紀紙結給緒編繪置美聲聽股能臨自與舊茶著行表被補製複要覺解計訊記許試話說請讀讓資路身載輯輸迎近返追退這通進運過道達選避部都重量錄長閉開間關限陪離雨需靜面音頁項題類顧飯首體麼點（），／：；？「」—·/語言設定選擇繁體中文英文播放暫停回顧絮語萌發新芽擁抱允許金色光芒明亮前行深呼吸安全安置從容放慢包袱睡飽覺離開旅館開始記錄";

/* ---------- 音樂系統 ---------- */
static const char *track_files[NUM_TRACKS][2] = {
    { "AEROHEAD - Fragments.mp3", "AEROHEAD - Fragments" },
    { "Nettson - Last Promise.mp3", "Nettson - Last Promise" },
    { "Savfk - Rounding.mp3", "Savfk - Rounding" }
};
static const char *track_names[NUM_TRACKS] = { "Fragments", "Last Promise", "Rounding" };
static Music bgm_tracks[NUM_TRACKS];
static int cur_track = 0;
static bool music_playing = false;

/* ---------- 動態粒子系統 ---------- */
typedef struct {
    Vector2 pos;
    float radius, speedY, waveOffset;
    Color color;
} Particle;
static Particle particles[NUM_PARTICLES];

static void InitParticles(int w, int h) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].pos = (Vector2){ (float)(rand() % w), (float)(rand() % h) };
        particles[i].radius = 12.0f + (float)(rand() % 26);
        particles[i].speedY = 0.25f + (float)(rand() % 45) / 100.0f;
        particles[i].waveOffset = (float)(rand() % 360);
        Color base = ecolor[rand() % 6];
        particles[i].color = (Color){ base.r, base.g, base.b, 65 };
    }
}

static void UpdateAndDrawParticles(int w, int h, float time) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].pos.y -= particles[i].speedY;
        particles[i].pos.x += sinf(time * 0.7f + particles[i].waveOffset) * 0.45f;
        if (particles[i].pos.y < -particles[i].radius * 2) {
            particles[i].pos.y = h + particles[i].radius * 2;
            particles[i].pos.x = (float)(rand() % w);
        }
        DrawCircleV(particles[i].pos, particles[i].radius, particles[i].color);
        DrawCircleV(particles[i].pos, particles[i].radius * 0.45f, (Color){ 255, 255, 255, 90 });
    }
}

static void DrawUnifiedBackground(int w, int h, float time, Rectangle glassCard) {
    DrawRectangleGradientV(0, 0, w, h, (Color){ 255, 242, 238, 255 }, (Color){ 232, 242, 252, 255 });
    UpdateAndDrawParticles(w, h, time);
    if (glassCard.width > 0 && glassCard.height > 0) {
        DrawRectangleRounded(glassCard, 0.06f, 8, (Color){ 255, 255, 255, 220 });
        DrawRectangleRoundedLines(glassCard, 0.06f, 8, (Color){ 255, 255, 255, 250 });
    }
}

/* ---------- 頁面列舉 ---------- */
typedef enum {
    P_LOBBY,
    P_CALENDAR,
    P_CI_EMOTION,
    P_CI_TEXT,
    P_CI_NEEDS,
    P_DETAIL,
    P_MUSIC,
    P_LANG
} Page;

static Page page = P_LOBBY;
static int cur_lang = 1; // 1: 繁中, 0: 英文
static int detail_tab = 0;
static int cur_service_idx = 0;

/* ---------- 檔案讀寫 ---------- */
static int line(FILE *f, char *s, int size) {
    size_t n;
    int c;
    if (!fgets(s, size, f)) return 0;
    n = strlen(s);
    if (n && s[n - 1] == '\n') {
        s[--n] = '\0';
        if (n && s[n - 1] == '\r') s[n - 1] = '\0';
        return 1;
    }
    while ((c = fgetc(f)) != '\n' && c != EOF) { }
    return -1;
}

static int number(const char *s, int low, int high, int *out) {
    char *end;
    long n;
    errno = 0;
    n = strtol(s, &end, 10);
    if (s == end || errno == ERANGE) return 0;
    while (*end == ' ' || *end == '\t') end++;
    if (*end || n < low || n > high) return 0;
    *out = (int)n;
    return 1;
}

static int read_number(FILE *f, int low, int high, int *out) {
    char s[64];
    return line(f, s, sizeof s) == 1 && number(s, low, high, out);
}

static int read_record(FILE *f, Record *r) {
    int i;
    memset(r, 0, sizeof *r);
    if (!read_number(f, 0, 5, &r->emotion) ||
        !read_number(f, 1, 5, &r->intensity) ||
        !read_number(f, 0, 4095, &r->needs) ||
        !read_number(f, 0, 31, &r->completed) ||
        !read_number(f, 0, 1, &r->checked_out) ||
        !read_number(f, 0, 5, &r->after) ||
        line(f, r->date, sizeof r->date) != 1) return 0;
    for (i = 0; i < 5; i++)
        if (line(f, r->text[i], TEXT) != 1) return 0;
    for (i = 0; i < 5; i++)
        if (line(f, r->notes[i], NOTE) != 1) return 0;
    return line(f, r->review, TEXT) == 1;
}

static void load(void) {
    FILE *f = fopen(DATA_FILE, "a+");
    char tag[32];
    int result, id;
    Record r;
    if (!f) {
        writable = 0;
        f = fopen(DATA_FILE, "r");
        if (!f) return;
    }
    rewind(f);
    while ((result = line(f, tag, sizeof tag)) != 0) {
        if (result != 1) break;
        if (!strcmp(tag, "IN2")) {
            if (count >= MAX_RECORDS || !read_record(f, &r)) break;
            records[count++] = r;
        } else if (!strcmp(tag, "EDIT2")) {
            if (!read_number(f, 1, count, &id) || !read_record(f, &r)) break;
            records[id - 1] = r;
        } else if (!strcmp(tag, "IN")) {
            memset(&r, 0, sizeof r);
            if (count >= MAX_RECORDS ||
                !read_number(f, 0, 5, &r.emotion) ||
                !read_number(f, 1, 5, &r.intensity) ||
                line(f, r.date, sizeof r.date) != 1 ||
                line(f, r.text[0], TEXT) != 1 ||
                line(f, r.text[3], TEXT) != 1) break;
            records[count++] = r;
        } else if (!strcmp(tag, "OUT")) {
            if (!read_number(f, 1, count, &id)) break;
            records[id - 1].checked_out = 1;
        } else break;
    }
    if (result != 0 || ferror(f)) writable = 0;
    fclose(f);
}

static int save(const Record *r, int id) {
    FILE *f;
    int i, ok;
    if (!writable) return 0;
    f = fopen(DATA_FILE, "a");
    if (!f) return 0;
    if (id < 0) fputs("IN2\n", f);
    else fprintf(f, "EDIT2\n%d\n", id + 1);

    fprintf(f, "%d\n%d\n%d\n%d\n%d\n%d\n%s\n",
            r->emotion, r->intensity, r->needs,
            r->completed, r->checked_out, r->after, r->date);

    for (i = 0; i < 5; i++) fprintf(f, "%s\n", r->text[i]);
    for (i = 0; i < 5; i++) fprintf(f, "%s\n", r->notes[i]);
    fprintf(f, "%s\n", r->review);

    ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;
    if (!ok) { writable = 0; return 0; }

    if (id < 0) records[count++] = *r;
    else records[id] = *r;
    return 1;
}

static void delete_record_item(int idx) {
    if (idx < 0 || idx >= count) return;
    for (int i = idx; i < count - 1; i++) records[i] = records[i + 1];
    count--;
    FILE *f = fopen(DATA_FILE, "w");
    if (f) {
        for (int k = 0; k < count; k++) {
            fprintf(f, "IN2\n%d\n%d\n%d\n%d\n%d\n%d\n%s\n",
                    records[k].emotion, records[k].intensity, records[k].needs,
                    records[k].completed, records[k].checked_out, records[k].after, records[k].date);
            for (int i = 0; i < 5; i++) fprintf(f, "%s\n", records[k].text[i]);
            for (int i = 0; i < 5; i++) fprintf(f, "%s\n", records[k].notes[i]);
            fprintf(f, "%s\n", records[k].review);
        }
        fclose(f);
    }
}

/* =====================================================
   動態字碼收集與高解析字型引擎
   ===================================================== */
static Font font;
static int font_loaded = 0;
static int font_dirty = 1;
static int uniq[MAX_CP];
static int ucount = 0;
static unsigned char seen[0x10000];

static void add_cp(int cp) {
    if (cp < 32 || cp >= 0x10000 || seen[cp]) return;
    seen[cp] = 1;
    if (ucount < MAX_CP) {
        uniq[ucount++] = cp;
        font_dirty = 1;
    }
}

static void need_text(const char *s) {
    int i = 0, b, cp;
    if (!s) return;
    while (s[i]) {
        cp = GetCodepointNext(s + i, &b);
        add_cp(cp);
        i += (b > 0) ? b : 1;
    }
}

static void load_font_engine(void) {
    static const char *paths[] = { "word.ttf", "font.ttf", "C:/Windows/Fonts/msjh.ttc", "C:/Windows/Fonts/kaiu.ttf" };
    if (font_loaded) {
        UnloadFont(font);
        font_loaded = 0;
    }
    for (int i = 0; i < 4; i++) {
        if (FileExists(paths[i])) {
            font = LoadFontEx(paths[i], FONT_BASE_SIZE, uniq, ucount);
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            font_loaded = 1;
            break;
        }
    }
    if (!font_loaded) font = GetFontDefault();
    font_dirty = 0;
}

/* =====================================================
   【關鍵優化】：清晰度 100% 且自然飽滿的加粗文字繪製
   只做極微量(0.5px)對角平滑補強，筆畫紮實不黏字、無殘影重疊！
   ===================================================== */
static inline void DrawTextExBold(const char *text, Vector2 pos, float fontSize, float spacing, Color color) {
    need_text(text);
    // 0.5px 微幅均勻擴展，呈現真正粗體印刷感，筆畫完全不模糊
    DrawTextEx(font, text, (Vector2){ pos.x + 0.5f, pos.y + 0.5f }, fontSize, spacing, color);
    DrawTextEx(font, text, pos, fontSize, spacing, color);
}

static inline void DrawCenteredTextBold(const char *text, Rectangle rec, float fontSize, float spacing, Color color) {
    need_text(text);
    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
    Vector2 pos = {
        rec.x + (rec.width - textSize.x) / 2.0f,
        rec.y + (rec.height - textSize.y) / 2.0f
    };
    DrawTextExBold(text, pos, fontSize, spacing, color);
}

static Sound snd_click;
static void InitCustomSounds(void) {
    int cSamples = 44100 * 0.05;
    short *cData = (short *)malloc(cSamples * sizeof(short));
    for (int i = 0; i < cSamples; i++) {
        float t = (float)i / 44100.0f;
        float env = 1.0f - (float)i / cSamples;
        cData[i] = (short)(sinf(2.0f * PI * 580.0f * t) * 16000.0f * env);
    }
    Wave cWave = { cSamples, 44100, 16, 1, cData };
    snd_click = LoadSoundFromWave(cWave);
    free(cData);
}

static bool DrawInteractiveButton(const char *text, Rectangle rec, Color baseColor, Color textColor, float fontSize) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);
    bool pressed = hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool clicked = hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    if (hovered) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

    Rectangle drawRec = rec;
    if (pressed) {
        drawRec.x += 3;
        drawRec.y += 3;
    } else {
        Rectangle shadowRec = { rec.x + 3, rec.y + 3, rec.width, rec.height };
        DrawRectangleRounded(shadowRec, 0.25f, 6, (Color){ 180, 185, 200, 130 });
    }

    Color col = baseColor;
    if (hovered && !pressed) {
        col = (Color){ (unsigned char)(baseColor.r + (255 - baseColor.r)*0.15f),
                       (unsigned char)(baseColor.g + (255 - baseColor.g)*0.15f),
                       (unsigned char)(baseColor.b + (255 - baseColor.b)*0.15f), 255 };
    }

    DrawRectangleRounded(drawRec, 0.25f, 6, col);
    DrawCenteredTextBold(text, drawRec, fontSize, 1.0f, textColor);

    if (clicked) {
        PlaySound(snd_click);
        return true;
    }
    return false;
}

static int GetDaysInMonth(int year, int month) {
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return 29;
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

static int GetFirstDayOfWeek(int year, int month) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = 1;
    mktime(&t);
    return t.tm_wday;
}

static int FindRecordByDate(const char *date_str) {
    for (int i = 0; i < count; i++) {
        if (strncmp(records[i].date, date_str, 10) == 0) return i;
    }
    return -1;
}

/* =====================================================
   主程式
   ===================================================== */
int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "情緒飯店 - Emotion Hotel");
    InitAudioDevice();
    InitCustomSounds();
    InitParticles(SCREEN_W, SCREEN_H);
    SetTargetFPS(60);

    for (int i = 32; i < 127; i++) add_cp(i);
    need_text(UI_POOL);
    for (int i = 0; i < 6; i++) {
        need_text(names_zh[i]);
        need_text(names_en[i]);
        need_text(welcome[i]);
        need_text(L_FEEDBACK[i]);
    }
    for (int i = 0; i < 12; i++) need_text(needs_list[i]);
    for (int i = 0; i < 6; i++) {
        for (int k = 0; k < service_count[i]; k++) need_text(services[i][k]);
    }

    load();
    for (int i = 0; i < count; i++) {
        need_text(records[i].date);
        need_text(records[i].review);
        for (int k = 0; k < 5; k++) {
            need_text(records[i].text[k]);
            need_text(records[i].notes[k]);
        }
    }
    load_font_engine();

    for (int i = 0; i < NUM_TRACKS; i++) {
        if (FileExists(track_files[i][0])) {
            bgm_tracks[i] = LoadMusicStream(track_files[i][0]);
        } else if (FileExists(track_files[i][1])) {
            bgm_tracks[i] = LoadMusicStream(track_files[i][1]);
        }
    }
    PlayMusicStream(bgm_tracks[0]);
    music_playing = true;

    time_t t_now = time(NULL);
    struct tm *now_tm = localtime(&t_now);
    int cur_year = now_tm->tm_year + 1900;
    int cur_month = now_tm->tm_mon + 1;

    char selected_date[32] = "";
    int active_field = 0;

    while (!WindowShouldClose()) {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        float runTime = (float)GetTime();

                // 專為 Windows 打造的無痛最大化/全螢幕切換
        if (IsKeyPressed(KEY_F11)) {
            if (IsWindowMaximized()) {
                RestoreWindow();
            } else {
                MaximizeWindow();
            }
        }

        if (music_playing) UpdateMusicStream(bgm_tracks[cur_track]);

        Vector2 mouse = GetMousePosition();

        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32) {
                char *target = NULL;
                int cap = TEXT;
                if (page == P_CI_TEXT && active_field >= 1 && active_field <= 5) {
                    target = work.text[active_field - 1];
                } else if (page == P_DETAIL && detail_tab == 2 && active_field == 7) {
                    target = work.notes[cur_service_idx];
                    cap = NOTE;
                } else if (page == P_DETAIL && detail_tab == 3 && active_field == 6) {
                    target = work.review;
                }

                if (target && (int)strlen(target) < cap - 4) {
                    int len = strlen(target);
                    int b = 0;
                    const char *u = CodepointToUTF8(key, &b);
                    memcpy(target + len, u, b);
                    target[len + b] = '\0';
                    add_cp(key);
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            char *target = NULL;
            if (page == P_CI_TEXT && active_field >= 1 && active_field <= 5) target = work.text[active_field - 1];
            else if (page == P_DETAIL && detail_tab == 2 && active_field == 7) target = work.notes[cur_service_idx];
            else if (page == P_DETAIL && detail_tab == 3 && active_field == 6) target = work.review;

            if (target && strlen(target) > 0) {
                int len = strlen(target);
                len--;
                while (len > 0 && (target[len] & 0xC0) == 0x80) len--;
                target[len] = '\0';
            }
        }

        if (font_dirty) load_font_engine();

        BeginDrawing();

        // ==================== 1. 主畫面（四顆大按鈕居中） ====================
        if (page == P_LOBBY) {
            float cardW = 560, cardH = 580;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            Rectangle titleBox = { 0, glassCard.y + 28, SCREEN_W, 60 };
            DrawCenteredTextBold(cur_lang ? "情緒飯店" : "Emotion Hotel", titleBox, 56, 3, (Color){ 40, 50, 75, 255 });

            Rectangle subBox = { 0, glassCard.y + 92, SCREEN_W, 32 };
            DrawCenteredTextBold(cur_lang ? "給每種情緒一個溫暖歇息的房間" : "A quiet haven for every thought and feeling", subBox, 22, 1, (Color){ 105, 120, 140, 255 });

            // 四個大按鈕 (由上到下：開始記錄 -> 音樂設定 -> 語言設定 -> 離開旅館)
            float btnW = 380, btnH = 62, btnX = (SCREEN_W - btnW) / 2.0f;
            Rectangle btnStart = { btnX, glassCard.y + 145, btnW, btnH };
            Rectangle btnMusic = { btnX, glassCard.y + 225, btnW, btnH };
            Rectangle btnLang  = { btnX, glassCard.y + 305, btnW, btnH };
            Rectangle btnExit  = { btnX, glassCard.y + 385, btnW, btnH };

            if (DrawInteractiveButton(cur_lang ? "開始記錄" : "Start Diary", btnStart, (Color){ 76, 175, 80, 255 }, WHITE, 26)) {
                page = P_CALENDAR;
            }
            if (DrawInteractiveButton(cur_lang ? "音樂設定" : "Music Settings", btnMusic, (Color){ 230, 126, 34, 255 }, WHITE, 26)) {
                page = P_MUSIC;
            }
            if (DrawInteractiveButton(cur_lang ? "語言設定" : "Language Settings", btnLang, (Color){ 70, 130, 180, 255 }, WHITE, 26)) {
                page = P_LANG; // 點進去第二頁選擇！
            }
            if (DrawInteractiveButton(cur_lang ? "離開旅館" : "Exit Hotel", btnExit, (Color){ 215, 80, 80, 255 }, WHITE, 26)) {
                break; // 離開
            }

            Rectangle hintBox = { 0, glassCard.y + cardH - 46, SCREEN_W, 30 };
            DrawCenteredTextBold(TextFormat("目前累積入住：%d 筆紀錄", count), hintBox, 18, 1, GRAY);

        // ==================== 2. 獨立語言設定頁面（第二頁） ====================
        } else if (page == P_LANG) {
            float cardW = 900, cardH = 580;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            DrawTextExBold(cur_lang ? "語言設定" : "Language Settings", (Vector2){ glassCard.x + 60, glassCard.y + 50 }, 36, 2, (Color){ 44, 62, 80, 255 });

            Rectangle btnZh = { glassCard.x + 60, glassCard.y + 140, 480, 68 };
            Rectangle btnEn = { glassCard.x + 60, glassCard.y + 235, 480, 68 };

            Color cZh = (cur_lang == 1) ? (Color){ 46, 204, 113, 255 } : (Color){ 235, 238, 245, 255 };
            Color cEn = (cur_lang == 0) ? (Color){ 46, 204, 113, 255 } : (Color){ 235, 238, 245, 255 };

            if (DrawInteractiveButton("繁體中文 (Traditional Chinese)", btnZh, cZh, (cur_lang == 1) ? WHITE : DARKGRAY, 24)) {
                cur_lang = 1;
            }
            if (DrawInteractiveButton("English", btnEn, cEn, (cur_lang == 0) ? WHITE : DARKGRAY, 24)) {
                cur_lang = 0;
            }

            Rectangle btnBack = { glassCard.x + 60, glassCard.y + 450, 160, 54 };
            if (DrawInteractiveButton(cur_lang ? "返回首頁" : "Back", btnBack, (Color){ 140, 150, 165, 255 }, WHITE, 22)) {
                page = P_LOBBY;
            }

        // ==================== 3. 月曆畫面（含回饋絮語） ====================
        } else if (page == P_CALENDAR) {
            float cardW = 1200, cardH = 680;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            DrawTextExBold(TextFormat("%d / %02d", cur_year, cur_month), (Vector2){ glassCard.x + 40, glassCard.y + 20 }, 34, 2, (Color){ 44, 62, 80, 255 });

            Rectangle btnPrev = { glassCard.x + 220, glassCard.y + 18, 48, 40 };
            Rectangle btnNext = { glassCard.x + 280, glassCard.y + 18, 48, 40 };
            Rectangle btnHome = { glassCard.x + cardW - 170, glassCard.y + 18, 130, 42 };

            if (DrawInteractiveButton("<", btnPrev, (Color){ 235, 238, 245, 255 }, BLACK, 24)) {
                if (--cur_month < 1) { cur_month = 12; cur_year--; }
            }
            if (DrawInteractiveButton(">", btnNext, (Color){ 235, 238, 245, 255 }, BLACK, 24)) {
                if (++cur_month > 12) { cur_month = 1; cur_year++; }
            }
            if (DrawInteractiveButton(cur_lang ? "返回首頁" : "Lobby", btnHome, (Color){ 140, 150, 165, 255 }, WHITE, 22)) {
                page = P_LOBBY;
            }

            const char *weeks[7] = { "日", "一", "二", "三", "四", "五", "六" };
            float gridStartX = glassCard.x + 35;
            float cellW = 158, cellH = 66;

            for (int i = 0; i < 7; i++) {
                Rectangle weekBox = { gridStartX + i * (cellW + 4), glassCard.y + 68, cellW, 26 };
                DrawCenteredTextBold(weeks[i], weekBox, 22, 1, (Color){ 140, 150, 165, 255 });
            }

            int days = GetDaysInMonth(cur_year, cur_month);
            int startDay = GetFirstDayOfWeek(cur_year, cur_month);

            int emo_counts[6] = {0};
            int month_records = 0;

            for (int d = 1; d <= days; d++) {
                int cellIdx = (d - 1) + startDay;
                int col = cellIdx % 7;
                int row = cellIdx / 7;
                Rectangle cell = { gridStartX + col * (cellW + 4), glassCard.y + 100 + row * (cellH + 5), cellW, cellH };

                char dateStr[32];
                snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", cur_year, cur_month, d);
                int recIdx = FindRecordByDate(dateStr);

                if (recIdx != -1) {
                    emo_counts[records[recIdx].emotion]++;
                    month_records++;
                }

                Color cellColor = (recIdx != -1) ? ecolor[records[recIdx].emotion] : WHITE;
                bool hovered = CheckCollisionPointRec(mouse, cell);
                if (hovered) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                DrawRectangleRounded(cell, 0.2f, 6, cellColor);
                DrawRectangleLinesEx(cell, hovered ? 2 : 1, hovered ? (Color){ 52, 152, 219, 255 } : (Color){ 225, 230, 240, 255 });

                DrawTextExBold(TextFormat("%d", d), (Vector2){ cell.x + 10, cell.y + 6 }, 20, 1, (recIdx != -1) ? WHITE : DARKGRAY);

                if (recIdx != -1) {
                    Rectangle labelBox = { cell.x, cell.y + 26, cell.width, 32 };
                    DrawCenteredTextBold(cur_lang ? names_zh[records[recIdx].emotion] : names_en[records[recIdx].emotion], labelBox, 20, 1, WHITE);
                }

                if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    PlaySound(snd_click);
                    strcpy(selected_date, dateStr);
                    if (recIdx != -1) {
                        work = records[recIdx];
                        work_idx = recIdx;
                        detail_tab = 0;
                        cur_service_idx = 0;
                        page = P_DETAIL;
                    } else {
                        memset(&work, 0, sizeof(Record));
                        strcpy(work.date, selected_date);
                        work.emotion = 1;
                        work.intensity = 3;
                        work_idx = -1;
                        page = P_CI_EMOTION;
                    }
                }
            }

            Rectangle statPanel = { gridStartX, glassCard.y + 530, cellW * 7 + 24, 130 };
            DrawRectangleRounded(statPanel, 0.15f, 6, (Color){ 255, 255, 255, 235 });
            DrawRectangleLinesEx(statPanel, 1, (Color){ 220, 226, 238, 255 });

            DrawTextExBold("本月情緒回顧 · 管家絮語", (Vector2){ statPanel.x + 25, statPanel.y + 12 }, 22, 1, (Color){ 44, 62, 80, 255 });

            if (month_records == 0) {
                DrawTextExBold("這個月還沒有任何房客入住紀錄喔！點擊上方的日期，為今天的心情留下一抹色彩吧。", 
                               (Vector2){ statPanel.x + 25, statPanel.y + 55 }, 20, 1, (Color){ 120, 130, 145, 255 });
            } else {
                int max_emo = 0;
                for (int i = 1; i < 6; i++) {
                    if (emo_counts[i] > emo_counts[max_emo]) max_emo = i;
                }
                Rectangle statTag = { statPanel.x + 25, statPanel.y + 44, 130, 36 };
                DrawRectangleRounded(statTag, 0.25f, 4, ecolor[max_emo]);
                DrawCenteredTextBold(cur_lang ? names_zh[max_emo] : names_en[max_emo], statTag, 20, 1, WHITE);

                DrawTextExBold(TextFormat("本月共入住 %d 天，其中「%s」最常來訪（共 %d 次）", month_records, names_zh[max_emo], emo_counts[max_emo]), 
                               (Vector2){ statPanel.x + 175, statPanel.y + 50 }, 20, 1, DARKGRAY);
                DrawTextExBold(L_FEEDBACK[max_emo], (Vector2){ statPanel.x + 25, statPanel.y + 90 }, 20, 1, (Color){ 70, 85, 105, 255 });
            }

        // ==================== 4. 入住流程 1：選擇情緒 ====================
        } else if (page == P_CI_EMOTION) {
            float cardW = 1200, cardH = 680;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            DrawTextExBold(TextFormat("選擇今日情緒  |  %s", work.date), (Vector2){ glassCard.x + 40, glassCard.y + 30 }, 30, 2, (Color){ 44, 62, 80, 255 });

            for (int i = 0; i < 6; i++) {
                Rectangle btn = { glassCard.x + 40 + (i % 3) * 380, glassCard.y + 90 + (i / 3) * 110, 350, 90 };
                Color c = (work.emotion == i) ? ecolor[i] : (Color){ 240, 242, 248, 255 };
                if (DrawInteractiveButton(cur_lang ? names_zh[i] : names_en[i], btn, c, (work.emotion == i) ? WHITE : DARKGRAY, 28)) {
                    work.emotion = i;
                }
            }

            DrawTextExBold(welcome[work.emotion], (Vector2){ glassCard.x + 40, glassCard.y + 340 }, 24, 1, (Color){ 200, 90, 120, 255 });

            DrawTextExBold("情緒強度 (1-5)：", (Vector2){ glassCard.x + 40, glassCard.y + 400 }, 24, 1, DARKGRAY);
            for (int i = 1; i <= 5; i++) {
                Rectangle btn = { glassCard.x + 220 + (i - 1) * 85, glassCard.y + 390, 70, 50 };
                Color c = (work.intensity == i) ? (Color){ 52, 73, 94, 255 } : (Color){ 240, 242, 248, 255 };
                if (DrawInteractiveButton(TextFormat("%d", i), btn, c, (work.intensity == i) ? WHITE : BLACK, 24)) {
                    work.intensity = i;
                }
            }

            Rectangle btnBack = { glassCard.x + 40, glassCard.y + 590, 160, 54 };
            Rectangle btnNext = { glassCard.x + 220, glassCard.y + 590, 160, 54 };

            if (DrawInteractiveButton("返回月曆", btnBack, (Color){ 235, 238, 245, 255 }, DARKGRAY, 22)) page = P_CALENDAR;
            if (DrawInteractiveButton("下一步", btnNext, (Color){ 52, 152, 219, 255 }, WHITE, 22)) {
                active_field = 1;
                page = P_CI_TEXT;
            }

        // ==================== 5. 入住流程 2：填寫單據 ====================
        } else if (page == P_CI_TEXT) {
            float cardW = 1200, cardH = 680;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            DrawTextExBold("填寫入住單（點擊即可輸入）", (Vector2){ glassCard.x + 40, glassCard.y + 20 }, 28, 2, (Color){ 44, 62, 80, 255 });

            for (int i = 0; i < 5; i++) {
                DrawTextExBold(fields[i], (Vector2){ glassCard.x + 40, glassCard.y + 70 + i * 98 }, 20, 1, DARKGRAY);
                Rectangle box = { glassCard.x + 40, glassCard.y + 98 + i * 98, 1120, 54 };
                DrawRectangleRounded(box, 0.15f, 4, WHITE);
                DrawRectangleLinesEx(box, (active_field == i + 1) ? 2 : 1, (active_field == i + 1) ? (Color){ 52, 152, 219, 255 } : (Color){ 215, 220, 230, 255 });
                DrawTextExBold(work.text[i], (Vector2){ box.x + 15, box.y + 15 }, 20, 1, BLACK);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, box)) {
                    active_field = i + 1;
                }
            }

            Rectangle btnPrev = { glassCard.x + 40, glassCard.y + 600, 150, 50 };
            Rectangle btnNext = { glassCard.x + 210, glassCard.y + 600, 150, 50 };

            if (DrawInteractiveButton("上一步", btnPrev, (Color){ 235, 238, 245, 255 }, DARKGRAY, 22)) page = P_CI_EMOTION;
            if (DrawInteractiveButton("下一步", btnNext, (Color){ 52, 152, 219, 255 }, WHITE, 22)) page = P_CI_NEEDS;

        // ==================== 6. 入住流程 3：需求複選 ====================
        } else if (page == P_CI_NEEDS) {
            float cardW = 1200, cardH = 680;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            DrawTextExBold("你現在需要什麼？（可複選）", (Vector2){ glassCard.x + 40, glassCard.y + 35 }, 30, 2, (Color){ 44, 62, 80, 255 });

            for (int i = 0; i < 12; i++) {
                Rectangle r = { glassCard.x + 40 + (i % 4) * 285, glassCard.y + 120 + (i / 4) * 110, 265, 80 };
                bool on = (work.needs & (1 << i)) != 0;
                Color nc = on ? (Color){ 46, 204, 113, 255 } : (Color){ 240, 242, 248, 255 };
                if (DrawInteractiveButton(needs_list[i], r, nc, on ? WHITE : DARKGRAY, 24)) {
                    work.needs ^= (1 << i);
                }
            }

            Rectangle btnPrev = { glassCard.x + 40, glassCard.y + 590, 160, 54 };
            Rectangle btnSave = { glassCard.x + 220, glassCard.y + 590, 200, 54 };

            if (DrawInteractiveButton("上一步", btnPrev, (Color){ 235, 238, 245, 255 }, DARKGRAY, 22)) page = P_CI_TEXT;
            if (DrawInteractiveButton("完成入住並保存", btnSave, (Color){ 46, 204, 113, 255 }, WHITE, 22)) {
                save(&work, work_idx);
                page = P_CALENDAR;
            }

        // ==================== 7. 房客詳情（4 分頁整合） ====================
        } else if (page == P_DETAIL) {
            float cardW = 1200, cardH = 680;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            int e = work.emotion;
            DrawTextExBold(TextFormat("房客詳情  |  %s", work.date), (Vector2){ glassCard.x + 40, glassCard.y + 30 }, 30, 2, (Color){ 44, 62, 80, 255 });

            const char *tab_names[4] = { "內容記錄", "需求複選", "Room Service", "回顧／退房" };
            for (int i = 0; i < 4; i++) {
                Rectangle tabRec = { glassCard.x + 40 + i * 200, glassCard.y + 75, 185, 46 };
                Color tc = (detail_tab == i) ? ecolor[e] : (Color){ 235, 238, 245, 255 };
                if (DrawInteractiveButton(tab_names[i], tabRec, tc, (detail_tab == i) ? WHITE : DARKGRAY, 22)) {
                    detail_tab = i;
                    active_field = 0;
                }
            }

            if (detail_tab == 0) {
                for (int i = 0; i < 5; i++) {
                    DrawTextExBold(fields[i], (Vector2){ glassCard.x + 40, glassCard.y + 140 + i * 78 }, 18, 1, DARKGRAY);
                    Rectangle b = { glassCard.x + 40, glassCard.y + 165 + i * 78, 1120, 48 };
                    DrawRectangleRounded(b, 0.15f, 4, WHITE);
                    DrawRectangleLinesEx(b, 1, (Color){ 220, 226, 238, 255 });
                    DrawTextExBold(work.text[i][0] ? work.text[i] : "（留白）", (Vector2){ b.x + 15, b.y + 12 }, 18, 1, BLACK);
                }

            } else if (detail_tab == 1) {
                DrawTextExBold("點選可修改此日期的需求勾選：", (Vector2){ glassCard.x + 40, glassCard.y + 145 }, 22, 1, DARKGRAY);
                for (int i = 0; i < 12; i++) {
                    Rectangle nr = { glassCard.x + 40 + (i % 4) * 285, glassCard.y + 195 + (i / 4) * 75, 265, 56 };
                    bool on = (work.needs & (1 << i)) != 0;
                    Color nc = on ? (Color){ 46, 204, 113, 255 } : (Color){ 240, 242, 248, 255 };
                    if (DrawInteractiveButton(needs_list[i], nr, nc, on ? WHITE : DARKGRAY, 22)) {
                        work.needs ^= (1 << i);
                    }
                }

            } else if (detail_tab == 2) {
                DrawTextExBold("此房間專屬練習（點擊標記完成）：", (Vector2){ glassCard.x + 40, glassCard.y + 140 }, 22, 1, DARKGRAY);
                for (int i = 0; i < service_count[e]; i++) {
                    Rectangle sr = { glassCard.x + 40, glassCard.y + 175 + i * 55, 620, 48 };
                    bool done = (work.completed & (1 << i)) != 0;
                    Color sc = done ? (Color){ 46, 204, 113, 255 } : (Color){ 240, 242, 248, 255 };
                    if (DrawInteractiveButton(TextFormat("[%s] %s", done ? "V" : " ", services[e][i]), sr, sc, done ? WHITE : BLACK, 20)) {
                        work.completed ^= (1 << i);
                        cur_service_idx = i;
                    }
                }
                DrawTextExBold(TextFormat("筆記：%s", services[e][cur_service_idx]), (Vector2){ glassCard.x + 690, glassCard.y + 140 }, 22, 1, (Color){ 200, 90, 120, 255 });
                Rectangle boxNote = { glassCard.x + 690, glassCard.y + 175, 470, 270 };
                DrawRectangleRounded(boxNote, 0.15f, 4, WHITE);
                DrawRectangleLinesEx(boxNote, (active_field == 7) ? 2 : 1, (active_field == 7) ? (Color){ 52, 152, 219, 255 } : (Color){ 215, 220, 230, 255 });
                DrawTextExBold(work.notes[cur_service_idx], (Vector2){ boxNote.x + 15, boxNote.y + 15 }, 20, 1, BLACK);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, boxNote)) active_field = 7;

            } else if (detail_tab == 3) {
                DrawTextExBold("回顧後的情緒強度（0~5）：", (Vector2){ glassCard.x + 40, glassCard.y + 145 }, 22, 1, DARKGRAY);
                for (int i = 0; i <= 5; i++) {
                    Rectangle btn = { glassCard.x + 40 + i * 90, glassCard.y + 185, 75, 48 };
                    Color c = (work.after == i) ? (Color){ 52, 73, 94, 255 } : (Color){ 240, 242, 248, 255 };
                    if (DrawInteractiveButton(TextFormat("%d", i), btn, c, (work.after == i) ? WHITE : BLACK, 22)) {
                        work.after = i;
                    }
                }

                DrawTextExBold("想留下什麼回顧？", (Vector2){ glassCard.x + 40, glassCard.y + 260 }, 22, 1, DARKGRAY);
                Rectangle boxRev = { glassCard.x + 40, glassCard.y + 295, 1120, 75 };
                DrawRectangleRounded(boxRev, 0.15f, 4, WHITE);
                DrawRectangleLinesEx(boxRev, (active_field == 6) ? 2 : 1, (active_field == 6) ? (Color){ 52, 152, 219, 255 } : (Color){ 215, 220, 230, 255 });
                DrawTextExBold(work.review, (Vector2){ boxRev.x + 15, boxRev.y + 20 }, 22, 1, BLACK);

                Rectangle btnCheckOut = { glassCard.x + 40, glassCard.y + 395, 420, 54 };
                Color coc = work.checked_out ? (Color){ 140, 150, 165, 255 } : (Color){ 255, 140, 0, 255 };
                if (DrawInteractiveButton(work.checked_out ? "已退房（點擊改為住宿中）" : "住宿中（點擊辦理退房）", btnCheckOut, coc, WHITE, 20)) {
                    work.checked_out = !work.checked_out;
                }

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, boxRev)) active_field = 6;
            }

            Rectangle btnSaveMod = { glassCard.x + 40, glassCard.y + 590, 180, 54 };
            Rectangle btnDelRec  = { glassCard.x + 240, glassCard.y + 590, 180, 54 };
            Rectangle btnBackCal = { glassCard.x + 440, glassCard.y + 590, 150, 54 };

            if (DrawInteractiveButton("保存所有修改", btnSaveMod, (Color){ 46, 204, 113, 255 }, WHITE, 22)) {
                save(&work, work_idx);
                page = P_CALENDAR;
            }
            if (DrawInteractiveButton("刪除此紀錄", btnDelRec, (Color){ 231, 76, 60, 255 }, WHITE, 22)) {
                delete_record_item(work_idx);
                page = P_CALENDAR;
            }
            if (DrawInteractiveButton("返回月曆", btnBackCal, (Color){ 235, 238, 245, 255 }, DARKGRAY, 22)) {
                page = P_CALENDAR;
            }

        // ==================== 8. 音樂設定 ====================
        } else if (page == P_MUSIC) {
            float cardW = 1000, cardH = 620;
            Rectangle glassCard = { (SCREEN_W - cardW) / 2.0f, (SCREEN_H - cardH) / 2.0f, cardW, cardH };
            DrawUnifiedBackground(SCREEN_W, SCREEN_H, runTime, glassCard);

            DrawTextExBold("音樂設定", (Vector2){ glassCard.x + 60, glassCard.y + 50 }, 36, 2, (Color){ 44, 62, 80, 255 });

            for (int i = 0; i < NUM_TRACKS; i++) {
                Rectangle btnTrack = { glassCard.x + 60, glassCard.y + 140 + i * 85, 480, 68 };
                Color c = (cur_track == i && music_playing) ? (Color){ 46, 204, 113, 255 } : (Color){ 235, 238, 245, 255 };
                if (DrawInteractiveButton(TextFormat("BGM %d: %s", i + 1, track_names[i]), btnTrack, c, (cur_track == i && music_playing) ? WHITE : BLACK, 22)) {
                    StopMusicStream(bgm_tracks[cur_track]);
                    cur_track = i;
                    PlayMusicStream(bgm_tracks[cur_track]);
                    music_playing = true;
                }
            }

            Rectangle btnToggle = { glassCard.x + 60, glassCard.y + 420, 220, 56 };
            Color tCol = music_playing ? (Color){ 231, 76, 60, 255 } : (Color){ 52, 152, 219, 255 };
            if (DrawInteractiveButton(music_playing ? "靜音 / Mute" : "播放 / Play", btnToggle, tCol, WHITE, 22)) {
                if (music_playing) {
                    PauseMusicStream(bgm_tracks[cur_track]);
                    music_playing = false;
                } else {
                    ResumeMusicStream(bgm_tracks[cur_track]);
                    music_playing = true;
                }
            }

            Rectangle btnBack = { glassCard.x + 60, glassCard.y + 510, 160, 54 };
            if (DrawInteractiveButton("返回首頁", btnBack, (Color){ 140, 150, 165, 255 }, WHITE, 22)) {
                page = P_LOBBY;
            }
        }

        EndDrawing();
    }

    UnloadSound(snd_click);
    for (int i = 0; i < NUM_TRACKS; i++) UnloadMusicStream(bgm_tracks[i]);
    CloseAudioDevice();
    UnloadFont(font);
    CloseWindow();
    return 0;
}
