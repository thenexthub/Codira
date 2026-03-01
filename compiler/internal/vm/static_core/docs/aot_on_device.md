# AOT on Device

## Framework AOT

Frameworks include ABCs for all APP using. Now we pass them to Appspawn then used by all apps by --boot-panda-files.
We have two strategies to enable Framework AOT on device. 

1. Integrate to device image, and will appear in /system/framework 

2. Compilation during device is inactive. Will appear in /data/service/el1/public/framework_ark_cache/ after device running a few days later.

And AOT files under /data is newer than files under /system, consider that we can use pgo file to recompile these system abcs on device and save to /data/xxx. But this only happend after first compilation on device, before that, we still need to load from /system.

Otherwise, we have several abc files in boot-panda-files. On device, some of these files maybe in boot an location, some of them maybe in abc location.

Therefore, we need to support try load AOT files from both /data/xxx and /system/framework/ (path of abc). We have `--enable-an` option to support try load AOT files from abc file location. To support load from /data/xxx, we add an new option `--boot-an-location`. On device, this option will be always setted to /data/xxx, because we cannot dynamicly custom arguments on device. We first try to load AOT files from path of boot-an-location, because files here is latest. If no latest AOT files, we try to load from panda files location (old, from image).

The logic of loading an files for boot panda files will change to follows:

```
bool FileManager::TryLoadAnFileForLocation(std::string_view abcPath)
{
    PandaString::size_type posStart = abcPath.find_last_of('/');
    PandaString::size_type posEnd = abcPath.find_last_of('.');
    if (posStart == std::string_view::npos || posEnd == std::string_view::npos) {
        return true;
    }
    LOG(DEBUG, PANDAFILE) << "current abc file path: " << abcPath;
    PandaString abcFilePrefix = PandaString(abcPath.substr(posStart, posEnd - posStart));
    
    // If set boot-an-location, load from this location first
    std::string_view anLocation = Runtime::GetOptions().GetBootAnLocation();
    if (!anLocation.empty()) {
        bool res = FileManager::TryLoadAnFileFromLocation(anLocation, abcFilePrefix, abcPath);
        if (res) {
            return true;
        }
    }

    // If load failed from boot-an-location, continue try load from location of abc
    anLocation = abcPath.substr(0, posStart);
    FileManager::TryLoadAnFileFromLocation(anLocation, abcFilePrefix, abcPath);
    return true;
}
```
