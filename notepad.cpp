#include <iostream>
#include <fstream>
#include <stack>
#include <deque>
#include <list>
#include <string>
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include <iterator>
#include <iomanip>

using namespace std;
list<string> FilesRecorder;
struct state
{
    int currentCol;
    int currentRow;
    list<list<char>> text;
    list<list<char>>::iterator rowItr;
    list<char>::iterator colItr;
};

void gotoxy(int x, int y)
{
    COORD coordinates;
    coordinates.X = x;
    coordinates.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordinates);
}

class Editor
{
private:
    int currentCol;
    int currentRow;
    list<list<char>> text;
    list<list<char>>::iterator rowItr;
    list<char>::iterator colItr;
    deque<state> undo;
    stack<state> redo;

public:
    // Constructor
    Editor()
    {
        list<char> row;
        text.push_back(row);
        rowItr = text.begin();
        colItr = (*rowItr).begin();
        currentCol = 0;
        currentRow = 0;
    }

    // Functions For Moving The Cursor Across The Text

    // Moves the cursor to the up
    void moveUp()
    {
        if (rowItr != text.begin())
        {
            rowItr--;
            colItr = (*rowItr).begin();
            advance(colItr, currentCol); // advance to current col
            currentRow--;

            if (distance(colItr, rowItr->end()) < 0)
            {
                colItr = rowItr->end();
            }
        }
    }

    void moveDown()
    {
        if (rowItr != prev(text.end()))
        {
            rowItr++;
            // Adjust colItr only if it goes beyond the end of the current row
            colItr = (*rowItr).begin();
            advance(colItr, currentCol); // advance to current col
            currentRow++;
            if (distance(colItr, rowItr->end()) < 0)
            {
                colItr = prev(rowItr->end());
            }
        }
    }

    // Moves the cursor to the left

    void moveLeft()
    {
        if (colItr != rowItr->begin())
        {
            colItr--;
            currentCol--;
        }
        else if (rowItr != text.begin())
        {
            rowItr--;
            currentCol = rowItr->size() - 1;
            currentRow++;
            colItr = rowItr->end();
        }
    }

    // Moves the cursor to the right

    void moveRight()
    {
        if (colItr != (*rowItr).end())
        {
            colItr++;
            currentCol++;
        }
        else if (currentRow != text.size() - 1)

        {
            rowItr++;
            currentCol = 0;
            currentRow++;
            colItr = rowItr->begin();
        }
    }

    // Functions For Inserting & Deleting Characters

    // Inserts a character at the cursor position

    void insertChar(char c)
    {
        colItr = (*rowItr).insert(colItr, c);
        colItr++;
        currentCol++;
    }

    // Deletes a character at the cursor position

    void deleteChar()
    {
        if (colItr != rowItr->begin())
        {
            colItr--;
            colItr = rowItr->erase(colItr);
            currentCol--;
        }
        else if (rowItr != text.begin())
        {

            rowItr--;
            colItr = (*rowItr).end();
            currentCol = rowItr->size();
            (*rowItr).splice((*rowItr).end(), (*next(rowItr)));
            text.erase(next(rowItr));
            advance(colItr, currentCol);
            currentRow--;
        }
    }

    // Removes a character at the right to the cursor position

    void removeChar()
    {
        if (colItr != (*rowItr).end())
        {
            colItr = (*rowItr).erase(colItr);
            currentCol--;
        }
        else if (currentRow != text.size() - 1)
        {
            (*rowItr).pop_back(); // removes last endline character
            auto nextRowItr = next(rowItr);
            (*rowItr).splice((*rowItr).end(), *nextRowItr);
            text.erase(nextRowItr);
        }
    }

    // Insert a new line

    void insertNewLine()
    {
        list<char> row(colItr, (*rowItr).end());
        auto temp = rowItr;
        (*temp).erase(colItr, (*temp).end());
        auto newItr = text.insert(next(rowItr), row);
        rowItr = newItr;
        colItr = (*rowItr).begin();
        currentRow++;
        currentCol = 0;
    }

