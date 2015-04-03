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
        dynamicModel_(dynamicModel),
        rate_(0)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    Model::~Model() {}

    void Model::configureLinearTargetModuleChain()
    {
        for (uint i = 0; i < (modules_.size() - 1); i++)
            modules_[i] -> addTargetModule(*modules_[i + 1]);
    }

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

            nModules_ = (int)modules_.size();

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

    const SignalBank& Model::getOutputSignalBank(int module) const
    {
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(module, nModules_));
        return modules_[module] -> getOutputSignalBank();
    }

    const SignalBank& Model::getOutputSignalBank(const string& outputName) const
    {
        int idx = std::find(outputNames_.begin(), outputNames_.end(), outputName)
            - outputNames_.begin();

        if (!isPositiveAndLessThanUpper(idx, nModules_))
        {
            LOUDNESS_WARNING(name_ << ": Invalid name. Options are:");
            for (int i = 0; i < nModules_; i++)
                std::cerr << outputNames_[i] << std::endl;
            LOUDNESS_ASSERT(false);
        }
        return modules_[idx] -> getOutputSignalBank();
    }

    const string& Model::getOutputName(int module) const
    {
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(module, nModules_));
        return outputNames_[module];
    }

    const vector<string>& Model::getOutputNames() const
    {
        return outputNames_;
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
    
    void Model::setRate(Real rate)
    {
        rate_ = rate;
    }

    Real Model::getRate() const
    {
        return rate_;
    }

}
