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

/// Initial Author <2017>: Arthur Glowacki


#include "io/net/zmq_publisher.h"

#include <iostream>
#include <string>


namespace io
{
namespace net
{

//-----------------------------------------------------------------------------

Zmq_Publisher::Zmq_Publisher(std::string conn_str) : Zmq_IO(ZMQ_PUB)
{
    _zmq_socket->bind(conn_str);
}

Zmq_Publisher::~Zmq_Publisher()
{
    //_zmq_socket->unbind();
}

//-----------------------------------------------------------------------------

void Zmq_Publisher::send_counts(data_struct::xrf::Stream_Block* stream_block)
{

    std::string raw_msg = "";
    char *tmp_real = new char[sizeof(real_t)];
    char *tmp_ushort = new char[sizeof(unsigned short)];
    char *tmp_uint = new char[sizeof(unsigned int)];

    //TODO:
    // add something to distinguish between counts and spectra
    // add something to tell if 4 or 8 byte real

    _convert_var_to_bytes(&raw_msg, tmp_uint, stream_block->detector_number, 4);
    _convert_var_to_bytes(&raw_msg, tmp_uint, stream_block->row(), 4);
    _convert_var_to_bytes(&raw_msg, tmp_uint, stream_block->col(), 4);
    _convert_var_to_bytes(&raw_msg, tmp_uint, stream_block->height(), 4);
    _convert_var_to_bytes(&raw_msg, tmp_uint, stream_block->width(), 4);


    _convert_var_to_bytes(&raw_msg, tmp_uint, stream_block->fitting_blocks.size(), 4);

    // iterate through fitting routine
    for( auto& itr : stream_block->fitting_blocks)
    {
        _convert_var_to_bytes(&raw_msg, tmp_uint, itr.second.fit_counts.size(), 4);
        // iterate through elements counts
        for(auto &itr2 : itr.second.fit_counts)
        {
            raw_msg += itr2.first;
            raw_msg += '\0';
            _convert_var_to_bytes(&raw_msg, tmp_real, itr2.second, sizeof(real_t));
        }
    }

    delete [] tmp_real;
    delete [] tmp_ushort;
    delete [] tmp_uint;
    send("XRF-Counts");
    send(raw_msg);
}

//-----------------------------------------------------------------------------

void Zmq_Publisher::send_spectra(data_struct::xrf::Stream_Block* stream_block)
{

}

//-----------------------------------------------------------------------------

} //end namespace net
}// end namespace io
