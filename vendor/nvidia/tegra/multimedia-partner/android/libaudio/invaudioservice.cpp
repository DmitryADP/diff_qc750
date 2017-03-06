/*
 * Copyright (c) 2012 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 * invaudioservice.cpp
 *
 * Extend the Binder interface to create a service that receives and serves
 * shared memory.
 *
 */

#define LOG_TAG "INvAudioService"
//#define LOG_NDEBUG 0

#include "nvaudio_service.h"

using namespace android;

enum {
    SET_WFD_AUDIO_BUFFER = IBinder::FIRST_CALL_TRANSACTION,
    SET_WFD_AUDIO_PARAMS,
    SET_AUDIO_PARAMETERS,
    GET_AUDIO_PARAMETERS
};

/* --- Client side --- */
class BpNvAudioALSAService: public BpInterface<INvAudioALSAService>
{
public:
    BpNvAudioALSAService(const sp<IBinder>& impl) : BpInterface<INvAudioALSAService>(impl)
    {
    }

    void setWFDAudioBuffer(const sp<IMemory>& mem)
    {
        Parcel data;

        data.writeInterfaceToken(INvAudioALSAService::getInterfaceDescriptor());
        data.writeStrongBinder(mem->asBinder());
        // This will result in a call to the onTransact()
        // method on the server in it's context (from it's binder threads)
        remote()->transact(SET_WFD_AUDIO_BUFFER, data, NULL);
        return;
    }

    status_t setWFDAudioParams(uint32_t sampleRate, uint32_t channels,
                               uint32_t bitsPerSample)
    {
        Parcel data, reply;

        data.writeInterfaceToken(INvAudioALSAService::getInterfaceDescriptor());
        data.writeInt32(sampleRate);
        data.writeInt32(channels);
        data.writeInt32(bitsPerSample);
        remote()->transact(SET_WFD_AUDIO_PARAMS, data, &reply);
        return reply.readInt32();
    }

    status_t setAudioParameters(audio_io_handle_t ioHandle,
                                const String8& keyValuePairs)
    {
        Parcel data, reply;

        data.writeInterfaceToken(INvAudioALSAService::getInterfaceDescriptor());
        data.writeInt32(ioHandle);
        data.writeString8(keyValuePairs);
        remote()->transact(SET_AUDIO_PARAMETERS, data, &reply);
        return reply.readInt32();
    }

    String8 getAudioParameters(audio_io_handle_t ioHandle,
                               const String8& keys)
    {
        Parcel data, reply;

        data.writeInterfaceToken(INvAudioALSAService::getInterfaceDescriptor());
        data.writeInt32(ioHandle);
        data.writeString8(keys);
        remote()->transact(GET_AUDIO_PARAMETERS, data, &reply);
        return reply.readString8();
    }
};

IMPLEMENT_META_INTERFACE(NvAudioALSAService, "android.nvidia.INvAudioALSAService");


/* --- Server side --- */

status_t BnNvAudioALSAService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case SET_WFD_AUDIO_BUFFER:
        {
            CHECK_INTERFACE(INvAudioALSAService, data, reply);
            sp<IMemory> mem = interface_cast<IMemory>(data.readStrongBinder());
            setWFDAudioBuffer(mem);
            break;
        }

        case SET_WFD_AUDIO_PARAMS:
        {
            CHECK_INTERFACE(INvAudioALSAService, data, reply);
            reply->writeInt32(setWFDAudioParams(data.readInt32(),
                                             data.readInt32(),
                                             data.readInt32()));

            break;
        }

        case SET_AUDIO_PARAMETERS:
        {
            CHECK_INTERFACE(INvAudioALSAService, data, reply);
            reply->writeInt32(setAudioParameters(data.readInt32(),
                                                 data.readString8()));

            break;
        }

        case GET_AUDIO_PARAMETERS:
        {
            CHECK_INTERFACE(INvAudioALSAService, data, reply);
            reply->writeString8(getAudioParameters(data.readInt32(),
                                                   data.readString8()));

            break;
        }

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
    return NO_ERROR;
}