    void display()
    {
        system("cls");
        int rowDistance = distance(text.begin(), rowItr);
        int colDistance = distance(rowItr->begin(), colItr);
        for (const auto &row : text)
        {
            for (const auto &ch : row)
            {
                cout << ch;
            }
            cout << endl;
        }
        gotoxy(colDistance, rowDistance);
    }

    // Function to display without flickering// Function to display without flickering
    void displayNoFlicker()
    {
        COORD bufferSize = {static_cast<SHORT>(80), static_cast<SHORT>(25)};
        COORD characterPos = {0, 0};
        SMALL_RECT writeArea = {0, 0, static_cast<SHORT>(79), static_cast<SHORT>(24)};

        CHAR_INFO *buffer = new CHAR_INFO[80 * 25];
        for (int i = 0; i < 80 * 25; i++)
        {
            buffer[i].Char.AsciiChar = ' ';
            buffer[i].Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
        }

        for (auto rowItr = text.begin(); rowItr != text.end(); ++rowItr)
        {
            for (auto colItr = rowItr->begin(); colItr != rowItr->end(); ++colItr)
            {
                buffer[characterPos.Y * 80 + characterPos.X].Char.AsciiChar = *colItr;
                buffer[characterPos.Y * 80 + characterPos.X].Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
                characterPos.X++;
            }
            characterPos.X = 0;
            characterPos.Y++;
        }

        WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), buffer, bufferSize, characterPos, &writeArea);
        gotoxy(currentCol, currentRow);

        delete[] buffer;
    }

    void print(char c)
    {
        gotoxy(currentCol, currentRow);
        cout << c;
    }

    // Save State Function

    state saveState()
    {
        state *s = new state;
        s->text.push_back(list<char>());

        auto r_itr = s->text.begin();
        for (auto row = text.begin(); row != text.end(); ++row, ++r_itr)
        {
            s->text.push_back(list<char>());

            auto col = (*row).begin();
            auto &currentRow = *r_itr;

            while (col != (*row).end())
            {
                char ch = *col;
                currentRow.push_back(ch);
                ++col;
            }
        }

        s->rowItr = s->text.begin();
        s->colItr = (*s->rowItr).begin();
        s->currentCol = currentCol;
        s->currentRow = currentRow;

        return *s;
    }

    // Function to load state of the text

    void loadState(state s)
    {
        text = s.text;
        rowItr = text.begin();
        currentCol = s.currentCol;
        currentRow = s.currentRow;

        int i = 0;
        while (i < s.currentRow)
        {
            ++rowItr;
            ++i;
        }

        colItr = (*rowItr).begin();

        i = 0;
        while (i < s.currentCol)
        {
            ++colItr;
            ++i;
        }
    }

    // Undo Function

    void undoFunction()
    {
        if (undo.size() > 0)
        {
            state st = undo.back();
            loadState(st);
            redo.push(undo.back());
            undo.pop_back();
        }
    }

    // Function to redo the undo

    void redoFunction()
    {
        if (redo.size() > 0)
        {
            state st = redo.top();
            loadState(st);
            undo.push_back(redo.top());
            redo.pop();
        }
    }

    // Function to update undos --> giving user the ability to undo his mistake

    void updateUndo()
    {
        if (undo.size() > 5)
            undo.erase(undo.begin());
        state s;
        s = saveState();
        undo.push_back(s);
    }

    // Function to write the text to file

    void writeToFile(ofstream &file)
    {

        for (auto row = text.begin(); row != text.end(); ++row)
        {
            for (auto col = (*row).begin(); col != (*row).end(); ++col)
            {
                file << *col;
            }
            file << endl;
        }
    }

    // Getter & Setter Functions
    list<list<char>> getText()
    {
        return text;
    }
    void setText(list<list<char>> text)
    {
        this->text = text;
    }
    list<list<char>>::iterator getRowIter()
    {
        return rowItr;
    }
    void setRowIter(list<list<char>>::iterator rowItr)
    {
        this->rowItr = rowItr;
    }
    list<char>::iterator getColIter()
    {
        return colItr;
    }
    void setColIter(list<char>::iterator colItr)
    {
        this->colItr = colItr;
    }

    int getCol()
    {
        return currentCol;
    }
    void setCol(int col)
    {
        this->currentCol = col;
    }
    int getRow()
    {
        return currentRow;
    }
    void setRow(int row)
    {
        this->currentRow = row;
    }

    // Function to create a file

    void createFile(const string &filename)
    {
        if (find(FilesRecorder.begin(), FilesRecorder.end(), filename) != FilesRecorder.end())
        {
            cout << setw(92) << filename << " already exists: " << endl;
            return;
        }
        FilesRecorder.push_back(filename);
        ofstream writeFile(filename.c_str());
        system("cls");
        editFile(writeFile);
        system("cls");
        writeFile.close();
        ofstream FilesRecorderWrt("SavedFiles.txt", ios::app);
        FilesRecorderWrt << filename << endl;
        FilesRecorderWrt.close();
    }

    // Function to load filenames into FileRecorder list

    void loadFiles()
    {
        string line;
        ifstream file("SavedFiles.txt");
        FilesRecorder.clear();
        while (getline(file, line))
        {
            FilesRecorder.push_back(line);
        }
        file.close();
    }

    // Function to open a file

    void openFile(const string &filename)
    {
        if (find(FilesRecorder.begin(), FilesRecorder.end(), filename) == FilesRecorder.end())
        {
            cout << setw(92) << filename << " not found" << endl;
            return;
        }
        ifstream readFile(filename.c_str());
        openSavedFile(readFile);
        readFile.close();
        ofstream writeFile(filename.c_str());
        system("cls");
        display();
        editFile(writeFile);
        system("cls");
        writeFile.close();
    }

    // Function to open a saved file

    void openSavedFile(ifstream &savedFile)
    {
        for (auto &row : text)
        {
            row.clear();
        }

        if (!savedFile.is_open())
        {
            cout << setw(92) << "Error: Unable to open the file" << endl;
            return;
        }
        char c;
        int cc = 0, cr = 0;
        while (!savedFile.eof())
        {
            cc = 0;
            savedFile.get(c);
            if (c == '\n')
            {
                text.push_back(list<char>());
                rowItr++;
                cr++;
            }
            else
            {
                (*rowItr).push_back(c);
                cc++;
            }
        }
        rowItr = text.begin();
        colItr = (*rowItr).begin();
        currentCol = 0;
        currentRow = 0;
    }

    void showAllFiles()
    {
        cout << setw(90) << "Saved Files\n";
        for (auto itr : FilesRecorder)
        {
            cout << setw(90) << itr << endl;
        }
    }

