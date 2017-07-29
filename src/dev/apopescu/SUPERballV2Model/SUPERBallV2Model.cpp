/*
 * Copyright © 2012, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All rights reserved.
 * 
 * The NASA Tensegrity Robotics Toolkit (NTRT) v1 platform is licensed
 * under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
*/

/**
 * @file SUPERBallV2Model.cpp
 * @brief Contains the implementation of class SUPERBallV2Model.
 * $Id$
 */

// This module
#include "SUPERBallV2Model.h"
// This library
#include "core/tgBasicActuator.h"
#include "core/tgRod.h"
#include "core/tgBox.h"
#include "core/tgSphere.h"
#include "tgcreator/tgBuildSpec.h"
#include "tgcreator/tgBasicActuatorInfo.h"
#include "tgcreator/tgRodInfo.h"
#include "tgcreator/tgBoxInfo.h"
#include "tgcreator/tgSphereInfo.h"
#include "tgcreator/tgStructure.h"
#include "tgcreator/tgStructureInfo.h"
// The Bullet Physics library
#include "LinearMath/btVector3.h"
// The C++ Standard Library
#include <stdexcept>
#include <cmath>

namespace
{
    // see tgBasicActuator and tgRod for a descripton of these rod parameters
    // (specifically, those related to the motor moving the strings.)
    // NOTE that any parameter that depends on units of length will scale
    // with the current gravity scaling. E.g., with gravity as 98.1,
    // the length units below are in decimeters.

    // Note: This current model of the SUPERball rod is 1.5m long by 3 cm radius,
    // which is 0.00424 m^3.
    // For SUPERball v1.5, mass = 3.5kg per strut, which comes out to 
    // 0.825 kg / (decimeter^3).

    // similarly, frictional parameters are for the tgRod objects.
    const struct Config
    {
        double density;
        double radius;
        double stiffness;
        double damping;
        double rod_length;
        double rod_space;    
        double friction;
        double rollFriction;
        double restitution;
        double pretension;
        bool   hist;
        double maxTens;
        double targetVelocity;
    } c =
   {
     6/(M_PI*(0.05/2.0*10.0)*(0.05/2.0*10.0)*1.65*10.0)/5,    // density (kg / length^3)
     0.05/2.0*10,     // radius (deca-meters)
     1000.0,   // stiffness (kg / sec^2) was 1500
     20.0,    // damping (kg / sec)
     1.65*10.0,     // rod_length (length)
     7.5,      // rod_space (length)
     0.9,      // friction (unitless)
     0.01,     // rollFriction (unitless)
     0.0,      // restitution (?)
     0,        // pretension -> set to 4 * 613, the previous value of the rest length controller
     0,			// History logging (boolean)
     100000,   // maxTens
     10000,    // targetVelocity

     // Use the below values for earlier versions of simulation.
     // 1.006,    
     // 0.31,     
     // 300000.0, 
     // 3000.0,   
     // 15.0,     
     // 7.5,      
  };
} // namespace

void addBarNodesWithCoMAzDec(tgStructure& s, double comx, double comy, double comz,
                             double azimuth, double declination,
                             double length);

SUPERBallV2Model::SUPERBallV2Model() : tgModel() 
{
}

SUPERBallV2Model::~SUPERBallV2Model()
{
}

