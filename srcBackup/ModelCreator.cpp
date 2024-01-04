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

class ModelCreator {
  ModelCreator() = delete;

  public:

  class Vector3 {
    public:
    double x, y, z;
    int num;
    Vector3(){}
    Vector3(double x_, double y_, double z_) {
      x = x_;
      y = y_;
      z = z_;
      num = -1;
    }
    Vector3(vector<double> v) {
      x = v[0];
      y = v[1];
      z = v[2];
      num = -1;
    }
    Vector3 scaleXY(double scalingFactor) {
      return Vector3(x * scalingFactor, y * scalingFactor, z);
    }
    bool isEqual(Vector3 v) {
      double epsilon = 0.000001;
      if ((v.x - x)*(v.x - x) + (v.y - y)*(v.y - y) + (v.z - z)*(v.z - z) < epsilon) {
        return true;
      }
      return false;
    }
  };

  class quad {
    public:
    Vector3 p0, p1, p2, p3;
    int zIndexAtGeneration;
    quad(Vector3 p0_, Vector3 p1_, Vector3 p2_, Vector3 p3_, int z) {
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
  };

  struct cuboid {
    Vector3 p0, p1, p2, p3, p4, p5, p6, p7, cellGrading;
    double p, T, U;
    int cellsX, cellsY, cellsZ = 0;
    cuboid(Vector3 p0_, Vector3 p1_, Vector3 p2_, Vector3 p3_, Vector3 p4_, Vector3 p5_, Vector3 p6_, Vector3 p7_, int cellsX_, int cellsY_, int cellsZ_, Vector3 cellGrading_, double p_, double T_, double U_) {
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
    cuboid(quad q0, quad q1, int cellsX_, int cellsY_, int cellsZ_, Vector3 cellGrading_) {
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
  const double ATMOSPHERE_T = 300;
  const Vector3 ATMOSPHERE_U = Vector3(0,0,0);

  const double CHAMBER_p = 140;
  const double CHAMBER_T = 600; // NOT ACTIVE
  const Vector3 CHAMBER_U = Vector3(0,0,100);

  static Vector3 pointAt(int zIndex, int radialIndex, vector<vector<double>> diameterHeights, double totalHeight, double radialPointCount) {
    double totalHeightWeight = 0;
    for (vector<double> diameterHeight : diameterHeights) {
      totalHeightWeight += diameterHeight[1];
    }
    double currentHeightFraction = 0;
    for (int i = 0; i <= zIndex; i++) {
      currentHeightFraction += diameterHeights[i][1];
    }
    currentHeightFraction /= totalHeightWeight;
    //double z = zIndex * (totalHeight/(diameterHeights.size() - 1));
    double z = currentHeightFraction * totalHeight;
    double angle = 2 * M_PI * radialIndex / radialPointCount;
    double x = sin(angle) * diameterHeights[zIndex][0];
    double y = cos(angle) * diameterHeights[zIndex][0];
    return Vector3(x,y,z);
  }

  static int getPointIndex(Vector3 &x, vector<Vector3> &v) {
    int index = -1;
    for (int i = 0; i < v.size(); i++) {
      if (v[i].isEqual(x)) {
        index = i;
      }
    }
    return index;
  }

  static void addPointsToList(Vector3 &x, vector<Vector3> &v) {
    int index = getPointIndex(x, v);
    if (index == -1) v.push_back(x);
  }

  static int getNum(Vector3 &x, vector<Vector3> &v) {
    return v[getPointIndex(x, v)].num;
  }

  static void createModel(vector<vector<double>> diameterHeights, int radialPointCount, vector<double> &cellResolutionsPerUnit, vector<double> &cellResolutionDistribution) {
    
    double totalHeight = 0;
    for (vector<double> dh : diameterHeights) {
      totalHeight += dh[1];
    }

    // generate a list of edge faces, or "walls" in the simulation

    vector<quad> edgeFaces;
    vector<quad> bottomFaces;
    vector<quad> bottomSideFaces;
    vector<quad> topFaces;
    vector<quad> topSideFaces;

    cout << "diameterHeights size: " + to_string(diameterHeights.size()) + " fujadshjfasdhjklf;jhasdfjhksadhfjl\n" << endl;
    for (double zIndex = 0; zIndex < diameterHeights.size() - 1; zIndex++) {
      for (double radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
        quad q = quad(
          pointAt(zIndex, radialIndex, diameterHeights, totalHeight, radialPointCount),
          pointAt(zIndex, radialIndex+1, diameterHeights, totalHeight, radialPointCount),
          pointAt(zIndex+1, radialIndex+1, diameterHeights, totalHeight, radialPointCount),
          pointAt(zIndex+1, radialIndex, diameterHeights, totalHeight, radialPointCount),
          zIndex
        ); // put in both bottom/top and bottom/top-side because the 
        if (diameterHeights[zIndex+1][2] == 0) { // chamber
          bottomFaces.push_back(q);
          bottomSideFaces.push_back(q);
        } else if (diameterHeights[zIndex+1][2] == 1) { // nozzle
          edgeFaces.push_back(q);
        } else { // atmosphere
          topFaces.push_back(q);
          topSideFaces.push_back(q);
        }
      }
    }

    // generate faces for the top + bottom endcaps

    for (double radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
      bottomFaces.push_back(quad(
        pointAt(0, radialIndex, diameterHeights, totalHeight, radialPointCount),
        pointAt(0, radialIndex+1, diameterHeights, totalHeight, radialPointCount),
        pointAt(0, radialIndex+1, diameterHeights, totalHeight, radialPointCount).scaleXY(0),
        pointAt(0, radialIndex, diameterHeights, totalHeight, radialPointCount).scaleXY(0),
        0
      ));
    }



    for (double radialIndex = 0; radialIndex < radialPointCount; radialIndex++) {
      topFaces.push_back(quad(
        pointAt(diameterHeights.size()-1, radialIndex, diameterHeights, totalHeight, radialPointCount),
        pointAt(diameterHeights.size()-1, radialIndex+1, diameterHeights, totalHeight, radialPointCount),
        pointAt(diameterHeights.size()-1, radialIndex+1, diameterHeights, totalHeight, radialPointCount).scaleXY(0),
        pointAt(diameterHeights.size()-1, radialIndex, diameterHeights, totalHeight, radialPointCount).scaleXY(0),
        diameterHeights.size()
      ));
    }

    // generate a list of cuboid segments, or "cells" in the simulation

    vector<cuboid> cells;
    vector<cuboid> atmoCells;

    for (quad face : edgeFaces) {
      cells.push_back(cuboid(
        face, // 1 first instead of 0 because OpenFOAM uses a normalized vector between point 0 and the points around it, which would have a distance of 0 if the 0-scaled face was first
        face.scaleXY(0), // I have a feeling that a scaling factor of 0, turning a cuboid into a wedge, would make OpenFOAM crash so a very small number will do in case this breaks it
        ///ceil(cellResolutionsPerUnit[0] * diameterHeights[face.zIndexAtGeneration][0]), // {side-to-side radius-wise,
        ///ceil(cellResolutionsPerUnit[1] * diameterHeights[face.zIndexAtGeneration][1]), //  vertical,
        ///ceil(cellResolutionsPerUnit[2] * diameterHeights[face.zIndexAtGeneration][0]), //  in-out}
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * diameterHeights[face.zIndexAtGeneration+1][1] * diameterHeights[face.zIndexAtGeneration+1][3]),
        cellResolutionsPerUnit[2],
        Vector3(cellResolutionDistribution)
      ));
    }

    for (quad face : bottomSideFaces) {
      cells.push_back(cuboid(
        face, // 1 first instead of 0 because OpenFOAM uses a normalized vector between point 0 and the points around it, which would have a distance of 0 if the 0-scaled face was first
        face.scaleXY(0), // I have a feeling that a scaling factor of 0, turning a cuboid into a wedge, would make OpenFOAM crash so a very small number will do in case this breaks it
        ///ceil(cellResolutionsPerUnit[0] * diameterHeights[face.zIndexAtGeneration][0]), // {side-to-side radius-wise,
        ///ceil(cellResolutionsPerUnit[1] * diameterHeights[face.zIndexAtGeneration][1]), //  vertical,
        ///ceil(cellResolutionsPerUnit[2] * diameterHeights[face.zIndexAtGeneration][0]), //  in-out}
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * diameterHeights[face.zIndexAtGeneration+1][1] * diameterHeights[face.zIndexAtGeneration+1][3]),
        cellResolutionsPerUnit[2],
        Vector3(cellResolutionDistribution)
      ));
    }

    for (quad face : topSideFaces) {
      cells.push_back(cuboid(
        face, // 1 first instead of 0 because OpenFOAM uses a normalized vector between point 0 and the points around it, which would have a distance of 0 if the 0-scaled face was first
        face.scaleXY(0), // I have a feeling that a scaling factor of 0, turning a cuboid into a wedge, would make OpenFOAM crash so a very small number will do in case this breaks it
        ///ceil(cellResolutionsPerUnit[0] * diameterHeights[face.zIndexAtGeneration][0]), // {side-to-side radius-wise,
        ///ceil(cellResolutionsPerUnit[1] * diameterHeights[face.zIndexAtGeneration][1]), //  vertical,
        ///ceil(cellResolutionsPerUnit[2] * diameterHeights[face.zIndexAtGeneration][0]), //  in-out}
        cellResolutionsPerUnit[0],
        ceil(cellResolutionsPerUnit[1] * diameterHeights[face.zIndexAtGeneration+1][1] * diameterHeights[face.zIndexAtGeneration+1][3]),
        cellResolutionsPerUnit[2],
        Vector3(cellResolutionDistribution)
      ));
    }

    // since OpenFOAM uses a list of points to construct faces...
    // get all unique vectors used and enumerate them

    vector<Vector3> points = {};

    for (quad face : edgeFaces) {
      addPointsToList(face.p0, points);
      addPointsToList(face.p1, points);
      addPointsToList(face.p2, points);
      addPointsToList(face.p3, points);
    }

    /*for (quad face : topSideFaces) {
      addPointsToList(face.p0, points);
      addPointsToList(face.p1, points);
      addPointsToList(face.p2, points);
      addPointsToList(face.p3, points);
    }*/

    for (quad face : bottomFaces) {
      addPointsToList(face.p0, points);
      addPointsToList(face.p1, points);
      addPointsToList(face.p2, points);
      addPointsToList(face.p3, points);
    }

    for (quad face : topFaces) {
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

    for (int i = 0; i < edgeFaces.size(); i++) {
      edgeFaces[i].p0.num = getNum(edgeFaces[i].p0, points);
      edgeFaces[i].p1.num = getNum(edgeFaces[i].p1, points);
      edgeFaces[i].p2.num = getNum(edgeFaces[i].p2, points);
      edgeFaces[i].p3.num = getNum(edgeFaces[i].p3, points);
      //cout << quadToString(edgeFaces[i]) + "\n";
    }

    /*for (int i = 0; i < topSideFaces.size(); i++) {
      topSideFaces[i].p0.num = getNum(topSideFaces[i].p0, points);
      topSideFaces[i].p1.num = getNum(topSideFaces[i].p1, points);
      topSideFaces[i].p2.num = getNum(topSideFaces[i].p2, points);
      topSideFaces[i].p3.num = getNum(topSideFaces[i].p3, points);
      //cout << quadToString(edgeFaces[i]) + "\n";
    }*/

    for (int i = 0; i < bottomFaces.size(); i++) {
      bottomFaces[i].p0.num = getNum(bottomFaces[i].p0, points);
      bottomFaces[i].p1.num = getNum(bottomFaces[i].p1, points);
      bottomFaces[i].p2.num = getNum(bottomFaces[i].p2, points);
      bottomFaces[i].p3.num = getNum(bottomFaces[i].p3, points);
      //cout << quadToString(edgeFaces[i]) + "\n";
    }

    for (int i = 0; i < topFaces.size(); i++) {
      topFaces[i].p0.num = getNum(topFaces[i].p0, points);
      topFaces[i].p1.num = getNum(topFaces[i].p1, points);
      topFaces[i].p2.num = getNum(topFaces[i].p2, points);
      topFaces[i].p3.num = getNum(topFaces[i].p3, points);
      //cout << quadToString(edgeFaces[i]) + "\n";
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

    vector<quad> empty = {};

    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "vertices", vector3ListToString(points));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "inlet", boundaryToString("patch", bottomFaces));
    //FileEditor::addTextToSectionInFile("/system/blockMeshDict", "inlet", boundaryToString("patch", empty));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "outlet", boundaryToString("patch", topFaces));
    /*for (quad q : edgeFaces) {
      cout << vector3ToString(q.p0) + "\n";
      cout << vector3ToString(q.p1) + "\n";
      cout << vector3ToString(q.p2) + "\n";
      cout << vector3ToString(q.p3) + "\n";
      cout << "--------------\n";
    }
    for (cuboid c : cells) {
      cout << vector3ToString(c.p0) + "\n";
      cout << vector3ToString(c.p1) + "\n";
      cout << vector3ToString(c.p2) + "\n";
      cout << vector3ToString(c.p3) + "\n";
      cout << vector3ToString(c.p4) + "\n";
      cout << vector3ToString(c.p5) + "\n";
      cout << vector3ToString(c.p6) + "\n";
      cout << vector3ToString(c.p7) + "\n";
      cout << "--------------\n";
    }*/
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "blocks", cuboidListToString(cells));
    //FileEditor::addTextToSectionInFile("/system/blockMeshDict", "atmo", cuboidListToString(atmoCells));
    FileEditor::addTextToSectionInFile("/system/blockMeshDict", "sides", boundaryToString("wall", edgeFaces));
    //FileEditor::addTextToSectionInFile("/system/blockMeshDict", "atmoedge", boundaryToString("noSlip", topSideFaces));
    

  }

  static string vector3ToString(Vector3 v) {
    return "(" + to_string(v.x) + " " + to_string(v.y) + " " + to_string(v.z) + ")";
  }

  static string vector3ListToString(vector<Vector3> vectors) {
    string output = "";
    for (Vector3 v : vectors) {
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