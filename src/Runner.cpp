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
const double ATMOSPHERE_REGION_HEIGHT = 80;

int main() {

  vector<structureLayer> structureData = {
    structureLayer(CHAMBER_RADIUS, THROAT_RADIUS, (CHAMBER_RADIUS-THROAT_RADIUS)*3, NOZZLE),
    structureLayer(THROAT_RADIUS, THROAT_RADIUS, THROAT_RADIUS*2, NOZZLE),
    structureLayer(THROAT_RADIUS, EXIT_RADIUS, EXIT_RADIUS*4, NOZZLE)
    //structureLayer(1, 1, 1, CHAMBER),
    //structureLayer(1, 1, 1, NOZZLE),
    //structureLayer(1, 1, 1, NOZZLE),
    //strucsstureLayer(1, 1, 1, NOZZLE),
    //structureLayer(1, 1, 1, ATMOSPHERE)
  };
  structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, CHAMBER_RADIUS, 1, CHAMBER));
  structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_HEIGHT, ATMOSPHERE, 1, ATMOSPHERE_TO_EXIT_RADIUS_RATIO));
  int angularResolution = 8;
  vector<double> cellResolutionsPerUnit = {1, 2, 3}; // {side-to-side radius-wise, vertical, in-out}
  vector<double> cellResolutionDistribution = {1, 1, 1}; // {1, 1, -more central- to +more outside+} - must be locked for the atmosphere and exit radius to be connected while having different sizes

  FileEditor::copyProjectToGeneratedDirectory();
  ModelCreator::createModel(structureData, angularResolution, cellResolutionsPerUnit, cellResolutionDistribution); // 296 last run           // 1219s for 0.000296 on nOuterCorrectors = 1
  FileEditor::copyProjectToRunDirectory();
  SolverStarter::solve();

}