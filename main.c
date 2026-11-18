/*
 * 情緒飯店 Emotion Hotel（raylib 完整合併版）
 *
 * 版本 A（月曆版）：月曆首頁、漂浮光點、毛玻璃卡片、中英切換、背景音樂、
 *                   點擊音效、本月情緒回顧與管家絮語、F11 全螢幕
 * 版本 B（紀錄版）：多筆入住、五格入住單、需求複選、Room Service、
 *                   回顧／退房、檔案保存（emotion_hotel.txt）
 *
 * 兩個版本的功能全部保留。
 * 需要的檔案（放在程式執行的資料夾）：
 *   字型：word.ttf 或 font.ttf（或修改下方 FONT_PATH）
 *   音樂：AEROHEAD - Fragments.mp3、Nettson - Last Promise.mp3、Savfk - Rounding.mp3
 *         （沒有音樂檔也能執行，只是沒有聲音）
 */
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#define SCREEN_W 920
#define SCREEN_H 670
#define MAX_RECORDS 200
#define TEXT 256
#define NOTE 768
#define NUM_TRACKS 3
#define NUM_PARTICLES 35
#define DATA_FILE "emotion_hotel.txt"
#define FONT_SIZE 48
#define MAX_CP 8192

/* 字型路徑：依序嘗試 word.ttf、font.ttf、FONT_PATH、標楷體 */
#define FONT_PATH "C:/Users/Admin/Downloads/Noto_Sans_TC/static/font.ttf.ttf"

/* 介面用到的所有中文字（啟動時先放進字型，避免執行中重載） */
static const char *UI_POOL = "@@POOL@@";

/* =====================================================
   資料結構
   ===================================================== */
typedef struct {
    int emotion;
    int intensity;
    int needs;          /* 12 項需求，用位元記錄 */
    int completed;      /* Room Service 完成狀態，用位元記錄 */
    int checked_out;
    int after;          /* 回顧後的情緒強度 */
    char date[32];      /* "YYYY-MM-DD HH:MM:SS" */
    /* text[0]事件 [1]原因 [2]身體感受 [3]日記 [4]給情緒的話 */
    char text[5][TEXT];
    char notes[5][NOTE];   /* 各項 Room Service 的筆記 */
    char review[TEXT];
} Record;

static Record records[MAX_RECORDS];
static int record_count = 0;
static int writable = 1;

/* 情緒色彩 (天藍/金黃/暗橘/緋紅/海綠/淡紫) */
static const Color emotion_colors[6] = {
    { 100, 149, 237, 255 }, { 255, 204, 0, 255 },  { 255, 140, 0, 255 },
    { 235, 77, 75, 255 },   { 46, 204, 113, 255 }, { 155, 89, 182, 255 }
};

static const Color C_DARK  = { 44, 62, 80, 255 };
static const Color C_GRAYT = { 140, 150, 165, 255 };
static const Color C_LIGHT = { 235, 238, 245, 255 };
static const Color C_CHIP  = { 240, 242, 248, 255 };
static const Color C_ACC   = { 200, 90, 120, 255 };
static const Color C_ROW   = { 255, 240, 244, 255 };

/* =====================================================
   語言字典
   ===================================================== */
typedef enum { LANG_EN = 0, LANG_ZH = 1 } Language;
static Language cur_lang = LANG_ZH;

#define T(a) ((a)[cur_lang])

static const char *L_TITLE[2] = { "Emotion Hotel", "情緒飯店" };
static const char *L_SUBTITLE[2] = { "A quiet haven for every thought and feeling", "給每種情緒一個溫暖歇息的房間" };
static const char *L_BTN_START[2] = { "Start Diary", "開始記錄" };
static const char *L_BTN_LANG[2] = { "Language Settings", "語言設定" };
static const char *L_BTN_MUSIC[2] = { "Music Settings", "音樂設定" };
static const char *L_BTN_QUIT[2] = { "Leave Hotel", "離開飯店" };
static const char *L_BACK[2] = { "Back", "返回" };
static const char *L_CHECKIN[2] = { "Check-in", "入住登記" };
static const char *L_DETAIL[2] = { "Stay Details", "房客詳情" };
static const char *L_EDIT[2] = { "Edit Record", "編輯紀錄" };
static const char *L_DELETE[2] = { "Delete Record", "刪除紀錄" };
static const char *L_DELETE_SURE[2] = { "Click again", "再按一次" };
static const char *L_SAVE[2] = { "Save Check-in", "儲存入住" };
static const char *L_SAVE_CHANGES[2] = { "Save Changes", "保存修改" };
static const char *L_CANCEL[2] = { "Cancel", "取消" };
static const char *L_NEXT[2] = { "Next", "下一步" };
static const char *L_PREV[2] = { "Previous", "上一步" };
static const char *L_EVENT_LABEL[2] = { "What happened today?", "今天發生了什麼事？" };
static const char *L_NOTE_LABEL[2] = { "Message to this emotion:", "想對這位情緒客人說的話：" };
static const char *L_SELECT_LANG[2] = { "Select Language", "選擇語言" };
static const char *L_MONTH_INSIGHT[2] = { "Monthly Insights", "本月情緒回顧與管家絮語" };
static const char *L_INTENSITY[2] = { "Intensity (1-5):", "情緒強度 (1-5)：" };
static const char *L_ALL_RECORDS[2] = { "All Records", "所有紀錄" };
static const char *L_CHECKIN_TODAY[2] = { "Check in Today", "今日入住" };
static const char *L_NEW_CHECKIN[2] = { "New Check-in", "新增入住" };
static const char *L_SHOW_ALL[2] = { "Show All", "顯示全部" };
static const char *L_FILTER_DATE[2] = { "Date: ", "日期：" };
static const char *L_EMPTY_LIST[2] = { "No stay records yet.", "目前沒有入住紀錄。" };
static const char *L_STAYING[2] = { "Staying", "住宿中" };
static const char *L_CHECKED_OUT[2] = { "Checked out", "已退房" };
static const char *L_BLANK[2] = { "(blank)", "（留白）" };
static const char *L_PLACEHOLDER[2] = { "Click to type", "點此輸入" };
static const char *L_DONE[2] = { "Completed", "已完成" };
static const char *L_NOT_DONE[2] = { "Not marked", "未標記完成" };
static const char *L_NOTES_TITLE[2] = { "Notes", "筆記" };
static const char *L_NEEDS_TITLE[2] = { "What do you need right now?", "你現在需要什麼？" };
static const char *L_NEEDS_HINT[2] = { "Tap to select, tap again to cancel", "可複選，再點一次可取消" };
static const char *L_NEEDS_HINT_DETAIL[2] = { "Tap to toggle, then press Save Changes.", "點選項目可勾選或取消，記得按下方的保存修改。" };
static const char *L_REVIEW_INT[2] = { "Intensity after review (0 = skip)", "回顧後的情緒強度（0 = 略過）" };
static const char *L_REVIEW_LABEL[2] = { "Anything to leave as a review? (optional)", "想留下什麼回顧？（可留白）" };
static const char *L_CHECKOUT_BTN[2][2] = {
    { "Staying (click to check out)", "Checked out (click to stay)" },
    { "住宿中（點此退房）", "已退房（點此改回住宿中）" }
};

static const char *L_TABS[4][2] = {
    { "Content", "內容" }, { "Needs", "需求" },
    { "Room Service", "Room Service" }, { "Review / Check-out", "回顧／退房" }
};

