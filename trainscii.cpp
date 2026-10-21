#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/ioctl.h>

class train {

private:
    // strings
    std::string boggy[5]={
        "□", "▢", "▤", "▥", "▭"
    };

    std::string engineRight="›";
    std::string engineLeft="‹";

    std::string track[18] = {
    "─", "━", "═",
    "╌", "╍", "┄", "┅", "┈", "┉",
    "⎯", "⎼", "⎻",
    "╴", "╶", "╸", "╺",
    "╼", "╾"
    };


private:
    // base data
    int row;
    int col;
    int length;

    std::string defaultBoggy=boggy[0];
    std::string defaultEngine=engineRight;
    std::string defaultTrack=track[0];

private:
    int makeTrack() {
        // function implementation
        // checking if length not exceedint the terminal
        struct winsize w;
        ioctl(0, TIOCGWINSZ, &w);
        if (length + col > w.ws_col) {
            return -1;
        }

        std::cout << "\033[" << row << ";" << col << "H" << defaultTrack << std::endl;
        for(int i = 1; i < length; i++) {
            std::cout << "\033[" << row << ";" << (col + i) << "H" << defaultTrack << std::endl;
        }

        return 0;
    }

private:
    // train data
    int trainBoggyCount=5;
    std::string trainArray;

    int trainRow = row;
    int trainCol = col;

    enum engineType {
        RIGHT,
        LEFT
    };

    engineType currentEngineType = RIGHT;

    int prepareTrainArray(){

    if(currentEngineType == RIGHT){
        for(int i = 0; i < trainBoggyCount; i++) {
            trainArray += defaultBoggy;
        }

        trainArray += defaultEngine;
    }else{
        trainArray += defaultEngine;
        for(int i = 0; i < trainBoggyCount; i++) {
            trainArray += defaultBoggy;
        }
    }
        return 0;
    }
    int destroyTrainArray() {
        trainArray.clear();
        return 0;
    }

    int placeTrain() {
        std::cout << "\033[" << trainRow << ";" << trainCol << "H" << trainArray << std::endl;
        return 0;
    }

public:
    train(int row, int col, int length) {
        // constructor implementation
        this->row = row;
        this->col = col;
        this->length = length;

        trainRow = row;
        trainCol = col;

        if(makeTrack() == -1) {
            std::cerr << "Error: Train length exceeds terminal width." << std::endl << std::flush;
            exit(1);
        }
        prepareTrainArray();
        placeTrain();
    }

    int nextFrame() {
        // function implementation

        if (trainCol + trainBoggyCount + 1 < col + length) {
            trainCol++;
        } else {
            return -1;
        }

        std::cout << "\033[" << trainRow << ";" << trainCol << "H" << trainArray << std::endl;
        std::cout << "\033[" << trainRow << ";" << (trainCol - 1) << "H" << defaultTrack << std::endl;
        std::cout << std::flush;

        return 0;
    }

    int prevFrame() {
        // function implementation

        if (trainCol > col) {
            trainCol--;
        } else {
            return -1;
        }

        std::cout << "\033[" << trainRow << ";" << trainCol << "H" << trainArray << std::endl;
        std::cout << "\033[" << trainRow << ";" << (trainCol + trainBoggyCount + 1) << "H" << defaultTrack << std::endl;
        std::cout << std::flush;

        return 0;
    }

    int chageBoggyCount(int newCount) {
        // function implementation
        if (newCount < 1) {
            std::cerr << "Error: Boggy count must be at least 1." << std::endl;
            return -1;
        }

        trainBoggyCount = newCount;
        destroyTrainArray();    
        prepareTrainArray();

        return 0;
    }

    int changeBoggyType(int newType) {
        // function implementation
        if (newType < 0 || newType >= 5) {
            std::cerr << "Error: Invalid boggy type." << std::endl;
            return -1;
        }

        defaultBoggy = boggy[newType];
        destroyTrainArray();
        prepareTrainArray();

        return 0;
    }

    int changeEngineType(int newType) {
        // function implementation
        if (newType < 0 || newType >= 2) {
            std::cerr << "Error: Invalid engine type." << std::endl;
            return -1;
        }

        currentEngineType = (newType == 0) ? RIGHT : LEFT;
        defaultEngine = (newType == 0) ? engineRight : engineLeft;
        destroyTrainArray();
        prepareTrainArray();

        return 0;
    }

    int changeTrackType(int newType) {
        // function implementation
        if (newType < 0 || newType >= 18) {
            std::cerr << "Error: Invalid track type." << std::endl;
            return -1;
        }

        defaultTrack = track[newType];
        makeTrack();
        placeTrain();

        return 0;
    }

};
