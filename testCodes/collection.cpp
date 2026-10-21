#include "../trainscii.cpp"

#include <iostream>
#include <string>
#include <cstdio>

static void put(int r, int c, const std::string& s) {
    std::cout << "\033[" << r << ";" << c << "H" << s;
}
static std::string rep(const std::string& g, int n) {
    std::string o; o.reserve(g.size() * n);
    for (int i = 0; i < n; ++i) o += g;
    return o;
}

int main() {
    const std::string boggies[5] = { "□","▢","▤","▥","▭" };
    const std::string engines[2] = { "›","‹" };
    const std::string tracks[18] = {
        "─","━","═","╌","╍","┄","┅","┈","┉",
        "⎯","⎼","⎻","╴","╶","╸","╺","╼","╾"
    };

    const int BOGGIES    = 4;
    const int TRAIN_LEN  = BOGGIES + 1;   // 5
    const int TRACK_GAP  = 3;             // 3 track cells after each train
    const int SPACE_GAP  = 3;             // 3 blank cells after that
    const int PER_TRACK  = 10;

    // Each train slot = 5 (train) + 3 (track) + 3 (space) = 11
    const int SLOT       = TRAIN_LEN + TRACK_GAP + SPACE_GAP;
    // Track length = 10 slots minus trailing space_gap
    const int TRACK_LEN  = PER_TRACK * SLOT - SPACE_GAP;

    const int LEFT       = 7;
    const int ROW0       = 4;
    const int ROW_GAP    = 2;
    const int ROW_STRIDE = 1 + ROW_GAP;

    std::cout << "\033[2J\033[H";

    put(1, 1, "+" + rep("-", TRACK_LEN + LEFT) + "+");
    put(2, 1, "| TRAIN CATALOG - 2 engines x 5 boggies x 18 tracks - 10/track - track gap 3 - space gap 3 |");

    for (int t = 0; t < 18; ++t) {
        int row = ROW0 + t * ROW_STRIDE;

        char lbl[32];
        std::snprintf(lbl, sizeof lbl, "T%02d %s ", t + 1, tracks[t].c_str());
        put(row, 1, lbl);

        // Draw full track bed first
        put(row, LEFT, rep(tracks[t], TRACK_LEN));

        // Now place trains + erase space_gap after each
        for (int k = 0; k < PER_TRACK; ++k) {
            int e = k / 5;
            int b = k % 5;
            std::string body = rep(boggies[b], BOGGIES);
            std::string sprite = (e == 0) ? (body + engines[0])
                                          : (engines[1] + body);

            int col = LEFT + k * SLOT;
            put(row, col, sprite);

            // Erase the space gap (not the track gap)
            // Track gap is already drawn (part of bed), space gap gets blanked
            if (k < PER_TRACK - 1) {
                int spaceStart = col + TRAIN_LEN + TRACK_GAP;
                put(row, spaceStart, rep(" ", SPACE_GAP));
            }
        }
    }

    int fr = ROW0 + 18 * ROW_STRIDE;
    put(fr, 1, "+" + rep("-", TRACK_LEN + LEFT) + "+");
    put(fr + 1, 1,
        "| 180 trains | track " + std::to_string(TRACK_LEN) +
        " | train " + std::to_string(TRAIN_LEN) +
        " | track gap " + std::to_string(TRACK_GAP) +
        " | space gap " + std::to_string(SPACE_GAP) + " |");

    std::cout << "\033[" << (fr + 3) << ";1H" << std::flush;
    std::cout << "\n\n";
    return 0;
}