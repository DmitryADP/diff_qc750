/*
 * Copyright (c) 2009-2012 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

/*
 * Based upon AudioPolicyManagerBase.h, provided under the following terms:
 *
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBAUDIO_NVAUDIOPOLICY_H
#define LIBAUDIO_NVAUDIOPOLICY_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <hardware_legacy/AudioPolicyManagerBase.h>
#include <hardware/audio.h>
#include <system/audio.h>
#include <binder/IServiceManager.h>

#include "invaudioservice.h"

#define AudioSystem android_audio_legacy::AudioSystem
#define AudioPolicyManagerBase android_audio_legacy::AudioPolicyManagerBase
#define AudioPolicyClientInterface android_audio_legacy::AudioPolicyClientInterface
#define AudioPolicyInterface android_audio_legacy::AudioPolicyInterface

using namespace android;

class NvAudioPolicyManager: public AudioPolicyManagerBase
{
public:
    NvAudioPolicyManager(AudioPolicyClientInterface *clientInterface);
    virtual ~NvAudioPolicyManager();

    virtual status_t setDeviceConnectionState(audio_devices_t device,
                                                      AudioSystem::device_connection_state state,
                                                      const char *device_address);
    virtual void setPhoneState(int state);
    virtual void setForceUse(AudioSystem::force_use usage, AudioSystem::forced_config config);
    virtual audio_io_handle_t getOutput(AudioSystem::stream_type stream,
                                        uint32_t samplingRate = 0,
                                        uint32_t format = AudioSystem::FORMAT_DEFAULT,
                                        uint32_t channels = 0,
                                        AudioSystem::output_flags flags =
                                                AudioSystem::OUTPUT_FLAG_INDIRECT);
    virtual status_t startOutput(audio_io_handle_t output,
                                 AudioSystem::stream_type stream,
                                 int session = 0);
    virtual status_t stopOutput(audio_io_handle_t output,
                                AudioSystem::stream_type stream,
                                int session = 0);
protected:

    // return appropriate device for streams handled by the specified strategy according to current
    // phone state, connected devices...
    // if fromCache is true, the device is returned from mDeviceForStrategy[],
    // otherwise it is determine by current state
    // (device connected,phone state, force use, a2dp output...)
    // This allows to:
    //  1 speed up process when the state is stable (when starting or stopping an output)
    //  2 access to either current device selection (fromCache == true) or
    // "future" device selection (fromCache == false) when called from a context
    //  where conditions are changing (setDeviceConnectionState(), setPhoneState()...) AND
    //  before updateDevicesAndOutputs() is called.
    virtual audio_devices_t getDeviceForStrategy(routing_strategy strategy,
                                                 bool fromCache);

    // compute the actual volume for a given stream according to the requested index and a particular
    // device
    virtual float computeVolume(int stream, int index, audio_io_handle_t output, audio_devices_t device);

    virtual status_t unregisterEffect(int id);
    virtual status_t setEffectEnabled(int id, bool enabled);
private:
    sp<INvAudioALSAService> nvaudioSvc;

    static float volIndexToAmpl(audio_devices_t device, const StreamDescriptor& streamDesc,
        int indexInUi);
    // updates device caching and output for streams that can influence the
    //    routing of notifications
    void handleNotificationRoutingForStream(AudioSystem::stream_type stream);

    void notifyEffectCount();
    void notifyMediaRouting(bool fromCache);
};

#endif // LIBAUDIO_NVAUDIOPOLICY_H
