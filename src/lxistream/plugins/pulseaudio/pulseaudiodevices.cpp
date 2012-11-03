/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

// Obtained example code from: http://www.ypass.net/blog/2009/10/pulseaudio-an-async-example-to-get-device-lists/

#include "pulseaudiodevices.h"

namespace LXiStreamDevice {
namespace PulseAudioBackend {

PulseAudioDevices::PulseAudioDevices(QObject *parent)
  : QObject(parent)
{
  buildDevicelist();
}

PulseAudioDevices::~PulseAudioDevices()
{
}

void PulseAudioDevices::stateCallback(pa_context *c, void *userdata)
{
  int * const ready = reinterpret_cast<int *>(userdata);

  switch(pa_context_get_state(c))
  {
  // There are just here for reference
  case PA_CONTEXT_UNCONNECTED:
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
  default:
    break;

  case PA_CONTEXT_FAILED:
  case PA_CONTEXT_TERMINATED:
    *ready = 2;
    break;

  case PA_CONTEXT_READY:
    *ready = 1;
    break;
  }
}

void PulseAudioDevices::sinklistCallback(pa_context *, const pa_sink_info *l, int eol, void *userdata)
{
  PulseAudioDevices * const me = reinterpret_cast<PulseAudioDevices *>(userdata);

  if (eol == 0)
    me->outputs[l->name] = *l;
}

void PulseAudioDevices::sourcelistCallback(pa_context *, const pa_source_info *l, int eol, void *userdata)
{
  PulseAudioDevices * const me = reinterpret_cast<PulseAudioDevices *>(userdata);

  if (eol == 0)
    me->inputs[l->name] = *l;
}

void PulseAudioDevices::buildDevicelist()
{
  pa_mainloop *pa_ml;
  pa_mainloop_api *pa_mlapi;
  pa_operation *pa_op;
  pa_context *pa_ctx;

  int state = 0;
  int ready = 0;

  // Create a mainloop API and connection to the default server
  pa_ml = pa_mainloop_new();
  pa_mlapi = pa_mainloop_get_api(pa_ml);
  pa_ctx = pa_context_new(pa_mlapi, "test");

  // This function connects to the pulse server
  pa_context_connect(pa_ctx, NULL, pa_context_flags_t(0), NULL);

  // This function defines a callback so the server will tell us it's state.
  // Our callback will wait for the state to be ready.  The callback will
  // modify the variable to 1 so we know when we have a connection and it's
  // ready.
  // If there's an error, the callback will set pa_ready to 2
  pa_context_set_state_callback(
        pa_ctx,
        &PulseAudioDevices::stateCallback,
        &ready);

  // Now we'll enter into an infinite loop until we get the data we receive
  // or if there's an error
  for (;;)
  {
    // We can't do anything until PA is ready, so just iterate the mainloop
    // and continue
    if (ready == 0)
    {
      pa_mainloop_iterate(pa_ml, 1, NULL);
      continue;
    }

    // We couldn't get a connection to the server, so exit out
    if (ready == 2)
    {
      pa_context_disconnect(pa_ctx);
      pa_context_unref(pa_ctx);
      pa_mainloop_free(pa_ml);
      return;
    }

    // At this point, we're connected to the server and ready to make
    // requests
    switch (state)
    {
    // State 0: we haven't done anything yet
    case 0:
      // This sends an operation to the server.  pa_sinklist_info is
      // our callback function and a pointer to our devicelist will
      // be passed to the callback The operation ID is stored in the
      // pa_op variable
      pa_op = pa_context_get_sink_info_list(
            pa_ctx,
            &PulseAudioDevices::sinklistCallback,
            this);

      // Update state for next iteration through the loop
      state++;
      break;

    case 1:
      // Now we wait for our operation to complete.  When it's
      // complete our pa_output_devicelist is filled out, and we move
      // along to the next state
      if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE)
      {
          pa_operation_unref(pa_op);

          // Now we perform another operation to get the source
          // (input device) list just like before.  This time we pass
          // a pointer to our input structure
          pa_op = pa_context_get_source_info_list(
                pa_ctx,
                &PulseAudioDevices::sourcelistCallback,
                this);

          // Update the state so we know what to do next
          state++;
      }
      break;

    case 2:
      if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE)
      {
          // Now we're done, clean up and disconnect and return
          pa_operation_unref(pa_op);
          pa_context_disconnect(pa_ctx);
          pa_context_unref(pa_ctx);
          pa_mainloop_free(pa_ml);
          return;
      }
      break;
    }

    // Iterate the main loop and go again.  The second argument is whether
    // or not the iteration should block until something is ready to be
    // done.  Set it to zero for non-blocking.
    pa_mainloop_iterate(pa_ml, 1, NULL);
  }
}

} } // End of namespaces
