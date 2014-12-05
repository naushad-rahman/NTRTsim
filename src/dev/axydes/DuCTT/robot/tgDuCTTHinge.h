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

#ifndef TG_ROD_HINGE_H
#define TG_ROD_HINGE_H

/**
 * @file tgDuCTTHinge.h
 * @brief Contains the definition of class tgHinge. A hinge joint.
 * @author Alexander Xydes
 * @copyright Copyright (C) 2014 NASA Ames Research Center
 * $Id$
 */

#include "core/tgModel.h"
#include "core/tgSubject.h"

class tgWorld;
class btHingeConstraint;
class btVector3;

class tgDuCTTHinge: public tgSubject<tgDuCTTHinge>, public tgModel
{
public: 
    
    struct Config
    {
    public:
        // To make the complier happy. Probably should never be called
        Config();

        Config(
                double minimum,
                double maximum,
                btVector3 axis = btVector3(1,0,0),
                bool useMotor = false,
                double maxMotorVelocity = 0.01,
                double maxMotorImpulse = 20,
                double eps = 0.01,
                double softness = 0.9,
                double bias = 0.3,
                double relaxation = 1.0
                );

        /**
         * Specifies the maximum angle of the joint.
         * Units are radians
         */
        double m_maximum;

        /**
         * Specifies the minimum angle of the joint.
         * Units are radians
         */
        double m_minimum;

        /**
         * Axis to rotate the hinge around, by default X axis
         */
        btVector3 m_axis;

        /**
         * Enable the joint motor.
         */
        bool m_useMotor;

        /**
         * Specifies the maximum impulse that the joint motor can expend.
         */
        double m_maxMotorImpulse;

        /**
         * Specifies the maximum velocity of the joint motor.
         * Units are radians/second
         */
        double m_maxMotorVelocity;

        /**
         * Specifies the maximum distance allowed between joint lengths to be
         * considered equal.
         * Units are radians
         */
        double m_eps;

        /**
         * Bullet Hinge constraint limit param: softness
         * 0->1
         * Not sure what this is for.
         */
        double m_softness;

        /**
         * Bullet Hinge constraint limit param: biasFactor
         * 0->1
         * Multiplier for constraint error, (constraint becomes more "soft" when
         * this factor is close to zero.
         */
        double m_bias;

        /**
         * Bullet Hinge constraint limit param: relaxationFactor
         * 0->1
         * Used to control bounce: 1.0 means full bounce, 0.0 means no bounce
         */
        double m_relaxation;
    };
    
    /**
     * Constructor using tags.
     * @param[in] constraint The btHingeConstraint object that this controls and logs.
     * Set up in tgHingeInfo.cpp
     * @param[in] tags as passed through tgStructure and tgStructureInfo
     * @param[in] config Holds member variables like min/max length, max force
     * and velocity, and the epsilon value to use when comparing joint distance.
     */
    tgDuCTTHinge(
        btHingeConstraint* constraint,
        const tgTags& tags,
        tgDuCTTHinge::Config& config);
    
    /**
     * Calls teardown
     */
    virtual ~tgDuCTTHinge();

    /**
     * Uses the Config object to set the maximum/minimum limits of the joint,
     * turns on the joint motor and sets the desired length to the minimum.
     */
    virtual void init();
    
    /**
     * Notifies observers of setup, calls setup on children
     * @param[in] world, the tgWorld the models are being built into
     */
    virtual void setup(tgWorld& world);

    /**
     * Notifies observers of teardown, teardown any children
     */
    virtual void teardown();

    /**
     * Step dt forward with the simulation.
     * Notifies observers of step, calls moveMotors, steps children.
     * @param[in] dt, must be >= 0.0
     */
    virtual void step(double dt);

    /**
     * Directly set the desired angle of the joint, does not call moveMotors
     * @param angle, desired angle of the joint. Units are radians.
     * @return true if successfully set desired angle of the joint, false if it defaulted to
     *      a maximum or minimum or failed.
     */
    virtual bool setPreferredAngle(double angle);

    /**
     * Directly set the maximum velocity of the motor attached
     * to the hinge joint. Does not call moveMotors.
     * @param vel, the desired max velocity of the joint motor
     */
    void setMaxVelocity(double vel);

    /**
     * Directly set the maximum impulse of the motor attached
     * to the hinge joint. Does not call moveMotors.
     * @param force, the desired max force of the joint motor
     */
    void setMaxImpulse(double impulse);

    /**
     * Called from public functions, it makes the actual length get closer
     * to m_preferredlength, according to config constraints.
     */
    virtual void moveMotors(double dt);

private:
    /**
     * A copy of the configuration POD supplied at constuction.
     * This is not const.
     */
    Config m_config;

    btHingeConstraint* m_hinge;

    /**
     * Hold the desired angle of the joint.
     */
    double m_preferredAngle;
};

#endif // TG_HINGE_H