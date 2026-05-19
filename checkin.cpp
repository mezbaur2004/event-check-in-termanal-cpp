#define UNICODE
#define _UNICODE

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

// ─── Configuration ────────────────────────────────────────────────────────────
// Change this to match the exact string your QR code encodes.
// Must be identical on every gate laptop.
const std::string MASTER_PASSWORD = "WHEATON INTERNATIONAL SCHOOL -EVENT";

// File that persists the count across crashes / reboots
const std::string COUNT_FILE = "count.txt";

// ─── Console Handle (global, set once in main) ────────────────────────────────
HANDLE hConsole = nullptr;

// ─── Color Codes (Windows console attribute bytes) ───────────────────────────
// Format: 0xBG where B=background nibble, G=foreground nibble
// Reference nibbles: 0=Black 1=DarkBlue 2=DarkGreen 3=Teal
//                    4=DarkRed 5=Purple 6=Olive 7=LightGray
//                    8=DarkGray 9=Blue A=Green B=Cyan
//                    C=Red D=Magenta E=Yellow F=White
const WORD COLOR_IDLE    = 0x07; // White text on Black   (default terminal)
const WORD COLOR_SUCCESS = 0xA0; // Black text on Green
const WORD COLOR_FAILURE = 0x40; // Black text on Red
const WORD COLOR_HEADER  = 0x0B; // Cyan text  on Black

// ─── Utility: Set console color ───────────────────────────────────────────────
void setColor(WORD attr) {
    SetConsoleTextAttribute(hConsole, attr);
}

// ─── Utility: Fill entire console background with a color ────────────────────
// SetConsoleTextAttribute only affects NEW characters written — existing text
// keeps its old color. To flash the whole screen we must repaint every cell.
void fillConsoleBackground(WORD attr) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    COORD topLeft = {0, 0};
    DWORD written;
    DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill all cells with spaces under the new color
    FillConsoleOutputAttribute(hConsole, attr, consoleSize, topLeft, &written);
    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, topLeft, &written);

    // Move cursor to top-left so subsequent text uses the new color
    SetConsoleCursorPosition(hConsole, topLeft);
    setColor(attr);
}

// ─── Utility: Move cursor to (x, y) ──────────────────────────────────────────
void gotoxy(int x, int y) {
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, pos);
}

// ─── Utility: Print a horizontally centered string ───────────────────────────
void printCentered(const std::string& text, int consoleWidth, WORD color) {
    setColor(color);
    int padding = (consoleWidth - (int)text.size()) / 2;
    if (padding < 0) padding = 0;
    std::cout << std::string(padding, ' ') << text << "\n";
}

// ─── Utility: Print a horizontal divider ─────────────────────────────────────
void printDivider(int width, char ch, WORD color) {
    setColor(color);
    std::cout << std::string(width, ch) << "\n";
}

// ─── Count Persistence ────────────────────────────────────────────────────────

// Load count from file on startup (survives crash/reboot)
int loadCount() {
    std::ifstream f(COUNT_FILE);
    if (!f.is_open()) return 0;
    int n = 0;
    f >> n;
    return (n > 0) ? n : 0;
}

// Persist count after every successful scan — flush guarantees the write
// hits disk before we return, so a power-cut mid-scan is safe.
void saveCount(int count) {
    std::ofstream f(COUNT_FILE, std::ios::trunc);
    if (f.is_open()) {
        f << count;
        f.flush();
    }
}

// ─── UI: Draw the idle waiting screen ────────────────────────────────────────
void drawIdleScreen(int count, int consoleWidth) {
    fillConsoleBackground(COLOR_IDLE);
    setColor(COLOR_IDLE);

    printDivider(consoleWidth, '=', COLOR_HEADER);
    printCentered("SECURE STATIC TICKET SCANNER", consoleWidth, COLOR_HEADER);
    printCentered("Gate Terminal  |  10,000 Attendees", consoleWidth, 0x08);
    printDivider(consoleWidth, '=', COLOR_HEADER);

    std::cout << "\n\n";

    // Big count display
    printCentered("TOTAL ADMITTED", consoleWidth, 0x07);
    std::cout << "\n";

    // Format count with leading zeros (visual consistency)
    std::ostringstream oss;
    oss << std::setw(5) << std::setfill('0') << count;
    std::string countStr = "[  " + oss.str() + "  ]";

    setColor(0x0E); // Yellow on Black
    int padding = (consoleWidth - (int)countStr.size()) / 2;
    std::cout << std::string(padding, ' ');
    // Print each char slightly "larger" via spaces
    for (char c : countStr) {
        std::cout << c << ' ';
    }
    std::cout << "\n\n\n";

    printDivider(consoleWidth, '-', 0x08);
    printCentered("SCAN BADGE TO ENTER", consoleWidth, 0x07);
    printCentered("Waiting for scanner...", consoleWidth, 0x08);
    printDivider(consoleWidth, '-', 0x08);
    std::cout << "\n";
    setColor(0x07);
}

