#import <AppKit/AppKit.h>
#import <WebRTC/RTCMTLNSVideoView.h>
#import <WebRTC/RTCNSGLVideoView.h>

@interface RTCDemoClient : NSObject

- (instancetype)init;
-(int)startCapture:(id<RTCVideoRenderer>) renderer;
-(int)stopCapture;
-(BOOL)isStartCapture;
-(void)dealloc;
-(int)test;
@end
