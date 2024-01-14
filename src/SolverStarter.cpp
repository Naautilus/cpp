#include <string>
#include <filesystem>
#include <iostream>
#include "FileEditor.cpp"
using namespace std;

class SolverStarter {
  SolverStarter() = delete;

  public:

  static void solve() {
    cout << "starting\n";

    string commandString = "";
    commandString += "cd " + runDir + projectName + ";";
    commandString += "blockMesh;";
    commandString += "foamRun;";
    //commandString += "postProcess -func 'patchFlowRate(patch=outlet)' -latestTime;";
    //commandString += "postProcess -func 'patchFlowRate(patch=outlet)';";
    //commandString += "pwd; pwd; pwd; pwd; pwd";
    //commandString += "foamPostProcess -func 'mag(mesh.boundaryMesh()[outlet].faceAreas())';";
    commandString += "paraFoam &";

    system(commandString.c_str());

    cout << "end\n";
  }
};