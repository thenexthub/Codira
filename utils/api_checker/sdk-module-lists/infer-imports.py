#!/usr/bin/env python3 -u

import os
import sys

denylist = [
    "Kernel", "Ruby", "Tk",
    "DriverKit", "HIDDriverKit", "SkywalkDriverKit",  # has C++ code
    "NetworkingDriverKit", "USBSerialDriverKit", "PCIDriverKit",  # has C++ code
    "USBDriverKit",  # has C++ code
]


def get_immediate_subdirectories(a_dir):
    return [name for name in os.listdir(a_dir)
            if os.path.isdir(os.path.join(a_dir, name))]


def collect_catalyst_frameworks(frameworks_path):
    names = []
    for frame in os.listdir(frameworks_path):
        if frame.endswith(".framework"):
            name = frame[:-len(".framework")]
            # using the existence of this interface file as a sign of catalyst
            # being supported
            macabi_interface_path = \
                os.path.join(frameworks_path, frame,
                             'Modules', name + '.languagemodule',
                             'x86_64-apple-ios-macabi.languageinterface')
            if os.path.exists(macabi_interface_path):
                if name not in denylist:
                    names.append(name)
    return names


def get_catalyst_frameworks(sdk_path):
    frameworks_path = sdk_path + "/System/Library/Frameworks"
    ios_support_path = sdk_path + \
        '/System/iOSSupport/System/Library/Frameworks/'
    return collect_catalyst_frameworks(frameworks_path) + \
        collect_catalyst_frameworks(ios_support_path)


def get_frameworks(sdk_path, language_frameworks_only):
    frameworks_path = sdk_path + "/System/Library/Frameworks"
    names = []
    for frame in os.listdir(frameworks_path):
        if frame[0] == '_':
            continue
        if frame.endswith(".framework"):
            name = frame[:-len(".framework")]
            header_dir_path = frameworks_path + '/' + frame + '/Headers'
            module_dir_path = frameworks_path + '/' + frame + '/Modules'
            languagemodule_path = module_dir_path + '/' + name + '.languagemodule'
            old_modulemap_path = frameworks_path + '/' + frame + '/module.map'
            old_modulemap_private_path = frameworks_path + '/' + frame + \
                '/module_private.map'

            if os.path.exists(languagemodule_path):
                if name not in denylist:
                    names.append(name)
                continue
            # We only care about Swift frameworks then we are done.
            if language_frameworks_only:
                continue

            if not os.path.exists(header_dir_path):
                if os.path.exists(module_dir_path):
                    print(header_dir_path,
                          " non-existent while 'Modules' exists",
                          file=sys.stderr)
                if os.path.exists(old_modulemap_path):
                    print(header_dir_path,
                          " non-existent while 'module.map' exists",
                          file=sys.stderr)
                if os.path.exists(old_modulemap_private_path):
                    print(header_dir_path,
                          " non-existent while 'module_private.map' exists",
                          file=sys.stderr)
                continue

            if should_exclude_framework(frameworks_path + '/' + frame):
                continue

            if name in denylist:
                continue
            names.append(name)
    return names


def get_overlays(sdk_path):
    overlay_path = sdk_path + "/usr/lib/language/"
    names = []
    for overlay in os.listdir(overlay_path):
        if overlay.endswith(".languagemodule"):
            overlay = overlay[:-len(".languagemodule")]
            if overlay in denylist:
                continue
            names.append(overlay)
    return names


def should_exclude_framework(frame_path):
    module_map_path = frame_path + '/Modules/module.modulemap'
    if not os.path.exists(module_map_path):
        return False
    contents = open(module_map_path).read()
    if "requires !language" in contents:
        return True

    return False


def print_clang_imports(frames, use_hash):
    for name in frames:
        if use_hash:
            print("#import <" + name + "/" + name + ".h>")
        else:
            print("@import " + name + ";")


def print_language_imports(frames):
    for name in frames:
        print("import " + name)


def main():
    global opts
    from optparse import OptionParser
    parser = OptionParser("""%prog [options] command

%prog outputs imports

  $ %prog -s <sdk-path> -o <output-mode> [--hash]

""")
    parser.add_option("-s", "--sdk", help="sdk path",
                      type=str, dest="sdk", default=None)
    parser.add_option("-o", "--output", help="output mode",
                      type=str, dest="out_mode", default="list")
    parser.add_option("--hash", action="store_true", dest="use_hash")
    parser.add_option("--language-frameworks-only", action="store_true")
    parser.add_option("--language-overlay-only", action="store_true")
    parser.add_option("--v", action="store_true")
    parser.add_option("--catalyst", action="store_true")
    (opts, cmd) = parser.parse_args()

    if not opts.sdk:
        parser.error("sdk not specified")

    if not opts.out_mode:
        parser.error(
            "output mode not specified: 'clang-import'/'language-import'/'list'")

    if opts.language_overlay_only:
        frames = get_overlays(opts.sdk)
    else:
        if opts.catalyst:
            if opts.language_frameworks_only:
                frames = get_catalyst_frameworks(opts.sdk)
            else:
                parser.error("only support find catalyst frameworks "
                             "with --language-frameworks-only")
        else:
            frames = get_frameworks(opts.sdk, opts.language_frameworks_only)
    if opts.v:
        for name in frames:
            print('Including: ', name, file=sys.stderr)
    if opts.out_mode == "clang-import":
        print_clang_imports(frames, opts.use_hash)
    elif opts.out_mode == "language-import":
        print_language_imports(frames)
    elif opts.out_mode == "list":
        for name in frames:
            print(name)
    else:
        parser.error(
            "output mode not found: 'clang-import'/'language-import'/'list'")


if __name__ == '__main__':
    main()
