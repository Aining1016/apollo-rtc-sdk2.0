#import "APPDemoViewController.h"
#import "RTCDemoClient.h"
#import <AVFoundation/AVFoundation.h>
#import <WebRTC/RTCMTLNSVideoView.h>
#import <WebRTC/RTCNSGLVideoView.h>
#import <WebRTC/RTCVideoTrack.h>

static NSUInteger const kFrameWidth = 800;
static NSUInteger const kFrameHeight = 500;

@class APPDemoMainView;
@protocol APPDemoMainViewDelegate

- (void)appRTCMainView:(APPDemoMainView*)mainView onOpenOrCloseCameraClick:(NSButton *)btOpenOrCloseCamera;
- (void)appRTCMainView:(APPDemoMainView*)mainView onTest:(NSButton *)btTest;

@end

@interface APPDemoMainView : NSView

@property(nonatomic, strong) id<APPDemoMainViewDelegate> delegate;
@property(nonatomic, readonly) NSView<RTCVideoRenderer>* localVideoView;
@property(nonatomic, readonly) NSView<RTCVideoRenderer>* remoteVideoView;

@end

@implementation APPDemoMainView{
    
    NSButton *btOpenOrCloseCamera;
    NSButton *btTest;
}

@synthesize delegate = _delegate;
@synthesize localVideoView = _localVideoView;
@synthesize remoteVideoView = _remoteVideoView;

#pragma mark - APPDemoMainView Private
- (instancetype)initWithFrame:(NSRect)frame {
  if (self = [super initWithFrame:frame]) {
      
      [self setupViews];
  }
  return self;
}

#pragma mark - APPDemoMainView Private
- (void)setupViews {
    
#pragma clang diagnostic ignored "-Wpartial-availability"
    if ([RTCMTLNSVideoView class] && [RTCMTLNSVideoView isMetalAvailable]) {
      _localVideoView = [[RTCMTLNSVideoView alloc] initWithFrame:CGRectMake(10, 500, 300, 300)];
      _remoteVideoView = [[RTCMTLNSVideoView alloc] initWithFrame:CGRectMake(310, 500, 300, 300)];
    }
    
    //[_localVideoView setTranslatesAutoresizingMaskIntoConstraints:NO];
    //_localVideoView.layer.backgroundColor = [NSColor colorWithRed:0.8 green:0 blue:0 alpha:1.0].CGColor;
    
    [self addSubview:_localVideoView];
    [self addSubview:_remoteVideoView];
    
    btOpenOrCloseCamera = [[NSButton alloc] initWithFrame:CGRectMake(10, 400, 80, 30)];
    [btOpenOrCloseCamera setTitle:@"start capture"];
    [btOpenOrCloseCamera setTarget:self];
    [btOpenOrCloseCamera setAction:@selector(onOpenOrCloseCameraClick)];
    [self addSubview:btOpenOrCloseCamera];
    
    btTest = [[NSButton alloc] initWithFrame:CGRectMake(10, 200, 80, 30)];
    [btOpenOrCloseCamera setTitle:@"start test"];
    [btOpenOrCloseCamera setTarget:self];
    [btOpenOrCloseCamera setAction:@selector(onOpenOrCloseCameraClick)];
    [self addSubview:btTest];
}

-(void)onOpenOrCloseCameraClick{
    
    [self.delegate appRTCMainView:self onOpenOrCloseCameraClick: btOpenOrCloseCamera];
}

-(void)onTest{
    
    [self.delegate appRTCMainView:self onTest: btTest];
}

@end

@interface APPDemoViewController ()<APPDemoMainViewDelegate>
//    <ARDAppClientDelegate, APPRTCMainViewDelegate>

//@property(nonatomic, readonly) APPDemoMainView* mainView;
@end

@implementation APPDemoViewController {

    RTCDemoClient *rtcDemoClient;
}

- (void)loadView {

    NSLog(@"loadView");
    
    rtcDemoClient = [[RTCDemoClient alloc] init];
    
    APPDemoMainView* view = [[APPDemoMainView alloc] initWithFrame:CGRectMake(0, 0, kFrameWidth, kFrameHeight)];
    self.view = view;
    view.delegate = self;
}

- (void)windowWillClose:(NSNotification*)notification {
  
}

- (APPDemoMainView*)mainView {
  return (APPDemoMainView*)self.view;
}

#pragma mark - APPDemoMainViewDelegate
- (void)appRTCMainView:(APPDemoMainView*)mainView onOpenOrCloseCameraClick:(NSButton *)btOpenOrCloseCamera{
    
    BOOL isStartCapture = [rtcDemoClient isStartCapture];
    if(isStartCapture){
        [rtcDemoClient stopCapture];
        [btOpenOrCloseCamera setTitle:@"start capture"];
    }else{
        [rtcDemoClient startCapture:self.mainView.localVideoView];
        [btOpenOrCloseCamera setTitle:@"stop capture"];
    }
}

- (void)appRTCMainView:(APPDemoMainView*)mainView onTest:(NSButton *)btTest{
    
    
}

@end
