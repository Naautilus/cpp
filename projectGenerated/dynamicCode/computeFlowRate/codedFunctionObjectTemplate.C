/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) YEAR OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "codedFunctionObjectTemplate.H"
#include "volFields.H"
#include "unitConversion.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(computeFlowRateFunctionObject, 0);

addRemovableToRunTimeSelectionTable
(
    functionObject,
    computeFlowRateFunctionObject,
    dictionary
);


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

extern "C"
{
    // dynamicCode:
    // SHA1 = 03321df1f56424215c53776f2ee61f551203d234
    //
    // unique function name that can be checked if the correct library version
    // has been loaded
    void computeFlowRate_03321df1f56424215c53776f2ee61f551203d234(bool load)
    {
        if (load)
        {
            // code that can be explicitly executed after loading
        }
        else
        {
            // code that can be explicitly executed before unloading
        }
    }
}


// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //

//{{{ begin localCode

//}}} end localCode


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

const fvMesh& computeFlowRateFunctionObject::mesh() const
{
    return refCast<const fvMesh>(obr_);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

computeFlowRateFunctionObject::computeFlowRateFunctionObject
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    functionObjects::regionFunctionObject(name, runTime, dict)
{
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

computeFlowRateFunctionObject::~computeFlowRateFunctionObject()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool computeFlowRateFunctionObject::read(const dictionary& dict)
{
    if (false)
    {
        Info<<"read computeFlowRate sha1: 03321df1f56424215c53776f2ee61f551203d234\n";
    }

//{{{ begin code
    
//}}} end code

    return true;
}


Foam::wordList computeFlowRateFunctionObject::fields() const
{
    if (false)
    {
        Info<<"fields computeFlowRate sha1: 03321df1f56424215c53776f2ee61f551203d234\n";
    }

    wordList fields;
//{{{ begin code
    
//}}} end code

    return fields;
}


bool computeFlowRateFunctionObject::execute()
{
    if (false)
    {
        Info<<"execute computeFlowRate sha1: 03321df1f56424215c53776f2ee61f551203d234\n";
    }

//{{{ begin code
    
//}}} end code

    return true;
}


bool computeFlowRateFunctionObject::write()
{
    if (false)
    {
        Info<<"write computeFlowRate sha1: 03321df1f56424215c53776f2ee61f551203d234\n";
    }

//{{{ begin code
    #line 93 "/home/matth/OpenFOAM/matth-11/run/foamTest2/system/controlDict/functions/computeFlowRate"
//const volScalarField rho = mesh().boundaryMesh()//.lookupObject<volScalarField>("rho");
        //const volVectorField U = mesh().boundaryMesh()//.lookupObject<volVectorField>("U");
        //forAll(mesh.boundaryMesh(), patchI) {
        //  const scalarField A = mesh().boundaryMesh().faceAreas();
        //}
        //
		    //auto FlowRate = A; //U.component(2)*rho;
		    //FlowRate.write();


        //using namespace std;

        label ibcFace = 10000; // this will change if the simulation resolution is changed!

        scalar xbc[ibcFace];
        scalar ybc[ibcFace];
        scalar zbc[ibcFace];

        FILE *f = fopen("faceAreas", "w");

        forAll(mesh().boundaryMesh(), patchI) {
          forAll(mesh().boundaryMesh()[patchI].faceAreas(), faceI) {
            xbc[faceI] = mesh().boundaryMesh()[patchI].faceAreas()[faceI].x();
            ybc[faceI] = mesh().boundaryMesh()[patchI].faceAreas()[faceI].y();
            zbc[faceI] = mesh().boundaryMesh()[patchI].faceAreas()[faceI].z();
            //std::cout << std::to_string(zbc[faceI]) + "\n";
            if (patchI == 1) {
              fprintf(f, "%.17g\n", std::sqrt(xbc[faceI]*xbc[faceI] + ybc[faceI]*ybc[faceI] + zbc[faceI]*zbc[faceI]));
            }
          }
        }

        fclose(f);

        //zbc.write();




        //std::ofstream o_file_bc;
        //o_file_bc.open("Boundary_Face_Vectors.txt");
        //if(o_file_bc.is_open()) {
        //  for(label i=0;i<=ibcFace-1;i++) { 
        //    o_file_bc<<xbc[i]<<" "<<ybc[i]<<" "<<zbc[i]<<" "<<endl;
        //  } 
        //}

        //Foam::tmp<Foam::scalarField> Foam::cellQuality::cellSurfaceArea() const {
        //  tmp<scalarField> tresult (new scalarField (mesh_.nCells(), 0.0));
        //  scalarField& result = tresult.ref();
        //  scalar sumCellArea;
        //  forAll(result,celli) {
        //  const labelList& cellFaces = mesh_.cells()[celli]; //with this line, I expect to access the faces belonging to cell under consideration.
        //  const vectorField& areas = mesh_.faceAreas();
        //  forAll(cellFaces,facei) {
        //    sumCellArea=0;
        //    sumCellArea+=mag(areas[facei]);
        //  }
        //  result[celli]=sumCellArea;
        //  }
        //  return tresult;
        //}
//}}} end code

    return true;
}


bool computeFlowRateFunctionObject::end()
{
    if (false)
    {
        Info<<"end computeFlowRate sha1: 03321df1f56424215c53776f2ee61f551203d234\n";
    }

//{{{ begin code
    
//}}} end code

    return true;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //

