// Copyright (c) 2008-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/mac_util.h"

#import <Cocoa/Cocoa.h>

#include "base/file_path.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/scoped_cftyperef.h"
#include "base/sys_string_conversions.h"

namespace mac_util {

std::string PathFromFSRef(const FSRef& ref) {
  scoped_cftyperef<CFURLRef> url(
      CFURLCreateFromFSRef(kCFAllocatorDefault, &ref));
  NSString *path_string = [(NSURL *)url.get() path];
  return [path_string fileSystemRepresentation];
}

bool FSRefFromPath(const std::string& path, FSRef* ref) {
  OSStatus status = FSPathMakeRef((const UInt8*)path.c_str(),
                                  ref, nil);
  return status == noErr;
}

// Adapted from http://developer.apple.com/carbon/tipsandtricks.html#AmIBundled
bool AmIBundled() {
  ProcessSerialNumber psn = {0, kCurrentProcess};

  FSRef fsref;
  if (GetProcessBundleLocation(&psn, &fsref) != noErr)
    return false;

  FSCatalogInfo info;
  if (FSGetCatalogInfo(&fsref, kFSCatInfoNodeFlags, &info,
                       NULL, NULL, NULL) != noErr) {
    return false;
  }

  return info.nodeFlags & kFSNodeIsDirectoryMask;
}

bool IsBackgroundOnlyProcess() {
  // This function really does want to examine NSBundle's idea of the main
  // bundle dictionary, and not the overriden MainAppBundle.  It needs to look
  // at the actual running .app's Info.plist to access its LSUIElement
  // property.
  NSDictionary* info_dictionary = [[NSBundle mainBundle] infoDictionary];
  return [[info_dictionary objectForKey:@"LSUIElement"] boolValue] != NO;
}

// No threading worries since NSBundle isn't thread safe.
static NSBundle* g_override_app_bundle = nil;

NSBundle* MainAppBundle() {
  if (g_override_app_bundle)
    return g_override_app_bundle;
  return [NSBundle mainBundle];
}

FilePath MainAppBundlePath() {
  NSBundle* bundle = MainAppBundle();
  return FilePath([[bundle bundlePath] fileSystemRepresentation]);
}

void SetOverrideAppBundle(NSBundle* bundle) {
  if (bundle != g_override_app_bundle) {
    [g_override_app_bundle release];
    g_override_app_bundle = [bundle retain];
  }
}

void SetOverrideAppBundlePath(const FilePath& file_path) {
  NSString* path = base::SysUTF8ToNSString(file_path.value());
  NSBundle* bundle = [NSBundle bundleWithPath:path];
  CHECK(bundle) << "Failed to load the bundle at " << file_path.value();

  SetOverrideAppBundle(bundle);
}

OSType CreatorCodeForCFBundleRef(CFBundleRef bundle) {
  OSType creator = kUnknownType;
  CFBundleGetPackageInfo(bundle, NULL, &creator);
  return creator;
}

OSType CreatorCodeForApplication() {
  CFBundleRef bundle = CFBundleGetMainBundle();
  if (!bundle)
    return kUnknownType;

  return CreatorCodeForCFBundleRef(bundle);
}

bool GetUserDirectory(NSSearchPathDirectory directory, FilePath* result) {
  DCHECK(result);
  NSArray* dirs =
      NSSearchPathForDirectoriesInDomains(directory, NSUserDomainMask, YES);
  if ([dirs count] < 1) {
    return false;
  }
  NSString* path = [dirs objectAtIndex:0];
  *result = FilePath([path fileSystemRepresentation]);
  return true;
}

FilePath GetUserLibraryPath() {
  FilePath user_library_path;
  if (!GetUserDirectory(NSLibraryDirectory, &user_library_path)) {
    LOG(WARNING) << "Could not get user library path";
  }
  return user_library_path;
}

CGColorSpaceRef GetSRGBColorSpace() {
  // Leaked.  That's OK, it's scoped to the lifetime of the application.
  static CGColorSpaceRef g_color_space_sRGB =
      CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  LOG_IF(ERROR, !g_color_space_sRGB) << "Couldn't get the sRGB color space";
  return g_color_space_sRGB;
}

CGColorSpaceRef GetSystemColorSpace() {
  // Leaked.  That's OK, it's scoped to the lifetime of the application.
  // Try to get the main display's color space.
  static CGColorSpaceRef g_system_color_space =
      CGDisplayCopyColorSpace(CGMainDisplayID());

  if (!g_system_color_space) {
    // Use a generic RGB color space.  This is better than nothing.
    g_system_color_space = CGColorSpaceCreateDeviceRGB();

    if (g_system_color_space) {
      LOG(WARNING) <<
          "Couldn't get the main display's color space, using generic";
    } else {
      LOG(ERROR) << "Couldn't get any color space";
    }
  }

  return g_system_color_space;
}

// a count of currently outstanding requests for full screen mode from browser
// windows, plugins, etc.
static int g_full_screen_requests = 0;

// Add a request for full screen mode.  If the menu bar is not currently
// hidden, hide it.  Must be called on main thread.
void RequestFullScreen() {
  DCHECK_GE(g_full_screen_requests, 0);
  if (g_full_screen_requests == 0)
    SetSystemUIMode(kUIModeAllSuppressed, kUIOptionAutoShowMenuBar);
  ++g_full_screen_requests;
}

// Release a request for full screen mode.  If there are no other outstanding
// requests, show the menu bar. Must be called on main thread.
void ReleaseFullScreen() {
  DCHECK_GT(g_full_screen_requests, 0);
  --g_full_screen_requests;
  if (g_full_screen_requests == 0)
    SetSystemUIMode(kUIModeNormal, 0);
}

void GrabWindowSnapshot(NSWindow* window,
    std::vector<unsigned char>* png_representation) {
  // Make sure to grab the "window frame" view so we get current tab +
  // tabstrip.
  NSView* view = [[window contentView] superview];
  NSBitmapImageRep* rep =
      [view bitmapImageRepForCachingDisplayInRect:[view bounds]];
  [view cacheDisplayInRect:[view bounds] toBitmapImageRep:rep];
  NSData* data = [rep representationUsingType:NSPNGFileType properties:nil];
  const unsigned char* buf = static_cast<const unsigned char*>([data bytes]);
  NSUInteger length = [data length];
  if (buf != NULL && length > 0){
    png_representation->assign(buf, buf + length);
    DCHECK(png_representation->size() > 0);
  }
}

void ActivateProcess(pid_t pid) {
  ProcessSerialNumber process;
  OSStatus status = GetProcessForPID(pid, &process);
  if (status == noErr) {
    SetFrontProcess(&process);
  } else {
    LOG(WARNING) << "Unable to get process for pid " << pid;
  }
}

// Takes a path to an (executable) binary and tries to provide the path to an
// application bundle containing it. It takes the outermost bundle that it can
// find (so for "/Foo/Bar.app/.../Baz.app/..." it produces "/Foo/Bar.app").
//   |exec_name| - path to the binary
//   returns - path to the application bundle, or empty on error
FilePath GetAppBundlePath(const FilePath& exec_name) {
  const char kExt[] = ".app";
  const size_t kExtLength = arraysize(kExt) - 1;

  // Split the path into components.
  std::vector<std::string> components;
  exec_name.GetComponents(&components);

  // It's an error if we don't get any components.
  if (!components.size())
    return FilePath();

  // Don't prepend '/' to the first component.
  std::vector<std::string>::const_iterator it = components.begin();
  std::string bundle_name = *it;
  DCHECK(it->length() > 0);
  // If the first component ends in ".app", we're already done.
  if (it->length() > kExtLength &&
      !it->compare(it->length() - kExtLength, kExtLength, kExt, kExtLength))
    return FilePath(bundle_name);

  // The first component may be "/" or "//", etc. Only append '/' if it doesn't
  // already end in '/'.
  if (bundle_name[bundle_name.length() - 1] != '/')
    bundle_name += '/';

  // Go through the remaining components.
  for (++it; it != components.end(); ++it) {
    DCHECK(it->length() > 0);

    bundle_name += *it;

    // If the current component ends in ".app", we're done.
    if (it->length() > kExtLength &&
        !it->compare(it->length() - kExtLength, kExtLength, kExt, kExtLength))
      return FilePath(bundle_name);

    // Separate this component from the next one.
    bundle_name += '/';
  }

  return FilePath();
}

bool SetFileBackupExclusion(const FilePath& file_path, bool exclude) {
  NSString* filePath =
      [NSString stringWithUTF8String:file_path.value().c_str()];

  // If being asked to exclude something in a tmp directory, just lie and say it
  // was done.  TimeMachine will already ignore tmp directories.  This keeps the
  // temporary profiles used by unittests from being added to the exclude list.
  // Otherwise, as /Library/Preferences/com.apple.TimeMachine.plist grows the
  // bots slow down due to reading/writing all the temporary profiles used over
  // time.

  NSString* tmpDir = NSTemporaryDirectory();
  // Make sure the temp dir is terminated with a slash
  if (tmpDir && ![tmpDir hasSuffix:@"/"])
    tmpDir = [tmpDir stringByAppendingString:@"/"];
  // '/var' is a link to '/private/var', make sure to check both forms.
  NSString* privateTmpDir = nil;
  if ([tmpDir hasPrefix:@"/var/"])
    privateTmpDir = [@"/private" stringByAppendingString:tmpDir];

  if ((tmpDir && [filePath hasPrefix:tmpDir]) ||
      (privateTmpDir && [filePath hasPrefix:privateTmpDir]) ||
      [filePath hasPrefix:@"/tmp/"] ||
      [filePath hasPrefix:@"/var/tmp/"] ||
      [filePath hasPrefix:@"/private/tmp/"] ||
      [filePath hasPrefix:@"/private/var/tmp/"]) {
    return true;
  }

  NSURL* url = [NSURL fileURLWithPath:filePath];
  // Note that we always set CSBackupSetItemExcluded's excludeByPath param
  // to true.  This prevents a problem with toggling the setting: if the file
  // is excluded with excludeByPath set to true then excludeByPath must
  // also be true when un-excluding the file, otherwise the un-excluding
  // will be ignored.
  bool success =
      CSBackupSetItemExcluded((CFURLRef)url, exclude, true) == noErr;
  if (!success)
    LOG(WARNING) << "Failed to set backup excluson for file '"
                 << file_path.value().c_str() << "'.  Continuing.";
  return success;
}

CFTypeRef GetValueFromDictionary(CFDictionaryRef dict,
                                 CFStringRef key,
                                 CFTypeID expected_type) {
  CFTypeRef value = CFDictionaryGetValue(dict, key);
  if (!value)
    return value;

  if (CFGetTypeID(value) != expected_type) {
    scoped_cftyperef<CFStringRef> expected_type_ref(
        CFCopyTypeIDDescription(expected_type));
    scoped_cftyperef<CFStringRef> actual_type_ref(
        CFCopyTypeIDDescription(CFGetTypeID(value)));
    LOG(WARNING) << "Expected value for key "
                 << base::SysCFStringRefToUTF8(key)
                 << " to be "
                 << base::SysCFStringRefToUTF8(expected_type_ref)
                 << " but it was "
                 << base::SysCFStringRefToUTF8(actual_type_ref)
                 << " instead";
    return NULL;
  }

  return value;
}

}  // namespace mac_util
