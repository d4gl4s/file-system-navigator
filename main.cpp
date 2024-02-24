#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

class Path {
private:
    std::vector<std::string> directories;

public:
    explicit Path(const std::string& filePath) {
        std::stringstream ss(filePath);
        std::string directory;
        while (std::getline(ss, directory, '\\')) {
            if (!directory.empty()) directories.push_back(directory);
        }
    }

    // Method to get the current absolute path
    [[nodiscard]] std::string pwd() const {
        std::string absolutePath;
        for (const std::string& dir : directories) {
            absolutePath += dir + "\\";
        }
        return absolutePath;
    }

    // Method to change directory (cd)
    void cd(int steps) {
        if (steps > 0 && directories.size() >= steps) {
            directories.erase(directories.begin(), directories.begin() + steps);
        } else if (steps < 0 && directories.size() >= -steps) {
            directories.insert(directories.begin(), -steps, "");
        } else {
            std::cout << "Invalid step count.\n";
        }
    }

    // Method to convert Path to string
    [[nodiscard]] std::string toString() const {
        std::string result;
        for (const std::string& dir : directories) {
            if (!result.empty()) {
                result += "->";
            }
            result += dir;
        }
        return result;
    }
};

void processCommand(const std::string& command, const Path& path);
std::string trim(const std::string& str);

int main() {
    std::string command;
    std::string curDirectory = std::filesystem::current_path().string();
    Path path(curDirectory);
    while (true) {
        std::cout << "Enter a command (pwd, ls, cd .., cd [path], mkdir [name], rm [path], exit): ";
        std::getline(std::cin, command);
        if (trim(command) == "exit") break;
        processCommand(command, path);
    }
    return 0;
}

void processCommand(const std::string& commandString, const Path& path) {
    std::vector<std::string> args;
    std::string command = trim(commandString);

    if (command == "pwd" || command == "ls") {
        if (command == "pwd") std::cout << path.pwd();
        //else ls();
    } else if (command == "cd" || command == "mkdir" || command == "rm") {
        // Split command and arguments
        size_t pos = command.find(' ');
        std::string cmd = command.substr(0, pos);
        std::string arg = command.substr(pos + 1);
        if(arg.empty()){
            std::cout << "Missing argument for command: " << cmd << std::endl;
            return;
        }
        // Call function with arguments
        //if (cmd == "cd")
        //    cd(arg);
        //else if (cmd == "mkdir")
        //    mkdir(arg);
        //else if (cmd == "rm")
        //    rm(arg);
    } else std::cout << "Invalid command. Please try again.\n";
}

// Trim leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
        return str;
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}