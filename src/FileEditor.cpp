#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
using namespace std;
#include <vector>
namespace fs = filesystem;

static string RUN_DIR = "/home/matth/OpenFOAM/matth-11/run";
static string CODE_DIR = "/home/matth/cpp";
static string PROJECT_NAME = "/foamTest2";
static string runDir = RUN_DIR;
static string codeDir = CODE_DIR;
static string projectName = PROJECT_NAME;

class FileEditor {

  public:

  //static void setFileConstants(string &runDir_, string &codeDir_, string &projectName_)  {
  //  runDir = runDir_;
  //  codeDir = codeDir_;
  //  projectName = projectName_;
  //}

  static void copyProjectToGeneratedDirectory() {
    fs::remove_all(codeDir + "/projectGenerated");
    fs::copy(codeDir + "/projectTemplate", codeDir + "/projectGenerated", fs::copy_options::recursive);
    cout << "files copied\n";
  }

  static void copyProjectToRunDirectory() {
    string projectDir = runDir + projectName;
    fs::remove_all(projectDir);
    fs::copy(codeDir + "/projectGenerated", projectDir, fs::copy_options::recursive);
    cout << "files copied\n";
  }

  static void copyRunDirectoryToGeneratedDirectory() {
    string projectDir = runDir + projectName;
    fs::remove_all(codeDir + "/projectGenerated");
    fs::copy(projectDir, codeDir + "/projectGenerated", fs::copy_options::recursive);
    cout << "files copied\n";
  }

