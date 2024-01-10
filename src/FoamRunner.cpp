#include <string>
#include <filesystem>
#include <iostream>
#include "ModelCreator.cpp"
using namespace std;

// millimeters
const double EXIT_RADIUS = 3.85*0.5;
const double THROAT_RADIUS = 2.85*0.5;
const double CHAMBER_RADIUS = 12.10*0.5;
const double ATMOSPHERE_TO_EXIT_RADIUS_RATIO = 5; // cells between the chamber and atmosphere regions need to have matching faces which only integers and specific ratios work for
const double ATMOSPHERE_REGION_RADIUS = EXIT_RADIUS * ATMOSPHERE_TO_EXIT_RADIUS_RATIO;
const double ATMOSPHERE_REGION_HEIGHT = 100;
const double NOZZLE_REGION_HEIGHT = EXIT_RADIUS*4;

int main() {
  vector<structureLayer> structureData = {
    structureLayer(THROAT_RADIUS, EXIT_RADIUS, EXIT_RADIUS*4, NOZZLE)
  };
  structureData.insert(structureData.begin(), structureLayer(THROAT_RADIUS, THROAT_RADIUS, THROAT_RADIUS*2, NOZZLE));
  structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, THROAT_RADIUS, (CHAMBER_RADIUS-THROAT_RADIUS)*3, NOZZLE));
  structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, CHAMBER_RADIUS, 1, CHAMBER));
  structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_HEIGHT, ATMOSPHERE, 0.3, ATMOSPHERE_TO_EXIT_RADIUS_RATIO, 1));
  structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS*0.2, ATMOSPHERE_REGION_RADIUS*3, ATMOSPHERE, 0.3, ATMOSPHERE_TO_EXIT_RADIUS_RATIO, 1));
  int angularResolution = 8;
  vector<double> cellResolutionsPerUnit = {1, 3, 3}; // {side-to-side radius-wise, vertical, in-out}
  vector<double> cellResolutionDistribution = {1, 1, 1}; // {1, 1, -more central- to +more outside+} - must be locked for the atmosphere and exit radius to be connected while having different sizes

  FileEditor::copyProjectToGeneratedDirectory();
  ModelCreator::createModel(structureData, angularResolution, cellResolutionsPerUnit, cellResolutionDistribution);
  FileEditor::copyProjectToRunDirectory();
  SolverStarter::solve();
  //FileEditor::copyRunDirectoryToGeneratedDirectory();
  //ModelCreator::calculateIsp();

  //vector<string> folders = FileEditor::getDirectories("");
  //for (string str : folders) {
  //  cout << str + "\n";
  //}

  //cout << "highest: " + FileEditor::getHighestNumberFilename("") + "\n";
  //cout << "print:\n" + FileEditor::getSectionOfFile(FileEditor::getHighestNumberFilename("") + "/U", "outlet") + "\n";
  //FileEditor::printDoubles(FileEditor::splitStringIntoDoublesByNewline(FileEditor::getSectionOfFile(FileEditor::getHighestNumberFilename("") + "/p", "outlet")));
  //string Udata = FileEditor::getSectionOfFile(FileEditor::getHighestNumberFilename("") + "/U", "outlet");
  ////cout << Udata;
  //vector<vector<double>> vals = (FileEditor::splitStringIntoVectorsByNewline(Udata));
  //for (vector<double> v : vals) {
  //  //FileEditor::printDoubles(v);
  //  cout << to_string(v.size()) + "\n";
  //}

}