static const char *L_EMOTION_NAMES[2][6] = {
    { "Sad", "Happy", "Anxious", "Angry", "Calm", "Tired" },
    { "悲傷", "快樂", "焦慮", "生氣", "平靜", "疲憊" }
};
static const char *L_WEEKS[2][7] = {
    { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" },
    { "日", "一", "二", "三", "四", "五", "六" }
};

/* 入住時的歡迎語 */
static const char *L_WELCOME[6][2] = {
    { "Let the rain keep you company for a while.", "讓雨聲先陪你待一下。" },
    { "Let this light stay a little longer.", "讓這份光多停留一會。" },
    { "You don't need to sort everything out at once.", "不用一次整理完所有事情。" },
    { "First, find a safe place for this strength.", "先替這股力量找一個安全的位置。" },
    { "No need to be calmer. Just feel the present.", "不需要追求更平靜，只要感覺現在。" },
    { "Let your body and heart use a little less strength.", "讓身體和心都少用一點力。" }
};

/* 入住單其餘三格（第 1、5 格使用 L_EVENT_LABEL / L_NOTE_LABEL） */
static const char *L_FIELDS_MID[3][2] = {
    { "Why do you think this emotion came?", "你覺得這個情緒為什麼來了？" },
    { "What does your body feel right now?", "身體現在有什麼感覺？" },
    { "Free diary", "自由日記" }
};

/* 12 項需求 */
static const char *L_NEEDS[12][2] = {
    { "Understood", "被理解" }, { "Accompanied", "被陪伴" },
    { "Time alone", "一個人待著" }, { "A good cry", "哭一下" },
    { "Rest", "休息" }, { "Talk to someone", "找人說話" },
    { "Solve problems", "解決問題" }, { "Exercise", "運動" },
    { "Write it down", "寫下來" }, { "Listen to music", "聽音樂" },
    { "Sleep", "睡一覺" }, { "I don't know", "我也不知道" }
};

/* 每個房間不同的 Room Service */
static const char *L_SERVICES[6][5][2] = {
    {
        { "Give yourself some space", "給自己一點空間" },
        { "Listen to a song", "聽一首歌" },
        { "Hug yourself", "抱住自己" },
        { "Drink warm water", "喝一杯溫水" },
        { "Allow yourself to cry", "允許自己哭一下" }
    },
    {
        { "Keep a happy card", "留下一張快樂小卡" },
        { "Remember today's goodness", "記住今天的美好" },
        { "Leave a note for future you", "留一句話給未來的自己" },
        { NULL, NULL }, { NULL, NULL }
    },
    {
        { "Take a breath", "呼吸一下" },
        { "Sort your worries", "把擔心分類" },
        { "Write one thing you can do now", "寫下現在能做的一件事" },
        { "Put your phone down for a while", "暫時放下手機" },
        { NULL, NULL }
    },
    {
        { "Put this paper down", "放下這張紙" },
        { "Write what you want to say", "寫下想說的話" },
        { "Take a breath", "呼吸一下" },
        { "Step away from the trigger", "暫時離開刺激來源" },
        { NULL, NULL }
    },
    {
        { "Take a breath", "呼吸一下" },
        { "Have some tea", "喝茶" },
        { "Notice five things around you", "感受周圍五件事" },
        { "Sit quietly for a moment", "安靜坐一下" },
        { NULL, NULL }
    },
    {
        { "Rest", "休息" },
        { "Drink some water", "補水" },
        { "Mute some notifications", "關閉部分通知" },
        { "Get ready for bed", "睡前準備" },
        { NULL, NULL }
    }
};
static const int service_count[6] = { 5, 3, 4, 4, 4, 4 };

/* 6 種情緒的專屬管家溫暖回饋語錄 */
static const char *L_FEEDBACK[6][2] = {
    { "Sadness visited often this month. Rain nourishes the soil; take gentle care of yourself.",
      "這個月「悲傷」常來作客。雨水洗刷過的地方總會萌發新芽，記得給自己多一點擁抱與允許。" },
    { "Joy shone brightly this month! Keep these golden memories safe in your pocket.",
      "這個月充滿了「快樂」的金色光芒！將這份明亮收藏在心底，作為未來前行的力量。" },
    { "Anxiety knocked frequently. You don't have to figure out everything at once. Breathe deeply.",
      "這個月「焦慮」有些頻繁。不用急著一次解決所有難題，一步一步來，深呼吸，你做得很好了。" },
    { "Anger showed up this month. It marks what you value deeply; give it a safe harbor to be felt.",
      "這個月感受到了不少「生氣」與起伏。這代表你重視自己的邊界，先找個安全的位置好好安置它。" },
    { "Calmness flowed through your days. Peace is a subtle power; enjoy this tranquil season.",
      "這個月主旋律是「平靜」。能與日常平和相處是一種溫柔的力量，願你繼續安住在這份從容中。" },
    { "Exhaustion has lingered. Your body and heart are whispering for true rest. Sleep well tonight.",
      "這個月累積了不少「疲憊」。你的身心正在提醒你放慢腳步，今晚放下所有包袱，好好睡個飽覺吧。" }
};

/* 狀態訊息 */
static const char *M_SAVED[2] = { "Saved.", "已保存。" };
static const char *M_LIMIT[2] = { "Record limit reached.", "紀錄已達上限。" };
static const char *M_NO_WRITE[2] = { "Cannot write data right now.", "目前不能寫入資料。" };
static const char *M_NO_SAVE[2] = { "Unable to save.", "無法保存。" };
static const char *M_SAVE_FAIL[2] = { "Save failed. Please back up and check the file.", "保存失敗，請先備份並檢查檔案。" };
static const char *M_BAD_FILE[2] = { "Data file is incomplete; saving paused. Please back it up and check it.", "資料檔不完整，已暫停寫入；請先備份並檢查檔案。" };
static const char *M_READONLY[2] = { "Data file opened read-only.", "資料檔以唯讀模式開啟。" };
static const char *M_NO_OPEN[2] = { "Cannot open the record file.", "無法開啟紀錄檔。" };
static const char *M_DEL_SURE[2] = { "Press Delete once more to confirm.", "再按一次刪除即可確認。" };
static const char *M_DELETED[2] = { "Record deleted.", "已刪除紀錄。" };

/* =====================================================
   狀態訊息列
   ===================================================== */
static char status[160];
static double status_until = 0;

static void say(const char **m)
{
    strncpy(status, T(m), sizeof status - 1);
    status[sizeof status - 1] = '\0';
    status_until = GetTime() + 4.0;
}

/* =====================================================
   檔案讀寫（格式與主控台版相同：IN2 / EDIT2）
   ===================================================== */

/* 1：成功；0：檔案結束；-1：過長或不完整 */
static int line(FILE *f, char *s, int size)
{
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

static int number(const char *s, int low, int high, int *out)
{
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

static int read_number(FILE *f, int low, int high, int *out)
{
    char s[64];
    return line(f, s, sizeof s) == 1 && number(s, low, high, out);
}

static int read_record(FILE *f, Record *r)
{
    int i;

    memset(r, 0, sizeof *r);

    if (!read_number(f, 0, 5, &r->emotion) ||
        !read_number(f, 1, 5, &r->intensity) ||
        !read_number(f, 0, 4095, &r->needs) ||
        !read_number(f, 0, 31, &r->completed) ||
        !read_number(f, 0, 1, &r->checked_out) ||
        !read_number(f, 0, 5, &r->after) ||
        line(f, r->date, sizeof r->date) != 1) {
        return 0;
    }
    for (i = 0; i < 5; i++)
        if (line(f, r->text[i], TEXT) != 1) return 0;
    for (i = 0; i < 5; i++)
        if (line(f, r->notes[i], NOTE) != 1) return 0;
    return line(f, r->review, TEXT) == 1;
}

static void load(void)
{
    FILE *f = fopen(DATA_FILE, "a+");
    char tag[32];
    int result;
    int id;
    Record r;

    if (!f) {
        writable = 0;
        f = fopen(DATA_FILE, "r");
        if (!f) {
            say(M_NO_OPEN);
            return;
        }
        say(M_READONLY);
    }

    rewind(f);

    while ((result = line(f, tag, sizeof tag)) != 0) {
        if (result != 1) break;

        if (!strcmp(tag, "IN2")) {
            if (record_count >= MAX_RECORDS || !read_record(f, &r)) break;
            records[record_count++] = r;

        } else if (!strcmp(tag, "EDIT2")) {
            if (!read_number(f, 1, record_count, &id) || !read_record(f, &r)) break;
            records[id - 1] = r;

        } else if (!strcmp(tag, "IN")) {
            /* 讀取最早版本的入住紀錄 */
            memset(&r, 0, sizeof r);
            if (record_count >= MAX_RECORDS ||
                !read_number(f, 0, 5, &r.emotion) ||
                !read_number(f, 1, 5, &r.intensity) ||
                line(f, r.date, sizeof r.date) != 1 ||
                line(f, r.text[0], TEXT) != 1 ||
                line(f, r.text[3], TEXT) != 1) {
                break;
            }
            records[record_count++] = r;

        } else if (!strcmp(tag, "OUT")) {
            if (!read_number(f, 1, record_count, &id)) break;
            records[id - 1].checked_out = 1;

        } else {
            break;
        }
    }

    if (result != 0 || ferror(f)) {
        writable = 0;
        say(M_BAD_FILE);
    }
    fclose(f);
}

static void write_body(FILE *f, const Record *r)
{
    int i;

    fprintf(f, "%d\n%d\n%d\n%d\n%d\n%d\n%s\n",
            r->emotion, r->intensity, r->needs,
            r->completed, r->checked_out, r->after, r->date);
    for (i = 0; i < 5; i++) fprintf(f, "%s\n", r->text[i]);
    for (i = 0; i < 5; i++) fprintf(f, "%s\n", r->notes[i]);
    fprintf(f, "%s\n", r->review);
}

/* id 為 -1：新增；其他值：更新既有紀錄 */
static int save(const Record *r, int id)
{
    FILE *f;
    int ok;

    if (!writable) {
        say(M_NO_WRITE);
        return 0;
    }

    f = fopen(DATA_FILE, "a");
    if (!f) {
        say(M_NO_SAVE);
        return 0;
    }

    if (id < 0) fputs("IN2\n", f);
    else fprintf(f, "EDIT2\n%d\n", id + 1);
    write_body(f, r);

    ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;

    if (!ok) {
        writable = 0;
        say(M_SAVE_FAIL);
        return 0;
    }

    if (id < 0) records[record_count++] = *r;
    else records[id] = *r;

    say(M_SAVED);
    return 1;
}

/* 刪除後把剩下的紀錄整份重寫（先寫暫存檔，成功後才取代原檔） */
static int rewrite_all(void)
{
    FILE *f;
    int i, ok;
    const char *tmp = DATA_FILE ".tmp";

    if (!writable) {
        say(M_NO_WRITE);
        return 0;
    }

    f = fopen(tmp, "w");
    if (!f) {
        say(M_NO_SAVE);
        return 0;
    }
    for (i = 0; i < record_count; i++) {
        fputs("IN2\n", f);
        write_body(f, &records[i]);
    }
    ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;

    if (!ok) {
        remove(tmp);
        say(M_SAVE_FAIL);
        return 0;
    }
    remove(DATA_FILE);
    if (rename(tmp, DATA_FILE) != 0) {
        writable = 0;
        say(M_SAVE_FAIL);
        return 0;
    }
    return 1;
}

/* =====================================================
   字型：只載入用得到的字，輸入新字時自動重載
   ===================================================== */
static Font font;
static int font_loaded = 0;
static int font_dirty = 1;
static int uniq[MAX_CP];
static int ucount = 0;
static unsigned char seen[0x10000];

static void add_cp(int cp)
{
    if (cp < 32 || cp >= 0x10000 || seen[cp]) return;
    seen[cp] = 1;
    if (ucount < MAX_CP) {
        uniq[ucount++] = cp;
        font_dirty = 1;
    }
}

static void need_text(const char *s)
{
    int i = 0, b, cp;
    while (s[i]) {
        cp = GetCodepointNext(s + i, &b);
        add_cp(cp);
        i += (b > 0) ? b : 1;
    }
}

static void load_font(void)
{
    static const char *paths[] = {
        "word.ttf", "font.ttf", FONT_PATH, "C:/Windows/Fonts/kaiu.ttf"
    };
    int i;

    if (font_loaded) {
        UnloadFont(font);
        font_loaded = 0;
    }
    for (i = 0; i < 4; i++) {
        if (FileExists(paths[i])) {
            font = LoadFontEx(paths[i], FONT_SIZE, uniq, ucount);
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            font_loaded = 1;
            break;
        }
    }
    if (!font_loaded) font = GetFontDefault();
    font_dirty = 0;
}

/* =====================================================
   文字繪製（自己逐字繪製，才能換行、量寬度與支援動態字型）
   ===================================================== */
static float cx_end, cy_end;   /* 最近一次 wrap 結束的位置（畫游標用） */

static float glyph_w(int cp, float size)
{
    int idx = GetGlyphIndex(font, cp);
    float adv = (float)font.glyphs[idx].advanceX;
    if (adv <= 0) adv = font.recs[idx].width;
    return adv * size / (float)font.baseSize;
}

static float text_w(const char *s, float size, float sp)
{
    float w = 0;
    int i = 0, b, cp;
    while (s[i]) {
        cp = GetCodepointNext(s + i, &b);
        w += glyph_w(cp, size) + sp;
        i += (b > 0) ? b : 1;
    }
    return w;
}

static void text_ex(const char *s, float x, float y, float size, float sp, Color c)
{
    float cx = x;
    int i = 0, b, cp;

    need_text(s);
    while (s[i]) {
        cp = GetCodepointNext(s + i, &b);
        i += (b > 0) ? b : 1;
        if (cp < 32) continue;
        DrawTextCodepoint(font, cp, (Vector2){cx, y}, size, c);
        cx += glyph_w(cp, size) + sp;
    }
}

static void text(const char *s, float x, float y, float size, Color c)
{
    text_ex(s, x, y, size, 0, c);
}

/* 加粗文字 */
static void text_bold(const char *s, float x, float y, float size, float sp, Color c)
{
    text_ex(s, x + 1.0f, y, size, sp, c);
    text_ex(s, x, y + 1.0f, size, sp, c);
    text_ex(s, x + 1.0f, y + 1.0f, size, sp, c);
    text_ex(s, x, y, size, sp, c);
}

static void center_bold(const char *s, Rectangle rec, float size, float sp, Color c)
{
    float tw = text_w(s, size, sp);
    text_bold(s, rec.x + (rec.width - tw) / 2.0f,
              rec.y + (rec.height - size) / 2.0f, size, sp, c);
}

/* 自動換行；dry=1 只計算不繪製；回傳行數 */
static int wrap(const char *s, float x, float y, float w, float size,
                Color c, int dry)
{
    float cx = x, cy = y, lh = size * 1.35f, gw;
    int i = 0, b, cp, lines = 1;

    need_text(s);
    while (s[i]) {
        cp = GetCodepointNext(s + i, &b);
        i += (b > 0) ? b : 1;
        if (cp < 32) continue;
        gw = glyph_w(cp, size);
        if (cx + gw > x + w) {
            cx = x;
            cy += lh;
            lines++;
        }
        if (!dry) DrawTextCodepoint(font, cp, (Vector2){cx, cy}, size, c);
        cx += gw;
    }
    cx_end = cx;
    cy_end = cy;
    return lines;
}

/* =====================================================
   漂浮光點、背景、按鈕、音效
   ===================================================== */
typedef struct {
    Vector2 pos;
    float radius;
    float speedY;
    float waveOffset;
    Color color;
} Particle;

static Particle particles[NUM_PARTICLES];

static void InitParticles(int w, int h)
{
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].pos = (Vector2){ (float)(rand() % w), (float)(rand() % h) };
        particles[i].radius = 10.0f + (float)(rand() % 22);
        particles[i].speedY = 0.25f + (float)(rand() % 45) / 100.0f;
        particles[i].waveOffset = (float)(rand() % 360);
        Color base = emotion_colors[rand() % 6];
        particles[i].color = (Color){ base.r, base.g, base.b, 65 };
    }
}

static void UpdateAndDrawParticles(int w, int h, float time)
{
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].pos.y -= particles[i].speedY;
        particles[i].pos.x += sinf(time * 0.7f + particles[i].waveOffset) * 0.35f;
        if (particles[i].pos.y < -particles[i].radius * 2) {
            particles[i].pos.y = h + particles[i].radius * 2;
            particles[i].pos.x = (float)(rand() % w);
        }
        DrawCircleV(particles[i].pos, particles[i].radius, particles[i].color);
        DrawCircleV(particles[i].pos, particles[i].radius * 0.45f, (Color){ 255, 255, 255, 90 });
    }
}

