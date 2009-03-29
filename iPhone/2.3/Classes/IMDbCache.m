// Copyright 2008 Cyrus Najmabadi
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#import "IMDbCache.h"

#import "Application.h"
#import "FileUtilities.h"
#import "Movie.h"
#import "NetworkUtilities.h"
#import "NowPlayingAppDelegate.h"
#import "NowPlayingModel.h"
#import "ThreadingUtilities.h"
#import "Utilities.h"

@interface IMDbCache()
@property (assign) NowPlayingModel* model;
@property (retain) NSLock* gate;
@end


@implementation IMDbCache

@synthesize model;
@synthesize gate;

- (void) dealloc {
    self.model = nil;
    self.gate = nil;

    [super dealloc];
}


- (id) initWithModel:(NowPlayingModel*) model_ {
    if (self = [super init]) {
        self.model = model_;
        self.gate = [[[NSRecursiveLock alloc] init] autorelease];
    }

    return self;
}


+ (IMDbCache*) cacheWithModel:(NowPlayingModel*) model {
    return [[[IMDbCache alloc] initWithModel:model] autorelease];
}


- (NSString*) imdbFile:(Movie*) movie {
    NSString* name = [[FileUtilities sanitizeFileName:movie.canonicalTitle] stringByAppendingPathExtension:@"plist"];
    return [[Application imdbDirectory] stringByAppendingPathComponent:name];
}


- (void) update:(NSArray*) movies {
    if (model.userAddress.length == 0) {
        return;
    }

    [ThreadingUtilities performSelector:@selector(backgroundEntryPoint:)
                               onTarget:self
               inBackgroundWithArgument:movies
                                   gate:gate
                                visible:NO];
}


- (void) downloadAddress:(Movie*) movie {
    if (movie.imdbAddress.length > 0) {
        // don't even bother if the movie has an imdb address in it
        return;
    }

    NSString* path = [self imdbFile:movie];
    NSDate* lastLookupDate = [FileUtilities modificationDate:path];

    if (lastLookupDate != nil) {
        NSString* value = [FileUtilities readObject:path];
        if (value.length > 0) {
            // we have a real imdb value for this movie
            return;
        }

        // we have a sentinel.  only update if it's been long enough
        if (ABS([lastLookupDate timeIntervalSinceNow]) < (3 * ONE_DAY)) {
            return;
        }
    }

    NSString* url = [NSString stringWithFormat:@"http://%@.appspot.com/LookupIMDbListings?q=%@", [Application host], [Utilities stringByAddingPercentEscapes:movie.canonicalTitle]];
    NSString* imdbAddress = [NetworkUtilities stringWithContentsOfAddress:url important:NO];
    if (imdbAddress == nil) {
        return;
    }

    // write down the response (even if it is empty).  An empty value will
    // ensure that we don't update this entry too often.
    [FileUtilities writeObject:imdbAddress toFile:path];
    [NowPlayingAppDelegate refresh];
}


- (void) deleteObsoleteAddresses:(NSArray*) movies {
    NSArray* paths = [FileUtilities directoryContentsPaths:[Application imdbDirectory]];
    NSMutableSet* set = [NSMutableSet setWithArray:paths];

    for (Movie* movie in movies) {
        NSString* filePath = [self imdbFile:movie];
        [set removeObject:filePath];
    }

    for (NSString* filePath in set) {
        NSDate* downloadDate = [FileUtilities modificationDate:filePath];

        if (downloadDate != nil) {
            if (ABS(downloadDate.timeIntervalSinceNow) > ONE_MONTH) {
                [FileUtilities removeItem:filePath];
            }
        }
    }
}


- (void) backgroundEntryPoint:(NSArray*) movies {
    [self deleteObsoleteAddresses:movies];

    for (Movie* movie in movies) {
        [self downloadAddress:movie];
    }
}


- (NSString*) imdbAddressForMovie:(Movie*) movie {
    return [FileUtilities readObject:[self imdbFile:movie]];
}

@end