  static void addTextToSectionInFile(const string &filename_, const string &token, const string &text) {
    // Open the input file
    string filename = codeDir + "/projectGenerated" + filename_;
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    // Read the content of the file into a string
    string fileContent((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    // Find the position of the token in the file content
    size_t startPos = fileContent.find(token);
    if (startPos == string::npos) {
        cerr << "Token not found in file.\n";
        inputFile.close();
        return;
    }
    cout << "Token found\n";
    // Add the text after the token
    while (!(fileContent.substr(startPos, 2) == "()" || fileContent.substr(startPos, 2) == "{}")) {
      cout << fileContent.at(startPos);
      startPos++;
    }
    if(startPos > fileContent.size()-1) {
      cerr << "Following brackets not found in file.\n";
      inputFile.close();
      return;
    }
    fileContent.insert(startPos + 1, text);
    // Close the input file
    inputFile.close();
    
    // Open the file in write mode to update its content
    ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }
    // Write the modified content back to the file
    outputFile << fileContent;
    // Close the output file
    outputFile.close();
  }

  static void writeOverFile(const string &filename_, const string &text) {
    string filename = codeDir + "/projectGenerated" + filename_;
    ofstream outputFile(filename, std::ofstream::trunc);
    outputFile << text;
    outputFile.close();
  }

  static string getSectionOfFile(const string &filename_, const string &token) {
    // Open the input file
    string filename = codeDir + "/projectGenerated" + filename_;
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return "";
    }
    // Read the content of the file into a string
    string fileContent((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    // Find the position of the token in the file content
    long startPos = fileContent.find(token);
    if (startPos == string::npos) {
        cerr << "Token not found in file.\n";
        inputFile.close();
        return "";
    }
    cout << "Token found\n";
    // Add the text after the token
    while (!(fileContent.substr(startPos, 1) == "(" || fileContent.substr(startPos, 1) == "{")) {
      //cout << fileContent.at(startPos);
      startPos++;
    }
    
    // Shift the token forward until the list does not have "uniform" preceding it

    if(startPos > fileContent.size()) {
      cerr << "Following brackets not found in file.\n";
      inputFile.close();
      return "";
    }
    // Find end position
    long endPos = startPos+1;
    long parenthesesCount = 0;
    bool previouslyGreaterThanZero = false;
    while (parenthesesCount > 0 || !previouslyGreaterThanZero) {
      if (/*endPos < fileContent.length()-10 && */(fileContent.substr(endPos, 7) == "uniform")) {
        cout << "uniform detected, shifting forward\n";
        if (filename_.substr(filename_.length()-2, 2) == "/U") {
          endPos += 50;
          startPos += 70;
        }
        endPos += 10;
        startPos += 10;
        cout << "filename_: " + filename_.substr(filename_.length()-2, 2) + "\n";
      } else {
        //cout << "uniform not detected; text is " + fileContent.substr(endPos, 7) + "\n";
        if (parenthesesCount > 0) previouslyGreaterThanZero = true;
      }
        
      if(fileContent.substr(endPos, 1) == "(") parenthesesCount++;
      if(fileContent.substr(endPos, 1) == "{") parenthesesCount++;
      if(fileContent.substr(endPos, 1) == ")") parenthesesCount--;
      if(fileContent.substr(endPos, 1) == "}") parenthesesCount--;
      cout << "|" + fileContent.substr(endPos, 1) + to_string(parenthesesCount) + (previouslyGreaterThanZero ? "!" : ".");
      endPos++;
    }

    //startPos = endPos-1;
    //parenthesesCount = 0;
    //do {
    //  if(fileContent.substr(startPos, 1) == "(") parenthesesCount--;
    //  if(fileContent.substr(startPos, 1) == "{") parenthesesCount--;
    //  if(fileContent.substr(startPos, 1) == ")") parenthesesCount++;
    //  if(fileContent.substr(startPos, 1) == "}") parenthesesCount++;
    //  startPos--;
    //  cout << "[" + fileContent.substr(startPos, 1) + to_string(parenthesesCount) + "]";
    //} while (parenthesesCount > 0);

    return trimToParentheses(fileContent.substr(startPos, endPos-startPos));
    //return fileContent.substr(startPos, endPos-startPos);
  }

  static vector<string> splitString(const std::string &input, const string &delimiter) {
    vector<string> result;
    istringstream stream(input);
    string token;
    while (getline(stream, token, delimiter.c_str()[0])) {
        result.push_back(token);
    }
    return result;
  }

  static vector<string> getDirectories(const string &s) {
    string filename = codeDir + "/projectGenerated" + s;
    vector<string> r;
    for(auto &p : filesystem::directory_iterator(filename))
      if (p.is_directory()) {
        r.push_back(p.path().string());
      }
    return r;
  }

  static string getHighestNumberFilename(const string &s) {
    vector<string> names = getDirectories(s);
    double highestNumber = -__DBL_MAX__;
    string highestNumberName = "failed";
    for (string nameUnformatted : names) {
      vector<string> names = splitString(nameUnformatted, "/");
      string name = names[names.size()-1];
      if (checkIfStringIsDouble(name)) {
        if (strtod(name.c_str(), NULL) > highestNumber) {
          highestNumber = strtod(name.c_str(), NULL);
          highestNumberName = name;
        }
      } else {
        cout << name + " is not a number\n";
      }
    }
    cout << "Highest number filename: /" + highestNumberName + "\n";
    return "/" + highestNumberName;
  }

  static bool checkIfStringIsDouble(string inputString) {
    char *end;
    double result = strtod(inputString.c_str(), &end);
    return !(end == inputString.c_str() || *end != '\0');
  }

  static string trimToParentheses(string input) {
    int startIndex = 0;
    while (input.substr(startIndex, 1) != "(") { startIndex++; }
    startIndex++;
    int endIndex = input.length() - 1;
    //cout << "endIndex: " + to_string(endIndex) + "\n";
    //while (input.substr(endIndex, 1) != ")") { endIndex--; }
    return input.substr(startIndex, endIndex-startIndex);
  }

  static vector<double> splitStringIntoDoublesByNewline(string input) {
    vector<string> inputValues = splitString(input, "\n");
    vector<double> outputValues = {};
    for (string str : inputValues) {
      if (checkIfStringIsDouble(str)) {
        outputValues.push_back(strtod(str.c_str(), NULL));
      }
    }
    return outputValues;
  }

  static vector<vector<double>> splitStringIntoVectorsByNewline(string input) {
    vector<string> inputValues = splitString(input, "\n");
    vector<vector<double>> outputValues = {};
    for (string str : inputValues) {
      if (str.substr(0,1) == "(") {
        vector<string> separatedStr = splitString(trimToParentheses(str), " ");
        if (separatedStr.size() != 3) {
          cout << "separatedStr of wrong size: " + to_string(separatedStr.size()) + "\n";
          return outputValues;
        }
        vector<double> separatedStrToDouble = {
          strtod(separatedStr[0].c_str(), NULL),
          strtod(separatedStr[1].c_str(), NULL),
          strtod(separatedStr[2].c_str(), NULL)
        };
        outputValues.push_back(separatedStrToDouble);
      }
    }
    return outputValues;
  }

  static void printDoubles(vector<double> input) {
    cout << "{ ";
    for (double num : input) {
      cout << to_string(num) + ", ";
    }
    cout << "}\n";
  }
};