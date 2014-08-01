//
//  params.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#include <string.h>
#include "params.h"

using namespace ndnrtc;
using namespace ndnlog;

char*
validatePrefix(const char* prefix)
{
    int strLen = strlen(prefix)+1;
    char* validated = (char*)malloc(strLen);
    memset(validated, 0, strLen);
    
    bool slash = false;
    int i = 0,k = 0;
    
    while (prefix[i] != '\0')
    {
        if (!(slash && prefix[i] == '/'))
        {
            validated[k] = prefix[i];
            k++;
        }

        slash = (prefix[i] == '/');
        i++;
    }

    // remove trailing slash
    if (slash)
        validated[k-1] = '\0';
    
    return validated;
}

int _ParamsStruct::validateVideoParams(const struct _ParamsStruct &params,
                                       struct _ParamsStruct &validatedResult)
{
    // check critical values
    if (params.loggingLevel != NdnLoggerDetailLevelNone &&
        (params.logFile == NULL ||
         strcmp(params.logFile,"") == 0))
        return RESULT_ERR;
    
    if (params.host == NULL ||
        strcmp(params.host, "") == 0)
        return RESULT_ERR;
    
    if (params.producerId == NULL ||
        strcmp(params.producerId, "") == 0)
        return RESULT_ERR;
    
    if (params.streamName == NULL ||
        strcmp(params.streamName, "") == 0)
        return RESULT_ERR;
    
    if (params.ndnHub == NULL ||
        strcmp(params.ndnHub, "") == 0)
        return RESULT_ERR;
    
    int res = RESULT_OK;
    ParamsStruct validated = DefaultParams;
    
    // check other values
    validated.loggingLevel = params.loggingLevel;
    validated.logFile = params.logFile;
    validated.useTlv = params.useTlv;
    validated.useRtx = params.useRtx;
    validated.useFec = params.useFec;
    validated.useCache = params.useCache;
    validated.useAvSync = params.useAvSync;
    validated.useAudio = params.useAudio;
    validated.useVideo = params.useVideo;
    validated.headlessMode = params.headlessMode;
    
    validated.captureDeviceId = params.captureDeviceId;
    validated.captureWidth = ParamsStruct::validate(params.captureWidth,
                                                    MinWidth, MaxWidth, res,
                                                    DefaultParams.captureWidth);
    validated.captureHeight = ParamsStruct::validate(params.captureHeight,
                                                     MinHeight, MaxHeight, res,
                                                     DefaultParams.captureHeight);
    validated.captureFramerate = ParamsStruct::validate(params.captureFramerate,
                                                        MinFrameRate, MaxFrameRate, res,
                                                        DefaultParams.captureFramerate);
    
    validated.renderWidth = ParamsStruct::validate(params.renderWidth,
                                                   MinWidth, MaxWidth, res,
                                                   DefaultParams.renderWidth);
    validated.renderHeight = ParamsStruct::validate(params.renderHeight,
                                                    MinHeight, MaxHeight, res,
                                                    DefaultParams.renderHeight);
    validated.nStreams = params.nStreams;
    validated.streamsParams = (CodecParams*)malloc(sizeof(CodecParams)*validated.nStreams);
    memcpy(validated.streamsParams, params.streamsParams, sizeof(CodecParams)*validated.nStreams);
    
    validated.host = params.host;
    validated.portNum = ParamsStruct::validateLE(params.portNum, MaxPortNum, res,
                                                 DefaultParams.portNum);
    
    validated.producerId = params.producerId;
    validated.streamName = params.streamName;
    validated.ndnHub = validatePrefix(params.ndnHub);
    