/* 統一繪製漸層底色與毛玻璃卡片 */
static void DrawUnifiedBackground(int w, int h, float time, Rectangle glassCard)
{
    DrawRectangleGradientV(0, 0, w, h, (Color){ 255, 242, 238, 255 }, (Color){ 232, 242, 252, 255 });
    UpdateAndDrawParticles(w, h, time);
    if (glassCard.width > 0 && glassCard.height > 0) {
        DrawRectangleRounded(glassCard, 0.08f, 8, (Color){ 255, 255, 255, 215 });
        DrawRectangleRoundedLines(glassCard, 0.08f, 8, (Color){ 255, 255, 255, 250 });
    }
}

static void BG(Rectangle glassCard)
{
    DrawUnifiedBackground(SCREEN_W, SCREEN_H, (float)GetTime(), glassCard);
}

/* 點擊音效 */
static Sound snd_click;
static void InitCustomSounds(void)
{
    int cSamples = (int)(44100 * 0.05);
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

/* 凹陷立體按鈕 */
static bool DrawInteractiveButton(const char *label, Rectangle rec, Color baseColor,
                                  Color textColor, float fontSize)
{
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
        col = (Color){ (unsigned char)(baseColor.r + (255 - baseColor.r) * 0.15f),
                       (unsigned char)(baseColor.g + (255 - baseColor.g) * 0.15f),
                       (unsigned char)(baseColor.b + (255 - baseColor.b) * 0.15f), 255 };
    }

    DrawRectangleRounded(drawRec, 0.25f, 6, col);
    center_bold(label, drawRec, fontSize, 1.0f, textColor);

    if (clicked) {
        PlaySound(snd_click);
        return true;
    }
    return false;
}

