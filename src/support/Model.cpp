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

    Model::Model(string name, bool isDynamic) :
        name_(name),
        isDynamic_(isDynamic),
        rate_(0.0)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    Model::~Model() {}

    bool Model::initialize(const SignalBank &input)
    {
        outputModules_.clear();
        modules_.clear();

        if(!initializeInternal(input))
        {
            LOUDNESS_ERROR(name_ << ": Not initialised!");
            return 0;
        }
        else
        {
            if (outputModules_.empty())
            {
                LOUDNESS_ERROR(name_ << ": Invalid, this Model has no outputs.");
                return 0;
            }
            LOUDNESS_DEBUG(name_ << ": initialised.");

            nModules_ = (int)modules_.size();

            //initialise all from root module
            modules_[0] -> initialize(input);

            configureSignalBankAggregation();

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

    void Model::configureLinearTargetModuleChain(int moduleIdx)
    {
        int nModulesMinus1 = int (modules_.size()) - 1;
        for (int i = moduleIdx; i < nModulesMinus1; ++i)
            modules_[i] -> addTargetModule(*modules_[i + 1]);
    }

    void Model::setOutputsToAggregate(const vector<string>& outputsToAggregate)
    {
        outputsToAggregate_ = outputsToAggregate;
    }

    void Model::addOutputToAggregate(string& outputToAggregate)
    {
        if (std::find (outputsToAggregate_.begin(), 
                        outputsToAggregate_.end(),
                        outputToAggregate) == outputsToAggregate_.end())
        {
            outputsToAggregate_.push_back(outputToAggregate);
        }
    }

    void Model::removeOutputToAggregate(string& outputToAggregate)
    {
        outputsToAggregate_.erase (std::remove (
                    outputsToAggregate_.begin(), 
                    outputsToAggregate_.end(), outputToAggregate));
    }

    const SignalBank& Model::getOutput(const string& outputName) const
    {
        auto search = outputModules_.find(outputName);
        LOUDNESS_ASSERT(search != outputModules_.end());
        return search -> second -> getOutput();
    }

    void Model::configureSignalBankAggregation()
    {
        for (const auto &outputName : outputsToAggregate_)
        {
            auto search = outputModules_.find(outputName);
            if (search != outputModules_.end())
            {
                LOUDNESS_DEBUG(name_ << ": Aggregating : " << search -> first);
                search -> second -> setOutputAggregated(true);
            }
        }
    }

    const string& Model::getName() const
    {
        return name_;
    }

    bool Model::isInitialized() const
    {
        return initialized_;
    }

    bool Model::isDynamic() const
    {
        return isDynamic_;
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
