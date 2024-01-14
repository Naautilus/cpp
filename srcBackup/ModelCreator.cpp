#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <algorithm>
#include "SolverStarter.cpp"
#define _USE_MATH_DEFINES
using namespace std;
namespace fs = filesystem;

enum structureType {CHAMBER, NOZZLE, ATMOSPHERE};

class structureLayer {
  public:
  double startRadius, endRadius, height, verticalDensityFactor, horizontalCellCountFactor, horizontallyExtendedCellsResolutionFactor;
  structureType type;
  structureLayer(){};
  structureLayer(double startRadius_, double endRadius_, double height_, structureType type_) {
    startRadius = startRadius_;
    endRadius = endRadius_;
    height = height_;
    type = type_;
    verticalDensityFactor = 1;
    horizontalCellCountFactor = 1;
    horizontallyExtendedCellsResolutionFactor = 1;
  }
  structureLayer(double startRadius_, double endRadius_, double height_, structureType type_, double verticalDensityFactor_, double horizontalCellCountFactor_, double horizontallyExtendedCellsResolutionFactor_) {
    startRadius = startRadius_;
    endRadius = endRadius_;
    height = height_;
    type = type_;
    verticalDensityFactor = verticalDensityFactor_;
    horizontalCellCountFactor = horizontalCellCountFactor_;
    horizontallyExtendedCellsResolutionFactor = horizontallyExtendedCellsResolutionFactor_;
  }
};


class ModelCreator {
  ModelCreator() = delete;

  public:

  enum endType {bottom, top};

  class vector3 {
    public:
    double x, y, z;
    int num;
    vector3(){}
    vector3(double x_, double y_, double z_) {
      x = x_;
      y = y_;
      z = z_;
      num = -1;
    }
    vector3(vector<double> v) {
      x = v[0];
      y = v[1];
      z = v[2];
      num = -1;
    }
    vector3 scaleXY(double scalingFactor) {
      return vector3(x * scalingFactor, y * scalingFactor, z);
    }
    bool isEqual(vector3 v) {
      double epsilon = 0.000001;
      if ((v.x - x)*(v.x - x) + (v.y - y)*(v.y - y) + (v.z - z)*(v.z - z) < epsilon) {
        return true;
      }
      return false;
    }
    vector3 operator+ (vector3 v) {
      return vector3(x+v.x, y+v.y, z+v.z);
    }
    vector3 operator* (double m) {
      return vector3(x*m, y*m, z*m);
    }
  };

  class quad {
    public:
    vector3 p0, p1, p2, p3;
    int zIndexAtGeneration;
    quad(vector3 p0_, vector3 p1_, vector3 p2_, vector3 p3_, int z) {
      p0 = p0_;
      p1 = p1_;
      p2 = p2_;
      p3 = p3_;
      zIndexAtGeneration = z;
    }
    quad scaleXY(double scalingFactor) {
      return quad(
        p0.scaleXY(scalingFactor),
        p1.scaleXY(scalingFactor),
        p2.scaleXY(scalingFactor),
        p3.scaleXY(scalingFactor),
        zIndexAtGeneration
      );
    }
    double getArea() {
      return calculateTriangleArea(p1, p0, p3) + calculateTriangleArea(p1, p2, p3);
    }
    vector3 crossProduct(const vector3 &v1, const vector3 &v2) {
      return vector3((v1.y * v2.z) - (v1.z * v2.y),
                    (v1.z * v2.x) - (v1.x * v2.z),
                    (v1.x * v2.y) - (v1.y * v2.x));
    }

