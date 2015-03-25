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
        targetModule_ = nullptr;
        initialized_ = 0;
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    };

    Module::~Module(){};

    bool Module::initialize()
    {
        return 0;
    }

    bool Module::initialize(const SignalBank &input)
    {
        if(initialized_)
        {
            LOUDNESS_ERROR(name_ << ": I'm already initialised!")
            return 0;
        }
        else if(!initializeInternal(input))
        {
            LOUDNESS_ERROR(name_ << ": Not initialised!");
            return 0;
        }
        else
        {
            LOUDNESS_DEBUG(name_ << ": Initialised.");
            if(output_.isInitialized())
            {
                if(targetModule_)
                    targetModule_->initialize(output_);
            }

            initialized_ = 1;
            return 1;
        }
    }

    void Module::process(){}

    void Module::process(const SignalBank &input)
    {
        if(initialized_ && input.getTrig())
        {
            output_.setTrig(true);
            processInternal(input);
        }
        else
            output_.setTrig(false);
        if(targetModule_)
            targetModule_->process(output_);
    }

    void Module::reset()
    {
        //clear output signal
        if(output_.isInitialized())
            output_.clear();

        //clear internal parameters
        resetInternal();
        if(targetModule_)
            targetModule_->reset();
    }

    void Module::setTargetModule(Module& targetModule)
    {
        targetModule_ = &targetModule;
    }

    void Module::removeTargetModule()
    {
        targetModule_ = nullptr;
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