/* =====================================================
   文字輸入框
   ===================================================== */
static char *edit = NULL;
static int edit_cap = 0;
static int field_hit = 0;

static void field(Rectangle r, char *buf, int cap, float size)
{
    Vector2 m = GetMousePosition();
    float lh = size * 1.35f, off;
    int lines, active;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(m, r)) {
        edit = buf;
        edit_cap = cap;
        field_hit = 1;
    }
    active = (edit == buf);

    DrawRectangleRounded(r, 0.15f, 4, WHITE);
    DrawRectangleLinesEx(r, active ? 2 : 1,
                         active ? (Color){ 52, 152, 219, 255 } : (Color){ 215, 220, 230, 255 });

    lines = wrap(buf, r.x + 14, r.y + 8, r.width - 28, size, BLACK, 1);
    off = lines * lh - (r.height - 16);
    if (off < 0) off = 0;

    BeginScissorMode((int)r.x + 2, (int)r.y + 2, (int)r.width - 4, (int)r.height - 4);
    if (!buf[0] && !active) text(T(L_PLACEHOLDER), r.x + 14, r.y + 8, size, GRAY);
    wrap(buf, r.x + 14, r.y + 8 - off, r.width - 28, size, BLACK, 0);
    if (active && ((int)(GetTime() * 2) % 2 == 0))
        DrawRectangle((int)cx_end + 1, (int)cy_end, 2, (int)size, (Color){ 52, 152, 219, 255 });
    EndScissorMode();
}

static void handle_input(void)
{
    int ch, len, b;
    const char *u;

    if (!edit) {
        while (GetCharPressed() > 0) { }
        return;
    }

    while ((ch = GetCharPressed()) > 0) {
        if (ch >= 32 && ch != 127) {
            u = CodepointToUTF8(ch, &b);
            len = (int)strlen(edit);
            if (len + b < edit_cap) {
                memcpy(edit + len, u, (size_t)b);
                edit[len + b] = '\0';
                add_cp(ch);
            }
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        len = (int)strlen(edit);
        if (len > 0) {
            len--;
            while (len > 0 && (edit[len] & 0xC0) == 0x80) len--;
            edit[len] = '\0';
        }
    }
}

/* =====================================================
   日曆與紀錄工具
   ===================================================== */
static int GetDaysInMonth(int year, int month)
{
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return 29;
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

static int GetFirstDayOfWeek(int year, int month)
{
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = 1;
    mktime(&t);
    return t.tm_wday;
}

/* 回傳該日期的紀錄筆數；latest 為當天最新一筆的編號 */
static int CountRecordsOnDate(const char *date_str, int *latest)
{
    int n = 0, i;
    *latest = -1;
    for (i = 0; i < record_count; i++) {
        if (strncmp(records[i].date, date_str, 10) == 0) {
            n++;
            *latest = i;
        }
    }
    return n;
}

static void DeleteRecord(int idx)
{
    if (idx < 0 || idx >= record_count) return;
    for (int i = idx; i < record_count - 1; i++) {
        records[i] = records[i + 1];
    }
    record_count--;
}

static void today_str(char *out, int size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t) strftime(out, (size_t)size, "%Y-%m-%d", t);
    else snprintf(out, (size_t)size, "1970-01-01");
}

/* =====================================================
   應用程式狀態
   ===================================================== */
typedef enum {
    STATE_MENU,
    STATE_CALENDAR,
    STATE_CHECKIN,        /* 入住 1/3：情緒與強度 */
    STATE_CHECKIN_TEXT,   /* 入住 2/3：五格入住單 */
    STATE_CHECKIN_NEEDS,  /* 入住 3/3：需求複選 */
    STATE_DETAIL,
    STATE_LIST,
    STATE_MUSIC_SETTING,
    STATE_LANG_SETTING
} AppState;

static AppState state = STATE_MENU;
static int quit = 0;

static int cur_year, cur_month;
static char selected_date[32] = "";   /* 入住日期 YYYY-MM-DD */
static char filter_date[16] = "";     /* 紀錄清單的日期篩選，空字串表示全部 */
static Record work;                   /* 目前編輯中的紀錄 */
static int editing_idx = -1;          /* -1 表示新入住 */
static int tab = 0;
static int svc_sel = 0;
static int del_confirm = 0;
static float list_scroll = 0;

/* 音樂 */
static const char *track_files[NUM_TRACKS] = { "AEROHEAD - Fragments.mp3", "Nettson - Last Promise.mp3", "Savfk - Rounding.mp3" };
static const char *track_names[NUM_TRACKS] = { "AEROHEAD - Fragments", "Nettson - Last Promise", "Savfk - Rounding" };
static Music bgm_tracks[NUM_TRACKS];
static int cur_track = 0;
static bool music_playing = false;

static void go(AppState s)
{
    state = s;
    edit = NULL;
}

static void start_checkin(const char *day)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (record_count >= MAX_RECORDS) {
        say(M_LIMIT);
        return;
    }
    memset(&work, 0, sizeof work);
    work.emotion = 1;
    work.intensity = 3;
    editing_idx = -1;
    snprintf(selected_date, sizeof selected_date, "%s", day);
    snprintf(work.date, sizeof work.date, "%s %02d:%02d:%02d", day,
             t ? t->tm_hour : 0, t ? t->tm_min : 0, t ? t->tm_sec : 0);
    go(STATE_CHECKIN);
}

