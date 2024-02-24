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

      // Method to change directory (cd)
    void cd(const std::string& path) {
        namespace fs = std::filesystem;

        if (path == ".." && !directories.empty()) directories.pop_back(); // Move up one directory
        else {
            std::string newPath = pwd() + "\\" + path;
            if (fs::is_directory(newPath))
                directories.push_back(path); // Move to the specified directory
            else
                std::cout << "Directory '" << path << "' not found.\n";
        }
    }


// Method to move a file
    void mv(const std::string& sourceFileName, const std::string& destinationDirectory) {
        namespace fs = std::filesystem;
        std::string sourceFilePath = pwd() + "\\" + sourceFileName;

        if (destinationDirectory == "..") {
            // Construct the path to the parent directory
            std::filesystem::path parentPath = std::filesystem::path(pwd()).parent_path().parent_path(); // for some reason have to call parent_path() twice to get the parent path;
            std::string parentDirectoryPath = parentPath.string();
            if(parentPath == parentPath.root_path()) return;
            std::string destinationPath = parentDirectoryPath + "\\" + sourceFileName;
            fs::rename(sourceFilePath, destinationPath);
            return;
        }
        std::string destinationDirectoryPath = pwd() + "\\" + destinationDirectory;
        if(!fs::exists(sourceFilePath)){
            std::cout << "File '" << sourceFileName << "' not found. \n";
            return;
        }
        if (fs::is_directory(destinationDirectoryPath))
            fs::rename(sourceFilePath, destinationDirectoryPath + "\\" + sourceFileName);
        else
            std::cout << "Destination directory '" << destinationDirectory << "' not found.\n";
    }

    // Method to remove a file or directory
    void rm(const std::string& fileName) {
        namespace fs = std::filesystem;
        std::string filePath = pwd() + "\\" + fileName;

        if(!fs::exists(filePath)){
            std::cout << "File or directory '" <<fileName<<"' not found.\n";
            return;
        }
        if(!fs::is_directory(filePath) || fs::is_directory(filePath) && fs::is_empty(filePath)){
            fs::remove(filePath);
            return;
        }

        // If it's a directory and contains objects, ask for confirmation
        std::string response;
        std::cout << "Directory '" << fileName << "' is not empty. Are you sure you want to delete it? (yes/no): ";
        std::cin >> response;
        if (response != "yes") return;
        fs::remove_all(filePath);
    }

    // Method to create a new directory
    void mkdir(const std::string& dirName) {
        namespace fs = std::filesystem;
        fs::create_directory(pwd() + "\\" + dirName);
    }

    // Method to get the current absolute path
    [[nodiscard]] std::string pwd() const {
        std::string absolutePath;
        for (const std::string& dir : directories) {
            absolutePath += dir + "\\";
        }
        return absolutePath;
    }

    // Method to list directory contents (ls)
    void ls() const {
        namespace fs = std::filesystem;
        std::cout << std::left << std::setw(5) << "Type";
        std::cout << std::setw(15) << "Size";
        std::cout << "Name" << std::endl;
        // Print separator line
        std::cout << std::setfill('-') << std::setw(45) << "" << std::endl;
        std::cout << std::setfill(' ');

       // Iterate over directory entries
        for (const auto& entry : fs::directory_iterator(pwd())) {
            std::cout << std::left << std::setw(5) << (entry.is_directory() ? "d" : "-");
            std::cout << std::setw(15) << (!entry.is_directory() ? (std::to_string(fs::file_size(entry))): "") ;
            std::cout << entry.path().filename().string() << std::endl;
        }
    }

    // Method to convert Path to string
    [[nodiscard]] std::string toString() const {
        std::string result;
        for (const std::string& dir : directories) {
            if (!result.empty()) result += "->";
            result += dir;
        }
        return result;
    }
};

void processCommand(const std::string& command, const Path& path);
std::string trim(const std::string& str);

int main() {
    std::string command;
    Path path(std::filesystem::current_path().string());

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

    if (command == "pwd" ) std::cout << path.pwd()<< std::endl;
    else if (command == "ls") path.ls();
    else if (command == "cd" || command == "mkdir" || command == "rm") {
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