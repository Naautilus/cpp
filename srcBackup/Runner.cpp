#include <string>
#include <filesystem>
#include <iostream>
#include "ModelCreator.cpp"
using namespace std;

// millimeters
const double EXIT_RADIUS = 3.85*0.5;
const double THROAT_RADIUS = 2.85*0.5;
const double CHAMBER_RADIUS = 12.10*0.5*0.5;
const double ATMOSPHERE_REGION_RADIUS = 20;
const double ATMOSPHERE_REGION_HEIGHT = 20;
const int CHAMBER = 0;
const int NOZZLE = 1;
const int ATMOSPHERE = 2;

int main() {

  vector<vector<double>> diameterHeights = {
    {THROAT_RADIUS, THROAT_RADIUS*2, NOZZLE, 1},
    {EXIT_RADIUS, EXIT_RADIUS*4, NOZZLE, 1}
    };
  //vector<vector<double>> diameterHeights = {{1,1}, {3,1}, {3,1}};
  //vector<double> diameters = {1, 1};
  //diameterHeights.insert(diameterHeights.begin(), {THROAT_RADIUS, 20, NOZZLE}); // CHAMBER_RADIUS-THROAT_RADIUS
  //diameterHeights.insert(diameterHeights.begin(), {CHAMBER_RADIUS, 20, CHAMBER, 1});
  //diameterHeights.insert(diameterHeights.begin(), {CHAMBER_RADIUS, 1, CHAMBER, 0.333});
  diameterHeights.insert(diameterHeights.begin(), {CHAMBER_RADIUS, 1, CHAMBER, 1});
  //diameterHeights.insert(diameterHeights.begin(), {0.001*CHAMBER_RADIUS, 2, CHAMBER});
  //diameterHeights.insert(diameterHeights.begin(), {CHAMBER_RADIUS, 1, CHAMBER});
  diameterHeights.insert(diameterHeights.end(), {ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS-EXIT_RADIUS, ATMOSPHERE, 1});
  diameterHeights.insert(diameterHeights.end(), {ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_HEIGHT, ATMOSPHERE, 1});
  //for (vector<double> dh : diameterHeights) {
  //  height += dh[1];
  //}
  int angularResolution = 8;
  vector<double> cellResolutionsPerUnit = {1, 3, 5}; // {side-to-side radius-wise, vertical, in-out}
  vector<double> cellResolutionDistribution = {1, 1, 2}; // {1, 1, -more central- to +more outside+}

  FileEditor::copyProjectToGeneratedDirectory();
  ModelCreator::createModel(diameterHeights, angularResolution, cellResolutionsPerUnit, cellResolutionDistribution);
  FileEditor::copyProjectToRunDirectory();
  SolverStarter::solve();

}