static void open_detail(int idx)
{
    work = records[idx];
    editing_idx = idx;
    tab = 0;
    svc_sel = 0;
    del_confirm = 0;
    go(STATE_DETAIL);
}

static const char *field_label(int i)
{
    if (i == 0) return T(L_EVENT_LABEL);
    if (i == 4) return T(L_NOTE_LABEL);
    return T(L_FIELDS_MID[i - 1]);
}

/* 需求複選：4 欄 3 列 */
static void draw_needs(float x0, float y0, float row_h, float size)
{
    int i;
    for (i = 0; i < 12; i++) {
        Rectangle r = { x0 + (i % 4) * 201, y0 + (i / 4) * row_h, 190, row_h - 12 };
        int on = (work.needs & (1 << i)) != 0;
        if (DrawInteractiveButton(T(L_NEEDS[i]), r, on ? emotion_colors[4] : C_CHIP,
                                  on ? WHITE : DARKGRAY, size))
            work.needs ^= 1 << i;
        if (on) DrawRectangleLinesEx(r, 2, C_DARK);
    }
}

/* =====================================================
   各個畫面
   ===================================================== */

/* ---------- 1. 主畫面 ---------- */
static void page_menu(void)
{
    Rectangle glassCard = { 200, 75, 520, 520 };
    BG(glassCard);

    Rectangle titleBox = { 0, 105, SCREEN_W, 60 };
    center_bold(T(L_TITLE), titleBox, 54, 3, (Color){ 40, 50, 75, 255 });

    Rectangle subBox = { 0, 172, SCREEN_W, 32 };
    center_bold(T(L_SUBTITLE), subBox, 22, 1, (Color){ 105, 120, 140, 255 });

    Rectangle btnStart = { 285, 225, 350, 64 };
    Rectangle btnLang  = { 285, 305, 350, 64 };
    Rectangle btnMusic = { 285, 385, 350, 64 };
    Rectangle btnQuit  = { 285, 465, 350, 64 };

    if (DrawInteractiveButton(T(L_BTN_START), btnStart, (Color){ 76, 175, 80, 255 }, WHITE, 26))
        go(STATE_CALENDAR);
    if (DrawInteractiveButton(T(L_BTN_LANG), btnLang, (Color){ 70, 130, 180, 255 }, WHITE, 26))
        go(STATE_LANG_SETTING);
    if (DrawInteractiveButton(T(L_BTN_MUSIC), btnMusic, (Color){ 230, 126, 34, 255 }, WHITE, 26))
        go(STATE_MUSIC_SETTING);
    if (DrawInteractiveButton(T(L_BTN_QUIT), btnQuit, C_GRAYT, WHITE, 26))
        quit = 1;

    text_ex("[F11: Fullscreen / 全螢幕]", 20, SCREEN_H - 35, 18, 1, (Color){ 140, 150, 170, 255 });
}

/* ---------- 2. 語言設定 ---------- */
static void page_lang(void)
{
    Rectangle glassCard = { 60, 40, 800, 580 };
    BG(glassCard);

    text_bold(T(L_SELECT_LANG), 100, 75, 36, 2, C_DARK);

    Rectangle btnZh = { 100, 160, 420, 64 };
    Rectangle btnEn = { 100, 255, 420, 64 };

    Color cZh = (cur_lang == LANG_ZH) ? (Color){ 46, 204, 113, 255 } : C_LIGHT;
    Color cEn = (cur_lang == LANG_EN) ? (Color){ 46, 204, 113, 255 } : C_LIGHT;

    if (DrawInteractiveButton("繁體中文", btnZh, cZh, (cur_lang == LANG_ZH) ? WHITE : DARKGRAY, 24))
        cur_lang = LANG_ZH;
    if (DrawInteractiveButton("English", btnEn, cEn, (cur_lang == LANG_EN) ? WHITE : DARKGRAY, 24))
        cur_lang = LANG_EN;

    Rectangle btnBack = { 100, 450, 150, 52 };
    if (DrawInteractiveButton(T(L_BACK), btnBack, C_GRAYT, WHITE, 22))
        go(STATE_MENU);
}

