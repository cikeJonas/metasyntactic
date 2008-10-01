// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.
// http://code.google.com/p/protobuf/
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

@interface PBFileDescriptor : NSObject {
    NSArray* messageTypes;
}

@property (retain) NSArray* messageTypes;

+ (PBFileDescriptor*) buildFrom:(PBFileDescriptorProto*) proto dependencies:(NSArray*) dependencies;
+ (PBFileDescriptor*) internalBuildGeneratedFileFrom:(NSString*) descriptorData dependencies:(NSArray*) dependencies;

- (NSArray*) getMessageTypes;

- (void) crossLink;


#if 0
    FileDescriptorProto* proto;
    NSArray* messageTypes;
    NSArray* enumTypes;
    NSArray* services;
    NSArray* extensions;
    NSArray* dependencies;
    NSArray* DescriptorPool* pool;
}

@property (retain) FileDescriptorProto* proto;
@property (retain) NSArray* enumTypes;
@property (retain) NSArray* services;
@property (retain) NSArray* extensions;
@property (retain) NSArray* dependencies;
@property (retain) NSArray* DescriptorPool* pool;

+ (PBFileDescriptor*) descriptorWithProto:(FileDescriptorProto*) proto
                           dependencies:(NSArray*) dependencies
                                   pool:(DescriptorPool*) pool;

- (FieldDescriptorProto*) toProto;
- (NSString*) name;
- (NSString*) package;
- (FileOptions*) options;
- (NSArray*) messageTypes;
- (NSArray*) enumTypes;
- (NSArray*) services;
- (NSArray*) extensions;
- (NSArray*) dependencies;

- (PBDescriptor*) findMessageTypeByName:(NSString*) name;
- (PBEnumDescriptor*) findEnumTypeByName:(NSString*) name;
- (ServiceDescriptor*) findServiceByName:(NSString*) name;
- (PBFieldDescriptor*) findExtensionByName:(NSString*) name;
#endif

@end