void SUPERBallV2Model::addNodes(tgStructure& s)
{   
    // Initial starting configuration:
    double l = c.rod_length;
    /* Hexagonal packing: */
    double b = l*sqrt(3.0/8.0);
    //b = 0.2*l;
    double delta = acos(1.0/sqrt(3.0));
    delta = 88.0/180.0 * M_PI;
    double alpha = M_PI/3.0;
    alpha = 62.0/180.0*M_PI;
    
    
    double u = sin(delta) * cos(alpha + M_PI/6.0);
    double h = cos(delta)/(2.0*u) * (l*u + sqrt(b*b/3.0 - 3.0*l*l*u*u) - b/sqrt(3.0));
    
    // The bar labeling and parametrization come from (Sultan 2001).
    std::cout << "nodes = [";
    // Bar 11, nodes 0, 1
    addBarNodesWithCoMAzDec(s, l/2.0*sin(delta)*cos(alpha)-b/2.0, 
                            l/2.0*sin(delta)*sin(alpha) - b/(2.0*sqrt(3.0)),
                            l/2.0*cos(delta),
                            alpha, delta, l);
    // Bar 21, nodes 2, 3
    addBarNodesWithCoMAzDec(s, l/2.0*sin(delta)*cos(alpha + 4.0*M_PI/3.0),
                            b/sqrt(3.0) + l/2.0*sin(delta)*sin(alpha + 4.0*M_PI/3.0),
                            l/2.0*cos(delta),
                            alpha + 4.0*M_PI/3.0, delta, l);
    // Bar 31, nodes 4, 5
    addBarNodesWithCoMAzDec(s, b/2.0 + l/2.0*sin(delta)*cos(alpha + 2.0*M_PI/3.0),
                            l/2.0*sin(delta)*sin(alpha + 2.0*M_PI/3.0) - b/(2.0*sqrt(3.0)),
                            l/2.0*cos(delta),
                            alpha + 2.0*M_PI/3.0, delta, l);
    // Bar 12, nodes 6, 7
    addBarNodesWithCoMAzDec(s,
                   l/4*sin(delta)*cos(alpha) + sqrt(3.0)/4.0*l*sin(delta)*sin(alpha) - b/2.0,
     b/(2.0*sqrt(3.0)) - sqrt(3.0)/4.0*l*sin(delta)*cos(alpha) + l/4.0*sin(delta)*sin(alpha),
                            3.0/2.0*l*cos(delta) - h,
                            alpha + 2.0*M_PI/3.0, delta, l);
    // Bar 22, nodes 8, 9
    addBarNodesWithCoMAzDec(s, b/2.0 - l/2.0*sin(delta)*cos(alpha),
                            b/(2.0*sqrt(3.0)) - l/2.0*sin(delta)*sin(alpha),
                            3.0/2.0*l*cos(delta) - h,
                            alpha, delta, l);
    // Bar 32, nodes 10, 11
    addBarNodesWithCoMAzDec(s,
                        l/4.0*sin(delta)*cos(alpha) - sqrt(3.0)/4.0*l*sin(delta)*sin(alpha),
           l/4.0*sin(delta)*sin(alpha) + sqrt(3.0)/4.0*l*sin(delta)*cos(alpha) - b/sqrt(3.0),
                            3.0/2.0*l*cos(delta) - h,
                            alpha + 4.0*M_PI/3.0, delta, l);
    std::cout << "]" << std::endl;
}

void SUPERBallV2Model::addRods(tgStructure& s)
{
    s.addPair( 0,  1, "rod");
    s.addPair( 2,  3, "rod");
    s.addPair( 4,  5, "rod");
    s.addPair( 6,  7, "rod");
    s.addPair( 8,  9, "rod");
    s.addPair( 10, 11, "rod");
}

void SUPERBallV2Model::addActuators(tgStructure& s)
{
    // Saddle
    s.addPair(5, 8,  "muscle"); // 0
    s.addPair(8, 3,  "muscle");
    s.addPair(3, 6,  "muscle");
    s.addPair(6, 1,  "muscle");
    s.addPair(1, 10,  "muscle");
    s.addPair(10, 5,  "muscle");
    
    // Vertical
    s.addPair(0, 5,  "muscle"); // 6
    s.addPair(4, 3,  "muscle");
    s.addPair(2, 1,  "muscle");
    s.addPair(10, 7,  "muscle");
    s.addPair(6, 9,  "muscle");
    s.addPair(8, 11,  "muscle");
    
    // Diagonal
    s.addPair(0, 10,  "muscle"); // 12
    s.addPair(2, 6,  "muscle");
    s.addPair(4, 8,  "muscle");
    s.addPair(1, 7,  "muscle");
    s.addPair(3, 9,  "muscle");
    s.addPair(5, 11,  "muscle");
    
    // Boundary/Base
    s.addPair(0, 4,  "muscle"); // 18
    s.addPair(4, 2,  "muscle");
    s.addPair(2, 0,  "muscle");
    s.addPair(7, 11,  "muscle");
    s.addPair(11, 9,  "muscle");
    s.addPair(9, 7,  "muscle");
}


