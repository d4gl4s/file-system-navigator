#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <map>

bool isSourceDirectory(const std::string& str);
std::string trim(const std::string& str);

bool isSourceDirectory(const std::string& str) {
    if (!str.empty() && str.back() == '/')
        return true;
    return false;
}

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

    static void printCommands() {
        std::map<std::string, std::string> commands = {
                {"pwd", "Print current directory path"},
                {"ls", "List directory contents"},
                {"cd ..", "Move to parent directory"},
                {"cd [path]", "Move to specified directory"},
                {"mkdir [name]", "Create a new directory"},
                {"rm [path]", "Remove a file or directory"},
                {"mv [source] [destination]", "Move a file or directory"},
                {"cp [source] [destination]", "Copy a file or directory"},
                {"exit", "Exit the program"},
                {"help", "View all commands"}
        };
        const int COMMAND_COL_WIDTH = 30;
        std::cout << std::setw( COMMAND_COL_WIDTH ) << std::left << "Command" << "Description" << std::endl;
        std::cout << std::setw(COMMAND_COL_WIDTH) << std::left << "-------" << "-----------" << std::endl;
        for (const auto& [command, description] : commands) {
            std::cout << std::setw(COMMAND_COL_WIDTH) << std::left << command << description << std::endl;
        }
        std::cout << std::endl;
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

    // Method to copy a file or directory
    void cp(const std::string& sourcePath, const std::string& destinationDirectory) {
        namespace fs = std::filesystem;
        std::string sourceFilePath = pwd() + "\\" + sourcePath;
        std::string destinationDirectoryPath = pwd() + "\\" + destinationDirectory;

        // If destination is a file, then simply copy the source file to the current directory with the specified name
        if(!isSourceDirectory(destinationDirectory)){
            fs::copy(sourceFilePath, pwd() + "\\" + destinationDirectory);
            return;
        }

        if(!fs::exists(sourceFilePath)){
            std::cout << "Source file or directory '" << sourcePath << "' not found \n";
            return;
        }


        // If source is a directory, recursively copy its contents
        if (fs::is_directory(sourceFilePath)) {
            if (!fs::exists(destinationDirectoryPath)){
                std::cout << "Destination directory '" << destinationDirectory << "' not found.\n";
                return;
            }
            for (const auto& entry : fs::recursive_directory_iterator(sourceFilePath)) {
                std::string relativePath = entry.path().string().substr(sourceFilePath.size());
                fs::copy(entry, destinationDirectoryPath + relativePath, fs::copy_options::recursive);
            }
        }

        // If source is a file, copy it to the destination directory
        if (fs::exists(destinationDirectoryPath) && fs::is_directory(destinationDirectoryPath))
            fs::copy(sourceFilePath, destinationDirectoryPath + "\\" + fs::path(sourceFilePath).filename().string());
        else
            std::cout << "Destination directory '" << destinationDirectory << "' not found.\n";
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
        std::cout  << std::endl;
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

void processCommand(const std::string& commandString,  Path& path);

int main() {
    std::string command;
    Path path(std::filesystem::current_path().string());
    path.printCommands();

    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, command);
        if (trim(command) == "exit") break;
        processCommand(command, path);
    }
    return 0;
}

void processCommand(const std::string& commandString, Path& path) {
    std::vector<std::string> args;
    std::string command = trim(commandString);

    // Split command and arguments
    std::istringstream iss(command);
    std::string cmd;
    std::vector<std::string> cmdArgs;
    iss >> cmd;
    std::string arg;
    while (iss >> arg) cmdArgs.push_back(arg);

    if (cmd == "pwd") std::cout << path.pwd() << std::endl;
    else if (cmd == "ls") path.ls();
    else if (cmd == "help") path.printCommands();
    else if (cmd == "cd") {
        if (cmdArgs.size() == 1) path.cd(cmdArgs[0]);
        else std::cout << "Invalid arguments for command 'cd'.\n";
    }
    else if (cmd == "mkdir") {
        if (cmdArgs.size() == 1) path.mkdir(cmdArgs[0]);
        else std::cout << "Invalid arguments for command 'mkdir'.\n";
    }
    else if (cmd == "rm") {
        if (cmdArgs.size() == 1) path.rm(cmdArgs[0]);
        else std::cout << "Invalid arguments for command 'rm'.\n";
    }
    else if (cmd == "mv") {
        if (cmdArgs.size() == 2) path.mv(cmdArgs[0], cmdArgs[1]);
        else std::cout << "Invalid arguments for command 'mv'.\n";
    }
    else if (cmd == "cp") {
        if (cmdArgs.size() == 2) path.cp(cmdArgs[0], cmdArgs[1]);
        else std::cout << "Invalid arguments for command 'cp'.\n";
    }
    else std::cout << "Invalid command. Please try again.\n";
}


// Trim leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
        return str;
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}