    // Function to calculate the magnitude of a vector
    double magnitude(const vector3 &v) {
        return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
    // Function to calculate the area of a triangle in 3D space
    double calculateTriangleArea(const vector3 &v1, const vector3 &v2, const vector3 &v3) {
      // Calculate two vectors representing two sides of the triangle
      vector3 side1(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
      vector3 side2(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);

      // Calculate the cross product of the two vectors
      vector3 cross = crossProduct(side1, side2);

      // Calculate the magnitude of the cross product
      double area = 0.5 * magnitude(cross);

      return area;
    }
  };

  struct cuboid {
    vector3 p0, p1, p2, p3, p4, p5, p6, p7, cellGrading;
    double p, T, U;
    int cellsX, cellsY, cellsZ = 0;
    cuboid(vector3 p0_, vector3 p1_, vector3 p2_, vector3 p3_, vector3 p4_, vector3 p5_, vector3 p6_, vector3 p7_, int cellsX_, int cellsY_, int cellsZ_, vector3 cellGrading_, double p_, double T_, double U_) {
      p0 = p0_;
      p1 = p1_;
      p2 = p2_;
      p3 = p3_;
      p4 = p4_;
      p5 = p5_;
      p6 = p6_;
      p7 = p7_;
      cellsX = cellsX_;
      cellsY = cellsY_;
      cellsZ = cellsZ_;
      cellGrading = cellGrading_;
    }
    cuboid(quad q0, quad q1, int cellsX_, int cellsY_, int cellsZ_, vector3 cellGrading_) {
      p0 = q0.p0;
      p1 = q0.p1;
      p2 = q0.p2;
      p3 = q0.p3;
      p4 = q1.p0;
      p5 = q1.p1;
      p6 = q1.p2;
      p7 = q1.p3;
      cellsX = cellsX_;
      cellsY = cellsY_;
      cellsZ = cellsZ_;
      cellGrading = cellGrading_;
    }
  };

  const double ATMOSPHERE_p = 14;
  const double ATMOSPHERE_T = 300; // NOT ACTIVE
  const vector3 ATMOSPHERE_U = vector3(0,0,0);

  const double CHAMBER_p = 140;
  const double CHAMBER_T = 600; // NOT ACTIVE
  const vector3 CHAMBER_U = vector3(0,0,100);

  static vector3 pointAt(int zIndex, int radialIndex, vector<structureLayer> structureData, int radialPointCount, endType endtype) {
    double z = 0;
    for (int i = 0; i < zIndex + (endtype == top); i++) {
      z += structureData[i].height;
    }
    double angle = 2 * M_PI * radialIndex / radialPointCount;
    double radius = (endtype == top ? structureData[zIndex].endRadius : structureData[zIndex].startRadius);
    double x = sin(angle) * radius;
    double y = cos(angle) * radius;
    return vector3(x,y,z);
  }

  static int getPointIndex(vector3 &x, vector<vector3> &v) {
    int index = -1;
    for (int i = 0; i < v.size(); i++) {
      if (v[i].isEqual(x)) {
        index = i;
      }
    }
    return index;
  }

  static void addPointsToList(vector3 &x, vector<vector3> &v) {
    int index = getPointIndex(x, v);
    if (index == -1) v.push_back(x);
  }

  static int getNum(vector3 &x, vector<vector3> &v) {
    return v[getPointIndex(x, v)].num;
  }

  static void createModel(vector<structureLayer> structureData, int radialPointCount, vector<double> &cellResolutionsPerUnit, vector<double> &cellResolutionDistribution) {

    // generate a list of edge faces, or "walls" in the simulation

    vector<quad> nozzleFaces;
    vector<quad> chamberSideFaces;
    vector<quad> chamberAllFaces;
    vector<quad> atmoAllFaces;
    vector<quad> atmoSideFaces;
    vector<quad> atmoBottomFaces;
    vector<cuboid> cells;
    vector<cuboid> atmoCells;

    cout << "structureData size: " + to_string(structureData.size()) + "\n" << endl;
    for (int zIndex = 0; zIndex <= structureData.size() - 1; zIndex++) {
      for (int radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
        quad q = quad(
          pointAt(zIndex, radialIndex, structureData, radialPointCount, bottom),
          pointAt(zIndex, radialIndex+1, structureData, radialPointCount, bottom),
          pointAt(zIndex, radialIndex+1, structureData, radialPointCount, top),
          pointAt(zIndex, radialIndex, structureData, radialPointCount, top),
          zIndex
        ); // put in both bottom/top and their sides because the sides 
        switch (structureData[zIndex].type) {
          case CHAMBER:
            chamberAllFaces.push_back(q);
            chamberSideFaces.push_back(q);
            break;
          case NOZZLE:
            nozzleFaces.push_back(q);
            break;
          case ATMOSPHERE:
            atmoAllFaces.push_back(q);
            atmoSideFaces.push_back(q);
            break;
        }
        //if (structureData[zIndex+1].type == CHAMBER) { // chamber
        //  chamberAllFaces.push_back(q);
        //  chamberSideFaces.push_back(q);
        //} else if (structureData[zIndex+1].type == NOZZLE) { // nozzle
        //  nozzleFaces.push_back(q);
        //} else { // atmosphere
        //  atmoAllFaces.push_back(q);
        //  atmoSideFaces.push_back(q);
        //}
      }
    }

    for (double radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
      chamberAllFaces.push_back(quad(
        pointAt(0, radialIndex, structureData, radialPointCount, bottom),
        pointAt(0, radialIndex+1, structureData, radialPointCount, bottom),
        pointAt(0, radialIndex+1, structureData, radialPointCount, bottom).scaleXY(0),
        pointAt(0, radialIndex, structureData, radialPointCount, bottom).scaleXY(0),
        0
      ));
    }

    for (double radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
      atmoAllFaces.push_back(quad(
        pointAt(structureData.size()-1, radialIndex, structureData, radialPointCount, top),
        pointAt(structureData.size()-1, radialIndex+1, structureData, radialPointCount, top),
        pointAt(structureData.size()-1, radialIndex+1, structureData, radialPointCount, top).scaleXY(1/structureData[structureData.size()-1].horizontalCellCountFactor),
        pointAt(structureData.size()-1, radialIndex, structureData, radialPointCount, top).scaleXY(1/structureData[structureData.size()-1].horizontalCellCountFactor),
        structureData.size()
      ));
      atmoAllFaces.push_back(quad(
        pointAt(structureData.size()-1, radialIndex, structureData, radialPointCount, top).scaleXY(1/structureData[structureData.size()-1].horizontalCellCountFactor),
        pointAt(structureData.size()-1, radialIndex+1, structureData, radialPointCount, top).scaleXY(1/structureData[structureData.size()-1].horizontalCellCountFactor),
        pointAt(structureData.size()-1, radialIndex+1, structureData, radialPointCount, top).scaleXY(0),
        pointAt(structureData.size()-1, radialIndex, structureData, radialPointCount, top).scaleXY(0),
        structureData.size()
      ));
    }

    int firstAtmoCellIndex = atmoAllFaces[0].zIndexAtGeneration;
    for (double radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
      atmoAllFaces.push_back(quad(
        pointAt(firstAtmoCellIndex, radialIndex, structureData, radialPointCount, bottom),
        pointAt(firstAtmoCellIndex, radialIndex+1, structureData, radialPointCount, bottom),
        pointAt(firstAtmoCellIndex, radialIndex+1, structureData, radialPointCount, bottom).scaleXY(1/structureData[firstAtmoCellIndex].horizontalCellCountFactor),
        pointAt(firstAtmoCellIndex, radialIndex, structureData, radialPointCount, bottom).scaleXY(1/structureData[firstAtmoCellIndex].horizontalCellCountFactor),
        firstAtmoCellIndex
      ));
    }

    // generate a list of cuboid segments, or "cells" in the simulation


    for (quad face : nozzleFaces) {
      cells.push_back(cuboid(
        face, // 1 first instead of 0 because OpenFOAM uses a normalized vector between point 0 and the points around it, which would have a distance of 0 if the 0-scaled face was first
        face.scaleXY(0),
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * structureData[face.zIndexAtGeneration].height * structureData[face.zIndexAtGeneration].verticalDensityFactor),
        cellResolutionsPerUnit[2] * structureData[face.zIndexAtGeneration].horizontalCellCountFactor,
        vector3(cellResolutionDistribution)
      ));
    }

    for (quad face : chamberSideFaces) {
      cells.push_back(cuboid(
        face,
        face.scaleXY(0),
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * structureData[face.zIndexAtGeneration].height * structureData[face.zIndexAtGeneration].verticalDensityFactor),
        cellResolutionsPerUnit[2] * structureData[face.zIndexAtGeneration].horizontalCellCountFactor,
        vector3(cellResolutionDistribution)
      ));
    }

