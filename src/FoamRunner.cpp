#include <string>
#include <filesystem>
#include <iostream>
#include <random>
#include "ModelCreator.cpp"
using namespace std;

// millimeters
const double EXIT_RADIUS = 3.85*0.5;
const double THROAT_RADIUS = 2.85*0.5;
const double CHAMBER_RADIUS = THROAT_RADIUS*1.5; // 12.10*0.5;
const double ATMOSPHERE_TO_EXIT_RADIUS_RATIO = 3; // cells between the chamber and atmosphere regions need to have matching faces which only integers and specific ratios work for
const double ATMOSPHERE_REGION_RADIUS = EXIT_RADIUS * ATMOSPHERE_TO_EXIT_RADIUS_RATIO;
const double ATMOSPHERE_REGION_HEIGHT = 30;
const double NOZZLE_REGION_HEIGHT = EXIT_RADIUS*4;

double runSimulation(vector<double> radii) {
  vector<structureLayer> structureData;
  radii.insert(radii.begin(), THROAT_RADIUS);
  radii.insert(radii.end(), EXIT_RADIUS);
  for (int i = 0; i < radii.size()-1; i++) {
    structureData.push_back(structureLayer(radii[i], radii[i+1], EXIT_RADIUS*4.0/(radii.size()-1), NOZZLE));
  }
  structureData.insert(structureData.begin(), structureLayer(THROAT_RADIUS, THROAT_RADIUS, THROAT_RADIUS*2, NOZZLE));
  structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, THROAT_RADIUS, (CHAMBER_RADIUS-THROAT_RADIUS)*3, NOZZLE));
  structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, CHAMBER_RADIUS, 1, CHAMBER));
  structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_HEIGHT, ATMOSPHERE, 0.5, ATMOSPHERE_TO_EXIT_RADIUS_RATIO, 0.7));
  //structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS*1, ATMOSPHERE_REGION_RADIUS*3, ATMOSPHERE, 0.5, ATMOSPHERE_TO_EXIT_RADIUS_RATIO, 0.7));
  int angularResolution = 6;
  vector<double> cellResolutionsPerUnit = {1, 1, 3}; // {side-to-side radius-wise, vertical, in-out}
  vector<double> cellResolutionDistribution = {1, 1, 1}; // {1, 1, -more central- to +more outside+} - must be locked for the atmosphere and exit radius to be connected while having different sizes

  FileEditor::copyProjectToGeneratedDirectory();
  ModelCreator::createModel(structureData, angularResolution, cellResolutionsPerUnit, cellResolutionDistribution);
  FileEditor::copyProjectToRunDirectory();
  SolverStarter::solve();
  FileEditor::copyRunDirectoryToGeneratedDirectory();
  return ModelCreator::calculateIsp();
}

double runSimulation_Fake(vector<double> radii) {
  double output = 0;
  for (int i = 0; i < radii.size(); i++) {
    output += sin(i*radii[i]/radii.size());
  }
  return output;
}

struct simulationData {
  public:
  vector<double> data;
  double score;
  simulationData(vector<double> data_, double score_) {
    data = data_;
    score = score_;
    if (score == 0) {
      score = runSimulation(data);
    }
  }
  double getScore() {
    if (score == 0) {
      score = runSimulation(data);
    }
    return score;
  }
};

vector<double> axisVector(int length, int axisIndex, double value) {
  vector<double> output;
  for (int i = 0; i < length; i++) {
    if (i == axisIndex) {
      output.push_back(value);
    } else {
      output.push_back(0);
    }
  }
  return output;
}

vector<double> multiply(vector<double> input, double factor) {
  vector<double> output;
  for (double num : input) {
    output.push_back(num*factor);
  }
  return output;
}

vector<double> add(vector<double> a, vector<double> b) {
  vector<double> output;
  for (int i = 0; i < a.size(); i++) {
    output.push_back(a[i]+b[i]);
  }
  return output;
}