private:
    bool IsKeyDown(int key)
    {
        return GetAsyncKeyState(key) & 0x8000;
    }
    bool IsCtrlZPressed()
    {
        return IsKeyDown(VK_CONTROL) && IsKeyDown('Z');
    }
    bool IsCtrlYPressed()
    {
        return IsKeyDown(VK_CONTROL) && IsKeyDown('Y');
    }

    // Function to check special characters
    bool isSpecialCharacter(char ch)
    {
        const char specialChars[] = "`~!@#$%^&*()_+=-|}{\":?<>[]\\';/.,\"";

        for (char specialChar : specialChars)
        {
            if (ch == specialChar)
            {
                return true;
            }
        }
        return false;
    }
    // Function to check whether the input is valid

    bool isValidInput(char c)
    {
        return isalpha(c) || isdigit(c) || islower(c) || isupper(c) || isspace(c) || isSpecialCharacter(c);
    }

    // Function to edit the file
    void editFile(ofstream &file)
    {
        char c;
        // system("cls");
        while (true)
        {
            c = _getch();

            if (IsKeyDown(VK_UP))
            {
                moveUp();
            }
            else if (IsKeyDown(VK_DOWN))
            {
                moveDown();
            }
            else if (IsKeyDown(VK_LEFT))
            {
                moveLeft();
            }
            else if (IsKeyDown(VK_RIGHT))
            {
                moveRight();
            }
            else if (IsKeyDown(VK_BACK))
            {
                deleteChar();
                updateUndo();
            }
            else if (IsKeyDown(VK_DELETE))
            {
                removeChar();
                updateUndo();
            }
            else if (IsKeyDown(VK_RETURN))
            {
                insertNewLine();
                updateUndo();
            }
            else if (IsCtrlZPressed())
            {
                undoFunction();
                display();
            }
            else if (IsCtrlYPressed())
            {
                redoFunction();
                display();
            }
            else if (IsKeyDown(VK_ESCAPE))
            {
                writeToFile(file);
                break;
            }
            else
            {
                if (isValidInput(c))
                {
                    insertChar(c);
                }
                updateUndo();
            }
            display();
        }
    }
};

