/**
 * File: life.cpp
 * --------------
 * Implements the Game of Life.
 */

#include <iostream>  // for cout
#include <fstream>   // for files
#include <string>    // for strings
using namespace std;

#include "console.h" // required of all files that contain the main function
#include "simpio.h"  // for getLine
#include "gevents.h" // for mouse event detection
#include "gtimer.h"
#include "strlib.h"
#include "random.h"
#include "filelib.h" // for files

#include "life-constants.h"  // for kMaxAge
#include "life-graphics.h"   // for class LifeDisplay

static void computeNext(LifeDisplay& display, Grid<int>& current, Grid<int>& next);
static bool isNumNeighboursCorrect(Grid<int>& current, int row, int col,
                                   int neighbours);
static bool isDirectionEmpty(Grid<int>& current, int row, int col,
                             int drow, int dcol);
static bool isEmpty(Grid<int>& current, int row, int col);

/**
 * Function: welcome
 * -----------------
 * Introduces the user to the Game of Life and its rules.
 */
static void welcome() {
    cout << "Welcome to the game of Life, a simulation of the lifecycle of a bacteria colony." << endl;
    cout << "Cells live and die by the following rules:" << endl << endl;
    cout << "\tA cell with 1 or fewer neighbors dies of loneliness" << endl;
    cout << "\tLocations with 2 neighbors remain stable" << endl;
    cout << "\tLocations with 3 neighbors will spontaneously create life" << endl;
    cout << "\tLocations with 4 or more neighbors die of overcrowding" << endl << endl;
    cout << "In the animation, new cells are dark and fade to gray as they age." << endl << endl;
    getLine("Hit [enter] to continue....   ");
}

/**
 * Function: randomInitialization
 * -----------------
 * Initialize a grid randomly.
 */
static void randomInitialize(LifeDisplay& display, Grid<int>& current) {
    // Get a random row and column.
    int row = randomInteger(40, 60),
        col = randomInteger(40, 60);
    current.resize(row, col);

    // initialize the grid
    display.setDimensions(current.numRows(), current.numCols());

    // set randomly
    for (int i = 0; i < current.numRows(); i++) {
        for (int j = 0; j < current.numCols(); j++) {
            int prob = randomInteger(0, 100);
            if (prob < 50) {
                current[i][j] = 0;
            } else {
                current[i][j] = randomInteger(1, kMaxAge);
            }
            display.drawCellAt(i, j, current.get(i, j));
        }
    }
    display.repaint();
}

/**
 * Function: fileInitialization
 * -----------------
 * Initialize a grid from a file.
 */
static void fileInitialize(LifeDisplay& display, Grid<int>& current, string filename) {
    ifstream input;

    // open the file
    int num = 0;
    while (true) {
        if (num != 0) {
            filename = getLine("Enter name of colony file (or RETURN to seed randomly): ");
        }
        if (fileExists(filename + ".txt")) {
            input.open(filename + ".txt");
            break;
        } else {
            cout << "Unable to open the file named \"" << filename <<
                    "\". Please select another file." << endl;
        }
        num ++;
    }

    // get the row and column
    int row, col;
    input >> row >> col;
    current.resize(row, col);

    // initialize the grid
    display.setDimensions(current.numRows(), current.numCols());

    // set from the file
    for (int i = 0; i < current.numRows(); i++) {
        string line;
        input >> line;
        for (int j = 0; j < current.numCols(); j++) {
            if (line[j] == '-') {
                current[i][j] = 0;
            } else {
                current[i][j] = 1;
            }
            display.drawCellAt(i, j, current.get(i, j));
        }
    }
    input.close();
    display.repaint();
}

/**
 * Function: initialization
 * -----------------
 * Initialize a grid either randomly or from a file.
 */
static void initialize(LifeDisplay& display, Grid<int>& current) {
    cout << "You can start your colony with random cells or read from a prepared file." << endl;
    string start = getLine("Enter name of colony file (or RETURN to seed randomly): ");
    if (start == "") {
        randomInitialize(display, current);
    } else {
        fileInitialize(display, current, start);
    }

}

/**
 * Function: computeNext
 * --------------
 * The rule for computing the next generation:
 * Rule 1
   A location that has zero or one neighbour will be empty in the next generation.
   Rule 2
   A location with two neighbours is stable.
   Rule 3
   A location with three neighbours will contain a cell in the next generation.
   If unoccupied, a new cell is born. If occupied, the cell remains.
   Rule 4
   A location with four or more neighbours will be empty in the next generation.
   Rule 5
   The births and deaths that tranform one generation to the next must take effect
   simultaneously.
 */
static void computeNext(LifeDisplay& display, Grid<int>& current, Grid<int>& next) {
    next.resize(current.numRows(), current.numCols());
    for (int i = 0; i < current.numRows(); i++) {
        for (int j = 0; j < current.numCols(); j++) {
            if (isNumNeighboursCorrect(current, i, j, 0) ||
                isNumNeighboursCorrect(current, i, j, 1)) {    // empty in the next generation
                next[i][j] = 0;
            } else if (isNumNeighboursCorrect(current, i, j, 2)) {  // stable in the next generation
                if (isEmpty(current, i, j)) {
                    next[i][j] = 0; // remain empty
                } else {
                    next[i][j] = current[i][j] + 1; // the cell ages
                }
            } else if (isNumNeighboursCorrect(current, i, j, 3)) {
                if (isEmpty(current, i, j)) {   // Empty before, a new cell is born.
                    next[i][j] = 1;
                } else {      // Currently contains a cell, it remains.
                    next[i][j] = current[i][j] + 1; // the cell ages
                }
            } else {     // four or more neighbours
                next[i][j] = 0; // empty in the next generation
            }
            display.drawCellAt(i, j, next.get(i, j));
        }
    }
}

