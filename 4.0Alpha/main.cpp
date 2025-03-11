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
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stack>
#include <sstream>
#include <cstdlib> // Allow me to use the paste feature.
#include <cstring>
#include <iomanip>
#include <string>
#include <ctime> // Will help display the time at the bottom.
#include <cctype>
#include <algorithm>
#include <cstdio> // Important to allow the user to delete a file.
#include <sys/stat.h> // Being used for the file size of the document.
#include <iomanip> 






std::string getFileSize(const std::string &filename){ // Calcuction to find the file size. 
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        if (rc == 0) {
            double size = stat_buf.st_size;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);

        if (size < 1024) {
            oss << size << " B";
        } else if (size < 1024 * 1024) {
            oss << size / 1024 << " KB";
        } else if (size < 1024 * 1024 * 1024) {
            oss << size / (1024 * 1024) << " MB";
        } else {
            oss << size / (1024 * 1024 * 1024) << " GB";
        }
        return oss.str(); 
    }
        return "0 B"; // Return 0 B if the file doesn't exist or can't be accessed
}


// --help command to show how to run the application.
void helpCommand(){
    std::cout << "Usage: nemos [OPTIONS]\n"
    << "nemos                      Will create or use untitled.txt document\n"
    << "nemos file.txt             Will create or use the file.txt\n"
    << "nemos --delete file.txt    Will delete the file that is given\n"    
    << "nemos --version            Show what version of Nemos is installed\n"
    << "nemos --help               Show the help message\n";


}
// --delete command to allow the user to delete a file. :)

