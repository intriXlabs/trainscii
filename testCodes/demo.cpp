#include "../trainscii.cpp"

// before you argue - yes it is ai made demo, deepseek free one for kind information.. i can;t put my hours to write demo so yeah

// subway_demo.cpp — a proper terminal subway diorama on the train foundation
//
// Build:  g++ -std=c++17 -O2 -o subway_demo subway_demo.cpp
// Run:    ./subway_demo            (fixed scenario, exits when all trains park)
//         ./subway_demo --loop     (runs forever, Ctrl-C to quit)
//
// This is intentionally a *big* demo. It is meant to be read as a worked
// example for adopters: every piece of scenery, every lane, every schedule
// is plain data or a plain function on top of the train library's public
// API. Nothing here reaches into the library's internals.
//
// Scene layout (50 rows x 210 cols):
//
//   rows  1..7    night sky, stars, moon
//   rows  8..14   far mountain range
//   rows 15..21   near mountain range with snow caps
//   rows 22..26   hillside village (houses, trees, a church)
//   rows 27..31   town skyline (buildings, windows, a clock tower)
//   rows 32..34   station: canopy, sign, platforms
//   rows 35..47   subway: 7 tracks, 7 trains, signals, tunnel mouths
//   rows 48..50   footer / legend / status
//
// The station sits over the middle tracks around columns 88..128, so the
// trains on those tracks visibly disappear under the canopy as they pass.
//
// The trains reverse automatically when the library returns -1 from
// nextFrame()/prevFrame(). That is the only choreography primitive used.

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cstdio>

// ===========================================================================
// Terminal helpers
// ===========================================================================
namespace term {

static volatile std::sig_atomic_t g_running = 1;
static void on_sigint(int) { g_running = 0; }

struct Guard {
    Guard() {
        std::signal(SIGINT, on_sigint);
        std::cout << "\033[?25l";   // hide cursor
        std::cout << "\033[2J\033[H" << std::flush; // clear once
    }
    ~Guard() {
        std::cout << "\033[?25h";   // show cursor
        std::cout << "\033[0m";     // reset attributes
        std::cout << "\033[49;1H" << std::flush;
    }
};

inline bool running() { return g_running != 0; }
inline void tick()    { usleep(10000); }   // 10 ms base tick

// 1-based cursor address + string, no newline.
inline void put(int row, int col, const std::string& s) {
    std::cout << "\033[" << row << ";" << col << "H" << s;
}
inline void putFlush(int row, int col, const std::string& s) {
    put(row, col, s);
    std::cout << std::flush;
}

// Repeat a UTF-8 glyph N times (glyphs may be multi-byte).
inline std::string rep(const std::string& g, int n) {
    std::string out;
    out.reserve(g.size() * n);
    for (int i = 0; i < n; ++i) out += g;
    return out;
}

} // namespace term

