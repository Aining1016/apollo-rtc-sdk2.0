#import "APPMacDemoDelegate.h"
#import "APPDemoViewController.h"
//#import <WebRTC/RTCSSLAdapter.h>

@interface APPMacDemoDelegate () <NSWindowDelegate>
@end

@implementation APPMacDemoDelegate {
  APPDemoViewController* _viewController;
  NSWindow* _window;
}

#pragma mark - NSApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  //RTCInitializeSSL();
  NSScreen* screen = [NSScreen mainScreen];
  NSRect visibleRect = [screen visibleFrame];
  NSRect windowRect = NSMakeRect(NSMidX(visibleRect),
                                 NSMidY(visibleRect),
                                 800,
                                 800);
  NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask;
  _window = [[NSWindow alloc] initWithContentRect:windowRect
                                        styleMask:styleMask
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
  _window.delegate = self;
  [_window makeKeyAndOrderFront:self];
  [_window makeMainWindow];
  _viewController = [[APPDemoViewController alloc] initWithNibName:nil
                                                           bundle:nil];
  [_window setContentView:[_viewController view]];
}

#pragma mark - NSWindow

- (void)windowWillClose:(NSNotification*)notification {
  [_viewController windowWillClose:notification];
  //RTCCleanupSSL();
  [NSApp terminate:self];
}

@end

