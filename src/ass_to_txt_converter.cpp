#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <functional>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#define IDC_EDIT 1001

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ProcessASSFile(const std::wstring& filePath, HWND hwndEdit);
std::string ConvertToUTF8(const std::wstring& wstr);
std::wstring ConvertToWideString(const std::string& str);
bool ContainsChinese(const std::string& text);
bool ContainsEnglish(const std::string& text);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    const wchar_t CLASS_NAME[] = L"ASSConverterWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"ASS to TXT Converter",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    DragAcceptFiles(hwnd, TRUE);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndEdit;

    switch (msg) {
    case WM_CREATE: {
        hwndEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 760, 540,
            hwnd, (HMENU)IDC_EDIT, GetModuleHandle(NULL), NULL
        );
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Courier New");
        SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SetWindowText(hwndEdit, L"Drag and drop an ASS file here...");
        break;
    }

    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        wchar_t filePath[MAX_PATH];
        DragQueryFile(hDrop, 0, filePath, MAX_PATH);
        DragFinish(hDrop);
        ProcessASSFile(filePath, hwndEdit);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void ProcessASSFile(const std::wstring& filePath, HWND hwndEdit) {
    // Open the file with wide-character stream to handle UTF-8
    std::wifstream inFile(filePath);
    if (!inFile.is_open()) {
        MessageBox(NULL, L"Failed to open the ASS file!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Set locale to support UTF-8
    inFile.imbue(std::locale("en_US.UTF-8"));

    // Read the entire file
    std::wstringstream buffer;
    buffer << inFile.rdbuf();
    std::wstring wcontent = buffer.str();
    inFile.close();

    // Convert wide string to UTF-8 string for processing
    std::string content = ConvertToUTF8(wcontent);

    // Handle UTF-8 BOM if present
    if (content.size() >= 3 && content[0] == (char)0xEF && content[1] == (char)0xBB && content[2] == (char)0xBF) {
        content = content.substr(3); // Skip BOM
    }

    std::vector<std::string> outputLines;
    std::string lastTimestamp;
    std::regex dialogueRegex(R"(Dialogue:[^,]+,(\d:\d{2}:\d{2}\.\d{2}),[^,]+,[^,]+,[^,]+,\d+,\d+,\d+,,(.+?)(?=\nDialogue:|\n\[|$))");
    std::smatch match;
    // Convert timestamp to seconds for duration comparison
    std::function<size_t(const std::string&)> toSeconds = [](const std::string& ts) -> size_t {
        size_t hours, minutes;
        size_t seconds;
        sscanf_s(ts.c_str(), "%zd:%zd:%zd.%*s", &hours, &minutes, &seconds);
        return hours * 3600 + minutes * 60 + seconds;
    };

    std::string::const_iterator searchStart(content.cbegin());

    while (std::regex_search(searchStart, content.cend(), match, dialogueRegex)) {
        std::string timestamp = match[1].str();
        std::string text = match[2].str();

        // Remove ASS tags
        text = std::regex_replace(text, std::regex(R"(\{.*?\})"), "");
        // Split by \N
        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            line = std::regex_replace(line, std::regex(R"(\\N)"), "");
            if (!line.empty()) {
                lines.push_back(line);
            }
        }

        // Check for bilingual (Chinese and English) content
        bool hasChinese = false, hasEnglish = false;
        for (const auto& l : lines) {
            if (ContainsChinese(l)) hasChinese = true;
            if (ContainsEnglish(l)) hasEnglish = true;
        }

        if (hasChinese && hasEnglish) {
            if (lastTimestamp != timestamp) {
                size_t currentTime = toSeconds(timestamp);
                static size_t lastTime = 0;
				if (lastTimestamp.empty() || (currentTime - lastTime) > 5) {
                    outputLines.push_back(timestamp);
                }
                lastTimestamp = timestamp;
                lastTime = currentTime;
            }
            for (const auto& l : lines) {
				size_t firstEnglish = l.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
                if (firstEnglish != std::string::npos) {
					outputLines.push_back(l.substr(firstEnglish)); // english part
					outputLines.push_back(l.substr(0, firstEnglish)); // chinese part
                }
            }
        }

        searchStart = match.suffix().first;
    }

    // Save to TXT file
    std::wstring txtPath = filePath.substr(0, filePath.find_last_of(L".")) + L".txt";
    std::ofstream outFile(txtPath, std::ios::binary);
    outFile << "\xEF\xBB\xBF"; // UTF-8 BOM
    for (const auto& line : outputLines) {
        outFile << line << "\n";
    }
    outFile.close();

    // Display in edit control
    std::wstring displayText;
    for (const auto& line : outputLines) {
        displayText += ConvertToWideString(line) + L"\r\n";
    }
    SetWindowText(hwndEdit, displayText.c_str());

    MessageBox(NULL, L"Conversion completed! Output saved to .txt file.", L"Success", MB_OK | MB_ICONINFORMATION);
}

std::string ConvertToUTF8(const std::wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

std::wstring ConvertToWideString(const std::string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

bool ContainsChinese(const std::string& text) {
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] & 0x80) { // Check for multi-byte characters
            return true; // Simplified check for Chinese (Unicode CJK range)
        }
    }
    return false;
}

bool ContainsEnglish(const std::string& text) {
    return std::regex_search(text, std::regex("[a-zA-Z]"));
}