#import <AppKit/AppKit.h>
#import "APPMacDemoDelegate.h"

int main(int argc, char* argv[]) {
  @autoreleasepool {
    [NSApplication sharedApplication];
    APPMacDemoDelegate* delegate = [[APPMacDemoDelegate alloc] init];
    [NSApp setDelegate:delegate];
    [NSApp run];
  }
}
