#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
using namespace std;
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
    size_t tokenPos = fileContent.find(token);
    if (tokenPos == string::npos) {
        cerr << "Token not found in file.\n";
        inputFile.close();
        return;
    }
    cout << "Token found\n";
    // Add the text after the token
    while (!(fileContent.substr(tokenPos, 2) == "()" || fileContent.substr(tokenPos, 2) == "{}")) {
      cout << fileContent.at(tokenPos);
      tokenPos++;
    }

    if(tokenPos > fileContent.size()-1) {
      cerr << "Following brackets not found in file.\n";
      inputFile.close();
      return;
    }

    fileContent.insert(tokenPos + 1, text);
    
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
    
    //cout << "Text " + text + " added successfully to position " + to_string(tokenPos) + ".\n";
  }
};