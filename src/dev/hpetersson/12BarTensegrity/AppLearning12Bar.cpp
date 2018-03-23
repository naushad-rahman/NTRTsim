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
 * @file AppLearning12Bar.cpp
 * @brief Contains the definition function main() for the 12 Bar Learning
 * application.
 * @author Hannah Petersson with code based from Brian Tietz and Mallory Daly
 * $Id$
 */

// This application
#include "ModelLearning12Bar.h"  /* Here, the yaml files should be instead */
// This library  
#include "core/terrain/tgBoxGround.h"
#include "core/tgModel.h"
#include "core/tgSimView.h" // Is this library necessary?
#include "core/tgSimViewGraphics.h"
#include "core/tgSimulation.h"
#include "core/tgWorld.h"
// The C++ Standard Library
#include <iostream>
#include <string>
#include <vector>
// Controllers
#include "examples/learningSpines/BaseSpineCPGControl.h" 	/* Change controller to Length..Con..12Bar..cpp */

/**
 * The entry point.
 * @param[in] argc the number of command-line arguments
 * @param[in] argv argv[0] is the executable name; argv[1], if supplied, is the
 * suffix for the controller
 * @return 0
 */
int main(int argc, char** argv)
{
    std::cout << "AppLearning12Bar" << std::endl;

    /* Insert the following for checking YAML file:
    // For this YAML parser app, need to check that an argument path was
    // passed in.
    if (argv[1] == NULL)
    {
      throw std::invalid_argument("No arguments passed in to the application. You need to specify which YAML file you wouldd like to build.");
    }*/

    // Create the ground and world. Specify ground rotation in radians
    const double yaw = 0.0;
    const double pitch = 0.0;
    const double roll = 0.0;
    const tgBoxGround::Config groundConfig(btVector3(yaw, pitch, roll));
    // the world will delete this
    tgBoxGround* ground = new tgBoxGround(groundConfig);

    // First create the world
    const tgWorld::Config config(981); // gravity, cm/sec^2 	/* OBS note the unit */
    tgWorld world(config); 

    // Second create the view
    const double timestep_physics = 1.0/1000.0; // Seconds 		/* Note timestep, could be divided by 10000 */
    const double timestep_graphics = 1.0/60.0; // Seconds		
    tgSimViewGraphics view(world, timestep_physics, timestep_graphics);

    // Third create the simulation
    tgSimulation simulation(view);

    // Fourth create the models with their controllers and add the models to the
    // simulation
    const int segments = 12; 			/* Here, add Tensegrity model insted */
    ModelLearning12Bar* myModel =
      new ModelLearning12Bar(segments);
    
    // Required for setting up learning file input/output. 			/* This is new */
    const std::string suffix((argc > 1) ? argv[1] : "default");
    
    const int segmentSpan = 3;							/* All of these are input parameters */
    const int numMuscles = 4;							/* to the CPG controller. Will probably */
    const int numParams = 2;							/* be able to delete all of this. */
    const int segNumber = 0;							/* From here... */
    
    const double controlTime = 0.1;
    const double lowPhase = -1 * M_PI;
    const double highPhase = M_PI;
    const double lowAmplitude = 0.0;
    const double highAmplitude = 30.0;
    
    const double tension = 0.0;
    const double kPosition = 400.0;
    const double kVelocity = 40.0; 

    BaseSpineCPGControl::Config control_config(segmentSpan, numMuscles, numMuscles, numParams, segNumber, controlTime,
												lowAmplitude, highAmplitude, lowPhase, highPhase,
												tension, kPosition, kVelocity);
										/* ... to here */
	
	/* Here, initialize parameters for LengthController */

    BaseSpineCPGControl* const myControl =
      new BaseSpineCPGControl(control_config, suffix, "learningSpines/OctahedralComplex/");    	/* Change to lengthcontroller here */

/* If sensor data logger wanted, add this:
    // Create data logger
    std::string log = "~/12-bar-tensegrity/NTRT_logs/log";
    // double samplingTime = 0.1;
    tgDataLogger2* myDataLogger = new tgDataLogger2(log);
    myDataLogger->addSenseable(myModel);
    // Create two sensor infos, one for tgRods and other for tgSpringCableActuators
    tgRodSensorInfo* myRodSensorInfo = new tgRodSensorInfo();
    tgSpringCableActuatorSensorInfo* mySCASensorInfo = new tgSpringCableActuatorSensorInfo();
    myDataLogger->addSensorInfo(myRodSensorInfo);
    myDataLogger->addSensorInfo(mySCASensorInfo);

    // Add data logger to the world
    // simulation.addDataManager(myDataLogger); // comment/uncomment to record data
*/

    // Attach the controller to the model
    myModel->attach(myControl);
    
    // Add the model to the world
    simulation.addModel(myModel);
    
    // Run simulation
    int i = 0;	
    while (i < 20000)			/* I'm guessing this is to not let it run in all eternity */
    {
        simulation.run(60000);
        simulation.reset();
        i++;
    }
    
    /// @todo Does the model assume ownership of the controller?		/* What is this? */
    /** No - a single controller could be attached to multiple subjects
    * However, having this here causes a segfault, since there is a call
    * to onTeardown() when the simulation is deleted
    */
    #if (0)
    delete myControl;
    #endif

    //Teardown is handled by delete, so that should be automatic
    return 0;
}
