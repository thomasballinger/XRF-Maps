/***
Copyright (c) 2016, UChicago Argonne, LLC. All rights reserved.

Copyright 2016. UChicago Argonne, LLC. This software was produced
under U.S. Government contract DE-AC02-06CH11357 for Argonne National
Laboratory (ANL), which is operated by UChicago Argonne, LLC for the
U.S. Department of Energy. The U.S. Government has rights to use,
reproduce, and distribute this software.  NEITHER THE GOVERNMENT NOR
UChicago Argonne, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If software is
modified to produce derivative works, such modified software should
be clearly marked, so as not to confuse it with the version available
from ANL.

Additionally, redistribution and use in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.

    * Neither the name of UChicago Argonne, LLC, Argonne National
      Laboratory, ANL, the U.S. Government, nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY UChicago Argonne, LLC AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL UChicago
Argonne, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***/

/// Initial Author <2016>: Arthur Glowacki


#include "nnls_fit_routine.h"

//debug
#include <iostream>

namespace fitting
{
namespace routines
{

NNLS_Fit_Routine::NNLS_Fit_Routine() : Matrix_Optimized_Fit_Routine()
{

    _max_iter = 200;

}

// ----------------------------------------------------------------------------

NNLS_Fit_Routine::NNLS_Fit_Routine(size_t max_iter) : Matrix_Optimized_Fit_Routine()
{

    _max_iter = max_iter;

}

// ----------------------------------------------------------------------------

NNLS_Fit_Routine::~NNLS_Fit_Routine()
{
    _element_row_index.clear();
}

// ----------------------------------------------------------------------------

//void NNLS_Fit_Routine::_generate_fitmatrix(const unordered_map<string, Spectra> * const element_models, const struct Range energy_range)
void NNLS_Fit_Routine::_generate_fitmatrix()
{

    _element_row_index.clear();
    _fitmatrix.resize(_energy_range.count(), _element_models.size());

    int i = 0;
    for(const auto& itr : _element_models)
    {
        //Spectra element_model = itr.second;
        for (int j=0; j<itr.second.size(); j++)
        {
            _fitmatrix(j,i) = itr.second[j];
        }
        //save element index for later
        _element_row_index[itr.first] = i;
        i++;
    }

}

// ----------------------------------------------------------------------------

OPTIMIZER_OUTCOME NNLS_Fit_Routine::fit_spectra(const models::Base_Model * const model,
                                                const Spectra * const spectra,
                                                const Fit_Element_Map_Dict * const elements_to_fit,
                                                std::unordered_map<std::string, real_t>& out_counts)
{
	data_struct::ArrayXr* result;
    int num_iter;
    real_t npg;
    Fit_Parameters fit_params = model->fit_parameters();
    fit_params.add_parameter(Fit_Param(STR_RESIDUAL, 0.0));
    ArrayXr background;
    if (fit_params.contains(STR_SNIP_WIDTH))
    {        
        ArrayXr bkg = snip_background(spectra,
            fit_params.value(STR_ENERGY_OFFSET),
            fit_params.value(STR_ENERGY_SLOPE),
            fit_params.value(STR_ENERGY_QUADRATIC),
            fit_params.value(STR_SNIP_WIDTH),
            _energy_range.min,
            _energy_range.max);

        background = bkg.segment(_energy_range.min, _energy_range.count());
    }
    else
    {
        background.setZero(_energy_range.count());
    }

    ArrayXr spectra_sub_background = spectra->segment(_energy_range.min, _energy_range.count());
    spectra_sub_background -= background;
    spectra_sub_background = spectra_sub_background.unaryExpr([](real_t v) { return v>0.0 ? v : (real_t)0.0; });
    nsNNLS::nnls<real_t> solver(&_fitmatrix, &spectra_sub_background, _max_iter);

    Spectra spectra_model = background;

	//Spectra spectra_model(_energy_range.count());
	//ArrayXr rhs = spectra->sub_spectra(_energy_range.min, _energy_range.count());
	//nsNNLS::nnls<real_t> solver(&_fitmatrix, &rhs, _max_iter);

    solver.optimize(num_iter, npg);
    if (num_iter < 0)
    {
        logE<<"NNLS_Fit_Routine::_fit_spectra: in optimization routine"<<"\n";
    }

    result = solver.getSolution();

    for(const auto& itr : *elements_to_fit)
    {
        out_counts[itr.first] = (*result)[_element_row_index[itr.first]];

		for (int j = 0; j < _energy_range.count(); j++)
		{
			real_t val = _fitmatrix(j, _element_row_index[itr.first]) * (*result)[_element_row_index[itr.first]];
			if (std::isfinite(val))
			{
				spectra_model[j] += val;
			}
		}
    }

    out_counts[STR_NUM_ITR] = static_cast<real_t>(num_iter);
    out_counts[STR_RESIDUAL] = npg;

	//lock and integrate results
	{
		std::lock_guard<std::mutex> lock(_int_spec_mutex);
		_integrated_fitted_spectra.add(spectra_model);
        _integrated_background.add(background);
	}

    if (num_iter == solver.getMaxit())
    {
        return OPTIMIZER_OUTCOME::EXHAUSTED;
    }
    return OPTIMIZER_OUTCOME::CONVERGED;

}

// ----------------------------------------------------------------------------

void NNLS_Fit_Routine::initialize(models::Base_Model * const model,
                                  const Fit_Element_Map_Dict * const elements_to_fit,
                                  const struct Range energy_range)
{
    Matrix_Optimized_Fit_Routine::initialize(model, elements_to_fit, energy_range);
    _generate_fitmatrix();
}

// ----------------------------------------------------------------------------

} //namespace routines
} //namespace fitting