/* ---------- 3. 月曆 ---------- */
static void page_calendar(void)
{
    Vector2 mouse = GetMousePosition();
    Rectangle glassCard = { 35, 10, 850, 645 };
    char today[16];
    BG(glassCard);

    text_bold(TextFormat("%d / %02d", cur_year, cur_month), 60, 22, 32, 2, C_DARK);

    Rectangle btnPrev = { 220, 22, 45, 36 };
    Rectangle btnNext = { 275, 22, 45, 36 };
    Rectangle btnAll  = { 335, 22, 130, 38 };
    Rectangle btnToday = { 475, 22, 150, 38 };
    Rectangle btnHome = { 745, 22, 115, 38 };

    if (DrawInteractiveButton("<", btnPrev, C_LIGHT, BLACK, 24)) {
        if (--cur_month < 1) { cur_month = 12; cur_year--; }
    }
    if (DrawInteractiveButton(">", btnNext, C_LIGHT, BLACK, 24)) {
        if (++cur_month > 12) { cur_month = 1; cur_year++; }
    }
    if (DrawInteractiveButton(T(L_ALL_RECORDS), btnAll, (Color){ 52, 152, 219, 255 }, WHITE, 18)) {
        filter_date[0] = '\0';
        list_scroll = 0;
        go(STATE_LIST);
    }
    if (DrawInteractiveButton(T(L_CHECKIN_TODAY), btnToday, (Color){ 76, 175, 80, 255 }, WHITE, 18)) {
        today_str(today, sizeof today);
        start_checkin(today);
    }
    if (DrawInteractiveButton(T(L_BACK), btnHome, C_GRAYT, WHITE, 22))
        go(STATE_MENU);

    for (int i = 0; i < 7; i++) {
        Rectangle weekBox = { 50 + i * 118, 62, 105, 25 };
        center_bold(L_WEEKS[cur_lang][i], weekBox, 20, 1, C_GRAYT);
    }

    int days = GetDaysInMonth(cur_year, cur_month);
    int startDay = GetFirstDayOfWeek(cur_year, cur_month);

    int emo_counts[6] = {0};
    int month_total_records = 0;
    int month_days = 0;
    char month_prefix[16];
    snprintf(month_prefix, sizeof month_prefix, "%04d-%02d", cur_year, cur_month);

    for (int i = 0; i < record_count; i++) {
        if (strncmp(records[i].date, month_prefix, 7) == 0) {
            emo_counts[records[i].emotion]++;
            month_total_records++;
        }
    }

    for (int d = 1; d <= days; d++) {
        int cellIdx = (d - 1) + startDay;
        int col = cellIdx % 7;
        int row = cellIdx / 7;
        Rectangle cell = { 50 + col * 118, 92 + row * 66, 108, 60 };

        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", cur_year, cur_month, d);
        int latest = -1;
        int n = CountRecordsOnDate(dateStr, &latest);
        if (n > 0) month_days++;

        Color cellColor = (n > 0) ? emotion_colors[records[latest].emotion] : WHITE;
        bool hovered = CheckCollisionPointRec(mouse, cell);
        if (hovered) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

        DrawRectangleRounded(cell, 0.2f, 6, cellColor);
        DrawRectangleLinesEx(cell, hovered ? 2 : 1, hovered ? (Color){ 52, 152, 219, 255 } : (Color){ 225, 230, 240, 255 });

        text_bold(TextFormat("%d", d), cell.x + 8, cell.y + 5, 18, 1, (n > 0) ? WHITE : DARKGRAY);

        if (n > 0) {
            Rectangle labelBox = { cell.x, cell.y + 24, cell.width, 30 };
            center_bold(L_EMOTION_NAMES[cur_lang][records[latest].emotion], labelBox, 19, 1, WHITE);
            if (n > 1)
                text_bold(TextFormat("x%d", n), cell.x + cell.width - 30, cell.y + 5, 16, 1, WHITE);
        }

        if (hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            PlaySound(snd_click);
            snprintf(selected_date, sizeof selected_date, "%s", dateStr);
            if (n == 0) {
                start_checkin(dateStr);
            } else if (n == 1) {
                open_detail(latest);
            } else {
                snprintf(filter_date, sizeof filter_date, "%s", dateStr);
                list_scroll = 0;
                go(STATE_LIST);
            }
        }
    }

    /* 管家回饋專區 */
    Rectangle statPanel = { 50, 502, 826, 142 };
    DrawRectangleRounded(statPanel, 0.15f, 6, (Color){ 255, 255, 255, 235 });
    DrawRectangleLinesEx(statPanel, 1, (Color){ 220, 226, 238, 255 });

    text_bold(T(L_MONTH_INSIGHT), statPanel.x + 20, statPanel.y + 10, 22, 1, C_DARK);

    if (month_total_records == 0) {
        const char *emptyMsg = (cur_lang == LANG_ZH)
            ? "這個月還沒有任何房客入住紀錄喔！點擊上方的日期，為今天的心情留下一抹色彩吧。"
            : "No records yet for this month! Click any date above to check in today's mood.";
        wrap(emptyMsg, statPanel.x + 20, statPanel.y + 50, statPanel.width - 40, 20, (Color){ 120, 130, 145, 255 }, 0);
    } else {
        int max_emo = 0;
        for (int i = 1; i < 6; i++) {
            if (emo_counts[i] > emo_counts[max_emo]) max_emo = i;
        }

        Rectangle statTag = { statPanel.x + 20, statPanel.y + 42, 120, 36 };
        DrawRectangleRounded(statTag, 0.25f, 4, emotion_colors[max_emo]);
        center_bold(L_EMOTION_NAMES[cur_lang][max_emo], statTag, 20, 1, WHITE);

        const char *countDesc = (cur_lang == LANG_ZH)
            ? TextFormat("本月共入住 %d 天（%d 筆），其中「%s」最常來訪（共 %d 次）", month_days, month_total_records, L_EMOTION_NAMES[cur_lang][max_emo], emo_counts[max_emo])
            : TextFormat("%d days (%d records). \"%s\" visited most often (%d times).", month_days, month_total_records, L_EMOTION_NAMES[cur_lang][max_emo], emo_counts[max_emo]);
        wrap(countDesc, statPanel.x + 155, statPanel.y + 48, statPanel.width - 175, 18, DARKGRAY, 0);

        wrap(L_FEEDBACK[max_emo][cur_lang], statPanel.x + 20, statPanel.y + 88,
             statPanel.width - 40, 18, (Color){ 70, 85, 105, 255 }, 0);
    }
}

/* ---------- 4-1. 入住登記：情緒與強度 ---------- */
static void page_checkin(void)
{
    Rectangle glassCard = { 40, 20, 840, 625 };
    BG(glassCard);

    text_bold(TextFormat("%s  |  %s", (editing_idx >= 0 ? T(L_EDIT) : T(L_CHECKIN)), selected_date),
              60, 35, 30, 2, C_DARK);
    text("1 / 3", 790, 42, 22, GRAY);

    for (int i = 0; i < 6; i++) {
        Rectangle btn = { 60 + i * 132, 90, 122, 50 };
        Color c = (work.emotion == i) ? emotion_colors[i] : C_CHIP;
        if (DrawInteractiveButton(L_EMOTION_NAMES[cur_lang][i], btn, c,
                                  (work.emotion == i) ? WHITE : DARKGRAY, 22)) {
            if (work.emotion != i) {
                /* 不同情緒的 Room Service 不同，切換時清除舊的完成狀態與筆記 */
                work.completed = 0;
                memset(work.notes, 0, sizeof work.notes);
            }
            work.emotion = i;
        }
    }

    wrap(L_WELCOME[work.emotion][cur_lang], 60, 160, 795, 22, C_ACC, 0);

    text_bold(T(L_INTENSITY), 60, 238, 22, 1, DARKGRAY);
    for (int i = 1; i <= 5; i++) {
        Rectangle btn = { 260 + (i - 1) * 75, 230, 62, 42 };
        Color c = (work.intensity == i) ? (Color){ 52, 73, 94, 255 } : C_CHIP;
        if (DrawInteractiveButton(TextFormat("%d", i), btn, c,
                                  (work.intensity == i) ? WHITE : BLACK, 22))
            work.intensity = i;
    }

    if (DrawInteractiveButton(T(L_CANCEL), (Rectangle){ 60, 570, 130, 52 }, C_LIGHT, DARKGRAY, 22))
        go(STATE_CALENDAR);
    if (DrawInteractiveButton(T(L_NEXT), (Rectangle){ 725, 570, 130, 52 }, (Color){ 52, 152, 219, 255 }, WHITE, 22))
        go(STATE_CHECKIN_TEXT);
}

/* ---------- 4-2. 入住登記：五格入住單 ---------- */
static void page_checkin_text(void)
{
    Rectangle glassCard = { 40, 20, 840, 625 };
    BG(glassCard);

    text_bold(TextFormat("%s  |  %s", (editing_idx >= 0 ? T(L_EDIT) : T(L_CHECKIN)), selected_date),
              60, 35, 30, 2, C_DARK);
    text("2 / 3", 790, 42, 22, GRAY);

    for (int i = 0; i < 5; i++) {
        text_bold(field_label(i), 60, 88 + i * 92, 20, 1, DARKGRAY);
        field((Rectangle){ 60, 114 + i * 92, 795, 62 }, work.text[i], TEXT, 20);
    }

    if (DrawInteractiveButton(T(L_PREV), (Rectangle){ 60, 570, 130, 52 }, C_LIGHT, DARKGRAY, 22))
        go(STATE_CHECKIN);
    if (DrawInteractiveButton(T(L_NEXT), (Rectangle){ 725, 570, 130, 52 }, (Color){ 52, 152, 219, 255 }, WHITE, 22))
        go(STATE_CHECKIN_NEEDS);
}

