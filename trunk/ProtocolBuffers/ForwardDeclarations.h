//
//  ForwardDeclarations.h
//  ProtocolBuffers
//
//  Created by Cyrus Najmabadi on 9/26/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

enum FieldDescriptorType;
enum ObjectiveCType;

@protocol Message;
@protocol Message_Builder;
@protocol RpcChannel;
@protocol RpcController;
@protocol Service;

@class AbstractMessage;
@class CodedInputStream;
@class CodedOutputStream;
@class Descriptors;
@class Descriptor;
@class Field;
@class Field_Builder;
@class FieldDescriptor;
@class FieldDescriptor_Type;
@class FieldSet;
@class FileDescriptor;
@class MethodDescriptor;
@class ServiceDescriptor;
@class DynamicMessage;
@class DynamicMessage_Builder;
@class ExtensionRegistry;
@class UnknownFieldSet;
@class UnknownFieldSet_Builder;
@class UnknownFieldSet_Field;
@class UnknownFieldSet_Field_Builder;
@class WireFormat;