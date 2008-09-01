// Copyright (C) 2008 Cyrus Najmabadi
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#import "TextFieldEditorViewController.h"

@implementation TextFieldEditorViewController

@synthesize textField;
@synthesize messageLabel;

- (void) dealloc {
    self.textField = nil;
    self.messageLabel = nil;

    [super dealloc];
}


- (id) initWithController:(AbstractNavigationController*) controller
                    title:(NSString*) title
                   object:(id) object_
                 selector:(SEL) selector_
                     text:(NSString*) text
                  message:(NSString*) message
              placeHolder:(NSString*) placeHolder
                     type:(UIKeyboardType) type {
    if (self = [super initWithController:controller withObject:object_ withSelector:selector_]) {
        self.textField = [[[UITextField alloc] initWithFrame:CGRectZero] autorelease];
        textField.autoresizingMask = UIViewAutoresizingFlexibleWidth;

        textField.text = text;
        textField.placeholder = placeHolder;
        textField.borderStyle = UITextBorderStyleRoundedRect;
        textField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        textField.font = [UIFont boldSystemFontOfSize:17];
        textField.keyboardType = type;
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.delegate = self;
        textField.autocorrectionType = UITextAutocorrectionTypeNo;

        self.messageLabel = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
        messageLabel.backgroundColor = [UIColor clearColor];
        messageLabel.text = message;
        messageLabel.textColor = [UIColor grayColor];
        [messageLabel sizeToFit];
        
        self.title = title;
    }

    return self;
}


- (void) loadView {
    [super loadView];
    
    [self.view addSubview:textField];
    textField.frame = CGRectMake(20, 50, self.view.frame.size.width - 40, 30);;
            
    [self.view addSubview:messageLabel];
    CGRect frame = messageLabel.frame;
    frame.origin.x = 20;
    frame.origin.y = 90;
    messageLabel.frame = frame;
    
    [textField becomeFirstResponder];
}


- (void) save:(id) sender {
    NSString* trimmedValue = [textField.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    [object performSelector:selector withObject:trimmedValue];
    [super save:sender];
}


- (BOOL) textFieldShouldClear:(UITextField*) textField {
    messageLabel.text = @"";
    return YES;
}


@end