/**
 * Function: isNumNeighboursCorrect
 * --------------
 * Checks if the number of neighbours is correct compared to the given number.
 * Check the eight directions, if not empty, add one to numNeighbours.
 * if numNeighbours is equal to the given number, return true.
 */
static bool isNumNeighboursCorrect(Grid<int>& current, int row, int col,
                                   int neighbours) {
    int numNeighbours = 0;

    for (int drow = -1; drow <= 1; drow ++) {
        for (int dcol = -1; dcol <= 1; dcol ++) {
            if (!isDirectionEmpty(current, row, col, drow, dcol)) {
                numNeighbours ++;
            }
        }
    }
    if (numNeighbours == neighbours) {
        return true;
    }
    return false;
}

/**
 * Function: isDirectionEmpty
 * --------------
 * Check if the eight directions are empty.
 * If the location is the corner or the edge, then first check the directions
 * that are within the grid.
 */
static bool isDirectionEmpty(Grid<int>& current, int row, int col,
                             int drow, int dcol){
    // calculate the coordiante of the given direction
    row += drow;
    col += dcol;

    // assume the location do not affect numNeighbour
    if (drow == 0 && dcol == 0) {
        return true;
    }

    // check if the direction is within the grid
    // if so, check if it is empty
    // if outside the grid, return true, which means that there are no neighbours
    if (current.inBounds(row, col)) {
        return current[row][col] == 0;
    } else {
        return true;
    }
}

/**
 * Function: isEmpty
 * --------------
 * Check if the location is empty.
 */
static bool isEmpty(Grid<int>& current, int row, int col) {
    return current[row][col] == 0;
}

/**
 * Function: advance
 * --------------
 * Advace the grid.
 */
static void advance(LifeDisplay& display, Grid<int>& current, Grid<int>& next) {
    computeNext(display, current, next);
    current = next;
    display.repaint();
}

/**
 * Function: runAnimation
 * --------------
 * Run the animation.
 * ms controls the speed of the animation.
 * When the mouse is clicked, the animation stops.
 */
static void runAnimation(LifeDisplay& display, Grid<int>& current, Grid<int>& next, int ms) {
    GTimer timer(ms);
    timer.start();
    while (true) {
        GEvent event = waitForEvent(TIMER_EVENT + MOUSE_EVENT);
        if (event.getEventClass() == TIMER_EVENT) {
            advance(display, current, next);
        } else if (event.getEventType() == MOUSE_PRESSED) {
            break;
        }
    }
    timer.stop();
}

/**
 * Function: setSpeed
 * --------------
 * Set the speed of the animation.
 */
static int setSpeed(LifeDisplay& display, Grid<int>& current, Grid<int>& next) {
    cout << "You choose how fast to run the simulation." << endl;
    cout << "\t1 = As fast as this chip can go!" << endl;
    cout << "\t2 = Not too fast, this is a school zone." << endl;
    cout << "\t3 = Nice and slow so I can watch everything that happens." << endl;
    cout << "\t4 = Require enter key be pressed before advancing to next generation." << endl;

    int ms = 0;     // speed of the animation

    while (true) {
        int choice = getInteger("Your choice: ");
        if (choice == 1) {
            ms = 10;
            break;
        } else if (choice == 2) {
            ms = 100;
            break;
        } else if (choice == 3) {
            ms = 1000;
            break;
        } else if (choice == 4) {
            while (true) {
               string adv = getLine("Please return to advance [or type out \"quit\" to end]: ");
               if (adv == "quit") {
                   break;
               } else {
                   advance(display, current, next);
               }
            }
            break;
        } else {
            cout << "Please enter a number between 1 and 4!" << endl;
        }
    }
    return ms;
}

/**
 * Function: start
 * --------------
 * Start the game.
 */
static void start(LifeDisplay& display, Grid<int>& current, Grid<int>& next) {
    // initialization
    initialize(display, current);

    // animate the world
    int ms = setSpeed(display, current, next);
    if (ms != 0) {
        runAnimation(display, current, next, ms);
    }
}
/**
 * Function: restart
 * --------------
 * Restart the game.
 */
static void restart(LifeDisplay& display, Grid<int>& current, Grid<int>& next) {
    while (true) {
        string run = getLine("Would you like to run another?");
        if (run == "yes") {
            start(display, current, next);
        } else if (run == "no") {
            break;
        } else {
            cout << "Please enter \"yes\" or \"no\"." << endl;
        }
    }
}

/**
 * Function: main
 * --------------
 * Provides the entry point of the entire program.
 */
int main() {
    LifeDisplay display;
    display.setTitle("Game of Life");
    welcome();

    Grid<int> current;
    Grid<int> next;

    // Play the game.
    start(display, current, next);
    restart(display, current, next);

    return 0;
}
