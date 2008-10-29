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

#import "Application.h"

#import "DifferenceEngine.h"
#import "FileUtilities.h"
#import "LocaleUtilities.h"
#import "Utilities.h"

@implementation Application

static NSLock* gate = nil;

// Special directories
static NSString* cacheFolder = nil;
static NSString* supportFolder = nil;
static NSString* tempFolder = nil;

// Application storage directories
static NSString* dataFolder = nil;
static NSString* imdbFolder = nil;
static NSString* userLocationsFolder = nil;
static NSString* scoresFolder = nil;
static NSString* reviewsFolder = nil;
static NSString* trailersFolder = nil;
static NSString* postersFolder = nil;

static NSString* dvdFolder = nil;
static NSString* dvdPostersFolder = nil;

/*
static NSString* numbersFolder = nil;
static NSString* numbersBudgetsFolder = nil;
static NSString* numbersDailyFolder = nil;
static NSString* numbersWeekendFolder = nil;
*/

static NSString* upcomingFolder = nil;
static NSString* upcomingCastFolder = nil;
static NSString* upcomingIMDbFolder = nil;
static NSString* upcomingPostersFolder = nil;
static NSString* upcomingSynopsesFolder = nil;
static NSString* upcomingTrailersFolder = nil;

static NSString** folders[] = {
    &dataFolder,
    &imdbFolder,
    &userLocationsFolder,
    &dvdFolder,
    &dvdPostersFolder,
/*
    &numbersFolder,
    &numbersBudgetsFolder,
    &numbersDailyFolder,
    &numbersWeekendFolder,
 */
    &scoresFolder,
    &reviewsFolder,
    &trailersFolder,
    &postersFolder,
    &upcomingFolder,
    &upcomingCastFolder,
    &upcomingIMDbFolder,
    &upcomingPostersFolder,
    &upcomingSynopsesFolder,
    &upcomingTrailersFolder
};


static DifferenceEngine* differenceEngine = nil;
static NSString* starString = nil;


+ (NSString*) cacheFolder {
    [gate lock];
    {
        if (cacheFolder == nil) {
            NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, /*expandTilde:*/YES);

            NSString* executableName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleExecutable"];
            NSString* folder = [[paths objectAtIndex:0] stringByAppendingPathComponent:executableName];

            [FileUtilities createDirectory:folder];

            cacheFolder = [folder retain];
        }
    }
    [gate unlock];

    return cacheFolder;
}


+ (NSString*) supportFolder {
    [gate lock];
    {
        if (supportFolder == nil) {
            NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, /*expandTilde:*/YES);

            NSString* executableName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleExecutable"];
            NSString* folder = [[paths objectAtIndex:0] stringByAppendingPathComponent:executableName];

            [FileUtilities createDirectory:folder];

            supportFolder = [folder retain];
        }
    }
    [gate unlock];

    return supportFolder;
}


+ (NSString*) tempFolder {
    [gate lock];
    {
        if (tempFolder == nil) {
            tempFolder = [NSTemporaryDirectory() retain];
        }
    }
    [gate unlock];

    return tempFolder;
}


+ (void) deleteFolders {
    [gate lock];
    {
        for (int i = 0; i < ArrayLength(folders); i++) {
            NSString** folderReference = folders[i];
            NSString* folder = *folderReference;

            if (folder != nil) {
                [FileUtilities removeItem:folder];
            }
        }
    }
    [gate unlock];
}


+ (void) createFolders {
    [gate lock];
    {
        for (int i = 0; i < ArrayLength(folders); i++) {
            NSString** folderReference = folders[i];
            NSString* folder = *folderReference;

            [FileUtilities createDirectory:folder];
        }
    }
    [gate unlock];
}


+ (void) initialize {
    if (self == [Application class]) {
        gate = [[NSRecursiveLock alloc] init];

        differenceEngine = [[DifferenceEngine engine] retain];

        [FileUtilities removeItem:[self supportFolder]];

        {
            dataFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"Data"] retain];
            imdbFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"IMDb"] retain];
            userLocationsFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"UserLocations"] retain];
            scoresFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"Scores"] retain];
            reviewsFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"Reviews"] retain];
            trailersFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"Trailers"] retain];
            postersFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"Posters"] retain];

            dvdFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"DVD"] retain];
            dvdPostersFolder = [[[self dvdFolder] stringByAppendingPathComponent:@"Posters"] retain];

            upcomingFolder = [[[self cacheFolder] stringByAppendingPathComponent:@"Upcoming"] retain];
            upcomingCastFolder = [[[self upcomingFolder] stringByAppendingPathComponent:@"Cast"] retain];
            upcomingIMDbFolder = [[[self upcomingFolder] stringByAppendingPathComponent:@"IMDb"] retain];
            upcomingPostersFolder = [[[self upcomingFolder] stringByAppendingPathComponent:@"Posters"] retain];
            upcomingSynopsesFolder = [[[self upcomingFolder] stringByAppendingPathComponent:@"Synopses"] retain];
            upcomingTrailersFolder = [[[self upcomingFolder] stringByAppendingPathComponent:@"Trailers"] retain];

            [self createFolders];