    for (quad face : atmoSideFaces) {
      cells.push_back(cuboid(
        face,
        face.scaleXY(1/structureData[face.zIndexAtGeneration].horizontalCellCountFactor),
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * structureData[face.zIndexAtGeneration].height * structureData[face.zIndexAtGeneration].verticalDensityFactor),
        cellResolutionsPerUnit[2] * (structureData[face.zIndexAtGeneration].horizontalCellCountFactor-1) * structureData[face.zIndexAtGeneration].horizontallyExtendedCellsResolutionFactor,
        vector3(cellResolutionDistribution)
      ));
    }

    for (quad face : atmoSideFaces) {
      cells.push_back(cuboid(
        face.scaleXY(1/structureData[face.zIndexAtGeneration].horizontalCellCountFactor),
        face.scaleXY(0),
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * structureData[face.zIndexAtGeneration].height * structureData[face.zIndexAtGeneration].verticalDensityFactor),
        cellResolutionsPerUnit[2],
        vector3(cellResolutionDistribution)
      ));
    }

    // since OpenFOAM uses a list of points to construct faces...
    // get all unique vectors used and enumerate them

    vector<vector3> points = {};

    for (quad face : nozzleFaces) {
      addPointsToList(face.p0, points);
      addPointsToList(face.p1, points);
      addPointsToList(face.p2, points);
      addPointsToList(face.p3, points);
    }

    for (quad face : chamberAllFaces) {
      addPointsToList(face.p0, points);
      addPointsToList(face.p1, points);
      addPointsToList(face.p2, points);
      addPointsToList(face.p3, points);
    }

    for (quad face : atmoAllFaces) {
      addPointsToList(face.p0, points);
      addPointsToList(face.p1, points);
      addPointsToList(face.p2, points);
      addPointsToList(face.p3, points);
    }

    for (cuboid cell : cells) {
      addPointsToList(cell.p0, points);
      addPointsToList(cell.p1, points);
      addPointsToList(cell.p2, points);
      addPointsToList(cell.p3, points);
      addPointsToList(cell.p4, points);
      addPointsToList(cell.p5, points);
      addPointsToList(cell.p6, points);
      addPointsToList(cell.p7, points);
    }

    for (cuboid cell : atmoCells) {
      addPointsToList(cell.p0, points);
      addPointsToList(cell.p1, points);
      addPointsToList(cell.p2, points);
      addPointsToList(cell.p3, points);
      addPointsToList(cell.p4, points);
      addPointsToList(cell.p5, points);
      addPointsToList(cell.p6, points);
      addPointsToList(cell.p7, points);
    }

    for (int i = 0; i < points.size(); i++) {
      points[i].num = i;
    }

    for (int i = 0; i < nozzleFaces.size(); i++) {
      nozzleFaces[i].p0.num = getNum(nozzleFaces[i].p0, points);
      nozzleFaces[i].p1.num = getNum(nozzleFaces[i].p1, points);
      nozzleFaces[i].p2.num = getNum(nozzleFaces[i].p2, points);
      nozzleFaces[i].p3.num = getNum(nozzleFaces[i].p3, points);
    }

    for (int i = 0; i < chamberAllFaces.size(); i++) {
      chamberAllFaces[i].p0.num = getNum(chamberAllFaces[i].p0, points);
      chamberAllFaces[i].p1.num = getNum(chamberAllFaces[i].p1, points);
      chamberAllFaces[i].p2.num = getNum(chamberAllFaces[i].p2, points);
      chamberAllFaces[i].p3.num = getNum(chamberAllFaces[i].p3, points);
    }

    for (int i = 0; i < atmoAllFaces.size(); i++) {
      atmoAllFaces[i].p0.num = getNum(atmoAllFaces[i].p0, points);
      atmoAllFaces[i].p1.num = getNum(atmoAllFaces[i].p1, points);
      atmoAllFaces[i].p2.num = getNum(atmoAllFaces[i].p2, points);
      atmoAllFaces[i].p3.num = getNum(atmoAllFaces[i].p3, points);
    }

    for (int i = 0; i < cells.size(); i++) {
      cells[i].p0.num = getNum(cells[i].p0, points);
      cells[i].p1.num = getNum(cells[i].p1, points);
      cells[i].p2.num = getNum(cells[i].p2, points);
      cells[i].p3.num = getNum(cells[i].p3, points);
      cells[i].p4.num = getNum(cells[i].p4, points);
      cells[i].p5.num = getNum(cells[i].p5, points);
      cells[i].p6.num = getNum(cells[i].p6, points);
      cells[i].p7.num = getNum(cells[i].p7, points);
    }

    for (int i = 0; i < atmoCells.size(); i++) {
      atmoCells[i].p0.num = getNum(atmoCells[i].p0, points);
      atmoCells[i].p1.num = getNum(atmoCells[i].p1, points);
      atmoCells[i].p2.num = getNum(atmoCells[i].p2, points);
      atmoCells[i].p3.num = getNum(atmoCells[i].p3, points);
      atmoCells[i].p4.num = getNum(atmoCells[i].p4, points);
      atmoCells[i].p5.num = getNum(atmoCells[i].p5, points);
      atmoCells[i].p6.num = getNum(atmoCells[i].p6, points);
      atmoCells[i].p7.num = getNum(atmoCells[i].p7, points);
    }

    //vector<quad> empty = {};

    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "vertices", vector3ListToString(points));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "inlet", boundaryToString("patch", chamberAllFaces));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "outlet", boundaryToString("patch", atmoAllFaces));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "blocks", cuboidListToString(cells));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "sides", boundaryToString("wall", nozzleFaces));
    
    

  }

  //static void calculateIsp() {
  //  FileEditor::writeOverFile("/areas", quadListToString_Area(atmoAllFaces));
  //  string maxTimestampFilename = FileEditor::getHighestNumberFilename("");
  //  vector<double> rho = FileEditor::splitStringIntoDoublesByNewline(FileEditor::getSectionOfFile(FileEditor::getHighestNumberFilename("") + "/rho", "outlet"));
  //  vector<vector<double>> U = FileEditor::splitStringIntoVectorsByNewline(FileEditor::getSectionOfFile(FileEditor::getHighestNumberFilename("") + "/U", "outlet"));
  //  for (int i = 0; i < rho.size(); i++) {
  //
  //  }
  //  //FileEditor::splitString(FileEditor::getSectionOfFile("/"))
  //}

  static string vector3ToString(vector3 v) {
    return "(" + to_string(v.x) + " " + to_string(v.y) + " " + to_string(v.z) + ")";
  }

  static string vector3ListToString(vector<vector3> vectors) {
    string output = "";
    for (vector3 v : vectors) {
      output += vector3ToString(v) + "";
      //cout << output + "\n";
    }
    return output;
  }

  static string quadToString(quad q) {
    return "(" + 
    to_string(q.p0.num) + " " + 
    to_string(q.p1.num) + " " + 
    to_string(q.p2.num) + " " + 
    to_string(q.p3.num) + ")";
  }

  static string quadListToString(vector<quad> quads) {
    string output = "";
    for (quad q  : quads) {
      output += quadToString(q) + " ";
    }
    return output;
  }

  static string quadToString_Area(quad q) {
    return to_string(q.getArea());
  }

  static string quadListToString_Area(vector<quad> quads) {
    string output = "";
    for (quad q  : quads) {
      output += quadToString_Area(q) + " ";
    }
    return output;
  }

  static string boundaryToString(string boundaryType, vector<quad> quads) {
    return "type " + boundaryType + "; faces (" + quadListToString(quads) + ");";
  }

  static string cuboidToString(cuboid c) {
    return "hex (" + 
    to_string(c.p0.num) + " " + 
    to_string(c.p1.num) + " " + 
    to_string(c.p2.num) + " " + 
    to_string(c.p3.num) + " " + 
    to_string(c.p4.num) + " " + 
    to_string(c.p5.num) + " " + 
    to_string(c.p6.num) + " " + 
    to_string(c.p7.num) + ") " +
    "(" + to_string(c.cellsX) + " " + to_string(c.cellsY) + " " + to_string(c.cellsZ) + ") " +
    " simpleGrading " +
    vector3ToString(c.cellGrading);
  }

  static string cuboidListToString(vector<cuboid> cells) {
    string cellsString = "";
    for (cuboid c : cells) {
      cellsString += cuboidToString(c) + " ";
    }
    return cellsString;
  }
};