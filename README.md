# ass_to_txt_converter

A lightweight Windows GUI application that converts ASS subtitle files into plain TXT format, extracting only bilingual (Chinese and English) subtitles with timestamps. Built with C++ and the Windows API, it supports drag-and-drop functionality for ease of use.

## Features

- **Drag-and-Drop Interface**: Simply drag an ASS subtitle file into the window to process it.
- **Bilingual Subtitle Extraction**: Extracts only subtitles containing both Chinese and English text, ignoring others.
- **Timestamp Grouping**: Groups subtitles within a 5-second window under a single timestamp for cohesive dialogue segments.
- **Formatted Output**: Outputs subtitles in a clean TXT file with English subtitles first, followed by Chinese, separated by newlines.
- **UTF-8 Support**: Handles UTF-8 encoded ASS files correctly, ensuring proper display of Chinese characters.

## Installation

1. **Download the Executable**:

   - Visit the Releases page.
   - Download the latest `ASSConverter.exe` from the assets.

2. **Run the Application**:

   - No installation is required. Double-click `ASSConverter.exe` to launch the GUI on Windows.

## Usage

1. Launch `ASSConverter.exe`.
2. Drag an ASS subtitle file (e.g., `subtitles.ass`) into the application window.
3. The program processes the file and displays the extracted subtitles in the GUI.
4. A TXT file (e.g., `subtitles.txt`) is generated in the same directory as the input file, containing the formatted bilingual subtitles with timestamps.

**Example Output**:

```
0:02:56
Yeah, that is some detail.
这细节真不错
0:03:02
Look, he's even got the spurs on the back of his boots.
看啊 他靴子背后竟然还有马刺
```

## Building from Source

### Prerequisites

- CMake 3.16 or higher
- A C++ compiler (e.g., MSVC on Windows)
- Git

### Steps

1. Clone the repository:

   ```bash
   git clone https://github.com/your-username/ASSConverter.git
   cd ASSConverter
   ```

2. Create a build directory and configure CMake:

   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

3. Build the project:

   ```bash
   cmake --build . --config Release
   ```

4. Find the executable in `build/bin/ASSConverter.exe`.

## GitHub Actions

This project uses GitHub Actions to automatically build and release the executable. Pushing a tag (e.g., `v1.0.0`) triggers the workflow to:

- Build `ASSConverter.exe` on Windows using CMake.
- Create a GitHub Release with the executable as an asset.

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/your-feature`).
3. Commit your changes (`git commit -m "Add your feature"`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Open a Pull Request.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- Built with the Windows API for native performance.
- Uses CMake for cross-platform build configuration.

---

Replace `your-username` with your GitHub username in the links above.