int DeleteFile(int argc, char *argv[], int i){
            if (i + 1 >= argc) {
                std::cerr << "Error: No filename has been entered! :(\n";
                std::cerr << "Please do 'nemos --delete file.txt' to delete a file! :)\n";

                return 1; // Error Code exit. :)
            }

            // Handle the --delete option
            std::string filename = argv[i + 1];
            std::cout << "Are you sure you want to delete " << filename << "? THIS ACTION CANNOT BE UNDONE! (Y/N): ";
            char response;
            std::cin >> response;
            if (response == 'Y' || response == 'y') {
                if (std::remove(filename.c_str()) == 0) {
                    std::cout << "File " << filename << " has been deleted! :)\n";
                } else {
                    std::cerr << "Error: Unable to delete the file " << filename << "! :(\n";
                }
            } else {
                std::cout << "File deletion has been canceled! :)\n";
            }
            return 0; // Exit after handling --delete
        }



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
    bool isModified = false; // Will  be used when the user tries to leave but may forget to save..
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
        isModified = false;
    }

    std::string getCurrentTime(){
        time_t now = time(0); //Getting the current time.
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now); //convert the local time
        strftime(buf, sizeof(buf),"%H:%M:%S", &tstruct);
        return std::string(buf);
    }
    void LiveTime(){
        while (true){
            std::string timeStr = getCurrentTime();
            attron(COLOR_PAIR(2));
            //mvprintw(LINES - 1, COLS - 10, "%s", timeStr.c_str());
            attroff(COLOR_PAIR(2));
            refresh();
            napms(1000); // Wait 1 second before updating time        
            }
    }

    void renameFile(std::string &filename) {
        drawMessage("Enter new filename: ");
        echo();
        char newFilename[256];
        int ch = getch();
        if (ch == 24){
            noecho();
            return;
        }
        ungetch(ch);
        getstr(newFilename);
        noecho();

        // Check if the new filename is the same as the current one.
        if (filename == newFilename){
            drawMessage("Error: The filename is the same as before! :(");
            return;
        }
        // Check if the new filename is already exists. :-)
        std::ifstream file(newFilename);
        if (file.good()){
            drawMessage("Error: Filename is taken! :(");
            return;
        }

        // Rename the file
        if (std::rename(filename.c_str(), newFilename) == 0) {
            filename = newFilename;
            drawMessage("File has been renamed! :)");
        } else {
            drawMessage("Error: Failed to be renamed! :(");
        }
    }
    std::string Date() {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d-%m-%Y");  // Day month Year will be displayed as normal.
        return oss.str();  // Return the formatted date as a string
    }


    void drawHelp() {
        clear();
        attron(COLOR_PAIR(3));
        mvprintw(1, 1, "Help Menu for NemoS 4.0:");
        mvprintw(3, 1, "Arrow Keys: Navigate");
        mvprintw(4, 1, "Enter: Insert new line");
        mvprintw(5, 1, "Backspace: Delete character");
        mvprintw(6, 1, "Ctrl+S: Save file");
        mvprintw(7, 1, "Ctrl+R: Rename file");
        mvprintw(8,1, "Ctrl+C: Copy text");
        mvprintw(9,1,  "Ctrl+V: Paste text");
        mvprintw(10,1,  "Ctrl+Z: Undo changes");
        mvprintw(11,1,   "Ctrl+Y: Redo changes");
        mvprintw(12,1, "Ctrl+F: Find text");
        mvprintw(13,1, "Ctrl+K: Replace text");
        mvprintw(14,1, "Ctrl+D: Show date");
        mvprintw(15,1, "Ctrl+T: Show time");
        mvprintw(16,1, "Ctrl+P: Print document"); // This will use a Linux terminal application known as lpr.
        mvprintw(17, 1, "Ctrl+X: Exit editor");
        mvprintw(18, 1, "Press any key to return to the editor...");

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
            isModified = true;
            refresh();
        } 
        else {
            drawMessage("Error: There is nothing to undo! :(");
        }
    }
    void redo() {
        if (!redoStack.empty()) {
            undoStack.push(content); // Save current content to undo stack
            content = redoStack.top(); // Restore content from redo stack
            redoStack.pop();
            isModified = true;
            refresh();
        }else 
        {
            drawMessage("Error: There is nothing to redo! :(");
        }
    }
    
    // This is the printing function - For printing documents. :)
    void printFile(const std::string &filename){
        std::ofstream tempFile("temp.txt");
        for (const auto& line : content){
            tempFile << line << "\n";
        }
        tempFile.close();
        int result = system("lpr temp.txt");
        if (result == 0) {
            drawMessage("Document sent to printer.");
        } else {
            drawMessage("Failed to send document to printer.");
        }

        // Optionally, remove the temporary file after printing
        std::remove("temp.txt");

        }


        //This function will display the word count to the taskbar the the bottom. 
    int countWords(const std::string& text) {
            std::istringstream stream(text);
            std::string word;
            int count = 0;
            int totalWords = 0;

            
            while (stream >> word) {
                //count++; The code below will make it ignore punctionation meaning !, . and ? as those are not words.
                word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());



                if (!word.empty()){
                    count++; 
                }
            }

            return count;
    }

    void find(){ //This function will see if your text you enter is found in the text editor for the opened file. :) 
        drawMessage("Enter text to find: ");
        echo();
        char searchStr[256];
        int ch = getch();

        if (ch == 24){
            noecho();
            return;
        }
        ungetch(ch);
        getstr(searchStr);
        noecho();

        bool found = false;

        for (size_t i =0; i < content.size(); ++i){
            size_t pos = content[i].find(searchStr);
            if (pos != std::string::npos){
                cursorY = i;
                cursorX = pos;
                found = true;
                break;
            }
        }
        if (found){
            drawMessage("Text has been found! :)");
        } else {
            drawMessage("Error: Text has not been found! :(");
        }
        getch();
    }

    // This function is very good as it will allow you to replace text - Useful when it comes to programming...
    void Replace(){
        drawMessage("Enter text to find: ");
        echo();
        char searchStr[256];
        int ch = getch();

        if (ch == 24){
            noecho();
            return;
        }
        ungetch(ch);
        getstr(searchStr);
        noecho();

        drawMessage("Enter text to replace: ");
        echo();
        char replaceStr[256];
        getstr(replaceStr);
        noecho();


        bool replaced = false;
        for (size_t i = 0; i < content.size(); ++i){
            size_t pos = content[i].find(searchStr);
            if (pos != std::string::npos){
                content[i].replace(pos, strlen(searchStr), replaceStr);
                replaced = true;
            }
        }
        if (replaced){
            drawMessage("Text has been replaced. :)");
        }
        else {
            drawMessage("Error: Text has not been found. :(");
        }
        getch();
    }
    
    void drawEditor(std::string &filename) {
        bool running = true;
        int viewX = 0, viewY = 0; // Tracks the visible area (scroll position)
        //std::thread timeThread(&NemoS::LiveTime, this);  // Pass 'this' to use the member function        timeThread.detach();
        while (running) {
            std::string fullText;
            for (const auto& line : content){
                fullText += line + " ";
            }

            int wordCount = countWords(fullText);
            //Will find out the file size for the nav bar.
            std::string FileSize = getFileSize(filename); // This will get the file size. :)
            //clear();
            // Draw the editor content
            for (int i = 0; i < LINES - 1; ++i) {
                move(i, 0);
                //clrtoeol();
                int lineIndex = i + viewY; // The actual index in the content vector

            if (lineIndex < content.size()) {
                std::string line = content[lineIndex];

                int availableLength = line.size();
                int charsToPrint = std::min(availableLength, COLS - 1);
                int startPos = 0;

                if (lineIndex == cursorY) { // Current line
                    bool TextOffLeft = false;
                    availableLength -= viewX;
                    charsToPrint = std::min(availableLength, COLS - 1);
                    startPos = viewX;
                    if (startPos >= line.size()){
                        startPos = line.size() > 0 ? line.size() -1 : 0;
                    }
                    std::string visibleLine = line.substr(startPos, charsToPrint); // Create the visible line

                    TextOffLeft = (viewX > 0 && visibleLine.find_first_not_of(" \t\n\r") != std::string::npos); // Use visibleLine's size
                    attron(COLOR_PAIR(1));
                    mvprintw(i, 0, "%s", line.substr(startPos, charsToPrint).c_str());
                    attroff(COLOR_PAIR(1));
                    clrtoeol(); 
                    if (TextOffLeft) {
                        attron(COLOR_PAIR(3)); // Use a color pair for the arrow
                        mvaddch(i, 0, '<');    // Draw the arrow at the left edge
                        attroff(COLOR_PAIR(3));
                    }
                } else { // Other lines
                    move(i, 0); // Essential!

                    mvprintw(i, 0, "%s", line.substr(0, charsToPrint).c_str());
                    clrtoeol();
                }
            } else {
                clrtoeol(); // Clear any remaining content on empty lines
            }
                
                bool lineExists = (i + viewY < content.size());
                bool lineHasText = lineExists && (content[i + viewY].find_first_not_of(" \t\n\r") != std::string::npos); // Check if line has non-space characters
                //bool TextOffLeft = (viewX > 0 && lineHasText);
                bool TextOffRight = (i + viewY < content.size() && content[i + viewY].size() > viewX + COLS - 1);

                if (TextOffRight){
                    attron(COLOR_PAIR(3));
                    mvaddch(i, COLS - 1, '>');                    
                    attroff(COLOR_PAIR(3));
                }

            }

            // Draw the status bar at the bottom
            move(LINES - 1, 0); // Move to the last line
            clrtoeol(); // Clear the status bar line
            attron(COLOR_PAIR(2));




            mvprintw(LINES - 1, 0, "NemoS 4.0 | File: %s | File Size: %s | Word Count: %d | Line: %d | Column: %d | Ctrl+H: Help | Ctrl+X: Exit ", filename.c_str(),FileSize.c_str(),wordCount, cursorY + 1, cursorX +1);
            attroff(COLOR_PAIR(2));
            cursorX = std::min(cursorX, (int)content[cursorY].size());
            cursorY = std::min(cursorY, (int)content.size() -1);
            // Place the cursor in the correct position
            move(cursorY - viewY, cursorX - viewX); // Adjust cursor position based on scroll
            refresh(); // Refresh the screen after updates
            int height,width;
            int visiblewidth = COLS -1;
            int maxX = std::max(0, (int)content[cursorY].size() - visiblewidth); // Correct maxX            getmaxyx(stdscr, height,width);
            int ch = getch(); // Get user input
            switch (ch) {
                case KEY_UP:
                    if (cursorY > 0) {
                        cursorY--;
                        cursorX = 0;
                        viewX = 0; // Reset viewX when moving up
                        if (cursorY < viewY) viewY--; // Scroll up if needed
                    }
                    break;

                case KEY_DOWN:
                    if (cursorY < content.size() - 1) {
                        cursorY++;
                        cursorX = 0;
                        viewX = 0;
                        if (cursorY >= viewY + LINES - 1) viewY = cursorY - LINES + 1; // Scroll down if needed
                                            
                    }
                    break;
                case KEY_LEFT:
                    if (cursorX > 0) {
                        cursorX--;
                        if (cursorX < viewX) {
                            viewX = cursorX; // Scroll left
                        }
                    } else if (cursorY > 0) {
                        cursorY--;
                        cursorX = content[cursorY].size(); // End of previous line

                        int visibleWidth = COLS - 1; // Get visible width
                        viewX = std::max(0, (int)content[cursorY].size() - visibleWidth); // Correct viewX

                        if (cursorY < viewY) {
                            viewY = cursorY; // Scroll up if needed
                        }
                    }
                    break;

                case KEY_RIGHT:
                    if (cursorX < content[cursorY].size()) {
                        cursorX++;

                        // *** The crucial fix: ***
                        int visibleWidth = COLS - 1; // Or adjust as needed for borders/etc.
                        if (cursorX >= viewX + visibleWidth) {
                            viewX = cursorX - visibleWidth + 1; // Scroll right
                        }
                    } else if (cursorY < content.size() - 1) { // Check for next line
                            cursorY++;        // Move to the next line
                            cursorX = 0;        // Start at the beginning of the next line
                            viewX = 0;        // Reset horizontal view
                    }
                    break;
                case 6: //Ctrl + F
                    find();
                    break;
                case 20: // Ctrl + T
                    timeout(0);
                    while(true){
                        std::string timeStr = getCurrentTime();
                        attron(COLOR_PAIR(2));
                        //mvprintw(LINES - 1, COLS - 10, "%s", timeStr.c_str());
                        drawMessage("The time is: " +  getCurrentTime());
                        attroff(COLOR_PAIR(2));
                        refresh();
                        napms(1000); // Wait 1 second before updating time        

                    
                        if (getch() !=ERR){
                            break;
                        }
                    }
                    timeout(-1);
                    break;

                case '\n': // Enter key
                    pushUndo();
                    content.insert(content.begin() + cursorY + 1, content[cursorY].substr(cursorX));
                    content[cursorY] = content[cursorY].substr(0, cursorX);
                    cursorY++;
                    cursorX = 0;
                    isModified = true;
                    viewX = 0; // Reset viewX after inserting a new line
                    if (cursorY >= viewY + LINES - 1) viewY++; // Scroll down if needed
                    break;
                case KEY_BACKSPACE:
                case 127:
                    if (cursorX > 0) {
                        pushUndo();
                        content[cursorY].erase(cursorX - 1, 1);
                        cursorX--;
                        isModified = true;
                    } else if (cursorY > 0) {
                        pushUndo();
                        cursorX = content[cursorY - 1].size();
                        content[cursorY - 1] += content[cursorY];
                        content.erase(content.begin() + cursorY);
                        cursorY--;
                        isModified = true; 
                    }
                    break;
                case 24: // Ctrl+X (Exit)
                if (isModified){   
                    attron(COLOR_PAIR(2));
                    mvprintw(LINES - 2, 0, "Warning: Are you sure you want to leave without saving? (Y/N)");
                    clrtoeol();
                    attroff(COLOR_PAIR(2));
                    refresh();

                    int answer = getch();
                    if (answer == 'Y' || answer == 'y') {
                        running = false;
                    }
                } else {
                    running = false;
                }

                    break;
                case 19: // Ctrl+S (Save)
                    saveFile(filename);
                    drawMessage("File has been saved! :)");
                    break;
                case '\t': // Allow the tab key to work correctly. 
                    pushUndo();
                    content[cursorY].insert(cursorX, "    ");
                    cursorX += 4;
                    isModified = true;
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
                case 11: // Control K
                    Replace();
                    break;
                //The case 22 will be ctrl V that will allow for pasting text into the application.
                case 22: { // Ctrl+V - Paste text from clipboard using xclip
                    pushUndo();
                    FILE *clipboard = popen("xclip -o -selection clipboard", "r");
                    if (clipboard) {
                        char buffer[256];
                        std::string clipboardText;
                        // Read all clipboard data
                        while (fgets(buffer, sizeof(buffer), clipboard)) {
                            clipboardText += buffer;
                        }
                        pclose(clipboard);
                        
                        if (clipboardText.empty()) {
                            drawMessage("Clipboard is empty or no owner for the clipboard selection.");
                            break;
                        }
                        isModified = true;
                        
                        // Split clipboardText into lines
                        std::istringstream iss(clipboardText);
                        std::string line;
                        std::vector<std::string> lines;
                        while (std::getline(iss, line)) {
                            lines.push_back(line);
                        }
                        
                        // Insert the first line at the current cursor position
                        std::string beforeCursor = content[cursorY].substr(0, cursorX);
                        std::string afterCursor = content[cursorY].substr(cursorX);
                        content[cursorY] = beforeCursor + lines[0];
                        
                        // Insert any subsequent lines as new lines in the editor
                        for (size_t i = 1; i < lines.size(); ++i) {
                            content.insert(content.begin() + cursorY + i, lines[i]);
                        }
                        
                        // Append the remaining part of the original line to the end of the last pasted line
                        content[cursorY + lines.size() - 1] += afterCursor;
                        
                        // Update the cursor position to the end of the pasted text
                        cursorY = cursorY + lines.size() - 1;
                        cursorX = lines.back().size();
                    } else {
                        drawMessage("Error: Clipboard empty or could not be accessed.");
                    }
                    break;
                }


                case 3: // The case 3 will be used to add the control C support - Will allow for copying text.
                    if (!content.empty() && cursorY < content.size()) {
                        std::string textToCopy = content[cursorY]; // Copy the current line
                        FILE *clipboard = popen("xclip -i -selection clipboard", "w");
                        if (clipboard) {
                            fputs(textToCopy.c_str(), clipboard); // Write the line to clipboard
                            pclose(clipboard);
                            drawMessage("Text copied to clipboard! :)");
                        } else {
                            drawMessage("Error 2: Install XClip to copy text.");
                        }
                    }                    
                    
                    break;
                case 4:
                    drawMessage("The date is: " + Date());
                    break;
                case 16:
                    printFile(filename);

                    // This small code for testing. :)
                    //drawMessage("It works");
                    break;

                default:
                    pushUndo();
                    content[cursorY].insert(cursorX, 1, ch);
                    cursorX++;
                    isModified = true;

                    // **Handle scrolling while typing**
                    if (cursorX >= viewX + COLS - 1) viewX++; // Scroll horizontally when typing beyond the view
                    if (cursorY >= viewY + LINES - 2) viewY++; // Scroll vertically if typing creates new lines
                    break;
            }
            refresh();
            // Ensure the cursor doesn't go out of bounds
            cursorX = std::min(cursorX, (int)content[cursorY].size());
            cursorY = std::min(cursorY, (int)content.size() - 1);

            //if (cursorX > content[cursorY].size()) cursorX = content[cursorY].size();
            //if (cursorY >= content.size()) cursorY = content.size() - 1;
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
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            helpCommand();
            return 0; // Exit after showing help
        }
        else if (arg == "--delete"){ // Allow for a file to be deleted. :)
            return DeleteFile(argc, argv, i);
        }
        else if (arg == "--version"){ // This will show the user what version of nemos they are using.
            std::cout << "nemos, version 4.0 Alpha\n";
            return 0;

        }
    }
    std::string filename;
    if (argc >= 2) {
        filename = argv[1];
    }

    NemoS editor;
    editor.run(filename);

    return 0;
}