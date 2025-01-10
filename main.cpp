#include <ncurses.h>
#include <fstream>
#include <string>
#include <vector>

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
            drawMessage("File renamed successfully!");
        } else {
            drawMessage("Failed to rename the file.");
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
        mvprintw(8, 1, "Ctrl+X: Exit editor");
        mvprintw(10, 1, "Press any key to return to the editor...");

        attroff(COLOR_PAIR(3));
        getch();
    }

    void drawEditor(std::string &filename) {
        bool running = true;

        while (running) {
            // Draw status bar
            attron(COLOR_PAIR(2));
            mvprintw(LINES - 1, 0, "NemoS | File: %s | Ctrl+S: Save | Ctrl+R: Rename | Ctrl+H: Help | Ctrl+X: Exit", filename.c_str());
            attroff(COLOR_PAIR(2));
            clear();
            attron(COLOR_PAIR(1));
            for (int i = 0; i < content.size(); ++i) {
                mvprintw(i, 0, "%s", content[i].c_str());
            }
            attroff(COLOR_PAIR(1));

            // Draw status bar
            attron(COLOR_PAIR(2));
            mvprintw(LINES - 1, 0, " NemoS | File: %s | Ctrl+S: Save | Ctrl+R: Rename | Ctrl+H: Help | Ctrl+X: Exit", filename.c_str());
            attroff(COLOR_PAIR(2));

            // Place the cursor
            move(cursorY, cursorX);

            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    if (cursorY > 0) cursorY--;
                    if (cursorX > content[cursorY].size()) cursorX = content[cursorY].size();
                    break;
                case KEY_DOWN:
                    if (cursorY < content.size() - 1) cursorY++;
                    if (cursorX > content[cursorY].size()) cursorX = content[cursorY].size();
                    break;
                case KEY_LEFT:
                    if (cursorX > 0) cursorX--;
                    break;
                case KEY_RIGHT:
                    if (cursorX < content[cursorY].size()) cursorX++;
                    break;
                case '\n': // Enter key
                    content.insert(content.begin() + cursorY + 1, content[cursorY].substr(cursorX));
                    content[cursorY] = content[cursorY].substr(0, cursorX);
                    cursorY++;
                    cursorX = 0;
                    break;
                case KEY_BACKSPACE:
                case 127:
                    if (cursorX > 0) {
                        content[cursorY].erase(cursorX - 1, 1);
                        cursorX--;
                    } else if (cursorY > 0) {
                        cursorX = content[cursorY - 1].size();
                        content[cursorY - 1] += content[cursorY];
                        content.erase(content.begin() + cursorY);
                        cursorY--;
                    }
                    break;
                case 24: // Ctrl+X
                    running = false;
                    break;
                case 19: // Ctrl+S
                    saveFile(filename);
                    drawMessage("File has been saved :)!");
                    break;
                case 18: // Ctrl+R
                    renameFile(filename);
                    break;
                case 8: // Ctrl+H
                    drawHelp();
                    break;
                default:
                    content[cursorY].insert(cursorX, 1, ch);
                    cursorX++;
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