// ─── UI: Draw the SUCCESS screen ─────────────────────────────────────────────
void drawSuccessScreen(int count, int consoleWidth) {
    fillConsoleBackground(COLOR_SUCCESS);

    std::cout << "\n\n\n\n";
    printDivider(consoleWidth, '#', 0xA0);
    printCentered("", consoleWidth, 0xA0);
printCentered("       .-''''''-.       ", consoleWidth, 0xA0);
printCentered("     .'  _    _  '.     ", consoleWidth, 0xA0);
printCentered("    /   (o)  (o)   \\    ", consoleWidth, 0xA0);
printCentered("   |                |   ", consoleWidth, 0xA0);
printCentered("   |    \\______/    |   ", consoleWidth, 0xA0);
printCentered("    \\              /    ", consoleWidth, 0xA0);
printCentered("     '.          .'     ", consoleWidth, 0xA0);
printCentered("       '-......-'       ", consoleWidth, 0xA0);
    printCentered("", consoleWidth, 0xA0);
    printDivider(consoleWidth, ' ', 0xA0);
    printCentered("A C C E S S   G R A N T E D", consoleWidth, 0xA0);
    printDivider(consoleWidth, ' ', 0xA0);

    // Show count
    std::ostringstream oss;
    oss << "Attendee #" << std::setw(5) << std::setfill('0') << count
        << "  admitted";
    printCentered(oss.str(), consoleWidth, 0xA0);

    printCentered("", consoleWidth, 0xA0);
    printDivider(consoleWidth, '#', 0xA0);
}

// ─── UI: Draw the FAILURE screen ─────────────────────────────────────────────
void drawFailureScreen(int consoleWidth) {
    fillConsoleBackground(COLOR_FAILURE);

    std::cout << "\n\n\n\n\n\n";
    printDivider(consoleWidth, '!', 0x40);
    printCentered("", consoleWidth, 0x40);
    printCentered("R  E  J  E  C  T  E  D", consoleWidth, 0x47);
    printCentered("", consoleWidth, 0x40);
    printCentered("INVALID CREDENTIAL - DO NOT ADMIT", consoleWidth, 0x40);
    printCentered("", consoleWidth, 0x40);
    printDivider(consoleWidth, '!', 0x40);
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    // ── Get console handle ────────────────────────────────────────────────
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // ── Maximise the console window on startup ────────────────────────────
    //ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    // Give Windows a moment to apply the resize before we query dimensions
    Sleep(150);

    // ── Set console title ─────────────────────────────────────────────────
    SetConsoleTitleA("GATE CHECK-IN TERMINAL");

    // ── Hide the cursor (cleaner look during flashes) ─────────────────────
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(hConsole, &cci);
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cci);

    // ── Query console width for centering ────────────────────────────────
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int consoleWidth = csbi.dwSize.X;
    if (consoleWidth < 40) consoleWidth = 80; // Sensible fallback

    // ── Load persisted count from disk ───────────────────────────────────
    int totalCount = loadCount();

    // ── Disable quick-edit mode ───────────────────────────────────────────
    // QuickEdit lets the user "pause" the app by clicking the console.
    // That would block the scanner — we must disable it.
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    GetConsoleMode(hIn, &prevMode);
    SetConsoleMode(hIn, prevMode & ~ENABLE_QUICK_EDIT_MODE);

    // ── Main scan loop ────────────────────────────────────────────────────
    std::string scannedInput;

    while (true) {
        // Draw idle screen and wait for scanner input
        drawIdleScreen(totalCount, consoleWidth);

        // std::getline blocks here until the scanner fires Enter.
        // The scanner types the barcode string and appends \r\n.
        // getline strips the \n; we strip any trailing \r below.
        if (!std::getline(std::cin, scannedInput)) {
            // EOF or stream error — attempt to recover
            std::cin.clear();
            Sleep(100);
            continue;
        }

        // Strip trailing carriage return (\r) — some scanners append it
        if (!scannedInput.empty() && scannedInput.back() == '\r') {
            scannedInput.pop_back();
        }

        // Skip empty inputs (stray Enter presses, etc.)
        if (scannedInput.empty()) continue;

        // ── Validate ──────────────────────────────────────────────────────
        if (scannedInput == MASTER_PASSWORD) {

            // Increment and persist FIRST — before any visual/audio work.
            // If the app crashes during the beep, the count is already saved.
            totalCount++;
            saveCount(totalCount);

            // Visual flash
            drawSuccessScreen(totalCount, consoleWidth);

            // Audio: two rising beeps (feels like a confirmation chime)
//            Beep(880,  120);
            Beep(300, 300);

            // Hold the green screen long enough for staff to see it
            Sleep(100);

        } else {

            // Visual flash
            drawFailureScreen(consoleWidth);

            // Audio: low buzzer
            Beep(300, 300);

            // Hold the red screen
            Sleep(100);
        }

        // ── Re-draw idle screen before next getline ───────────────────────
        // (drawIdleScreen is called at top of loop — nothing to do here)
    }

    // Restore console mode on exit (never reached in normal operation,
    // but good practice for when Ctrl+C is pressed)
    SetConsoleMode(hIn, prevMode);
    return 0;
}
