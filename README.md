# NemoS - Text Editor For Linux:

NemoS - Is a simple Terminal Text Editor that is based on Nano.

# Install to your system:
**Note: You need to be the root user.**

sudo bash install.sh 

# Running the Application:
nemos - Will create a untitlied.txt file.
nemos txt.txt - Will use the file or create a new one if txt.txt does not exist.


# Instuctions:
 Arrow Keys: Navigate 
 Enter: Insert new line 
 Backspace: Delete character 
 Ctrl+S: Save file 
 Ctrl+R: Rename file
 Ctrl+X: Exit editor
 
 
# Screenshots:

![Screenshot From 2025-01-10 22-01-23](https://github.com/user-attachments/assets/d4c91337-ab96-44ae-a3c4-0a2420d767fd)

![Screenshot From 2025-01-10 22-02-13](https://github.com/user-attachments/assets/6b6bcd53-76bc-4191-b9b2-0754face7f59)


# Compile:

g++ main.cpp -o nemos -lncurses