/* ---------- 4-3. 入住登記：需求複選與保存 ---------- */
static void page_checkin_needs(void)
{
    Rectangle glassCard = { 40, 20, 840, 625 };
    BG(glassCard);

    text_bold(TextFormat("%s  |  %s", (editing_idx >= 0 ? T(L_EDIT) : T(L_CHECKIN)), selected_date),
              60, 35, 30, 2, C_DARK);
    text("3 / 3", 790, 42, 22, GRAY);

    text_bold(T(L_NEEDS_TITLE), 60, 100, 26, 1, DARKGRAY);
    text(T(L_NEEDS_HINT), 60, 138, 18, GRAY);
    draw_needs(60, 180, 90, 22);

    if (DrawInteractiveButton(T(L_PREV), (Rectangle){ 60, 570, 130, 52 }, C_LIGHT, DARKGRAY, 22))
        go(STATE_CHECKIN_TEXT);
    if (DrawInteractiveButton(T(L_SAVE), (Rectangle){ 655, 570, 200, 52 }, (Color){ 46, 204, 113, 255 }, WHITE, 22)) {
        if (save(&work, editing_idx)) go(STATE_CALENDAR);
    }
}

/* ---------- 5. 房客詳情 ---------- */
static void page_detail(void)
{
    Rectangle glassCard = { 40, 20, 840, 625 };
    Vector2 mouse = GetMousePosition();
    int e = work.emotion;
    int i;
    BG(glassCard);

    text_bold(TextFormat("%s  |  %s", T(L_DETAIL), work.date), 60, 32, 30, 2, C_DARK);

    Rectangle tag = { 60, 80, 135, 44 };
    DrawRectangleRounded(tag, 0.25f, 4, emotion_colors[e]);
    center_bold(L_EMOTION_NAMES[cur_lang][e], tag, 24, 1, WHITE);
    text_bold(TextFormat("Lv: %d / 5", work.intensity), 215, 90, 24, 1, DARKGRAY);
    text_bold(work.checked_out ? T(L_CHECKED_OUT) : T(L_STAYING), 360, 90, 24, 1, C_GRAYT);

    for (i = 0; i < 4; i++) {
        Rectangle r = { 60 + i * 200, 135, 190, 42 };
        if (DrawInteractiveButton(T(L_TABS[i]), r, tab == i ? emotion_colors[e] : C_CHIP,
                                  tab == i ? WHITE : DARKGRAY, 20)) {
            tab = i;
            edit = NULL;
        }
    }

    if (tab == 0) {
        float y = 190;
        for (i = 0; i < 5; i++) {
            const char *v = work.text[i][0] ? work.text[i] : T(L_BLANK);
            int n;
            text_bold(field_label(i), 60, y, 18, 1, DARKGRAY);
            y += 24;
            n = wrap(v, 60, y, 795, 18, (Color){ 30, 40, 55, 255 }, 0);
            y += n * 24 + 6;
        }

    } else if (tab == 1) {
        text(T(L_NEEDS_HINT_DETAIL), 60, 192, 18, GRAY);
        draw_needs(60, 225, 88, 22);

    } else if (tab == 2) {
        for (i = 0; i < service_count[e]; i++) {
            Rectangle r = { 60, 190 + i * 48, 795, 42 };
            Rectangle box = { 72, r.y + 7, 28, 28 };
            int done = (work.completed & (1 << i)) != 0;

            DrawRectangleRounded(r, 0.2f, 6, svc_sel == i ? WHITE : C_ROW);
            DrawRectangleRec(box, done ? C_ACC : WHITE);
            DrawRectangleLinesEx(box, 2, DARKGRAY);
            text(L_SERVICES[e][i][cur_lang], 116, r.y + 8, 22, DARKGRAY);
            text(done ? T(L_DONE) : T(L_NOT_DONE), 700, r.y + 11, 18, GRAY);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                CheckCollisionPointRec(mouse, r)) {
                if (CheckCollisionPointRec(mouse, box)) work.completed ^= 1 << i;
                svc_sel = i;
                edit = NULL;
            }
        }
        text_bold(T(L_NOTES_TITLE), 60, 436, 20, 1, DARKGRAY);
        text(L_SERVICES[e][svc_sel][cur_lang], 140, 438, 20, C_ACC);
        field((Rectangle){ 60, 466, 795, 96 }, work.notes[svc_sel], NOTE, 20);

    } else {
        text_bold(T(L_REVIEW_INT), 60, 192, 20, 1, DARKGRAY);
        for (i = 0; i <= 5; i++) {
            Rectangle r = { 60 + i * 86, 224, 76, 46 };
            if (DrawInteractiveButton(TextFormat("%d", i), r,
                                      work.after == i ? emotion_colors[1] : C_CHIP, DARKGRAY, 22))
                work.after = i;
        }
        text_bold(T(L_REVIEW_LABEL), 60, 292, 20, 1, DARKGRAY);
        field((Rectangle){ 60, 322, 795, 110 }, work.review, TEXT, 20);
        if (DrawInteractiveButton(L_CHECKOUT_BTN[cur_lang][work.checked_out ? 1 : 0],
                                  (Rectangle){ 60, 452, 420, 52 }, C_CHIP, DARKGRAY, 20))
            work.checked_out = !work.checked_out;
    }

    if (DrawInteractiveButton(T(L_EDIT), (Rectangle){ 60, 575, 140, 52 }, (Color){ 52, 152, 219, 255 }, WHITE, 20)) {
        selected_date[0] = '\0';
        strncpy(selected_date, work.date, 10);
        selected_date[10] = '\0';
        go(STATE_CHECKIN);
    }
    if (DrawInteractiveButton(del_confirm ? T(L_DELETE_SURE) : T(L_DELETE),
                              (Rectangle){ 215, 575, 150, 52 }, (Color){ 231, 76, 60, 255 }, WHITE, 20)) {
        if (!del_confirm) {
            del_confirm = 1;
            say(M_DEL_SURE);
        } else {
            DeleteRecord(editing_idx);
            if (rewrite_all()) say(M_DELETED);
            go(STATE_CALENDAR);
        }
    }
    if (DrawInteractiveButton(T(L_BACK), (Rectangle){ 380, 575, 120, 52 }, C_LIGHT, DARKGRAY, 20))
        go(STATE_CALENDAR);
    if (DrawInteractiveButton(T(L_SAVE_CHANGES), (Rectangle){ 705, 575, 150, 52 }, (Color){ 46, 204, 113, 255 }, WHITE, 20))
        save(&work, editing_idx);
}