void printVector(vector<double> input, bool useNewline = true) {
  int precision = 5;
  cout << "{";
  for(int i = 0; i < input.size(); i++) {
    cout << to_string(pow(10,-precision)*round(input[i]*pow(10,precision))) + (i == input.size()-1 ? "" : ", ");
  }
  cout << "}";
  if (useNewline) {
    cout << "\n";
  }
}

string vectorToString(vector<double> input, bool useNewline = true) {
  string output = "";
  int precision = 5;
  output += "{";
  for(int i = 0; i < input.size(); i++) {
    output += to_string(pow(10,-precision)*round(input[i]*pow(10,precision))) + (i == input.size()-1 ? "" : ", ");
  }
  output += "}";
  if (useNewline) {
    output += "\n";
  }
  return output;
}

vector<double> modifierData(simulationData originalData) {
  // return a list of [length] modifier doubles which in total have the magnitude (pythagorean) of 1

  //unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
  //default_random_engine e(seed);
  //vector<double> output;
  //double magnitudeSquared;
  //for (int i = 0; i < length; i++) {
  //  auto rand = uniform_real_distribution<double>(-1, 1);
  //  output.push_back(rand(e));
  //  magnitudeSquared += output[i]*output[i];
  //}
  //double magnitude = sqrt(magnitudeSquared);
  //for (int i = 0; i < length; i++) {
  //  output[i] /= magnitude;
  //}
  //return output;

  double gradientDistance = 0.0001;
  double magnitudeSquared = 0;
  vector<double> gradient;
  for (int i = 0; i < originalData.data.size(); i++) {
    vector<double> modifiedData = add(originalData.data, axisVector(originalData.data.size(), i, gradientDistance));
    gradient.push_back(runSimulation(modifiedData) - originalData.getScore());
    magnitudeSquared += gradient[i]*gradient[i];
  }
  double magnitude = sqrt(magnitudeSquared);
  for (int i = 0; i < gradient.size(); i++) {
    gradient[i] = gradient[i] / magnitude;
  }
  cout << "gradient: ";
  printVector(gradient, false);
  cout << ", original magnitude " + to_string(magnitude) + "\n";
  return gradient;

}

simulationData optimizeAlongModifierSet_Directional(simulationData startingData, vector<double> modifierData) {

  cout << "--------------------------\n";
  // constants
  const double targetPerformanceIncreaseFactor = 1.0001; // the increase in performance to target for followup expansion
  const double startingExpansionExponent = -10; // this will start the modifier data multiplier at e^thisValue
  const double exponentIncreaseFactor = 0.5; // this will increase the modifier data multiplier magnitude by e^thisValue every iteration; approx =*(1+thisValue) when close to 0, =*2.71 for thisValue==1

  // set starting performance data
  simulationData bestPerformance = startingData;

  // run one modified simulation using startingData + modifierData*e^startingExpansionExponent. If it performs worse or under targetPerformanceIncreaseFactor, return startingData.
  simulationData modifiedData = simulationData(add(startingData.data, multiply(modifierData, exp(startingExpansionExponent))),0);
  if (modifiedData.getScore() < bestPerformance.getScore()) {
    cout << "modified score was less than starting score\n";
    return bestPerformance;
  }
  double initialPerformanceIncreaseFactor = modifiedData.getScore()/bestPerformance.getScore();
  //cout << "initialPerformanceIncreaseFactor: " + to_string(initialPerformanceIncreaseFactor) + "\n";
  //if (initialPerformanceIncreaseFactor < targetPerformanceIncreaseFactor) {
  //  cout << "Performance increase factor was less than minimum\n";
  //  return bestPerformance;
  //}
  double expansionExponent = startingExpansionExponent + log(log(targetPerformanceIncreaseFactor)/log(initialPerformanceIncreaseFactor)); // ln(1.01)/ln(1.001)
  cout << "expansionExponent set to " + to_string(expansionExponent) + " after startingExpansionExponent of " + to_string(startingExpansionExponent) + " gave an increase of " + to_string(initialPerformanceIncreaseFactor) + "\n";
  if (expansionExponent < startingExpansionExponent) {
    cout << "Followup modifier data value multiplier was less than the initial multiplier; decrease startingExpansionExponent or increase targetPerformanceIncreaseFactor\n";
    return bestPerformance;
  }
  // Otherwise, scale exponent to the desired initial increase in performance and run repeatedly until the data stops improving.
  int iterations = 0;
  while (true) {
    expansionExponent += exponentIncreaseFactor;
    modifiedData = simulationData(add(startingData.data, multiply(modifierData, exp(expansionExponent))),0);
    if (modifiedData.getScore() > bestPerformance.getScore()) {
      bestPerformance = modifiedData;
      iterations++;
    } else {
      break;
    }
  }
  cout << "New maximum found after " + to_string(iterations) + " iterations\n";
  return bestPerformance;
}

