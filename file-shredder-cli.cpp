/*
 * This file is part of the project "file-shredder-cli".
 * Copyright (C) 2024, mxrcode (https://github.com/mxrcode/)
 *
 * This code is licensed under the GNU Lesser General Public License (LGPL) v3.0.
 * You may obtain a copy of the License at:
 * https://www.gnu.org/licenses/lgpl-3.0.html
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */

#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>

#include <codecvt>
#include <locale>

 // Windows-specific headers for setting UTF-8 encoding
#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

#pragma warning(disable : 4996) // Disable warning for std::codecvt_utf8_utf16

constexpr size_t BUFFER_SIZE = 4096; // 4 KB

std::string filepath_to_utf8(const std::filesystem::path& filepath) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(filepath.wstring());
}

void zeroFill(const std::filesystem::path& filepath) {
    try {
        std::fstream file(filepath, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << filepath_to_utf8(filepath) << "\n";
            return;
        }

        auto filesize = std::filesystem::file_size(filepath);
        std::vector<char> buffer(BUFFER_SIZE, 0);

        for (std::uintmax_t i = 0; i < filesize; i += BUFFER_SIZE) {
            std::uintmax_t bytes_to_write = (((BUFFER_SIZE) < (filesize - i)) ? (BUFFER_SIZE) : (filesize - i)); // std::min(BUFFER_SIZE, filesize - i);
            file.write(buffer.data(), bytes_to_write);
            if (!file) {
                std::cerr << "Failed to write to file: " << filepath_to_utf8(filepath) << "\n";
                return;
            }
        }

        file.close();
        std::cout << "The file has been successfully filled with zeros: " << filepath_to_utf8(filepath) << "\n";
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch (const std::ios_base::failure& e) {
        std::cerr << "I/O error: " << e.what() << "\n";
    }
}

bool getUserChoice(const std::string& message) {
    char choice;
    while (true) {
        std::cout << message;
        std::cin >> choice;
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        if (choice == 'y' || choice == 'Y' || choice == 'n' || choice == 'N') {
            break;
        }
        std::cout << "Invalid choice. Please enter 'y' or 'n'.\n";
    }

    if (choice == 'y' || choice == 'Y') {
        return true;
    }

    return false;
}

bool confirmDestroy() {
    return getUserChoice("Do you want to fill this file with zeros? (Y/n): ");
}

bool confirmDelete() {
    return getUserChoice("Do you want to delete this file after filling it with zeros? (Y/n): ");
}

void waitEnter() {
    std::cout << "Press Enter to exit...";
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

void outputHelp() {
    std::cout << "Usage: file-shredder-cli <file1> <file2> <file3> ...\n";
    std::cout << "OR\n";
    std::cout << "Drag and drop the file(s) onto the program icon to overwrite them with zeros.\n";
    std::cout << "\n";
    std::cout << "Version: 1.0.0\n";
    std::cout << "Author: mxrcode (https://github.com/mxrcode/)\n";
    std::cout << "\n";
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");
#endif

    if (argc < 2) {
        outputHelp();
        waitEnter();
        return 1;
    }

	if (argc == 2) {
		std::string arg = filepath_to_utf8(argv[1]);
		if (arg == "-h" || arg == "--h" || arg == "--help") {
			outputHelp();
			return 0;
		}
	}

    for (int i = 1; i < argc; ++i) {
        const std::filesystem::path filepath = argv[i];

        std::cout << "Processing file " << (i) << " of " << (argc - 1) << ": " << filepath_to_utf8(filepath) << "\n";

        if (!std::filesystem::exists(filepath)) {
            std::cerr << "File not found: " << filepath << "\n";
            continue; // Skip to the next file
        }

        if (std::filesystem::is_directory(filepath)) {
            std::cout << "Skipping directory: " << filepath << "\n";
            continue; // Skip directories
        }

        if (confirmDestroy()) {
            zeroFill(filepath);

            if (confirmDelete()) {
                try {
                    if (std::filesystem::remove(filepath)) {
                        std::cout << "File successfully deleted.\n";
                    }
                    else {
                        std::cerr << "Failed to delete the file: " << filepath << "\n";
                    }
                }
                catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << "Error deleting file: " << e.what() << "\n";
                }
            }
        }
        else {
            std::cout << "The file has not been destroyed: " << filepath << "\n";
        }
    }

    waitEnter();
    return 0;
}