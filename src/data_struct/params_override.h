/***
-Copyright (c) 2016, UChicago Argonne, LLC. All rights reserved.
-
-Copyright 2016. UChicago Argonne, LLC. This software was produced
-under U.S. Government contract DE-AC02-06CH11357 for Argonne National
-Laboratory (ANL), which is operated by UChicago Argonne, LLC for the
-U.S. Department of Energy. The U.S. Government has rights to use,
-reproduce, and distribute this software.  NEITHER THE GOVERNMENT NOR
-UChicago Argonne, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
-ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If software is
-modified to produce derivative works, such modified software should
-be clearly marked, so as not to confuse it with the version available
-from ANL.
-
-Additionally, redistribution and use in source and binary forms, with
-or without modification, are permitted provided that the following
-conditions are met:
-
-    * Redistributions of source code must retain the above copyright
-      notice, this list of conditions and the following disclaimer.
-
-    * Redistributions in binary form must reproduce the above copyright
-      notice, this list of conditions and the following disclaimer in
-      the documentation and/or other materials provided with the
-      distribution.
-
-    * Neither the name of UChicago Argonne, LLC, Argonne National
-      Laboratory, ANL, the U.S. Government, nor the names of its
-      contributors may be used to endorse or promote products derived
-      from this software without specific prior written permission.
-
-THIS SOFTWARE IS PROVIDED BY UChicago Argonne, LLC AND CONTRIBUTORS
-"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
-LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
-FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL UChicago
-Argonne, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
-INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
-BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
-CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
-LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
-ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
-POSSIBILITY OF SUCH DAMAGE.
-***/

#ifndef PARAMS_OVERRIDE_H
#define PARAMS_OVERRIDE_H


#include <map>
#include <string>
#include <list>
#include "core/defines.h"

#include "data_struct/fit_parameters.h"
#include "data_struct/fit_element_map.h"

namespace data_struct
{

using namespace std;

//-----------------------------------------------------------------------------
/**
 * @brief The Params_Override struct
 */
class DLL_EXPORT Params_Override
{

public:

    Params_Override()
    {
        si_escape_factor = 0.0;
        ge_escape_factor = 0.0;
        si_escape_enabled = false;
        ge_escape_enabled = false;
        fit_snip_width = 0.5;
        us_amp_sens_num = 0.0;
        us_amp_sens_unit = 0.0;
        ds_amp_sens_num = 0.0;
        ds_amp_sens_unit = 0.0;
        theta_pv = "";
        sr_current = 0.0;
        US_IC = 0.0;
        DS_IC = 0.0;
        detector_element = "Si";
        dataset_directory = "";
        detector_num = -1;
        be_window_thickness = "24.0";
        det_chip_thickness = "350.0";
        ge_dead_layer = "0.0";
        airpath = "0";
        theta_pv = "";
    }

    Params_Override(string dir, int detector)
    {

        si_escape_factor = 0.0;
        ge_escape_factor = 0.0;
        si_escape_enabled = false;
        ge_escape_enabled = false;
        fit_snip_width = 0.0;
        us_amp_sens_num = 0.0;
        us_amp_sens_unit = 0.0;
        ds_amp_sens_num = 0.0;
        ds_amp_sens_unit = 0.0;
        theta_pv = "";
        sr_current = 0.0;
        US_IC = 0.0;
        DS_IC = 0.0;

        dataset_directory = dir;
        detector_num = detector;
        detector_element = "Si";
    }

    ~Params_Override()
    {
        // TODO: fix this: this memeory is used when optimising and is copyed to another data struct
//		for (auto& itr : elements_to_fit)
//		{
//			delete itr.second;
//		}
		elements_to_fit.clear();
    }

	void parse_and_gen_branching_ratios()
	{
		branching_ratios.clear();
		for (const auto& itr : branching_family_L)
		{
			istringstream f(itr);
			string s;
			string el_name;
			getline(f, s, ':');
			getline(f, el_name, ',');
			el_name.erase(std::remove_if(el_name.begin(), el_name.end(), ::isspace), el_name.end());

			float factor = 1.0;
			// 1
			std::getline(f, s, ',');
			factor = std::stof(s);
			branching_ratios[el_name][4] = factor;
			branching_ratios[el_name][5] = factor;
			branching_ratios[el_name][7] = factor;
			branching_ratios[el_name][8] = factor;
			branching_ratios[el_name][9] = factor;

			// 2
			std::getline(f, s, ',');
			factor = std::stof(s);
			branching_ratios[el_name][2] = factor;
			branching_ratios[el_name][6] = factor;
			branching_ratios[el_name][11] = factor;
			
			//3
			std::getline(f, s, ',');
			factor = std::stof(s);
			branching_ratios[el_name][0] = factor;
			branching_ratios[el_name][1] = factor;
			branching_ratios[el_name][3] = factor;
			branching_ratios[el_name][10] = factor;

		}
		for (const auto& itr : branching_ratio_L)
		{
			istringstream f(itr);
			string s;
			string el_name;
			getline(f, s, ':');
			getline(f, el_name, ',');
			el_name.erase(std::remove_if(el_name.begin(), el_name.end(), ::isspace), el_name.end());

			
			for (unsigned int i = 0; i < 12; i++)
			{
				float factor = 1.0;
				std::getline(f, s, ',');
				factor = std::stof(s);
				branching_ratios[el_name][i] = factor;
			}
		}
		for (const auto& itr : branching_ratio_K)
		{
			istringstream f(itr);
			string s;
			string el_name;
			getline(f, s, ':');
			getline(f, el_name, ',');
			el_name.erase(std::remove_if(el_name.begin(), el_name.end(), ::isspace), el_name.end());


			for (unsigned int i = 0; i < 4; i++)
			{
				float factor = 1.0;
				std::getline(f, s, ',');
				factor = std::stof(s);
				branching_ratios[el_name][i] = factor;
			}
		}

	}

	map<int, float> get_custom_factor(string el_name)
	{
		map<int, float> factors;
		if (branching_ratios.count(el_name) > 0)
		{
			return branching_ratios.at(el_name);
		}

		return factors;
	}

    string dataset_directory;
    int detector_num;
    Fit_Parameters fit_params;
    Fit_Element_Map_Dict elements_to_fit;
    string detector_element;
    
    real_t si_escape_factor;
    real_t ge_escape_factor;
    bool si_escape_enabled;
    bool ge_escape_enabled;
    real_t fit_snip_width;

    string be_window_thickness;
    string det_chip_thickness;
    string ge_dead_layer;
    string airpath;

    string us_amp_sens_num_pv;
    string us_amp_sens_unit_pv;
    string ds_amp_sens_num_pv;
    string ds_amp_sens_unit_pv;

    string theta_pv;

    vector<string> branching_family_L;
    vector<string> branching_ratio_L;
    vector<string> branching_ratio_K;

	unordered_map< string, map<int, float> > branching_ratios;

    real_t sr_current;
    real_t US_IC;
    real_t DS_IC;

    real_t us_amp_sens_num;
    real_t us_amp_sens_unit;
    real_t ds_amp_sens_num;
    real_t ds_amp_sens_unit;

};

//-----------------------------------------------------------------------------

} //namespace data_struct

#endif // PARAMS_OVERRIDE_H
