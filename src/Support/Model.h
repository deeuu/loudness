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

#ifndef MODEL_H
#define MODEL_H

#include "Module.h"

namespace loudness{

    /**
     * @class Model 
     * 
     * @brief An abstract used to build loudness models from modules.
     *
     * The architecture of this class is similar to the abstract class Module,
     * but allows for accessing the outputs of multiple processing modules. A
     * reference to an input SignalBank is first passed to initialize() which in
     * turn calls the pure virtual function initializeInternal(). Thus, all
     * derived models must implement this function. It is here where multiple
     * modules are instantiated, initialised and connected.  A vector of module
     * pointers (modules_) is used to gain access to the individual modules.
     *
     * Unlike Module, there is no processInternal() function. Instead, process()
     * will call the process function on the first (root) module in the chain
     * which will trigger the remaining modules internally.
     * 
     * @author Dominic Ward
     *
     * @sa Module
     */
    class Model
    {
    public: 

        /**
        * @brief Constructs a model to operate at @a rate Hz.
        * 
        * The frameRate of the SignalBank used to initialise the model
        * should not be less than @a rate.
        *
        * @param rate
        */
        Model(string name = "Model", bool dynamicModel = true);
        virtual ~Model();

        /**
        * @brief Initialises the model and all associated modules.
        *
        * The frameRate of the SignalBank used to initialise the model
        * should not be less than the model rate. If it is, the rate is
        * automatically corrected.
        *
        * @param input The input SignalBank.
        *
        * @return 
        */
        bool initialize(const SignalBank &input);

        /**
        * @brief Processes the input SignalBank.
        *
        * @param input The input SignalBank to be processed. Must be same
        * structure as the one used to initialise the module.
        */
        void process(const SignalBank &input);

        /**
        * @brief Resets all modules. The output SignalBanks are also cleared.
        */
        void reset();

        /**
         * @brief Returns the initialisation state.
         *
         * @return true if initialised, false otherwise.
         */
        bool isInitialized() const;

        /**
         * @brief Returns the type of loudness model.
         *
         * @return true if dynamic, false otherwise.
         */
        bool isDynamicModel() const;

        /** Returns a reference to the output SignalBank of a module specified
         * by it's index.
         */
        const SignalBank& getModuleOutput(int module) const;

        /** Returns a reference to the output SignalBank of a module specified
         * by it's name.
         */
        const SignalBank& getModuleOutput(const string& moduleName) const;

        /** Returns a reference to the output SignalBank of the final module
         * used by the model. For most models, this will be the integrated
         * loudness.
         */
        const SignalBank& getModelOutput() const;
        
        /**
         * @brief Returns the number of initialised modules comprising the
         * model.
         */
        int getNModules() const;

        /**
         * @brief Sets the time step (hop size in secs) for a dynamic loudness
         * model.
         */
        void setTimeStep(Real timeStep);

        /**
         * @brief Returns the time step (hop size in secs) for a dynamic
         * loudness model.
         */
        Real getTimeStep() const;

        /**
         * @brief Returns the name of a module residing within the model.
         *
         * @param module Module index.
         */
        const string& getModuleName(int module) const;
        
        /**
         * @brief Returns the name of the model.
         */
        const string& getName() const;

    protected:
        virtual bool initializeInternal(const SignalBank &input) = 0;

        string name_;
        bool dynamicModel_, initialized_;
        int nModules_;
        Real timeStep_ = 0;
        vector<unique_ptr<Module>> modules_;
        vector<string> moduleNames_;
    };
}

#endif
