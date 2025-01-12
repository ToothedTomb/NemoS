/* MIT License

Copyright (c) 2025 Jonathan Steadman

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

// Notes you can get xclip installed with using the installer.sh file.

// It is required when you want to paste text into the application. 
#include <ncurses.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stack>
#include <sstream>
#include <cstdlib> // Allow me to use the paste feature.


class NemoS {
public:
    NemoS() {
        initscr();             // Start ncurses
        raw();                 // Disable line buffering
        keypad(stdscr, TRUE);  // Enable special keys
        noecho();              // Don't echo input
        start_color();         // Enable colors

        // Define pinkish color pairs
        init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Text in pink
        init_pair(2, COLOR_BLACK, COLOR_MAGENTA); // Status bar
        init_pair(3, COLOR_MAGENTA, COLOR_BLACK); // Help text
    }

    ~NemoS() {
        endwin(); // End ncurses
    }

    void run(std::string &filename) {
        if (filename.empty()) {
            filename = "untitled.txt";
        }
        loadFile(filename);
        drawEditor(filename);
    }

private:
    std::vector<std::string> content; // Stores the text file content
    int cursorX = 0, cursorY = 0;     // Cursor position
    std::stack<std::vector<std::string>> undoStack; // Undo stack
    std::stack<std::vector<std::string>> redoStack; // Redo stack
    void loadFile(const std::string &filename) {
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            content.push_back(line);
        }
        if (content.empty()) content.push_back(""); // Ensure at least one line
    }

    void saveFile(const std::string &filename) {
        std::ofstream file(filename);
        for (const auto &line : content) {
            file << line << "\n";
        }
    }

    void renameFile(std::string &filename) {
        drawMessage("Enter new filename: ");
        echo();
        char newFilename[256];
        getstr(newFilename);
        noecho();

        // Rename the file
        if (std::rename(filename.c_str(), newFilename) == 0) {
            filename = newFilename;
            drawMessage("File has been renamed. :)!");
        } else {
            drawMessage("Error: Failed to be renamed. Try again. :(");
        }
    }

    void drawHelp() {
        clear();
        attron(COLOR_PAIR(3));
        mvprintw(1, 1, "Help Menu for NemoS:");
        mvprintw(3, 1, "Arrow Keys:  Navigate");
        mvprintw(4, 1, "Enter: Insert new line");
        mvprintw(5, 1, "Backspace: Delete character");
        mvprintw(6, 1, "Ctrl+S: Save file");
        mvprintw(7, 1, "Ctrl+R: Rename file");
        mvprintw(8,1, "Ctrl+C: Copy text");
        mvprintw(9,1,  "Ctrl+V: Paste text");
        mvprintw(10,1,  "Ctrl+Z: Undo changes");
        mvprintw(11,1,   "Ctrl+Y: Redo changes");
        mvprintw(12, 1, "Ctrl+X: Exit editor");
        mvprintw(13, 1, "Press any key to return to the editor...");

        attroff(COLOR_PAIR(3));
        getch();
    }
    void pushUndo(){
        undoStack.push(content);
        while (!redoStack.empty()) redoStack.pop();

    }
    void undo() {
        if (!undoStack.empty()) {
            redoStack.push(content); // Save current content to redo stack
            content = undoStack.top(); // Restore previous content from undo stack
            undoStack.pop();
            refresh();
        }
    }
    void redo() {
        if (!redoStack.empty()) {
            undoStack.push(content); // Save current content to undo stack
            content = redoStack.top(); // Restore content from redo stack
            redoStack.pop();
            refresh();
        }
    }

        //This function will display the word count to the taskbar the the bottom. 
    int countWords(const std::string& text) {
            std::istringstream stream(text);
            std::string word;
            int count = 0;
            while (stream >> word) {
                count++;
            }
            return count;
    }


    /*int countWords(const std::string& text) {
        std::istringstream stream(text);
        std::string word;
        int count = 0;
        while (stream >> word) {
            count++;
        }
        return count;
    }*/
    void drawEditor(std::string &filename) {
        bool running = true;
        int viewX = 0, viewY = 0; // Tracks the visible area (scroll position)

        while (running) {
            std::string fullText;
            for (const auto& line : content){
                fullText += line + " ";
            }
            int wordCount = countWords(fullText);
            // Draw the editor content
            for (int i = 0; i < LINES - 1; ++i) { // Leave the last line for the status bar
                move(i, 0);
                clrtoeol(); // Clear the line before redrawing

                if (i + viewY < content.size()) {
                    std::string line = content[i + viewY];
                    if (viewX < line.size()) {
                        attron(COLOR_PAIR(1));
                        mvprintw(i, 0, "%s", line.substr(viewX, COLS - 1).c_str());
                        attroff(COLOR_PAIR(1));
                    }
                }
            }

            // Draw the status bar at the bottom
            move(LINES - 1, 0); // Move to the last line
            clrtoeol(); // Clear the status bar line
            attron(COLOR_PAIR(2));




            mvprintw(LINES - 1, 0, "NemoS 3.0 | File: %s | Word Count: %d | Line: %d | Column: %d | Ctrl+H: Help | Ctrl+X: Exit ", filename.c_str(),wordCount, cursorY + 1, cursorX +1);
            attroff(COLOR_PAIR(2));

            // Place the cursor in the correct position
            move(cursorY - viewY, cursorX - viewX); // Adjust cursor position based on scroll
            refresh(); // Refresh the screen after updates
            FILE *clipboard = popen("xclip -o -selection clipboard", "r");

            int ch = getch(); // Get user input
            switch (ch) {
                case KEY_UP:
                    if (cursorY > 0) cursorY--;
                    if (cursorY < viewY) viewY--; // Scroll up
                    break;
                case KEY_DOWN:
                    if (cursorY < content.size() - 1) cursorY++;
                    if (cursorY >= viewY + LINES - 2) viewY++; // Scroll down
                    break;
                case KEY_LEFT:
                    if (cursorX > 0) cursorX--;
                    if (cursorX < viewX) viewX--; // Scroll left
                    break;
                case KEY_RIGHT:
                    if (cursorX < content[cursorY].size()) cursorX++;
                    if (cursorX >= viewX + COLS - 1) viewX++; // Scroll right
                    break;
                case '\n': // Enter key
                pushUndo();
                    content.insert(content.begin() + cursorY + 1, content[cursorY].substr(cursorX));
                    content[cursorY] = content[cursorY].substr(0, cursorX);
                    cursorY++;
                    cursorX = 0;
                    break;
                case KEY_BACKSPACE:
                case 127:
                    if (cursorX > 0) {
                        pushUndo();
                        content[cursorY].erase(cursorX - 1, 1);
                        cursorX--;
                    } else if (cursorY > 0) {
                        pushUndo();
                        cursorX = content[cursorY - 1].size();
                        content[cursorY - 1] += content[cursorY];
                        content.erase(content.begin() + cursorY);
                        cursorY--;
                    }
                    break;
                case 24: // Ctrl+X (Exit)
                    running = false;
                    break;
                case 19: // Ctrl+S (Save)
                    saveFile(filename);
                    drawMessage("File has been saved :)!");
                    break;
                case 18: // Ctrl+R (Rename)
                    renameFile(filename);
                    break;
                case 26: // Ctrl + Z Undo
                    undo();
                    break;
                case 25: // ctrl + y for redo
                    redo();
                    break;
                case 8: // Ctrl+H (Help)
                    drawHelp();
                    break;
                //The case 22 will be ctrl V that will allow for pasting text into the application.
                case 22:
                    pushUndo();
                    if (clipboard){
                        char buffer[256];
                        std::string clipboardText;
                        while (fgets(buffer, sizeof(buffer), clipboard)){
                            clipboardText += buffer;
                        }
                        pclose(clipboard);



                        content[cursorY].insert(cursorX,clipboardText);
                        cursorX += clipboardText.size();
                    } else{
                        drawMessage("Error 2: Install XClip to paste text.");
                    }
                    break;
                case 3: // The case 3 will be used to add the control C support - Will allow for copying text.
                    drawMessage("Control-C is working! :)");
                    break;

                default:
                    pushUndo();
                    content[cursorY].insert(cursorX, 1, ch);
                    cursorX++;

                    // **Handle scrolling while typing**
                    if (cursorX >= viewX + COLS - 1) viewX++; // Scroll horizontally when typing beyond the view
                    if (cursorY >= viewY + LINES - 2) viewY++; // Scroll vertically if typing creates new lines
                    break;
            }

            // Ensure the cursor doesn't go out of bounds
            if (cursorX > content[cursorY].size()) cursorX = content[cursorY].size();
            if (cursorY >= content.size()) cursorY = content.size() - 1;
        }
    }






    void drawMessage(const std::string &message) {
        attron(COLOR_PAIR(2));
        mvprintw(LINES - 2, 0, "%s", message.c_str());
        attroff(COLOR_PAIR(2));
        refresh();
        getch(); // Wait for key press
    }
};

int main(int argc, char *argv[]) {
    std::string filename;
    if (argc >= 2) {
        filename = argv[1];
    }

    NemoS editor;
    editor.run(filename);

    return 0;
}
