#import "RTCDemoClient.h"
#import "device/APPCaptureController.h"
#import "device/APPSettingsModel.h"

#import <WebRTC/RTCDefaultVideoDecoderFactory.h>
#import <WebRTC/RTCDefaultVideoEncoderFactory.h>
#import <WebRTC/RTCCameraVideoCapturer.h>
#import <WebRTC/RTCVideoSource.h>
#import <WebRTC/RTCVideoTrack.h>
#import <WebRTC/RTCPeerConnectionFactory.h>

#import "RTCLogging.h"

//#include "../../hello/api_wrapper.h"

static NSString * const kARDMediaStreamId = @"ARDAMS";

@interface RTCDemoClient()

@property(nonatomic, strong) RTCPeerConnectionFactory *factory;
@property(nonatomic, strong) RTCVideoSource *source;
@property(nonatomic, strong) APPCaptureController* captureController;
@property(nonatomic, strong) RTCVideoTrack* localVideoTrack;
@property(nonatomic, strong) RTCVideoTrack* remoteVideoTrack;
@property(nonatomic, strong) RTCCameraVideoCapturer *capturer;

@end

@implementation RTCDemoClient{
    
    BOOL _isStartCapture;
    //ApiWrapper *apiWrapper_;
}

@synthesize factory = _factory;
@synthesize source = _source;
@synthesize captureController = _captureController;
@synthesize localVideoTrack = _localVideoTrack;
@synthesize remoteVideoTrack = _remoteVideoTrack;
@synthesize capturer = _capturer;

- (instancetype)init
{
    if (self = [super init]) {
        
        APPSettingsModel *settings = [[APPSettingsModel alloc] init];
        
        RTCDefaultVideoDecoderFactory *decoderFactory = [[RTCDefaultVideoDecoderFactory alloc] init];
        RTCDefaultVideoEncoderFactory *encoderFactory = [[RTCDefaultVideoEncoderFactory alloc] init];
        encoderFactory.preferredCodec = [settings currentVideoCodecSettingFromStore];
        _factory = [[RTCPeerConnectionFactory alloc] initWithEncoderFactory:encoderFactory decoderFactory:decoderFactory];
        
        //apiWrapper_ = new ApiWrapper();
        
        _isStartCapture = NO;
    }
    return self;
}

-(int)startCapture:(id<RTCVideoRenderer>) renderer{
    
    self.source = [_factory videoSource];
    self.capturer = [[RTCCameraVideoCapturer alloc] initWithDelegate:_source];
    self.captureController = [[APPCaptureController alloc] initWithCapturer:_capturer settings:[[APPSettingsModel alloc] init]];
    [self.captureController startCapture];
    _isStartCapture = YES;
    
    self.localVideoTrack = [_factory videoTrackWithSource:_source trackId:kARDMediaStreamId];
    [self.localVideoTrack addRenderer:renderer];
    
    return 0;
}

-(int)stopCapture{
    
    [self.captureController stopCapture];
    self.captureController = nil;
    _isStartCapture = NO;
    
    return 0;
}

-(BOOL)isStartCapture{
    return _isStartCapture;
}

-(int)test{
    
    //apiWrapper_->testMediaChannel();
    
    return 0;
}

-(void)dealloc
{
    self.source = nil;
    [super dealloc];
}

@end