//            static NSString* numbersFolder = nil;
//            static NSString* numbersBudgetsFolder = nil;
//            static NSString* numbersDailyFolder = nil;
//            static NSString* numbersWeekendFolder = nil;
        }
    }
}


+ (void) resetFolders {
    [self deleteFolders];
    [self createFolders];
}


+ (NSString*) dataFolder {
    return dataFolder;
}


+ (NSString*) imdbFolder {
    return imdbFolder;
}


+ (NSString*) userLocationsFolder {
    return userLocationsFolder;
}

/*
+ (NSString*) numbersFolder {
    return [self createFolder:&numbersFolder parent:[Application cacheFolder] name:@"Numbers"];
}


+ (NSString*) numbersDetailsFolder {
    return [self createFolder:&numbersWeekendFolder parent:[Application numbersFolder] name:@"Details"];
}
 */


+ (NSString*) postersFolder {
    return postersFolder;
}


+ (NSString*) scoresFolder {
    return scoresFolder;
}


+ (NSString*) reviewsFolder {
    return reviewsFolder;
}


+ (NSString*) trailersFolder {
    return trailersFolder;
}


+ (NSString*) dvdFolder {
    return dvdFolder;
}


+ (NSString*) dvdPostersFolder {
    return dvdPostersFolder;
}


+ (NSString*) upcomingFolder {
    return upcomingFolder;
}


+ (NSString*) upcomingCastFolder {
    return upcomingCastFolder;
}


+ (NSString*) upcomingIMDbFolder {
    return upcomingIMDbFolder;
}


+ (NSString*) upcomingPostersFolder {
    return upcomingPostersFolder;
}


+ (NSString*) upcomingSynopsesFolder {
    return upcomingSynopsesFolder;
}


+ (NSString*) upcomingTrailersFolder {
    return upcomingTrailersFolder;
}


+ (NSString*) randomString {
    NSMutableString* string = [NSMutableString string];
    for (int i = 0; i < 8; i++) {
        [string appendFormat:@"%c", ((rand() % 26) + 'a')];
    }
    return string;
}


+ (NSString*) uniqueTemporaryFolder {
    NSString* finalDir;

    [gate lock];
    {
        NSFileManager* manager = [NSFileManager defaultManager];

        NSString* tempDir = [Application tempFolder];
        do {
            NSString* random = [Application randomString];
            finalDir = [tempDir stringByAppendingPathComponent:random];
        } while ([manager fileExistsAtPath:finalDir]);

        [FileUtilities createDirectory:finalDir];
    }
    [gate unlock];

    return finalDir;
}


+ (void) openBrowser:(NSString*) address {
    if (address.length == 0) {
        return;
    }

    NSURL* url = [NSURL URLWithString:address];
    [[UIApplication sharedApplication] openURL:url];
}


+ (void) openMap:(NSString*) address {
    /*
    NSString* urlString =
    [NSString stringWithFormat:@"http://maps.google.com/maps?q=%@",
     [Utilities stringByAddingPercentEscapes:address]];
     [self openBrowser:urlString];
*/

    [self openBrowser:address];
}


+ (void) makeCall:(NSString*) phoneNumber {
    if (![[[UIDevice currentDevice] model] isEqual:@"iPhone"]) {
        // can't make a phonecall if you're not an iPhone.
        return;
    }

    NSString* urlString = [NSString stringWithFormat:@"tel:%@", phoneNumber];

    [self openBrowser:urlString];
}


+ (DifferenceEngine*) differenceEngine {
    NSAssert([NSThread isMainThread], @"Cannot access difference engine from background thread.");
    return differenceEngine;
}


+ (NSString*) host {
#if !TARGET_IPHONE_SIMULATOR
    return @"metaboxoffice2";
#endif
    /*
    return @"metaboxoffice6";
    /*/
     return @"metaboxoffice2";
    //*/
}


+ (unichar) starCharacter {
    return (unichar)0x2605;
}


+ (NSString*) starString {
    if (starString == nil) {
        unichar c = [Application starCharacter];
        starString = [NSString stringWithCharacters:&c length:1];
    }

    return starString;
}


+ (BOOL) useKilometers {
    // yeah... so the UK supposedly uses metric...
    // except they don't. so we special case them to stick with 'miles' in the UI.
    BOOL isMetric = [[[NSLocale currentLocale] objectForKey:NSLocaleUsesMetricSystem] boolValue];
    BOOL isUK = [@"GB" isEqual:[LocaleUtilities isoCountry]];
    
    return isMetric && !isUK;
}


@end