void SUPERBallV2Model::setup(tgWorld& world)
{
    // Rod properties
    tgRod::Config rodConfig(c.radius, c.density, c.friction, 
				c.rollFriction, c.restitution);
    
    // Actuator/string properties
    tgBasicActuator::Config muscleConfig(c.stiffness, c.damping, c.pretension, c.hist, 
					    c.maxTens, c.targetVelocity);
					    /*
	double motorWidth = 0.05*10;
	double motorHeight = 0.05*10;
	double motorDensity = 2.0 / 10.0;
	double motorFriction = 0.9;
	double motorRollfriction = 0.01;
	double motorRestitution = 0;
	tgBox::Config motorConfig(motorWidth, motorHeight, motorDensity, motorFriction, motorRollfriction, motorRestitution);
	
	double endcapRadius = 0.2*10;
	double endcapDensity = 0.5*10;
	double endcapFriction = 0.9;
	double endcapRollfriction = 0.01;
	double endcapRestitution = 0;
	tgSphere::Config endcapConfig(endcapRadius, 2, endcapFriction, endcapRollfriction, endcapRestitution);
    
    // Create the build spec that uses tags to turn the structure into a real model
    tgBuildSpec spec;
    spec.addBuilder("cylinder", new tgRodInfo(rodConfig));
    spec.addBuilder("muscle", new tgBasicActuatorInfo(muscleConfig));
    spec.addBuilder("motor", new tgBoxInfo(motorConfig));
    spec.addBuilder("endcap", new tgSphereInfo(endcapConfig));
    
    // Create a single SUPERball rod
    tgStructure SBrod;
    double barlength = 1.6*10.0; // deca-meters
    SBrod.addNode(0, 0, 0, "endcap"); // 0
    //SBrod.addNode(barlength, 0, 0); // 1
    //SBrod.addPair(0, 1, "endcap");
    
    SBrod.move(btVector3(0, 5, 0));
    
    // Create a structureInfo structure to represent our model
    tgStructureInfo structureInfo(SBrod, spec);

    // Use the structureInfo to build models into the Bullet world
    structureInfo.buildInto(*this, world);

    // Pull out the models that we want to control. 
    allActuators = tgCast::filter<tgModel, tgBasicActuator> (getDescendants());

    // call the onSetup methods of all observed things e.g. controllers
    notifySetup();

    // Actually setup the children
    tgModel::setup(world);
    */
    
    
    // Start creating the structure
    tgStructure s;
    addNodes(s);
    addRods(s);
    addActuators(s);
    
    
    // Rotate the model in the world so the default view shows normal x, y, z orientation.
    btVector3 rotationPoint = btVector3(0, 0, 0); // origin
    btVector3 rotationAxis = btVector3(1, 0, 0); // x-axis rotation
    double rotationAngle = M_PI/2;
    s.addRotation(rotationPoint, rotationAxis, rotationAngle);
    rotationPoint = btVector3(0, 0, 0); // origin
    rotationAxis = btVector3(0, 0, 1); // z-axis rotation
    rotationAngle = M_PI;
    s.addRotation(rotationPoint, rotationAxis, rotationAngle);
    
    s.move(btVector3(0, 2, 0)); // Move CoM to a certain point.
    
    // Create the build spec that uses tags to turn the structure into a real model
    tgBuildSpec spec;
    spec.addBuilder("rod", new tgRodInfo(rodConfig));
    spec.addBuilder("muscle", new tgBasicActuatorInfo(muscleConfig));
    
    // Create your structureInfo
    tgStructureInfo structureInfo(s, spec);

    // Use the structureInfo to build ourselves
    structureInfo.buildInto(*this, world);

    // We could now use tgCast::filter or similar to pull out the
    // models (e.g. muscles) that we want to control. 
    allActuators = tgCast::filter<tgModel, tgBasicActuator> (getDescendants());

    // call the onSetup methods of all observed things e.g. controllers
    notifySetup();

    // Actually setup the children
    tgModel::setup(world);
    
}

void SUPERBallV2Model::step(double dt)
{
    // Precondition
    if (dt <= 0.0)
    {
        throw std::invalid_argument("dt is not positive");
    }
    else
    {
        // Notify observers (controllers) of the step so that they can take action
        notifyStep(dt);
        tgModel::step(dt);  // Step any children
    }
}

void addBarNodesWithCoMAzDec(tgStructure& s, double comx, double comy, double comz,
                             double azimuth, double declination,
                             double length)
{
    // Node 1:
    double nx = comx - length/2.0*sin(declination)*cos(azimuth);
    double ny = comy - length/2.0*sin(declination)*sin(azimuth);
    double nz = comz - length/2.0*cos(declination);
    std::cout << nx << "," << ny << "," << nz << std::endl;
    s.addNode(nx, ny, nz);
    
    // Node 2:
    nx = comx + length/2.0*sin(declination)*cos(azimuth);
    ny = comy + length/2.0*sin(declination)*sin(azimuth);
    nz = comz + length/2.0*cos(declination);
    std::cout << nx << "," << ny << "," << nz << std::endl;
    s.addNode(nx, ny, nz);
}

void SUPERBallV2Model::onVisit(tgModelVisitor& r)
{
    tgModel::onVisit(r);
}

const std::vector<tgBasicActuator*>& SUPERBallV2Model::getAllActuators() const
{
    return allActuators;
}
    
void SUPERBallV2Model::teardown()
{
    notifyTeardown();
    tgModel::teardown();
}