// ===========================================================================
// Scene primitives — small, composable, all drawn once at startup.
// ===========================================================================
namespace scene {

// ---- sky -----------------------------------------------------------------
void drawStars() {
    // Hand-placed stars. Kept sparse so the sky doesn't fight the mountains.
    struct Star { int r, c; const char* g; };
    static const Star stars[] = {
        {1,  18, "."}, {1,  47, "*"}, {1,  73, "."}, {1, 104, "*"}, {1, 141, "."}, {1, 178, "*"},
        {2,   6, "*"}, {2,  61, "."}, {2,  92, "*"}, {2, 123, "."}, {2, 159, "*"}, {2, 195, "."},
        {3,  33, "."}, {3,  84, "*"}, {3, 117, "."}, {3, 151, "*"}, {3, 186, "."},
        {4,  12, "*"}, {4,  52, "."}, {4,  99, "*"}, {4, 133, "."}, {4, 168, "*"}, {4, 201, "."},
        {5,  27, "."}, {5,  67, "*"}, {5, 112, "."}, {5, 147, "*"}, {5, 181, "."},
        {6,  41, "*"}, {6,  76, "."}, {6, 108, "*"}, {6, 162, "."}, {6, 193, "*"},
    };
    for (const auto& s : stars)
        term::put(s.r, s.c, s.g);
    std::cout << std::flush;
}

void drawMoon(int row, int col) {
    // A crescent made from two overlapping disks of different glyphs.
    term::put(row,     col,     "  ..::  ");
    term::put(row + 1, col,     " .::::: ");
    term::put(row + 2, col,     "::::::. ");
    term::put(row + 3, col,     " :::::. ");
    term::put(row + 4, col,     "  '::'  ");
}

// ---- mountains -----------------------------------------------------------
// Far range: a single silhouette, low contrast, drawn as a wide band.
void drawFarMountains() {
    static const char* rows[] = {
        "                 .-'''-..                          .-'''-..                          .-'''-..",
        "            .-'``         ``'-..              .-'``         ``'-..              .-'``         ``'-..",
        "       .-'``                    ``'-..   .-'``                    ``'-..   .-'``                    ``'-..",
        "  .-'``                               `'-'                         `'-'                              ``'-.",
        ".'                                                                                                        `.",
    };
    for (int i = 0; i < 5; ++i)
        term::putFlush(8 + i, 1, rows[i]);
}

// Near range: taller, snow-capped, drawn with a second pass for the caps.
void drawNearMountains() {
    static const char* rows[] = {
        "                    /\\                                /\\                                /\\",
        "              /\\   /  \\        /\\                    /  \\       /\\                     /  \\",
        "        /\\   /  \\ /    \\  /\\  /  \\             /\\  /    \\  /\\ /  \\              /\\   /    \\",
        "   /\\  /  \\ /    V      \\/  \\/    \\      /\\   /  \\/      \\/  V    \\        /\\  /  \\ /      \\",
        "  /  \\/    V              V          \\    /  \\ /                    \\  /\\  /  \\/    V        \\",
        " /                                     \\  /    V                      \\/  \\/                  \\",
        "/                                       \\/                                                    \\",
    };
    for (int i = 0; i < 7; ++i)
        term::putFlush(15 + i, 1, rows[i]);

    // Snow caps: small `*` clusters near each peak.
    struct Cap { int r, c; };
    static const Cap caps[] = {
        {15, 21}, {15, 22}, {15, 23}, {16, 22}, {16, 23},
        {15, 55}, {15, 56}, {15, 57}, {16, 56}, {16, 57},
        {15, 87}, {15, 88}, {15, 89}, {16, 88}, {16, 89},
        {14, 20}, {14, 54}, {14, 86},
    };
    for (const auto& c : caps)
        term::put(c.r, c.c, "*");
    std::cout << std::flush;
}

// ---- village -------------------------------------------------------------
// A tiny house: pitched roof, one window, one door.
void drawHouse(int row, int col, const std::string& wall, const std::string& roof) {
    term::put(row,     col, "  " + roof + roof + roof + roof + "  ");
    term::put(row + 1, col, " " + roof + roof + roof + roof + roof + roof + " ");
    term::put(row + 2, col, wall + wall + wall + wall + wall + wall + wall + wall);
    term::put(row + 3, col, wall + "[]" + wall + wall + wall + "[]" + wall);
    term::put(row + 4, col, wall + "[]" + wall + wall + wall + "[]" + wall);
    term::put(row + 5, col, wall + wall + wall + "||" + wall + wall + wall);
}

// A tree: trunk + canopy.
void drawTree(int row, int col, const std::string& leaf) {
    term::put(row,     col, "  " + leaf + leaf + leaf + "  ");
    term::put(row + 1, col, " " + leaf + leaf + leaf + leaf + leaf + " ");
    term::put(row + 2, col, leaf + leaf + leaf + leaf + leaf + leaf + leaf);
    term::put(row + 3, col, "  ||  ");
    term::put(row + 4, col, "  ||  ");
}

// A church: tall nave + steeple + cross.
void drawChurch(int row, int col, const std::string& wall) {
    term::put(row,     col, "     +     ");
    term::put(row + 1, col, "     |     ");
    term::put(row + 2, col, "    /^\\    ");
    term::put(row + 3, col, "   /^^^\\   ");
    term::put(row + 4, col, "  /^^^^^\\  ");
    term::put(row + 5, col, " /^^^^^^^\\ ");
    term::put(row + 6, col, wall + wall + wall + wall + wall + wall + wall + wall + wall + wall + wall);
    term::put(row + 7, col, wall + " [] " + wall + wall + wall + " [] " + wall);
    term::put(row + 8, col, wall + " [] " + wall + " || " + wall + " [] " + wall);
    term::put(row + 9, col, wall + wall + wall + wall + " || " + wall + wall + wall + wall);
}

void drawVillage() {
    // Hillside contour under the mountains.
    term::putFlush(22, 1, " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ");
    term::putFlush(23, 1, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ");

    drawChurch(17, 18, "#");
    drawHouse(19, 40, "#", "^");
    drawHouse(19, 58, "#", "A");
    drawTree (20, 78, "*");
    drawHouse(19, 96, "#", "^");
    drawTree (20, 118, "*");
    drawHouse(19, 132, "#", "A");
    drawHouse(19, 156, "#", "^");
    drawTree (20, 178, "*");
    std::cout << std::flush;
}

// ---- town ----------------------------------------------------------------
// One building: a rectangle of `wall` with rows of lit windows.
void drawBuilding(int row, int col, int width, int height,
                  const std::string& wall, const std::string& win) {
    // Roof
    term::put(row, col, "+" + term::rep("-", width - 2) + "+");
    for (int r = 1; r < height - 1; ++r) {
        std::string line = "|";
        for (int c = 1; c < width - 1; ++c) {
            // Window pattern: every 3rd column is a window, every 2nd row lit.
            bool isWin = (c % 3 == 1) && (r % 2 == 1);
            line += isWin ? win : wall;
        }
        line += "|";
        term::put(row + r, col, line);
    }
    term::put(row + height - 1, col, "+" + term::rep("-", width - 2) + "+");
}

void drawClockTower(int row, int col) {
    term::put(row,     col, "  +-----+  ");
    term::put(row + 1, col, " /|     |\\ ");
    term::put(row + 2, col, "/ |  o  | \\");
    term::put(row + 3, col, "| |     | |");
    term::put(row + 4, col, "| +-----+ |");
    term::put(row + 5, col, "|  | | |  |");
    term::put(row + 6, col, "|  | | |  |");
    term::put(row + 7, col, "|  | | |  |");
    term::put(row + 8, col, "|__|_|_|__|");
}

void drawTown() {
    drawBuilding(25,  6, 14, 7, "#", "o");
    drawBuilding(25, 22, 10, 5, "#", "o");
    drawClockTower(24, 36);
    drawBuilding(25, 50, 12, 6, "#", "o");
    drawBuilding(25, 66,  8, 4, "#", "o");
    drawBuilding(25, 140, 12, 6, "#", "o");
    drawBuilding(25, 156, 10, 5, "#", "o");
    drawBuilding(25, 170, 14, 7, "#", "o");
    drawBuilding(25, 188,  8, 4, "#", "o");
    std::cout << std::flush;
}

// ---- station -------------------------------------------------------------
void drawStation() {
    const int W = 42;            // canopy width
    const int C = 88;            // canopy left column

    // Canopy roof (top + underside)
    term::put(28, C, "/" + term::rep("=", W) + "\\");
    term::put(29, C, "|" + term::rep(" ", W) + "|");
    // Pillars
    for (int x = 0; x < W; x += 7)
        term::put(30, C + 1 + x, "|");
    // Canopy underside
    term::put(31, C, "\\" + term::rep("=", W) + "/");

    // Station sign hanging under the canopy
    term::put(29, C + 12, "+----------------+");
    term::put(30, C + 12, "|  CENTRAL  STN  |");
    term::put(31, C + 12, "+----------------+");

    // Platform edges on both sides of the canopy
    term::put(33, C - 4, term::rep("=", W + 8));
    term::put(34, C - 4, term::rep("-", W + 8));

    // "SOUTHBOUND" / "NORTHBOUND" track labels at the far left
    term::put(35, 2, "SOUTHBOUND >>");
    term::put(46, 2, "<< NORTHBOUND");

    std::cout << std::flush;
}

// ---- signals -------------------------------------------------------------
// A trackside signal: post + lamp. Static red/green for visual texture.
void drawSignal(int row, int col, const char* lamp) {
    term::put(row,     col, " | ");
    term::put(row + 1, col, "-" + std::string(lamp) + "-");
    term::put(row + 2, col, " | ");
}

void drawSignals() {
    static const char* lamps[] = { "O", "o", "O", "o", "O", "o" };
    int idx = 0;
    for (int row = 35; row <= 45; row += 2) {
        drawSignal(row, 186, lamps[idx % 6]);
        drawSignal(row,  10, lamps[(idx + 3) % 6]);
        ++idx;
    }
    std::cout << std::flush;
}

// ---- tunnel mouths -------------------------------------------------------
void drawTunnel(int row, int col) {
    term::put(row,     col, " .--------. ");
    term::put(row + 1, col, "/          \\");
    term::put(row + 2, col, "|  ######  |");
    term::put(row + 3, col, "|  ######  |");
    term::put(row + 4, col, "|__________|");
}

void drawTunnels() {
    // Left and right tunnel mouths on every other track.
    for (int row = 35; row <= 45; row += 4) {
        drawTunnel(row, 1);
        drawTunnel(row, 198);
    }
    std::cout << std::flush;
}

// ---- footer --------------------------------------------------------------
void drawFooter() {
    term::putFlush(48, 2,
        "[ subway demo ]  7 lanes  |  track length 190  |  speeds 8/2/5/1/6/3/4  |  Ctrl-C to quit");
    term::putFlush(49, 2,
        "legend:  . * stars   ^ A roofs   o windows   # walls   = canopy   O o signals");
}

} // namespace scene

// ===========================================================================
// Lane — one train, one speed, turnaround on -1.
// ===========================================================================
struct Lane {
    train t;
    int   speed;
    int   dir;
    bool  done;
    int   laps;
    int   maxLaps;

