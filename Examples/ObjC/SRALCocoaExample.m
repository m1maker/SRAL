#import <AppKit/AppKit.h>

#define SRAL_STATIC
#include <SRAL.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@property (strong) NSTextField *engineLabel;
@property (strong) NSTextField *speakingLabel;
@property (strong) NSTimer *speakingTimer;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSRect frame = NSMakeRect(200, 200, 400, 200);
    self.window = [[NSWindow alloc]
        initWithContentRect:frame
                  styleMask:(NSWindowStyleMaskTitled |
                             NSWindowStyleMaskClosable |
                             NSWindowStyleMaskMiniaturizable)
                    backing:NSBackingStoreBuffered
                      defer:NO];
    [self.window setTitle:@"SRAL Cocoa Example"];

    self.engineLabel = [NSTextField labelWithString:@"Engine: (initializing...)"];
    [self.engineLabel setFrame:NSMakeRect(20, 150, 360, 20)];
    [[self.window contentView] addSubview:self.engineLabel];

    self.speakingLabel = [NSTextField labelWithString:@"Speaking: No"];
    [self.speakingLabel setFrame:NSMakeRect(20, 125, 360, 20)];
    [[self.window contentView] addSubview:self.speakingLabel];

    NSButton *speakButton = [NSButton buttonWithTitle:@"Speak"
                                               target:self
                                               action:@selector(speakClicked:)];
    [speakButton setFrame:NSMakeRect(125, 70, 150, 40)];
    [[self.window contentView] addSubview:speakButton];

    if (!SRAL_Initialize(0)) {
        [self.engineLabel setStringValue:@"Engine: Failed to initialize SRAL!"];
        return;
    }

    int engine = SRAL_GetCurrentEngine();
    const char *name = SRAL_GetEngineName(engine);
    [self.engineLabel setStringValue:
        [NSString stringWithFormat:@"Engine: %s", name ? name : "Unknown"]];

    // Continuously poll engine and speaking status
    self.speakingTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                                                          target:self
                                                        selector:@selector(updateStatus:)
                                                        userInfo:nil
                                                         repeats:YES];

    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

- (void)speakClicked:(id)sender {
    SRAL_Speak("Hello, this is a test of the SRAL library.", true);
}

- (void)updateStatus:(NSTimer *)timer {
    int engine = SRAL_GetCurrentEngine();
    const char *name = SRAL_GetEngineName(engine);
    [self.engineLabel setStringValue:
        [NSString stringWithFormat:@"Engine: %s", name ? name : "None"]];

    [self.speakingLabel setStringValue:
        SRAL_IsSpeaking() ? @"Speaking: Yes" : @"Speaking: No"];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    [self.speakingTimer invalidate];
    SRAL_Uninitialize();
}

@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        AppDelegate *delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];

        [app run];
    }
    return 0;
}