int choiceMenu()
{
    int choice;
    cout << setw(85) << "Menu" << endl;
    cout << setw(95) << "_____________________" << endl;
    cout << setw(92) << "1. Create File" << endl;
    cout << setw(90) << "2. Open File" << endl;
    cout << setw(95) << "3. Show All Files" << endl;
    cout << setw(85) << "4. Exit" << endl;
    cout << setw(92) << "Your Choice: ";
    cin >> choice;
    return choice;
}

void subMenu(string optName)
{
    cout << setw(82) << "Menu > " << optName << endl;
    cout << setw(100) << "____________________________" << endl;
}

void header()
{
    cout << endl;
    cout << endl;
    cout << "                                                               /$$   /$$             /$$               /$$   /$$\n";
    cout << "                                                               | $$$ | $$            | $$              | $$  / $$\n";
    cout << "                                                               | $$$$| $$  /$$$$$$  /$$$$$$    /$$$$$$ |  $$/ $$/\n";
    cout << "                                                               | $$ $$ $$ /$$__  $$|_  $$_/   /$$__  $$ \\  $$$$/ \n";
    cout << "                                                               | $$  $$$$| $$  \\ $$  | $$    | $$$$$$$$  >$$  $$ \n";
    cout << "                                                               | $$\\  $$$| $$  | $$  | $$ /$$| $$_____/ /$$/\\  $$\n";
    cout << "                                                               | $$ \\  $$|  $$$$$$/  |  $$$$/|  $$$$$$$| $$  \\ $$\n";
    cout << "                                                               |__/  \\__/ \\______/    \\___/   \\_______/|__/  |__/\n";
    cout << endl;
    cout << endl;
    system("color f0");

    // std::cout << resetColors; // Reset colors to default
}
void clearScreen()
{
    system("cls");
    header();
}
int main()
{

    int r = 0, c = 0;
    Editor fileEditor;
    fileEditor.loadFiles();
    ifstream readFile;
    ofstream writeFile;
    int opt = 0;
    fileEditor.setRow(r);
    fileEditor.setCol(c);
    system("cls");
    header();
    while (opt != 4)
    {
        opt = choiceMenu();
        if (opt == 1)
        {
            clearScreen();
            subMenu("Create File");
            string filename = "";
            cout << setw(92) << "Enter filename: ";
            cin.ignore();
            getline(cin, filename);
            fileEditor.createFile(filename + ".txt");
        }
        else if (opt == 2)
        {
            clearScreen();
            subMenu("Open File");
            string filename = "";
            fileEditor.showAllFiles();
            cout << setw(92) << "Enter filename: ";
            cin.ignore();
            getline(cin, filename);
            fileEditor.openFile(filename + ".txt");
        }
        else if (opt == 3)
        {
            clearScreen();
            subMenu("Show All Files");
            fileEditor.showAllFiles();
        }
        cout << "Press any key to continue...";
        getch();
        clearScreen();
    }

    return 0;
}