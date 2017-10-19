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


#include "io/net/zmq_subscriber.h"

#include <iostream>
#include <string>


namespace io
{
namespace net
{

//-----------------------------------------------------------------------------

Zmq_Subscriber::Zmq_Subscriber(std::string conn_str) : Zmq_IO(ZMQ_SUB)
{
    char subscription[] = "XRF-Counts";
    _zmq_socket->connect(conn_str);
    _zmq_socket->setsockopt(ZMQ_SUBSCRIBE, subscription, std::strlen(subscription));
}

//-----------------------------------------------------------------------------

void Zmq_Subscriber::get_counts(data_struct::xrf::Stream_Block* out_stream_block)
{

    int idx = 0;
    int idx2 = 0;
    size_t row = 0;
    size_t col = 0;
    size_t height = 0;
    size_t width = 0;
    size_t proc_type_count = 0;
    size_t proc_type = 0;
    size_t fit_block_size = 0;
    char name[256] = {0};
    real_t val = 0.0;
    char *message;
    int message_len;

    get(message, message_len);

    memcpy(&out_stream_block->detector_number, message+idx, 4);
    idx+=4;
    memcpy(&row, message+idx, 4);
    idx+=4;
    memcpy(&col, message+idx, 4);
    idx+=4;
    memcpy(&height, message+idx, 4);
    idx+=4;
    memcpy(&width, message+idx, 4);
    idx+=4;

    data_struct::xrf::Stream_Block* s_block = new data_struct::xrf::Stream_Block(row, col, height, width);
    out_stream_block = s_block;


    memcpy(&proc_type_count, message+idx, 4);
    idx+=4;
    for(int proc_type_itr=0; proc_type_itr < proc_type_count; proc_type_itr++)
    {
        memcpy(&proc_type, message+idx, 4);
        idx+=4;
        out_stream_block->fitting_blocks[proc_type] = data_struct::xrf::Stream_Fitting_Block();

        //get fit_block[proc_type] size
        memcpy(&fit_block_size, message+idx, 4);
        idx+=4;
        for(int i=0; i < fit_block_size; i++)
        {
            idx2 = idx;
            // find null term
            while( idx2 < message_len && message[idx2] != '\0')
            {
                idx2++;
            }

            if(idx2 - idx > 255)
            {
                memcpy(&name[0], message+idx, 255);
            }
            else
            {
                memcpy(&name[0], message+idx, idx2 - idx);
            }
            idx = idx2+1;
            memcpy(&val, message+idx, sizeof(real_t));

            out_stream_block->fitting_blocks[proc_type].fit_counts[std::string(name)] = val;
        }
    }
}

//-----------------------------------------------------------------------------

void Zmq_Subscriber::get_spectra(data_struct::xrf::Stream_Block* out_stream_block)
{

}

//-----------------------------------------------------------------------------

} //end namespace net
}// end namespace io