    validated.segmentSize = ParamsStruct::validate(params.segmentSize,
                                                   MinSegmentSize, MaxSegmentSize, res,
                                                   DefaultParams.segmentSize);
    validated.freshness = params.freshness;
    validated.producerRate = ParamsStruct::validate(params.producerRate,
                                                    MinFrameRate, MaxFrameRate, res,
                                                    DefaultParams.producerRate);
    validated.playbackRate = ParamsStruct::validate(params.playbackRate,
                                                    MinFrameRate, MaxFrameRate, res,
                                                    DefaultParams.playbackRate);
    validated.interestTimeout = params.interestTimeout;
    validated.bufferSize = ParamsStruct::validate(params.bufferSize,
                                                  MinBufferSize, MaxBufferSize, res,
                                                  DefaultParams.bufferSize);
    validated.jitterSize = ParamsStruct::validate(params.jitterSize,
                                                  MinJitterSize, INT_MAX, res,
                                                  DefaultParams.jitterSize);
    validated.slotSize = ParamsStruct::validate(params.slotSize,
                                                MinSlotSize, MaxSlotSize, res,
                                                DefaultParams.slotSize);
    validated.skipIncomplete = params.skipIncomplete;
    
    if (validated.slotSize < validated.segmentSize)
    {
        validated.slotSize = validated.segmentSize;
        res = RESULT_WARN;
    }
    
    validatedResult = validated;
    
    return res;
}

int _ParamsStruct::validateAudioParams(const struct _ParamsStruct &params,
                                       struct _ParamsStruct &validatedResult)
{
    // check critical values
    if (params.host == NULL ||
        strcmp(params.host, "") == 0)
        return RESULT_ERR;
    
    if (params.producerId == NULL ||
        strcmp(params.producerId, "") == 0)
        return RESULT_ERR;
    
    if (params.streamName == NULL ||
        strcmp(params.streamName, "") == 0)
        return RESULT_ERR;
    
    if (params.ndnHub == NULL ||
        strcmp(params.ndnHub, "") == 0)
        return RESULT_ERR;
    
    // check other values
    int res = RESULT_OK;
    ParamsStruct validated = DefaultParamsAudio;
    
    validated.loggingLevel = params.loggingLevel;
    validated.logFile = params.logFile;
    validated.useTlv = params.useTlv;
    validated.useRtx = params.useRtx;
    validated.useFec = false;
    validated.useCache = params.useCache;
    validated.useAvSync = params.useAvSync;
    validated.useAudio = params.useAudio;
    validated.useVideo = params.useVideo;
    validated.headlessMode = params.headlessMode;
    
    validated.host = params.host;
    validated.portNum = ParamsStruct::validateLE(params.portNum, MaxPortNum,
                                                 res, DefaultParamsAudio.portNum);
    
    validated.producerId = params.producerId;
    validated.streamName = params.streamName;
    validated.ndnHub = validatePrefix(params.ndnHub);
    
    validated.nStreams = params.nStreams;
    validated.streamsParams = (CodecParams*)malloc(sizeof(CodecParams)*validated.nStreams);
    memcpy(validated.streamsParams, params.streamsParams, sizeof(CodecParams)*validated.nStreams);
    
    validated.segmentSize = ParamsStruct::validate(params.segmentSize,
                                                   MinSegmentSize, MaxSegmentSize, res,
                                                   DefaultParamsAudio.segmentSize);
    validated.freshness = params.freshness;
    validated.producerRate = ParamsStruct::validate(params.producerRate,
                                                    MinFrameRate, MaxFrameRate, res,
                                                    DefaultParamsAudio.producerRate);
    validated.playbackRate = ParamsStruct::validate(params.playbackRate,
                                                    MinFrameRate, MaxFrameRate, res,
                                                    DefaultParamsAudio.playbackRate);
    validated.interestTimeout = params.interestTimeout;
    validated.bufferSize = ParamsStruct::validate(params.bufferSize,
                                                  MinBufferSize, MaxBufferSize, res,
                                                  DefaultParamsAudio.bufferSize);
    validated.jitterSize = ParamsStruct::validate(params.jitterSize,
                                                  MinJitterSize, INT_MAX, res,
                                                  DefaultParams.jitterSize);

    validated.slotSize = ParamsStruct::validate(params.slotSize, MinSlotSize,
                                                MaxSlotSize, res,
                                                DefaultParamsAudio.slotSize);
    validated.skipIncomplete = params.skipIncomplete;
    
    if (validated.slotSize < validated.segmentSize)
    {
        validated.slotSize = validated.segmentSize;
        res = RESULT_WARN;
    }
    
    validatedResult = validated;
    
    return res;
}