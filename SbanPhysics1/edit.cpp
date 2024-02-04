#define NOMINMAX
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <Windows.h>
#include <wchar.h>
#include <regex>

// 左右の文字を読み込む関数
std::string readCharacter(const std::string& filename) {
    std::ifstream file(filename);
    std::string character;
    if (file.is_open()) {
        std::getline(file, character); // 1行だけを読み込む
    } else {
        std::cerr << "ファイルを開けませんでした: " << filename << std::endl;
        exit(EXIT_FAILURE); // ファイルが開けない場合はプログラムを終了
    }
    return character;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 作業ディレクトリの取得
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        std::wcerr << L"Error getting module file name." << std::endl;
        return 1;
    }

    // exePathからディレクトリパスを抽出
    std::wstring dirPath = exePath;
    dirPath = dirPath.substr(0, dirPath.find_last_of(L"\\/"));

    // _wchdirを使用してカレントディレクトリを変更
    _wchdir(dirPath.c_str());

    // ファイルパス
    std::string original_path = "Main.cpp";
    std::string temp_path = "Main_temp.cpp";
    std::string lyrics_path = "lyrics.txt"; // 歌詞ファイルのパス

    // ユーザー入力の受け取り
    double scale_value, speed_value,cangle_value,cspeed_value;
    int font_size;
    std::string font_path;

    std::cout << u8"ひらがなを1とした場合の漢字の比率(標準値:1.25): ";
    std::cin >> scale_value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << u8"フォントサイズを入力してください(標準値:40): ";
    std::cin >> font_size;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << u8"新しいフォントパスを入力してください(区切りは/,ダブルクオーテーションは自動で付加されるため不要): ";
    std::getline(std::cin, font_path);

    std::cout << u8"移動速度を入力してください(標準値:1.5): ";
    std::cin >> speed_value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << u8"左右から出てくる文字の角度を入力してください(標準値:0):";
    std::cin >> cangle_value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << u8"左右から出てくる文字のスピードを入力してください(標準値:0.02):";
    std::cin >> cspeed_value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 左右の文字をファイルから読み込む
    std::string left_character = readCharacter("left.txt");
    std::string right_character = readCharacter("right.txt");


    // 歌詞の新しい内容を保持するためのベクター
    std::vector<std::string> new_lyrics;
    std::ifstream lyrics_file(lyrics_path);
    if (!lyrics_file.is_open()) {
        std::cerr << u8"歌詞ファイルを開けませんでした。" << std::endl;
        return 1;
    }

    // 歌詞ファイルから行を読み込む
    std::string lyric_line;
    while (std::getline(lyrics_file, lyric_line)) {
        new_lyrics.push_back("\t\tU\"" + lyric_line + "\",");
    }
    lyrics_file.close();

    // メインファイルを開く
    std::ifstream file(original_path);
    if (!file.is_open()) {
        std::cerr << u8"ファイルを開けませんでした。" << std::endl;
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    bool is_inside_lyrics_block = false;
    while (std::getline(file, line)) {
        if (line.find("const double scale =") != std::string::npos) {
            lines.push_back("\t\t\tconst double scale = " + std::to_string(scale_value) + ";");
            continue;
        }
        if (line.find("const int fontSize =") != std::string::npos) {
            lines.push_back("\tconst int fontSize = " + std::to_string(font_size) + ";");
            continue;
        }
        if (line.find("const char32_t* fontPath =") != std::string::npos) {
            lines.push_back("\tconst char32_t* fontPath = U\"" + font_path + "\";");
            continue;
        }
        if (line.find("constexpr double speed =") != std::string::npos) {
            lines.push_back("\tconstexpr double speed = " + std::to_string(speed_value) + ";");
            continue;
        }
        if (line.find("double collisionAngle = ") != std::string::npos) {
            lines.push_back("\tdouble collisionAngle = " + std::to_string(cangle_value) + ";");
            continue;
        }
        if (line.find("double collisionSpeed = ") != std::string::npos) {
            lines.push_back("\tdouble collisionSpeed = " + std::to_string(cspeed_value) + ";");
            continue;
        }
        if (line.find("double collisionSpeed = ") != std::string::npos) {
            lines.push_back("\tdouble collisionSpeed = " + std::to_string(cspeed_value) + ";");
            continue;
        }
        std::regex pattern1(R"(font\(U"\."\).drawAt\(dotPos, Palette::White\);)");
        std::string replacement1 = "\t\tfont(U\"" + left_character + "\").drawAt(dotPos, Palette::White);";
        line = std::regex_replace(line, pattern1, replacement1);

        std::regex pattern2(R"(font\(U"\."\).drawAt\(dotPos2, Palette::White\);)");
        std::string replacement2 = "\t\tfont(U\"" + right_character + "\").drawAt(dotPos2, Palette::White);";
        line = std::regex_replace(line, pattern2, replacement2);
        
        // 既存の歌詞ブロックの処理
        if (line.find("const Array<String> texts =") != std::string::npos) {
            is_inside_lyrics_block = true;
            lines.push_back(line);
            lines.push_back("\t{");
            continue;
        }
        if (is_inside_lyrics_block && line.find("};") != std::string::npos) {
            for (const auto& lyric : new_lyrics) {
                lines.push_back(lyric);
            }
            lines.push_back("\t};");
            is_inside_lyrics_block = false;
            continue;
        }
        if (!is_inside_lyrics_block) {
            lines.push_back(line);
        }
    }
    file.close();

    // 一時ファイルに新しい内容を書き込む
    std::ofstream outfile(temp_path, std::ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << u8"一時ファイルを開けませんでした。" << std::endl;
        return 1;
    }
    for (const auto& edited_line : lines) {
        outfile << edited_line << '\n';
    }
    outfile.close();

    // 古いファイルを削除し、新しいファイル名に変更
    if (std::remove(original_path.c_str()) != 0) {
        std::perror("Error deleting original file");
        return 1;
    }
    if (std::rename(temp_path.c_str(), original_path.c_str()) != 0) {
        std::perror("Error renaming temporary file");
        return 1;
    }

    std::cout << u8"ファイルが編集されました。" << std::endl;
    std::cout << u8"比率が " << scale_value << " に、フォントサイズが " << font_size << " に、移動速度が " << speed_value << " に、フォントパスが " << font_path << " に、左右から出てくる文字の角度が " << cangle_value << " に、左右から出てくる文字のスピードが " << cspeed_value << " に、左から出てくる文字が " << left_character << " に、右から出てくる文字が " << right_character << " に更新されました。" << std::endl;

    std::cout << "終了するにはEnterキーを入力..." << std::endl;
    std::cin.get();

    return 0;
}
