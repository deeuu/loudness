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

#include "Module.h"

namespace loudness{
    
    Module::Module(string name):
        name_(name)
    {
        initialized_ = 0;
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    };

    Module::~Module(){};

    bool Module::initialize()
    {
        if(initialized_)
        {
            LOUDNESS_WARNING(name_ << ": Reinitialising ...")
        }

        if(!initializeInternal())
        {
            LOUDNESS_ERROR(name_ << ": Not initialised!");
            return 0;
        }
        else
        {
            LOUDNESS_DEBUG(name_ << ": Initialised.");
            if(output_.isInitialized())
            {
                for (uint i = 0; i < targetModules_.size(); i++)
                    targetModules_[i] -> initialize(output_);
            }

            initialized_ = 1;
            return 1;
        }
 
    }

    bool Module::initialize(const SignalBank &input)
    {
        if(initialized_)
        {
            LOUDNESS_WARNING(name_ << ": Reinitialising ...")
        }

        if(!initializeInternal(input))
        {
            LOUDNESS_ERROR(name_ << ": Not initialised!");
            return 0;
        }
        else
        {
            LOUDNESS_DEBUG(name_ << ": Initialised.");
            if(output_.isInitialized())
            {
                for (uint i = 0; i < targetModules_.size(); i++)
                    targetModules_[i] -> initialize(output_);
            }

            initialized_ = 1;
            return 1;
        }
    }

    void Module::process()
    {
        if(initialized_)
        {
            LOUDNESS_PROCESS_DEBUG(name_ << ": processing ...");
            output_.setTrig(true);
            processInternal();
        }
        else
            output_.setTrig(false);

        for (uint i = 0; i < targetModules_.size(); i++)
            targetModules_[i] -> process(output_);

    }

    void Module::process(const SignalBank &input)
    {
        if(initialized_ && input.getTrig())
        {
            LOUDNESS_PROCESS_DEBUG(name_ << ": processing SignalBank ...");
            output_.setTrig(true);
            processInternal(input);
        }
        else
            output_.setTrig(false);

        for (uint i = 0; i < targetModules_.size(); i++)
            targetModules_[i] -> process(output_);
    }

    void Module::reset()
    {
        //clear output signal
        if(output_.isInitialized())
            output_.clear();
        
        //call module specific reset
        resetInternal();

        //clear internal parameters
        for (uint i = 0; i < targetModules_.size(); i++)
            targetModules_[i] -> reset();
    }

    void Module::addTargetModule(Module& targetModule)
    {
        LOUDNESS_DEBUG(name_ << ": Adding " << targetModule.getName() << " as target.");
        targetModules_.push_back(&targetModule);
    }

    void Module::removeLastTargetModule()
    {
        targetModules_.pop_back();
    }

    bool Module::isInitialized() const
    {
        return initialized_;
    }

    const SignalBank& Module::getOutput() const
    {
        return output_;
    }

    const std::string& Module::getName() const
    {
        return name_;
    }
}

