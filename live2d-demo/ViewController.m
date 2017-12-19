//
//  ViewController.m
//  live2d-demo
//
//  Created by Dylan Wang on 08/12/2017.
//  Copyright © 2017 Chaoxin. All rights reserved.
//

#import "ViewController.h"
#import "Live2dHelper.h"

@interface ViewController ()<GLKViewControllerDelegate>

@property (nonatomic, strong) Live2dHelper *runner;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    GLKView *view = (GLKView *)self.view;
    view.backgroundColor = [UIColor whiteColor];
    view.context= [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:view.context];
    _runner = [Live2dHelper new];
    self.delegate = self;
}

#pragma mark -
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    // TODO: there are some bug to fix
    [_runner onTick:self.timeSinceLastUpdate];
}

- (void)glkViewControllerUpdate:(GLKViewController *)controller {
    
}

@end

