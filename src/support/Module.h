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

#ifndef MODULE_H
#define MODULE_H

#include "SignalBank.h"

namespace loudness{

    /**
     * @class Module
     * 
     * @brief Abstract class for all modules.
     *
     * All modules are derived from Module. Derived modules must implement the
     * pure virtual functions initializeInternal(), processInternal() and
     * resetInternal(). 
     *
     * After calling a module's constructor with the appropriate input
     * arguments, the module can be initialised by passing a reference to the
     * input SignalBank to initialize(), which in turn calls
     * initializeInternal() where the bulk of the work takes place. Upon
     * successful initialisation, an output SignalBank is setup for storing the
     * processing result. If a module has a target module (see
     * setTargetModule()), then it is also initialised by passing the output
     * SignalBank to the input of the target module function
     * initializeInternal(). This allows the ins and outs of modules to be
     * connected and thus create a processing pipeline. 
     *
     * To process a SignalBank, pass a reference to the process() function.
     * This will call the module specific processInternal() function which will
     * pass the processed SignalBank (output) to it's target for further
     * processing (input). The input SignalBank passed to process() must have
     * the same number channels, number of samples and centre frequencies as the
     * one used to initialise the module. Some modules do not take an input
     * SignalBank but instead generate their own input. In this case,
     * initialize() and process() are called with no arguments. Note that
     * processInternal() is only called if the SignalBank trigger is 1 (which is
     * the default), otherwise the output bank will not be updated.
     *
     * @author Dominic Ward
     *
     * @sa SignalBank
     */
    class Module
    {
    public:
        Module(string name = "Module");
        virtual ~Module();

        /**
         * @brief Initialises a module with no input.
         *
         * This is used for modules which generate their own input for
         * processing. This is virtual to enable such modules to override the
         * default function (which does nothing).
         *
         * @return true if module has been initialised, false otherwise.
         */
        virtual bool initialize();

        /**
         * @brief Initialises a module with an input SignalBank.
         *
         * This calls initializeInternal() and checks for a target if
         * successfully initialised. If a target module exists, it too is
         * initialised.
         *
         * @param input The input SignalBank for processing.
         *
         * @return true if module has been initialised, false otherwise.
         */
        virtual bool initialize(const SignalBank &input);

        /**
         * @brief Processes a self-generated input.
         *
         * As with initialize() this function is virtual for overriding the
         * default function. This is useful for modules that generate their own
         * input.
         */
        virtual void process();

        /**
         * @brief Processes the input SignalBank.
         *
         * @param input The input SignalBank to be processed. Must be same
         * structure as the one used to initialise the module.
         */
        virtual void process(const SignalBank &input);

        /**
         * @brief Restores a module to intialisation state and clears the
         * contents of it's output SignalBank.
         */
        void reset();

        /**
         * @brief Adds a new target module to the object.  
         *
         * Once processed, the output SignalBank is passed as input to the
         * target module for processing. Note that the target module must
         * continue to exist for the lifetime of the aggregate object when
         * initialize(), process() or reset() are called. This Module does not
         * own the target module.
         *
         * @param targetModule A reference to the target module.
         */
        void addTargetModule(Module& targetModule);
        
        /**
         * @brief Removes the most recent target module added. Removing a module
         * will not destroy the module pointed to.
         */
        void removeLastTargetModule();

        /**
         * @brief Returns the module initialisation state.
         *
         * @return true if initialised, false otherwise.
         */
        bool isInitialized() const;

        /**
         * @brief Returns a const reference to the output SignalBank used for
         * storing the processing result.
         *
         */
        const SignalBank& getOutput() const;

        /**
         * @brief Returns the name of the module.
         *
         * @return String corresponding to the module name.
         */
        const string& getName() const;

    protected:
        //Pure virtual functions
        virtual bool initializeInternal(const SignalBank &input) = 0;
        virtual void processInternal(const SignalBank &input) = 0;
        virtual void resetInternal() = 0;

        //members
        bool initialized_;
        vector<Module*> targetModules_;
        SignalBank output_;
        string name_;
    };
}

#endif