    Lane(int row, int col, int len, int boggy, int type, int spd, int maxL)
        : t(row, col, len), speed(spd), dir(+1), done(false), laps(0), maxLaps(maxL)
    {
        t.chageBoggyCount(boggy);
        t.changeBoggyType(type);
    }

    bool step(long tick) {
        if (done || tick % speed != 0) return false;
        int rc = (dir > 0) ? t.nextFrame() : t.prevFrame();
        if (rc == -1) {
            ++laps;
            if (maxLaps > 0 && laps >= maxLaps) { done = true; return false; }
            dir = -dir;
            t.changeEngineType(dir > 0 ? 0 : 1);
            return false;
        }
        return true;
    }
};

// ===========================================================================
int main(int argc, char** argv) {
    bool loop = false;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--loop") loop = true;

    term::Guard guard;

    // ---- static scene ----------------------------------------------------
    scene::drawStars();
    scene::drawMoon(2, 190);
    scene::drawFarMountains();
    scene::drawNearMountains();
    scene::drawVillage();
    scene::drawTown();
    scene::drawStation();
    scene::drawSignals();
    scene::drawTunnels();
    scene::drawFooter();

    // ---- trains ----------------------------------------------------------
    // Tracks run on rows 35..45, one lane every 1-2 rows. The station
    // canopy (cols 88..130) sits over the middle tracks, so trains on
    // rows 36..44 visibly pass under it.
    const int LEN = 190;
    const int COL = 5;

    std::vector<Lane> lanes;
    lanes.reserve(7);

    // row, col,  len, boggy, type, speed, maxLaps
    lanes.emplace_back(35, COL, LEN,  6, 0, 8, loop ? 0 : 1);
    lanes.emplace_back(37, COL, LEN, 10, 2, 2, loop ? 0 : 2);
    lanes.emplace_back(39, COL, LEN,  4, 4, 5, loop ? 0 : 1);
    lanes.emplace_back(41, COL, LEN,  8, 1, 1, loop ? 0 : 3);
    lanes.emplace_back(43, COL, LEN,  5, 3, 6, loop ? 0 : 1);
    lanes.emplace_back(45, COL, LEN, 12, 2, 3, loop ? 0 : 1);
    lanes.emplace_back(36, COL, LEN,  3, 0, 4, loop ? 0 : 2);

    // ---- animation loop --------------------------------------------------
    long tick = 0;
    while (term::running()) {
        bool anyAlive = false;
        for (auto& ln : lanes) {
            if (!ln.done) anyAlive = true;
            ln.step(tick);
        }
        if (!anyAlive) break;
        ++tick;
        term::tick();
    }

    return 0;
}