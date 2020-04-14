#ifndef __APOLLO_INTERNAL_TYPES_H__
#define __APOLLO_INTERNAL_TYPES_H__

#include "media/base/codec.h"

namespace webrtc {

const cricket::AudioCodec kPcmuCodec(0, "PCMU", 8000, 64000, 1);
const cricket::AudioCodec kIsacCodec(103, "ISAC", 16000, 32000, 1);
const cricket::AudioCodec kOpusCodec(111, "opus", 48000, 32000, 2);
const cricket::AudioCodec kG722CodecVoE(9, "G722", 16000, 64000, 1);
const cricket::AudioCodec kG722CodecSdp(9, "G722", 8000, 64000, 1);
const cricket::AudioCodec kCn8000Codec(13, "CN", 8000, 0, 1);
const cricket::AudioCodec kCn16000Codec(105, "CN", 16000, 0, 1);
const cricket::AudioCodec kTelephoneEventCodec1(106,
                                                "telephone-event",
                                                8000,
                                                0,
                                                1);
const cricket::AudioCodec kTelephoneEventCodec2(107,
                                                "telephone-event",
                                                32000,
                                                0,
                                                1);
const cricket::VideoCodec kVp8Codec(96, "VP8");
const cricket::VideoCodec kVp9codec(98, "VP9");
const cricket::VideoCodec kH264Codec(97, "H264");
const cricket::VideoCodec kH264SvcCodec(99, "H264-SVC");
const cricket::DataCodec  kGoogleDataCodec(101, "google-data");



}//namespace webrtc

#endif