simulationData optimizeAlongModifierSet(simulationData startingData, vector<double> modifierData) {
  simulationData a = optimizeAlongModifierSet_Directional(startingData, modifierData);
  //simulationData b = optimizeAlongModifierSet_Directional(startingData, multiply(modifierData, -1));
  //if (a.getScore() > b.getScore()) {
    return a;
  //}
  //return b;
}

void printSimulationData(simulationData s) {
  printVector(s.data, false);
  cout << " with a score of " + to_string(s.getScore()) + "\n";
}

string simulationDataToString(simulationData s) {
  return vectorToString(s.data, false) + " with a score of " + to_string(s.getScore()) + "\n";
}

int main() {

  FILE *f = fopen("dataLogger", "w");

  // initialize radius parameters
  int radiusDataSize = 10;
  vector<double> radii;
  for (int i = 0; i < radiusDataSize; i++) {
    radii.push_back(THROAT_RADIUS * (i+1)/(radiusDataSize+2) + EXIT_RADIUS * (1-(i+1)/(radiusDataSize+2)));
  }
  simulationData bestData = simulationData(radii, runSimulation(radii));
  printSimulationData(bestData);
  for (int i = 0; i < 1000; i++) {
    bestData = optimizeAlongModifierSet(bestData, modifierData(bestData));
    printSimulationData(bestData);
    fprintf(f, "%s", simulationDataToString(bestData).c_str());
  }

  fclose(f);

  


  //runSimulation(radii);

  

  //vector<structureLayer> structureData = {
  //  structureLayer(THROAT_RADIUS, EXIT_RADIUS, EXIT_RADIUS*4, NOZZLE)
  //};
  //structureData.insert(structureData.begin(), structureLayer(THROAT_RADIUS, THROAT_RADIUS, THROAT_RADIUS*2, NOZZLE));
  //structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, THROAT_RADIUS, (CHAMBER_RADIUS-THROAT_RADIUS)*3, NOZZLE));
  //structureData.insert(structureData.begin(), structureLayer(CHAMBER_RADIUS, CHAMBER_RADIUS, 1, CHAMBER));
  //structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_HEIGHT, ATMOSPHERE, 0.5, ATMOSPHERE_TO_EXIT_RADIUS_RATIO, 0.7));
  ////structureData.insert(structureData.end(), structureLayer(ATMOSPHERE_REGION_RADIUS, ATMOSPHERE_REGION_RADIUS*1, ATMOSPHERE_REGION_RADIUS*3, ATMOSPHERE, 0.5, ATMOSPHERE_TO_EXIT_RADIUS_RATIO, 0.7));
  //int angularResolution = 6;
  //vector<double> cellResolutionsPerUnit = {1, 1, 3}; // {side-to-side radius-wise, vertical, in-out}
  //vector<double> cellResolutionDistribution = {1, 1, 1}; // {1, 1, -more central- to +more outside+} - must be locked for the atmosphere and exit radius to be connected while having different sizes
//
  //FileEditor::copyProjectToGeneratedDirectory();
  //ModelCreator::createModel(structureData, angularResolution, cellResolutionsPerUnit, cellResolutionDistribution);
  //FileEditor::copyProjectToRunDirectory();
  //SolverStarter::solve();
  //FileEditor::copyRunDirectoryToGeneratedDirectory();
  //ModelCreator::calculateIsp();

}