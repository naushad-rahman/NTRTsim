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
 * @file NeuroEvolution.cpp
 * @brief Top level class for NeuroEvolution
 * @date August 2013
 * @author Atil Iscen
 * $Id$
 */

#include "NeuroEvolution.h"
#include "../Configuration/configuration.h"
#include <iostream>
#include <numeric>
#include <string>
#include <sstream>
#include <stdexcept>

using namespace std;

#ifdef _WIN32

//  Windows
#define rdtsc  __rdtsc

#else

//  For everything else
unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

#endif

NeuroEvolution::NeuroEvolution(string suff, string config)
{
	suffix=suff;
	currentTest=0;
	generationNumber=0;

	configuration myconfigdataaa;
	myconfigdataaa.readFile(config);
	populationSize=myconfigdataaa.getintvalue("populationSize");
	numberOfElementsToMutate=myconfigdataaa.getintvalue("numberOfElementsToMutate");
	numberOfTestsBetweenGenerations=myconfigdataaa.getintvalue("numberOfTestsBetweenGenerations");
	numberOfControllers=myconfigdataaa.getintvalue("numberOfControllers"); //shared with ManhattanToyController
	leniencyCoef=myconfigdataaa.getDoubleValue("leniencyCoef");
	coevolution=myconfigdataaa.getintvalue("coevolution");

	srand(rdtsc());
	eng.seed(rdtsc());

	for(int j=0;j<numberOfControllers;j++)
	{
		cout<<"creating Populations"<<endl;
		populations.push_back(new NeuroEvoPopulation(populationSize,myconfigdataaa));
	}
	evolutionLog.open(("logs/evolution"+suffix+".csv").c_str(),ios::out);
	if (!evolutionLog.is_open())
	{
		throw std::runtime_error("Logs does not exist. Please create a logs folder in your build directory or update your cmake file");
	}
}

NeuroEvolution::~NeuroEvolution()
{
	// @todo - solve the invalid pointer that occurs here
	#if (0)
	for(std::size_t i = 0; i < populations.size(); i++)
	{
		delete populations[i];
	}
	populations.clear();
	#endif
}

void NeuroEvolution::mutateEveryController()
{
	for(int i=0;i<populations.size();i++)
	{
		populations.at(i)->mutate(&eng,numberOfElementsToMutate);
	}
}



void NeuroEvolution::orderAllPopulations()
{
	generationNumber++;
	double aveScore1 = 0.0;
	double aveScore2 = 0.0;
#if (0)
	// Disable definition of unused variables to suppress compiler warning
	double maxScore1,maxScore2;
#endif
	for(int i=0;i<scoresOfTheGeneration.size();i++)
	{
		aveScore1+=scoresOfTheGeneration[i][0];
		aveScore2+=scoresOfTheGeneration[i][1];
	}
	aveScore1 /= scoresOfTheGeneration.size();
	aveScore2 /= scoresOfTheGeneration.size();


	for(int i=0;i<populations.size();i++)
	{
		populations.at(i)->orderPopulation();
	}
	evolutionLog<<generationNumber*numberOfTestsBetweenGenerations<<","<<aveScore1<<","<<aveScore2<<",";
	evolutionLog<<populations.at(0)->getMember(0)->maxScore<<","<<populations.at(0)->getMember(0)->maxScore1<<","<<populations.at(0)->getMember(0)->maxScore2<<endl;
	
	
	// what if member at 0 isn't the best of all time for some reason? 
	// This seems biased towards average scores
	ofstream logfileLeader;
	for(int i=0;i<populations.size();i++)
	{
		stringstream ss;
		ss<<"logs/bestParameters-"<<suffix<<"-"<<i<<".nnw";
//		populations[i]->getMember(0)->getNn()->saveWeights(ss.str().c_str());
		populations[i]->getMember(0)->saveToFile(ss.str().c_str());
	}
}

double diffclock(clock_t clock1,clock_t clock2)
{
	double diffticks=clock1-clock2;
	double diffms=(diffticks*10)/CLOCKS_PER_SEC;
	return diffms;
}

vector <NeuroEvoMember *> NeuroEvolution::nextSetOfControllers()
{
	int testsToDo=0;
	if(coevolution)
		testsToDo=numberOfTestsBetweenGenerations; //stop when we reach x amount of random tests
	else
		testsToDo=populationSize; //stop when we test each element once

	if(currentTest == testsToDo)
	{
		orderAllPopulations();
		mutateEveryController();
		cout<<"mutated the populations"<<endl;
		this->scoresOfTheGeneration.clear();

		if(coevolution)
			currentTest=0;//Start from 0
		else
			currentTest=populationSize-numberOfElementsToMutate; //start from the mutated ones only (last x)
	}

	selectedControllers.clear();
	for(int i=0;i<populations.size();i++)
	{
		int selectedOne=0;
		if(coevolution)
			selectedOne=rand()%populationSize; //select random one from each pool
		else
			selectedOne=currentTest; //select the same from each pool

//		cout<<"selected: "<<selectedOne<<endl;
		selectedControllers.push_back(populations.at(i)->getMember(selectedOne));
	}
	currentTest++;
//	cout<<"currentTest:"<<currentTest<<endl;

	return selectedControllers;
}

void NeuroEvolution::updateScores(vector <double> multiscore)
{
	if(multiscore.size()==2)
		this->scoresOfTheGeneration.push_back(multiscore);
	else
		multiscore.push_back(-1.0);
	double score=1.0* multiscore[0] - 0.0 * multiscore[1];
	for(int oneElem=0;oneElem<selectedControllers.size();oneElem++)
	{
		NeuroEvoMember * controllerPointer=selectedControllers.at(oneElem);

		controllerPointer->pastScores.push_back(score);
		double prevScore=controllerPointer->maxScore;
		if(prevScore>score)
		{
				double newScore= leniencyCoef * prevScore + (1.0 - leniencyCoef) * score;
				controllerPointer->maxScore=newScore;
		}
		else
		{
			controllerPointer->maxScore=score;
			controllerPointer->maxScore1=multiscore[0];
			controllerPointer->maxScore2=multiscore[1];
		}
	}

	//Record it to the file
	ofstream payloadLog;
	payloadLog.open("logs/scores.csv",ios::app);
	payloadLog<<multiscore[0]<<","<<multiscore[1]<<endl;
	payloadLog.close();
	return;
}
