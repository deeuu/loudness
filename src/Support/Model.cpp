/*
 * Copyright (C) 2014 Dominic Ward <contactdominicward@gmail.com>
 *
 * This file is part of Loudness
 *
 * Loudness is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Loudness is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Loudness.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "Model.h"

namespace loudness{

    Model::Model(string name, bool dynamicModel) :
        name_(name),
        dynamicModel_(dynamicModel)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    Model::~Model() {}

    bool Model::initialize(const SignalBank &input)
    {
        if(!initializeInternal(input))
        {
            LOUDNESS_ERROR(name_ << ": Not initialised!");
            return 0;
        }
        else
        {
            LOUDNESS_DEBUG(name_ << ": initialised.");

            //set up the chain
            nModules_ = (int)modules_.size();

            for (int i = 0; i < nModules_; i++)
            {
                if (i < (nModules_ - 1))
                    modules_[i] -> setTargetModule(*modules_[i + 1]);
                moduleNames_.push_back(modules_[i] -> getName());
            }

            //initialise all
            modules_[0] -> initialize(input);

            LOUDNESS_DEBUG(name_ 
                    << ": Module targets set and initialised.");

            initialized_ = 1;
            return 1;
        }
    }

    void Model::process(const SignalBank &input)
    {
        if (initialized_)
            modules_[0] -> process(input);
        else
            LOUDNESS_WARNING(name_ << ": Not initialised!");
    }

    void Model::reset()
    {
        if (initialized_)
            modules_[0] -> reset();
    }

    const SignalBank& Model::getModuleOutput(int module) const
    {
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(module, nModules_));
        return modules_[module] -> getOutput();
    }

    const SignalBank& Model::getModuleOutput(const string& moduleName) const
    {
        int idx = std::find(moduleNames_.begin(), moduleNames_.end(), moduleName)
            - moduleNames_.begin();

        if (!isPositiveAndLessThanUpper(idx, nModules_))
        {
            LOUDNESS_WARNING(name_ << ": Invalid Module name. Options are:");
            for (int i = 0; i < nModules_; i++)
                std::cerr << moduleNames_[i] << std::endl;
            LOUDNESS_ASSERT(false);
        }
        return modules_[idx] -> getOutput();
    }

    const SignalBank& Model::getModelOutput() const
    {
        LOUDNESS_ASSERT(initialized_);
        return modules_[nModules_ - 1] -> getOutput();
    }

    const string& Model::getModuleName(int module) const
    {
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(module, nModules_));
        return moduleNames_[module];
    }

    const string& Model::getName() const
    {
        return name_;
    }

    bool Model::isInitialized() const
    {
        return initialized_;
    }

    bool Model::isDynamicModel() const
    {
        return dynamicModel_;
    }

    int Model::getNModules() const
    {
        return nModules_;
    }            
    
    void Model::setTimeStep(Real timeStep)
    {
        timeStep_ = timeStep;
    }

    Real Model::getTimeStep() const
    {
        return timeStep_;
    }

}