/* ---------- 6. 入住紀錄清單 ---------- */
static void page_list(void)
{
    Rectangle glassCard = { 40, 20, 840, 625 };
    Vector2 mouse = GetMousePosition();
    int idxs[MAX_RECORDS];
    int n = 0, open_i = -1;
    char buf[96];
    float max_scroll;
    BG(glassCard);

    text_bold(T(L_ALL_RECORDS), 60, 32, 32, 2, C_DARK);
    if (filter_date[0])
        text(TextFormat("%s%s", T(L_FILTER_DATE), filter_date), 60, 78, 20, GRAY);

    for (int i = record_count - 1; i >= 0; i--) {      /* 最新的在最上面 */
        if (!filter_date[0] || strncmp(records[i].date, filter_date, 10) == 0)
            idxs[n++] = i;
    }
    if (!n) text(T(L_EMPTY_LIST), 60, 130, 26, GRAY);

    list_scroll += GetMouseWheelMove() * 50;
    max_scroll = n * 62 - 445;
    if (max_scroll < 0) max_scroll = 0;
    if (list_scroll > 0) list_scroll = 0;
    if (list_scroll < -max_scroll) list_scroll = -max_scroll;

    BeginScissorMode(60, 108, 800, 450);
    for (int j = 0; j < n; j++) {
        int i = idxs[j];
        float y = 112 + j * 62 + list_scroll;
        Rectangle r = { 60, y, 800, 56 };
        int hover;

        if (y + 56 < 108 || y > 558) continue;
        hover = CheckCollisionPointRec(mouse, r) && mouse.y >= 108 && mouse.y <= 558;
        if (hover) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

        DrawRectangleRounded(r, 0.2f, 6, hover ? WHITE : C_ROW);
        DrawRectangle((int)r.x, (int)r.y + 6, 10, 44, emotion_colors[records[i].emotion]);

        snprintf(buf, sizeof buf, "#%d  %s", i + 1, records[i].date);
        text(buf, r.x + 24, y + 15, 20, DARKGRAY);
        text(L_EMOTION_NAMES[cur_lang][records[i].emotion], r.x + 340, y + 15, 20, DARKGRAY);
        text(records[i].checked_out ? T(L_CHECKED_OUT) : T(L_STAYING), r.x + 450, y + 15, 20, GRAY);
        snprintf(buf, sizeof buf, "Lv %d/5", records[i].intensity);
        text(buf, r.x + 680, y + 15, 20, GRAY);

        if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) open_i = i;
    }
    EndScissorMode();

    if (DrawInteractiveButton(T(L_BACK), (Rectangle){ 60, 575, 130, 52 }, C_LIGHT, DARKGRAY, 22)) {
        go(STATE_CALENDAR);
    }
    if (DrawInteractiveButton(T(L_NEW_CHECKIN), (Rectangle){ 205, 575, 190, 52 }, (Color){ 76, 175, 80, 255 }, WHITE, 20)) {

        char day[32];
        if (filter_date[0]) snprintf(day, sizeof day, "%s", filter_date);
        else today_str(day, sizeof day);
        start_checkin(day);
    }

    if (filter_date[0]) {
        if (DrawInteractiveButton(T(L_SHOW_ALL), (Rectangle){ 410, 575, 160, 52 }, (Color){ 52, 152, 219, 255 }, WHITE, 20)) {
            filter_date[0] = '\0';
            list_scroll = 0;
        }
    }

    if (open_i >= 0) {
        PlaySound(snd_click);
        open_detail(open_i);
    }
}

/* ---------- 7. 音樂設定 ---------- */
static void page_music(void)
{
    Rectangle glassCard = { 60, 40, 800, 580 };
    BG(glassCard);

    text_bold(T(L_BTN_MUSIC), 100, 75, 36, 2, C_DARK);

    for (int i = 0; i < NUM_TRACKS; i++) {
        Rectangle btnTrack = { 100, 150 + i * 80, 420, 60 };
        Color c = (cur_track == i && music_playing) ? (Color){ 46, 204, 113, 255 } : C_LIGHT;
        if (DrawInteractiveButton(TextFormat("BGM %d: %s", i + 1, track_names[i]), btnTrack, c,
                                  (cur_track == i && music_playing) ? WHITE : BLACK, 22)) {
            StopMusicStream(bgm_tracks[cur_track]);
            cur_track = i;
            PlayMusicStream(bgm_tracks[cur_track]);
            music_playing = true;
        }
    }

    Rectangle btnToggle = { 100, 410, 200, 52 };
    Color tCol = music_playing ? (Color){ 231, 76, 60, 255 } : (Color){ 52, 152, 219, 255 };
    if (DrawInteractiveButton(music_playing ? "Mute / 靜音" : "Play / 播放", btnToggle, tCol, WHITE, 22)) {
        if (music_playing) {
            PauseMusicStream(bgm_tracks[cur_track]);
            music_playing = false;
        } else {
            ResumeMusicStream(bgm_tracks[cur_track]);
            music_playing = true;
        }
    }

    Rectangle btnBack = { 100, 485, 140, 50 };
    if (DrawInteractiveButton(T(L_BACK), btnBack, C_GRAYT, WHITE, 22))
        go(STATE_MENU);
}

static void draw_status(void)
{
    if (GetTime() < status_until) {
        DrawRectangle(0, SCREEN_H - 28, SCREEN_W, 28, DARKGRAY);
        text(status, 12, SCREEN_H - 25, 20, WHITE);
    }
}

/* =====================================================
   主程式
   ===================================================== */
int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, "Emotion Hotel");
    SetExitKey(KEY_NULL);          /* 避免輸入文字時按 ESC 直接關閉 */
    InitAudioDevice();
    InitCustomSounds();
    srand((unsigned)time(NULL));
    InitParticles(SCREEN_W, SCREEN_H);
    SetTargetFPS(60);

    /* 字型：先登記所有會用到的字，再載入 */
    for (int i = 32; i < 127; i++) add_cp(i);
    need_text(UI_POOL);
    load();
    for (int i = 0; i < record_count; i++) {
        need_text(records[i].date);
        need_text(records[i].review);
        for (int k = 0; k < 5; k++) {
            need_text(records[i].text[k]);
            need_text(records[i].notes[k]);
        }
    }
    need_text(status);
    load_font();

    /* 音樂 */
    for (int i = 0; i < NUM_TRACKS; i++) {
        bgm_tracks[i] = LoadMusicStream(track_files[i]);
    }
    PlayMusicStream(bgm_tracks[0]);
    music_playing = true;

    time_t t_now = time(NULL);
    struct tm *now_tm = localtime(&t_now);
    cur_year = now_tm->tm_year + 1900;
    cur_month = now_tm->tm_mon + 1;

    while (!quit && !WindowShouldClose()) {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        if (font_dirty) load_font();

        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        if (music_playing) {
            UpdateMusicStream(bgm_tracks[cur_track]);
        }

        field_hit = 0;
        handle_input();

        BeginDrawing();

        switch (state) {
        case STATE_MENU:          page_menu();          break;
        case STATE_LANG_SETTING:  page_lang();          break;
        case STATE_CALENDAR:      page_calendar();      break;
        case STATE_CHECKIN:       page_checkin();       break;
        case STATE_CHECKIN_TEXT:  page_checkin_text();  break;
        case STATE_CHECKIN_NEEDS: page_checkin_needs(); break;
        case STATE_DETAIL:        page_detail();        break;
        case STATE_LIST:          page_list();          break;
        case STATE_MUSIC_SETTING: page_music();         break;
        }

        draw_status();
        EndDrawing();

        /* 點到輸入框以外的地方就結束輸入 */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !field_hit) edit = NULL;
    }

    UnloadSound(snd_click);
    for (int i = 0; i < NUM_TRACKS; i++) {
        UnloadMusicStream(bgm_tracks[i]);
    }
    CloseAudioDevice();
    if (font_loaded) UnloadFont(font);
    CloseWindow();
    return 0;
}