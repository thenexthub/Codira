/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
import * as abilityInfo from "abilityInfo";
import * as applicationInfo from "applicationInfo";
import * as bundleInfo from "bundleInfo";
import * as customizeData from "customizeData";
import * as elementName from "elementName";
import * as hapModuleInfo from "hapModuleInfo";
import * as moduleInfo from "moduleInfo";
import {Want} from "ohos_app_ability_want";
import * as ohosBundle from "ohos_bundle";
import * as shortcutInfo from "shortcutInfo";

loadLibrary("ani_bundle");

//@ohos.bundle.d.ts
let bundleName = "ohos.bundle";
let bundleFlags = 419;
let userId = 391550;
let abilityName = "abilityName";
let uid = 2147483647;
let hapFilePath = "/home/zhaojinguo/taihe0411/taihe";
let arrString = ["this", "is", "bundle"];

function test_bundle_enum_BundleFlag_with_DEFAULT() {
    const first = ohosBundle.BundleFlag.GET_BUNDLE_DEFAULT;

    console.log("BundleFlag.GET_BUNDLE_DEFAULT is : " + first);
    arktest.assertEQ(first, ohosBundle.BundleFlag.GET_BUNDLE_DEFAULT);
}

function test_bundle_enum_BundleFlag_with_INFO() {
    const first = ohosBundle.BundleFlag.GET_ALL_APPLICATION_INFO;

    console.log("BundleFlag.GET_ALL_APPLICATION_INFO is : " + first);
    arktest.assertEQ(first, ohosBundle.BundleFlag.GET_ALL_APPLICATION_INFO);
}

function test_bundle_enum_BundleFlag_with_DISABLE() {
    const first = ohosBundle.BundleFlag.GET_APPLICATION_INFO_WITH_DISABLE;

    console.log("BundleFlag.GET_APPLICATION_INFO_WITH_DISABLE is : " + first);
    arktest.assertEQ(
        first, ohosBundle.BundleFlag.GET_APPLICATION_INFO_WITH_DISABLE);
}

function test_bundle_enum_with_colorMode() {
    const first = ohosBundle.ColorMode.AUTO_MODE;

    console.log("ColorMode.AUTO_MODE is : " + first);
    arktest.assertEQ(first, ohosBundle.ColorMode.AUTO_MODE);
}

function test_bundle_enum_with_grantStatus() {
    const first = ohosBundle.GrantStatus.PERMISSION_DENIED;

    console.log("GrantStatus.PERMISSION_DENIED is : " + first);
    arktest.assertEQ(first, ohosBundle.GrantStatus.PERMISSION_DENIED);
}

function test_bundle_enum_with_abilityType() {
    let first = ohosBundle.AbilityType.UNKNOWN;

    console.log("AbilityType.UNKNOWN is : " + first);
    arktest.assertEQ(first, ohosBundle.AbilityType.UNKNOWN);
}

function test_bundle_enum_with_abilitySubType() {
    const first = ohosBundle.AbilitySubType.UNSPECIFIED;

    console.log("AbilitySubType.UNSPECIFIED is : " + first);
    arktest.assertEQ(first, ohosBundle.AbilitySubType.UNSPECIFIED);
}

function test_bundle_enum_with_displayOrientation() {
    const first = ohosBundle.DisplayOrientation.UNSPECIFIED;

    console.log("DisplayOrientation.UNSPECIFIED is : " + first);
    arktest.assertEQ(first, ohosBundle.DisplayOrientation.UNSPECIFIED);
}

function test_bundle_enum_with_launchMode() {
    const first = ohosBundle.LaunchMode.SINGLETON;

    console.log("LaunchMode.SINGLETON is : " + first);
    arktest.assertEQ(first, ohosBundle.LaunchMode.SINGLETON);
}

function test_bundle_enum_with_installErrorCode_SUCCESS() {
    const first = ohosBundle.InstallErrorCode.SUCCESS;

    console.log("InstallErrorCode.SUCCESS is : " + first);
    arktest.assertEQ(first, ohosBundle.InstallErrorCode.SUCCESS);
}

function test_bundle_enum_with_installErrorCode_TIMEOUT() {
    const first =
        ohosBundle.InstallErrorCode.STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT;

    console.log(
        "InstallErrorCode.STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT is : " +
        first);
    arktest.assertEQ(
        first,
        ohosBundle.InstallErrorCode.STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT);
}

function test_bundle_enum_with_installErrorCode_FOUND() {
    const first = ohosBundle.InstallErrorCode.STATUS_ABILITY_NOT_FOUND;

    console.log("InstallErrorCode.STATUS_ABILITY_NOT_FOUND is : " + first);
    arktest.assertEQ(
        first, ohosBundle.InstallErrorCode.STATUS_ABILITY_NOT_FOUND);
}

let bundleOpt: ohosBundle.BundleOptions = ohosBundle.getBundleOptions();
function test_bundle_interface_bundleOptions_with_userId_noSet() {
    let id = bundleOpt.userId;

    console.log("BundleOptions userId is : " + id);
    arktest.assertEQ(id, undefined);
}

function test_bundle_interface_bundleOptions_with_userId() {
    bundleOpt.userId = userId;
    let id = bundleOpt.userId;

    console.log("BundleOptions userId is : " + id);
    arktest.assertEQ(id, userId);
}

function test_bundle_function_with_getBundleInfo() {
    ohosBundle.getBundleInfo("getBundleInfo", bundleFlags);

    console.log("ohos bundle function is : getBundleInfo ");
}

function test_bundle_function_with_getAbilityInfo() {
    ohosBundle.getAbilityInfo(bundleName, abilityName);

    console.log("ohos bundle function is : getAbilityInfo ");
}

function test_bundle_function_with_getApplicationInfo3param() {
    let instance: ohosBundle.OverloadInterface = ohosBundle.get_interface();
    instance.getApplicationInfo("getApplicationInfo", bundleFlags, userId);

    console.log("ohos bundle function is : getApplicationInfo 3 param.");
}

function test_bundle_function_with_getApplicationInfo2param() {
    let instance: ohosBundle.OverloadInterface = ohosBundle.get_interface();
    instance.getApplicationInfo("getApplicationInfo", bundleFlags);

    console.log("ohos bundle function is : getApplicationInfo 2 param.");
}

function test_bundle_function_with_getApplicationInfoOptional_1() {
    let instance: ohosBundle.OverloadInterface = ohosBundle.get_interface();
    instance.getApplicationInfo("getApplicationInfo", bundleFlags, userId);

    console.log("ohos bundle function is : getApplicationInfo have Optional.");
}

function test_bundle_function_with_getApplicationInfoOptional_2() {
    let instance: ohosBundle.OverloadInterface = ohosBundle.get_interface();
    instance.getApplicationInfo("getApplicationInfo", bundleFlags);

    console.log("ohos bundle function is : getApplicationInfo no Optional.");
}

function test_bundle_function_with_queryAbilityByWant() {
    ohosBundle.queryAbilityByWant(bundleFlags, userId);

    console.log("ohos bundle function is : queryAbilityByWant ");
}

function test_bundle_function_with_getAllBundleInfo() {
    ohosBundle.getAllBundleInfo(userId);

    console.log("ohos bundle function is : getAllBundleInfo ");
}

function test_bundle_function_with_getAllApplicationInfo() {
    ohosBundle.getAllApplicationInfo(bundleFlags, userId);

    console.log("ohos bundle function is : getAllApplicationInfo ");
}

function test_bundle_function_with_getNameForUid() {
    ohosBundle.getNameForUid(uid);

    console.log("ohos bundle function is : getNameForUid ");
}

function test_bundle_function_with_getBundleArchiveInfo() {
    ohosBundle.getBundleArchiveInfo(hapFilePath, bundleFlags);

    console.log("ohos bundle function is : getBundleArchiveInfo ");
}

function test_bundle_function_with_getLaunchWantForBundle() {
    ohosBundle.getLaunchWantForBundle(bundleName);

    console.log("ohos bundle function is : getLaunchWantForBundle ");
}

function test_bundle_function_with_getAbilityLabel() {
    ohosBundle.getAbilityLabel(bundleName, abilityName);

    console.log("ohos bundle function is : getAbilityLabel ");
}

function test_bundle_function_with_getAbilityIcon() {
    ohosBundle.getAbilityIcon(bundleName, abilityName);

    console.log("ohos bundle function is : getAbilityIcon ");
}

function test_bundle_function_with_isAbilityEnabled() {
    ohosBundle.isAbilityEnabled();

    console.log("ohos bundle function is : isAbilityEnabled ");
}

function test_bundle_function_with_isApplicationEnabled() {
    ohosBundle.isApplicationEnabled(bundleName);

    console.log("ohos bundle function is : isApplicationEnabled ");
}

// abilityInfo.d.ts
let abInfo: abilityInfo.AbilityInfo = abilityInfo.getAbilityInfo();
function test_abilityinfo_with_bundleName() {
    let info = abInfo.bundleName;

    console.log("abInfo.bundleName is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getBundleName");
}

function test_abilityinfo_with_name() {
    let info = abInfo.name;

    console.log("abInfo.name is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getName");
}

function test_abilityinfo_with_label() {
    let info = abInfo.label;

    console.log("abInfo.label is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getLabel");
}

function test_abilityinfo_with_description() {
    let info = abInfo.description;

    console.log("abInfo.description is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getDescription");
}

function test_abilityinfo_with_icon() {
    let info = abInfo.icon;

    console.log("abInfo.icon is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getIcon");
}

function test_abilityinfo_with_labelId() {
    let info = abInfo.labelId;

    console.log("abInfo.labelId is : " + info);
    arktest.assertEQ(info, 100 as short);
}

function test_abilityinfo_with_descriptionId() {
    let info = abInfo.descriptionId;

    console.log("abInfo.descriptionId is : " + info);
    arktest.assertEQ(info, 100 as short);
}

function test_abilityinfo_with_iconId() {
    let info = abInfo.iconId;

    console.log("abInfo.iconId is : " + info);
    arktest.assertEQ(info, 100 as short);
}

function test_abilityinfo_with_moduleName() {
    let info = abInfo.moduleName;

    console.log("abInfo.moduleName is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getModuleName");
}

function test_abilityinfo_with_process() {
    let info = abInfo.process;

    console.log("abInfo.process is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getProcess");
}

function test_abilityinfo_with_targetAbility() {
    let info = abInfo.targetAbility;

    console.log("abInfo.targetAbility is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getTargetAbility");
}

function test_abilityinfo_with_backgroundModes() {
    let info = abInfo.backgroundModes;

    console.log("abInfo.backgroundModes is : " + info);
    arktest.assertEQ(info, 100 as short);
}

function test_abilityinfo_with_isVisible() {
    let info = abInfo.isVisible;

    console.log("abInfo.isVisible is : " + info);
    arktest.assertEQ(info, true);
}

function test_abilityinfo_with_formEnabled() {
    let info = abInfo.formEnabled;

    console.log("abInfo.formEnabled is : " + info);
    arktest.assertEQ(info, true);
}

function test_abilityinfo_with_permissions() {
    let info = abInfo.permissions[0];

    console.log("abInfo.permissions is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getTargetAbility");
}

function test_abilityinfo_with_deviceTypes() {
    let info = abInfo.deviceTypes[0];

    console.log("abInfo.deviceTypes is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getDeviceTypes");
}

function test_abilityinfo_with_deviceCapabilities() {
    let info = abInfo.deviceCapabilities[0];

    console.log("abInfo.deviceCapabilities is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getDeviceCapabilities");
}

function test_abilityinfo_with_readPermission() {
    let info = abInfo.readPermission;

    console.log("abInfo.readPermission is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getReadPermission");
}

function test_abilityinfo_with_writePermission() {
    let info = abInfo.writePermission;

    console.log("abInfo.writePermission is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getWritePermission");
}

function test_abilityinfo_with_uri() {
    let info = abInfo.uri;

    console.log("abInfo.uri is : " + info);
    arktest.assertEQ(info, "AbilityInfo::getUri");
}

function test_abilityinfo_with_enabled() {
    let info = abInfo.enabled;

    console.log("abInfo.enabled is : " + info);
    arktest.assertEQ(info, true);
}

// bundleInfo.d.ts
let usedScene: bundleInfo.UsedScene = bundleInfo.getUsedScene();
let reqPerDetail: bundleInfo.ReqPermissionDetail =
    bundleInfo.getReqPermissionDetail();
let bunInfo: bundleInfo.BundleInfo = bundleInfo.getBundleInfo();

function test_bundleinfo_interface_usedScene_with_abilities() {
    usedScene.abilities = ["this ", "is ", "abilities"];
    let info = usedScene.abilities[2];

    console.log("usedScene.abilities is : " + info);
    arktest.assertEQ(info, "abilities");
}

function test_bundleinfo_interface_usedScene_with_when() {
    usedScene.when = "this is when";
    let info = usedScene.when;

    console.log("usedScene.when is : " + info);
    arktest.assertEQ(info, "this is when");
}

function test_bundleinfo_interface_reqperdetail_with_name() {
    reqPerDetail.name = "this is name";
    let info = reqPerDetail.name;

    console.log("reqPerDetail.name is : " + info);
    arktest.assertEQ(info, "this is name");
}

function test_bundleinfo_interface_reqperdetail_with_reason() {
    reqPerDetail.reason = "this is reason";
    let info = reqPerDetail.reason;

    console.log("reqPerDetail.reason is : " + info);
    arktest.assertEQ(info, "this is reason");
}

function test_bundleinfo_interface_with_name() {
    let info = bunInfo.name;

    console.log("bunInfo.name is : " + info);
    arktest.assertEQ(info, "bundleInfo::getName");
}

function test_bundleinfo_interface_with_type() {
    let info = bunInfo.type;

    console.log("bunInfo.type is : " + info);
    arktest.assertEQ(info, "bundleInfo::getType");
}

function test_bundleinfo_interface_with_appId() {
    let info = bunInfo.appId;

    console.log("bunInfo.appId is : " + info);
    arktest.assertEQ(info, "bundleInfo::getAppId");
}

function test_bundleinfo_interface_withuid() {
    let info = bunInfo.uid;

    console.log("bunInfo.uid is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_installTime() {
    let info = bunInfo.installTime;

    console.log("bunInfo.installTime is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_updateTime() {
    let info = bunInfo.updateTime;

    console.log("bunInfo.updateTime is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_reqPermissions() {
    let info = bunInfo.reqPermissions[0];

    console.log("bunInfo.reqPermissions is : " + info);
    arktest.assertEQ(info, "bundleInfo::getReqPermissions");
}

function test_bundleinfo_interface_with_vendor() {
    let info = bunInfo.vendor;

    console.log("bunInfo.vendor is : " + info);
    arktest.assertEQ(info, "bundleInfo::getVendor");
}

function test_bundleinfo_interface_with_versionCode() {
    let info = bunInfo.versionCode;

    console.log("bunInfo.versionCode is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_versionName() {
    let info = bunInfo.versionName;

    console.log("bunInfo.versionName is : " + info);
    arktest.assertEQ(info, "bundleInfo::getVersionName");
}

function test_bundleinfo_interface_with_compatibleVersion() {
    let info = bunInfo.compatibleVersion;

    console.log("bunInfo.compatibleVersion is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_targetVersion() {
    let info = bunInfo.targetVersion;

    console.log("bunInfo.targetVersion is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_isCompressNativeLibs() {
    let info = bunInfo.isCompressNativeLibs;

    console.log("bunInfo.isCompressNativeLibs is : " + info);
    arktest.assertEQ(info, true);
}

function test_bundleinfo_interface_with_entryModuleName() {
    let info = bunInfo.entryModuleName;

    console.log("bunInfo.entryModuleName is : " + info);
    arktest.assertEQ(info, "bundleInfo::getEntryModuleName");
}

function test_bundleinfo_interface_with_cpuAbi() {
    let info = bunInfo.cpuAbi;

    console.log("bunInfo.cpuAbi is : " + info);
    arktest.assertEQ(info, "bundleInfo::getCpuAbi");
}

function test_bundleinfo_interface_with_isSilentInstallation() {
    let info = bunInfo.isSilentInstallation;

    console.log("bunInfo.isSilentInstallation is : " + info);
    arktest.assertEQ(info, "bundleInfo::getIsSilentInstallation");
}

function test_bundleinfo_interface_with_minCompatibleVersionCode() {
    let info = bunInfo.minCompatibleVersionCode;

    console.log("bunInfo.minCompatibleVersionCode is : " + info);
    arktest.assertEQ(info, 127 as short);
}

function test_bundleinfo_interface_with_entryInstallationFree() {
    let info = bunInfo.entryInstallationFree;

    console.log("bunInfo.entryInstallationFree is : " + info);
    arktest.assertEQ(info, true);
}

function test_bundleinfo_interface_with_reqPermissionStates() {
    let info = bunInfo.reqPermissionStates[0];

    console.log("bunInfo.reqPermissionStates is : " + info);
    arktest.assertEQ(info, 127 as short);
}

// customizeData.d.ts
let custData: customizeData.CustomizeData = customizeData.getCustomizeData();
function test_bundleinfo_interface_custdata_with_name() {
    custData.name = "tom";
    let info = custData.name;

    console.log("custData.name is : " + info);
    arktest.assertEQ(info, "tom");
}

function test_bundleinfo_interface_custdata_with_value() {
    custData.value = "tom";
    let info = custData.value;

    console.log("custData.value is : " + info);
    arktest.assertEQ(info, "tom");
}

function test_bundleinfo_interface_custdata_with_extra() {
    custData.extra = "tom";
    let info = custData.extra;

    console.log("custData.extra is : " + info);
    arktest.assertEQ(info, "tom");
}

// hapModuleInfo.d.ts
let hapInfo: hapModuleInfo.HapModuleInfo = hapModuleInfo.getHapModuleInfo();
function test_bundle_interface_hapModuleInfo_with_name() {
    let info = hapInfo.name;

    console.log("hapInfo.name is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getName");
}

function test_bundle_interface_hapModuleInfo_with_description() {
    let info = hapInfo.description;

    console.log("hapInfo.description is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getDescription");
}

function test_bundle_interface_hapModuleInfo_with_descriptionid() {
    let info = hapInfo.descriptionId;

    console.log("hapInfo.descriptionId is : " + info);
    arktest.assertEQ(info, 1024 as short);
}

function test_bundle_interface_hapModuleInfo_with_icon() {
    let info = hapInfo.icon;

    console.log("hapInfo.icon is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getIcon");
}

function test_bundle_interface_hapModuleInfo_with_label() {
    let info = hapInfo.label;

    console.log("hapInfo.label is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getLabel");
}

function test_bundle_interface_hapModuleInfo_with_labelid() {
    let info = hapInfo.labelId;

    console.log("hapInfo.labelId is : " + info);
    arktest.assertEQ(info, 1024 as short);
}

function test_bundle_interface_hapModuleInfo_with_iconid() {
    let info = hapInfo.iconId;

    console.log("hapInfo.iconId is : " + info);
    arktest.assertEQ(info, 1024 as short);
}

function test_bundle_interface_hapModuleInfo_with_backgroundImg() {
    let info = hapInfo.backgroundImg;

    console.log("hapInfo.backgroundImg is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getBackgroundImg");
}

function test_bundle_interface_hapModuleInfo_with_supportedModes() {
    let info = hapInfo.supportedModes;

    console.log("hapInfo.supportedModes is : " + info);
    arktest.assertEQ(info, 1024 as short);
}

function test_bundle_interface_hapModuleInfo_with_reqCapabilities() {
    let info = hapInfo.reqCapabilities[0];

    console.log("hapInfo.reqCapabilities is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getReqCapabilities");
}

function test_bundle_interface_hapModuleInfo_with_deviceTypes() {
    let info = hapInfo.deviceTypes[0];

    console.log("hapInfo.deviceTypes is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getDeviceTypes");
}

function test_bundle_interface_hapModuleInfo_with_moduleName() {
    let info = hapInfo.moduleName;

    console.log("hapInfo.moduleName is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getModuleName");
}

function test_bundle_interface_hapModuleInfo_with_mainAbilityName() {
    let info = hapInfo.mainAbilityName;

    console.log("hapInfo.mainAbilityName is : " + info);
    arktest.assertEQ(info, "HapModuleInfo::getMainAbilityName");
}

function test_bundle_interface_hapModuleInfo_with_installationFree() {
    let info = hapInfo.installationFree;

    console.log("hapInfo.installationFree is : " + info);
    arktest.assertEQ(info, true);
}

// moduleInfo.d.ts
let moInfo: moduleInfo.ModuleInfo = moduleInfo.getModuleInfo();
function test_bundle_interface_moduleInfo_with_moduleName() {
    let info = moInfo.moduleName;

    console.log("moInfo.moduleName is : " + info);
    arktest.assertEQ(info, "this is moduleinfo with name");
}

function test_bundle_interface_moduleInfo_with_moduleSourceDir() {
    let info = moInfo.moduleSourceDir;

    console.log("moInfo.moduleSourceDir is : " + info);
    arktest.assertEQ(info, "this is moduleinfo with moduleSourceDir");
}

// shortcutInfo.d.ts
let shortInfo: shortcutInfo.ShortcutInfo = shortcutInfo.getShortcutInfo();
function test_bundle_interface_shortcutInfo_with_id() {
    let info = shortInfo.id;

    console.log("shortInfo.id is : " + info);
    arktest.assertEQ(info, "ShortcutInfo::GetId");
}

function test_bundle_interface_shortcutInfo_with_bundleName() {
    let info = shortInfo.bundleName;

    console.log("shortInfo.bundleName is : " + info);
    arktest.assertEQ(info, "ShortcutInfo::GetBundleName");
}

function test_bundle_interface_shortcutInfo_with_hostAbility() {
    let info = shortInfo.hostAbility;

    console.log("shortInfo.hostAbility is : " + info);
    arktest.assertEQ(info, "ShortcutInfo::GetHostAbility");
}

function test_bundle_interface_shortcutInfo_with_icon() {
    let info = shortInfo.icon;

    console.log("shortInfo.icon is : " + info);
    arktest.assertEQ(info, "ShortcutInfo::GetIcon");
}

function test_bundle_interface_shortcutInfo_with_iconId() {
    let info = shortInfo.iconId;

    console.log("shortInfo.iconId is : " + info);
    arktest.assertEQ(info, 4096);
}

function test_bundle_interface_shortcutInfo_with_label() {
    let info = shortInfo.label;

    console.log("shortInfo.label is : " + info);
    arktest.assertEQ(info, "ShortcutInfo::GetLabel");
}

function test_bundle_interface_shortcutInfo_with_labelId() {
    let info = shortInfo.labelId;

    console.log("shortInfo.labelId is : " + info);
    arktest.assertEQ(info, 4096);
}

function test_bundle_interface_shortcutInfo_with_disableMessage() {
    let info = shortInfo.disableMessage;

    console.log("shortInfo.disableMessage is : " + info);
    arktest.assertEQ(info, "ShortcutInfo::GetDisableMessage");
}

function test_bundle_interface_shortcutInfo_with_isStatic() {
    let info = shortInfo.isStatic;

    console.log("shortInfo.isStatic is : " + info);
    arktest.assertEQ(info, true);
}

function test_bundle_interface_shortcutInfo_with_isHomeShortcut() {
    let info = shortInfo.isHomeShortcut;

    console.log("shortInfo.isHomeShortcut is : " + info);
    arktest.assertEQ(info, true);
}

function test_bundle_interface_shortcutInfo_with_isEnabled() {
    let info = shortInfo.isEnabled;

    console.log("shortInfo.isEnabled is : " + info);
    arktest.assertEQ(info, true);
}

// elementName.d.ts
function test_bundle_interface_elementName_with_deviceId_noSet() {
    let eleName: elementName.ElementName = elementName.getElementName()
    let info = eleName.deviceId;
    console.log("eleName.deviceId: " + info)
    arktest.assertEQ(info, undefined);
}

function test_bundle_interface_elementName_with_deviceId() {
    let eleName: elementName.ElementName = elementName.getElementName()
    eleName.deviceId = "ID20250422";
    let info = eleName.deviceId;
    console.log("eleName.deviceId: " + info)
    arktest.assertEQ(info, "ID20250422");
}

function test_bundle_interface_elementName_with_bundleName() {
    let eleName: elementName.ElementName = elementName.getElementName()
    eleName.bundleName = "ID20250422";
    let info = eleName.bundleName;

    arktest.assertEQ(info, "ID20250422");
}

function test_bundle_interface_elementName_with_abilityName() {
    let eleName: elementName.ElementName = elementName.getElementName()
    eleName.abilityName = "ID20250422";
    let info = eleName.abilityName;

    arktest.assertEQ(info, "ID20250422");
}

function test_bundle_interface_elementName_with_uri_noSet() {
    let eleName: elementName.ElementName = elementName.getElementName()
    let info = eleName.uri;

    arktest.assertEQ(info, undefined);
}

function test_bundle_interface_elementName_with_uri() {
    let eleName: elementName.ElementName = elementName.getElementName()
    eleName.uri = "ID20250422";
    let info = eleName.uri;

    arktest.assertEQ(info, "ID20250422");
}

function test_bundle_interface_elementName_with_shortName_noSet() {
    let eleName: elementName.ElementName = elementName.getElementName()
    let info = eleName.shortName;

    arktest.assertEQ(info, undefined);
}

function test_bundle_interface_elementName_with_shortName() {
    let eleName: elementName.ElementName = elementName.getElementName()
    eleName.shortName = "ID20250422";
    let info = eleName.shortName;

    arktest.assertEQ(info, "ID20250422");
}

// applicationInfo.d.ts
let appInfo: applicationInfo.ApplicationInfo =
    applicationInfo.getApplicationInfo();
function test_bundle_interface_applicationInfo_with_name() {
    let info = appInfo.name;

    arktest.assertEQ(info, "ApplicationInfo::getName")
}

function test_bundle_interface_applicationInfo_with_description() {
    let info = appInfo.description;

    arktest.assertEQ(info, "ApplicationInfo::getDescription")
}

function test_bundle_interface_applicationInfo_with_descriptionId() {
    let info = appInfo.descriptionId;

    arktest.assertEQ(info, 102 as short)
}

function test_bundle_interface_applicationInfo_with_systemApp() {
    let info = appInfo.systemApp;

    arktest.assertEQ(info, true)
}

function test_bundle_interface_applicationInfo_with_enabled() {
    let info = appInfo.enabled;

    arktest.assertEQ(info, true)
}

function test_bundle_interface_applicationInfo_with_label() {
    let info = appInfo.label;

    arktest.assertEQ(info, "ApplicationInfo::getLabel")
}

function test_bundle_interface_applicationInfo_with_labelId() {
    let info = appInfo.labelId;

    arktest.assertEQ(info, "ApplicationInfo::getLabelId")
}

function test_bundle_interface_applicationInfo_with_icon() {
    let info = appInfo.icon;

    arktest.assertEQ(info, "ApplicationInfo::getIcon")
}

function test_bundle_interface_applicationInfo_with_iconId() {
    let info = appInfo.iconId;

    arktest.assertEQ(info, 102 as short)
}

function test_bundle_interface_applicationInfo_with_process() {
    let info = appInfo.process;

    arktest.assertEQ(info, "ApplicationInfo::getProcess")
}

function test_bundle_interface_applicationInfo_with_supportedModes() {
    let info = appInfo.supportedModes;

    arktest.assertEQ(info, 102 as short)
}

function test_bundle_interface_applicationInfo_with_moduleSourceDirs() {
    let info = appInfo.moduleSourceDirs[0];

    arktest.assertEQ(info, "ApplicationInfo::getProcess")
}

function test_bundle_interface_applicationInfo_with_permissions() {
    let info = appInfo.permissions[0];

    arktest.assertEQ(info, "ApplicationInfo::getPermissions")
}

function test_bundle_interface_applicationInfo_with_entryDir() {
    let info = appInfo.entryDir;

    arktest.assertEQ(info, "ApplicationInfo::getEntryDir")
}

function test_bundle_interface_applicationInfo_with_codePath() {
    let info = appInfo.codePath;

    arktest.assertEQ(info, "ApplicationInfo::getCodePath")
}

function test_bundle_interface_applicationInfo_with_removable() {
    let info = appInfo.removable;

    arktest.assertEQ(info, true)
}

function test_bundle_interface_applicationInfo_with_accessTokenId() {
    let info = appInfo.accessTokenId;

    arktest.assertEQ(info, 102 as short)
}

function test_bundle_interface_applicationInfo_with_uid() {
    let info = appInfo.uid;

    arktest.assertEQ(info, 102 as short)
}

function test_bundle_interface_applicationInfo_with_entityType() {
    let info = appInfo.entityType;

    arktest.assertEQ(info, "ApplicationInfo::getEntityType")
}

// ohos.app.ability.Want.d.ts
function test_bundle_class_want_with_bundleName_noSet() {
    let wantTest = new Want();
    let info = wantTest.bundleName;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_bundleName() {
    let wantTest = new Want();
    wantTest.bundleName = bundleName;
    let info = wantTest.bundleName;

    arktest.assertEQ(info, bundleName)
}

function test_bundle_class_want_with_abilityName_noSet() {
    let wantTest = new Want();
    let info = wantTest.abilityName;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_abilityName() {
    let wantTest = new Want();
    wantTest.abilityName = bundleName;
    let info = wantTest.abilityName;

    arktest.assertEQ(info, bundleName)
}

function test_bundle_class_want_with_deviceId_noSet() {
    let wantTest = new Want();
    let info = wantTest.deviceId;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_deviceId() {
    let wantTest = new Want();
    wantTest.deviceId = bundleName;
    let info = wantTest.deviceId;

    arktest.assertEQ(info, bundleName)
}

function test_bundle_class_want_with_uri_noSet() {
    let wantTest = new Want();
    let info = wantTest.uri;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_uri() {
    let wantTest = new Want();
    wantTest.uri = bundleName;
    let info = wantTest.uri;

    arktest.assertEQ(info, bundleName)
}

function test_bundle_class_want_with_type_noSet() {
    let wantTest = new Want();
    let info = wantTest.type;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_type() {
    let wantTest = new Want();
    wantTest.type = bundleName;
    let info = wantTest.type;

    arktest.assertEQ(info, bundleName)
}

function test_bundle_class_want_with_flags_noSet() {
    let wantTest = new Want();
    let info = wantTest.flags;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_flags() {
    let wantTest = new Want();
    wantTest.flags = 16.08f;
    let info = wantTest.flags;

    arktest.assertEQ(info, (16.08f).toFloat())
}

function test_bundle_class_want_with_action_noSet() {
    let wantTest = new Want();
    let info = wantTest.action;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_action() {
    let wantTest = new Want();
    wantTest.action = bundleName;
    let info = wantTest.action;

    arktest.assertEQ(info, bundleName);
}

function test_bundle_class_want_with_parameters_noSet() {
    let wantTest = new Want();
    let info = wantTest.parameters;

    arktest.assertEQ(info, undefined);
}

class A {
    constructor() {}
}

function test_bundle_class_want_with_parameters() {
    let wantTest = new Want();
    const a = new A();
    let Settings: Record<string, object> = {
        "theme": a,
        "fontSize": a,
        "language": a,
    };
    wantTest.parameters = Settings;
    let info: Record<string, object>|undefined = wantTest.parameters;

    arktest.assertEQ(info?.["theme"], Settings["theme"]);
}

function test_bundle_class_want_with_entities_noSet() {
    let wantTest = new Want();
    let info = wantTest.entities;

    arktest.assertEQ(info, undefined);
}

function test_bundle_class_want_with_entities() {
    let wantTest = new Want();
    wantTest.entities = arrString;
    let info = wantTest.entities;

    arktest.assertEQ(info?.[0], arrString[0])
}

function test_bundle_class_want_with_moduleName_noSet() {
    let wantTest = new Want();
    let info = wantTest.moduleName;

    arktest.assertEQ(info, undefined)
}

function test_bundle_class_want_with_moduleName() {
    let wantTest = new Want();
    wantTest.moduleName = bundleName;
    let info = wantTest.moduleName;

    arktest.assertEQ(info, bundleName)
}

function main() {
    console.log("##############start#############");
    const suite = new arktest.ArkTestsuite("BundleTest Tests");

    // abilityInfo.d.ts
    suite.addTest(
        "test bundle abilityInfo with bundleName",
        test_abilityinfo_with_bundleName)
    suite.addTest(
        "test bundle abilityInfo with name", test_abilityinfo_with_name)
    suite.addTest(
        "test bundle abilityInfo with label", test_abilityinfo_with_label)
    suite.addTest(
        "test bundle abilityInfo with description",
        test_abilityinfo_with_description)
    suite.addTest(
        "test bundle abilityInfo with icon", test_abilityinfo_with_icon)
    suite.addTest(
        "test bundle abilityInfo with labelId", test_abilityinfo_with_labelId)
    suite.addTest(
        "test bundle abilityInfo with descriptionId",
        test_abilityinfo_with_descriptionId)
    suite.addTest(
        "test bundle abilityInfo with iconId", test_abilityinfo_with_iconId)
    suite.addTest(
        "test bundle abilityInfo with moduleName",
        test_abilityinfo_with_moduleName)
    suite.addTest(
        "test bundle abilityInfo with process", test_abilityinfo_with_process)
    suite.addTest(
        "test bundle abilityInfo with targetAbility",
        test_abilityinfo_with_targetAbility)
    suite.addTest(
        "test bundle abilityInfo with backgroundModes",
        test_abilityinfo_with_backgroundModes)
    suite.addTest(
        "test bundle abilityInfo with isVisible",
        test_abilityinfo_with_isVisible)
    suite.addTest(
        "test bundle abilityInfo with formEnabled",
        test_abilityinfo_with_formEnabled)
    suite.addTest(
        "test bundle abilityInfo with permissions",
        test_abilityinfo_with_permissions)
    suite.addTest(
        "test bundle abilityInfo with deviceTypes",
        test_abilityinfo_with_deviceTypes)
    suite.addTest(
        "test bundle abilityInfo with deviceCapabilities",
        test_abilityinfo_with_deviceCapabilities)
    suite.addTest(
        "test bundle abilityInfo with readPermission",
        test_abilityinfo_with_readPermission)
    suite.addTest(
        "test bundle abilityInfo with writePermission",
        test_abilityinfo_with_writePermission)
    suite.addTest("test bundle abilityInfo with uri", test_abilityinfo_with_uri)
    suite.addTest(
        "test bundle abilityInfo with enabled", test_abilityinfo_with_enabled)

    // applicationInfo.d.ts
    suite.addTest(
        "test applicationInfo interface with name",
        test_bundle_interface_applicationInfo_with_name);
    suite.addTest(
        "test applicationInfo interface with description",
        test_bundle_interface_applicationInfo_with_description);
    suite.addTest(
        "test applicationInfo interface with descriptionId",
        test_bundle_interface_applicationInfo_with_descriptionId);
    suite.addTest(
        "test applicationInfo interface with systemApp",
        test_bundle_interface_applicationInfo_with_systemApp);
    suite.addTest(
        "test applicationInfo interface with enabled",
        test_bundle_interface_applicationInfo_with_enabled);
    suite.addTest(
        "test applicationInfo interface with label",
        test_bundle_interface_applicationInfo_with_label);
    suite.addTest(
        "test applicationInfo interface with labelId",
        test_bundle_interface_applicationInfo_with_labelId);
    suite.addTest(
        "test applicationInfo interface with icon",
        test_bundle_interface_applicationInfo_with_icon);
    suite.addTest(
        "test applicationInfo interface with iconId",
        test_bundle_interface_applicationInfo_with_iconId);
    suite.addTest(
        "test applicationInfo interface with process",
        test_bundle_interface_applicationInfo_with_process);
    suite.addTest(
        "test applicationInfo interface with supportedModes",
        test_bundle_interface_applicationInfo_with_supportedModes);
    suite.addTest(
        "test applicationInfo interface with moduleSourceDirs",
        test_bundle_interface_applicationInfo_with_moduleSourceDirs);
    suite.addTest(
        "test applicationInfo interface with permissions",
        test_bundle_interface_applicationInfo_with_permissions);
    suite.addTest(
        "test applicationInfo interface with entryDir",
        test_bundle_interface_applicationInfo_with_entryDir);
    suite.addTest(
        "test applicationInfo interface with codePath",
        test_bundle_interface_applicationInfo_with_codePath);
    suite.addTest(
        "test applicationInfo interface with removable",
        test_bundle_interface_applicationInfo_with_removable);
    suite.addTest(
        "test applicationInfo interface with accessTokenId",
        test_bundle_interface_applicationInfo_with_accessTokenId);
    suite.addTest(
        "test applicationInfo interface with uid",
        test_bundle_interface_applicationInfo_with_uid);
    suite.addTest(
        "test applicationInfo interface with entityType",
        test_bundle_interface_applicationInfo_with_entityType);

    // bundleInfo.d.ts
    suite.addTest(
        "test bundle bundleInfo interface usedscene with when",
        test_bundleinfo_interface_usedScene_with_when)
    suite.addTest(
        "test bundle bundleInfo interface usedscene with abilities",
        test_bundleinfo_interface_usedScene_with_abilities)
    suite.addTest(
        "test bundle bundleInfo interface reqperdetail with name",
        test_bundleinfo_interface_reqperdetail_with_name)
    suite.addTest(
        "test bundle bundleInfo interface reqperdetail with reason",
        test_bundleinfo_interface_reqperdetail_with_reason)
    suite.addTest(
        "test bundle bundleInfo interface with when",
        test_bundleinfo_interface_with_name)
    suite.addTest(
        "test bundle bundleInfo interface with type",
        test_bundleinfo_interface_with_type)
    suite.addTest(
        "test bundle bundleInfo interface with appId",
        test_bundleinfo_interface_with_appId)
    suite.addTest(
        "test bundle bundleInfo interface with uid",
        test_bundleinfo_interface_withuid)
    suite.addTest(
        "test bundle bundleInfo interface with installTime",
        test_bundleinfo_interface_with_installTime)
    suite.addTest(
        "test bundle bundleInfo interface with updateTime",
        test_bundleinfo_interface_with_updateTime)
    suite.addTest(
        "test bundle bundleInfo interface with reqPermissions",
        test_bundleinfo_interface_with_reqPermissions)
    suite.addTest(
        "test bundle bundleInfo interface with vendor",
        test_bundleinfo_interface_with_vendor)
    suite.addTest(
        "test bundle bundleInfo interface with versionCode",
        test_bundleinfo_interface_with_versionCode)
    suite.addTest(
        "test bundle bundleInfo interface with versionName",
        test_bundleinfo_interface_with_versionName)
    suite.addTest(
        "test bundle bundleInfo interface with compatibleVersion",
        test_bundleinfo_interface_with_compatibleVersion)
    suite.addTest(
        "test bundle bundleInfo interface with targetVersion",
        test_bundleinfo_interface_with_targetVersion)
    suite.addTest(
        "test bundle bundleInfo interface with isCompressNativeLibs",
        test_bundleinfo_interface_with_isCompressNativeLibs)
    suite.addTest(
        "test bundle bundleInfo interface with entryModuleName",
        test_bundleinfo_interface_with_entryModuleName)
    suite.addTest(
        "test bundle bundleInfo interface with cpuAbi",
        test_bundleinfo_interface_with_cpuAbi)
    suite.addTest(
        "test bundle bundleInfo interface with isSilentInstallation",
        test_bundleinfo_interface_with_isSilentInstallation)
    suite.addTest(
        "test bundle bundleInfo interface with minCompatibleVersionCode",
        test_bundleinfo_interface_with_minCompatibleVersionCode)
    suite.addTest(
        "test bundle bundleInfo interface with entryInstallationFree",
        test_bundleinfo_interface_with_entryInstallationFree)
    suite.addTest(
        "test bundle bundleInfo interface with reqPermissionStates",
        test_bundleinfo_interface_with_reqPermissionStates)

    // customizeData.d.ts
    suite.addTest(
        "test bundle bundleInfo interface CustomizeData with name",
        test_bundleinfo_interface_custdata_with_name)
    suite.addTest(
        "test bundle bundleInfo interface CustomizeData with value",
        test_bundleinfo_interface_custdata_with_value)
    suite.addTest(
        "test bundle bundleInfo interface CustomizeData with extra",
        test_bundleinfo_interface_custdata_with_extra)

    // elementName.d.ts
    suite.addTest(
        "test elementName interface with deviceId",
        test_bundle_interface_elementName_with_deviceId);
    suite.addTest(
        "test elementName interface with deviceId _noSet",
        test_bundle_interface_elementName_with_deviceId_noSet);
    suite.addTest(
        "test elementName interface with bundleName",
        test_bundle_interface_elementName_with_bundleName);
    suite.addTest(
        "test elementName interface with abilityName",
        test_bundle_interface_elementName_with_abilityName);
    suite.addTest(
        "test elementName interface with uri _noSet",
        test_bundle_interface_elementName_with_uri_noSet);
    suite.addTest(
        "test elementName interface with uri",
        test_bundle_interface_elementName_with_uri);
    suite.addTest(
        "test elementName interface with shortName _noSet",
        test_bundle_interface_elementName_with_shortName_noSet);
    suite.addTest(
        "test elementName interface with shortName",
        test_bundle_interface_elementName_with_shortName);

    // hapModuleInfo.d.ts
    suite.addTest(
        "test hapModuleInfo interface with name",
        test_bundle_interface_hapModuleInfo_with_name)
    suite.addTest(
        "test hapModuleInfo interface with description",
        test_bundle_interface_hapModuleInfo_with_description)
    suite.addTest(
        "test hapModuleInfo interface with descriptionId",
        test_bundle_interface_hapModuleInfo_with_descriptionid)
    suite.addTest(
        "test hapModuleInfo interface with icon",
        test_bundle_interface_hapModuleInfo_with_icon)
    suite.addTest(
        "test hapModuleInfo interface with label",
        test_bundle_interface_hapModuleInfo_with_label)
    suite.addTest(
        "test hapModuleInfo interface with labelId",
        test_bundle_interface_hapModuleInfo_with_labelid)
    suite.addTest(
        "test hapModuleInfo interface with iconId",
        test_bundle_interface_hapModuleInfo_with_iconid)
    suite.addTest(
        "test hapModuleInfo interface with backgroundImg",
        test_bundle_interface_hapModuleInfo_with_backgroundImg)
    suite.addTest(
        "test hapModuleInfo interface with supportedModes",
        test_bundle_interface_hapModuleInfo_with_supportedModes)
    suite.addTest(
        "test hapModuleInfo interface with reqCapabilities",
        test_bundle_interface_hapModuleInfo_with_reqCapabilities)
    suite.addTest(
        "test hapModuleInfo interface with deviceTypes",
        test_bundle_interface_hapModuleInfo_with_deviceTypes)
    suite.addTest(
        "test hapModuleInfo interface with moduleName",
        test_bundle_interface_hapModuleInfo_with_moduleName)
    suite.addTest(
        "test hapModuleInfo interface with mainAbilityName",
        test_bundle_interface_hapModuleInfo_with_mainAbilityName)
    suite.addTest(
        "test hapModuleInfo interface with installationFree",
        test_bundle_interface_hapModuleInfo_with_installationFree)

    // moduleInfo.d.ts
    suite.addTest(
        "test ModuleInfo interface with moduleName",
        test_bundle_interface_moduleInfo_with_moduleName)
    suite.addTest(
        "test ModuleInfo interface with moduleSourceDir",
        test_bundle_interface_moduleInfo_with_moduleSourceDir)

    // shortcutInfo.d.ts
    suite.addTest(
        "test shortcutInfo interface with id",
        test_bundle_interface_shortcutInfo_with_id)
    suite.addTest(
        "test shortcutInfo interface with bundleName",
        test_bundle_interface_shortcutInfo_with_bundleName)
    suite.addTest(
        "test shortcutInfo interface with hostAbility",
        test_bundle_interface_shortcutInfo_with_hostAbility)
    suite.addTest(
        "test shortcutInfo interface with icon",
        test_bundle_interface_shortcutInfo_with_icon)
    suite.addTest(
        "test shortcutInfo interface with iconId",
        test_bundle_interface_shortcutInfo_with_iconId)
    suite.addTest(
        "test shortcutInfo interface with label",
        test_bundle_interface_shortcutInfo_with_label)
    suite.addTest(
        "test shortcutInfo interface with labelId",
        test_bundle_interface_shortcutInfo_with_labelId)
    suite.addTest(
        "test shortcutInfo interface with disableMessage",
        test_bundle_interface_shortcutInfo_with_disableMessage)
    suite.addTest(
        "test shortcutInfo interface with isStatic",
        test_bundle_interface_shortcutInfo_with_isStatic)
    suite.addTest(
        "test shortcutInfo interface with isHomeShortcut",
        test_bundle_interface_shortcutInfo_with_isHomeShortcut)
    suite.addTest(
        "test shortcutInfo interface with isEnabled",
        test_bundle_interface_shortcutInfo_with_isEnabled)

    //@ohos.bundle.d.ts
    suite.addTest(
        "test bundle ohos bundle with BundleFlag _DEFAULT",
        test_bundle_enum_BundleFlag_with_DEFAULT)
    suite.addTest(
        "test bundle ohos bundle with BundleFlag _DISABLE",
        test_bundle_enum_BundleFlag_with_DISABLE)
    suite.addTest(
        "test bundle ohos bundle with BundleFlag _INFO",
        test_bundle_enum_BundleFlag_with_INFO)
    suite.addTest(
        "test bundle ohos bundle with ColorMode",
        test_bundle_enum_with_colorMode)
    suite.addTest(
        "test bundle ohos bundle with GrantStatus",
        test_bundle_enum_with_grantStatus)
    suite.addTest(
        "test bundle ohos bundle with AbilityType",
        test_bundle_enum_with_abilityType)
    suite.addTest(
        "test bundle ohos bundle with AbilitySubType",
        test_bundle_enum_with_abilitySubType)
    suite.addTest(
        "test bundle ohos bundle with DisplayOrientation",
        test_bundle_enum_with_displayOrientation)
    suite.addTest(
        "test bundle ohos bundle with LaunchMode",
        test_bundle_enum_with_launchMode)
    suite.addTest(
        "test bundle ohos bundle with InstallErrorCode _FOUND",
        test_bundle_enum_with_installErrorCode_FOUND)
    suite.addTest(
        "test bundle ohos bundle with InstallErrorCode _SUCCESS",
        test_bundle_enum_with_installErrorCode_SUCCESS)
    suite.addTest(
        "test bundle ohos bundle with InstallErrorCode _TIMEOUT",
        test_bundle_enum_with_installErrorCode_TIMEOUT)
    suite.addTest(
        "test bundle ohos bundle interface BundleOptions with userId _noSet",
        test_bundle_interface_bundleOptions_with_userId_noSet)
    suite.addTest(
        "test bundle ohos bundle interface BundleOptions with userId",
        test_bundle_interface_bundleOptions_with_userId)
    suite.addTest(
        "test bundle ohos bundle function with getBundleInfo",
        test_bundle_function_with_getBundleInfo)
    suite.addTest(
        "test bundle ohos bundle function with getAbilityInfo",
        test_bundle_function_with_getAbilityInfo)
    suite.addTest(
        "test bundle ohos bundle function with getApplicationInfo 3 param",
        test_bundle_function_with_getApplicationInfo3param)
    suite.addTest(
        "test bundle ohos bundle function with getApplicationInfo 2 param",
        test_bundle_function_with_getApplicationInfo2param)
    suite.addTest(
        "test bundle ohos bundle function with getApplicationInfo have Optional",
        test_bundle_function_with_getApplicationInfoOptional_1)
    suite.addTest(
        "test bundle ohos bundle function with getApplicationInfo no Optional",
        test_bundle_function_with_getApplicationInfoOptional_2)
    suite.addTest(
        "test bundle ohos bundle function with queryAbilityByWant",
        test_bundle_function_with_queryAbilityByWant)
    suite.addTest(
        "test bundle ohos bundle function with getAllBundleInfo",
        test_bundle_function_with_getAllBundleInfo)
    suite.addTest(
        "test bundle ohos bundle function with getAllApplicationInfo",
        test_bundle_function_with_getAllApplicationInfo)
    suite.addTest(
        "test bundle ohos bundle function with getNameForUid",
        test_bundle_function_with_getNameForUid)
    suite.addTest(
        "test bundle ohos bundle function with getBundleArchiveInfo",
        test_bundle_function_with_getBundleArchiveInfo)
    suite.addTest(
        "test bundle ohos bundle function with getLaunchWantForBundle",
        test_bundle_function_with_getLaunchWantForBundle)
    suite.addTest(
        "test bundle ohos bundle function with getAbilityLabel",
        test_bundle_function_with_getAbilityLabel)
    suite.addTest(
        "test bundle ohos bundle function with getAbilityIcon",
        test_bundle_function_with_getAbilityIcon)
    suite.addTest(
        "test bundle ohos bundle function with isAbilityEnabled",
        test_bundle_function_with_isAbilityEnabled)
    suite.addTest(
        "test bundle ohos bundle function with isApplicationEnabled",
        test_bundle_function_with_isApplicationEnabled)

    // ohos.app.ability.Want.d.ts
    suite.addTest(
        "test applicatohos.app.ability.Want class with bundleName _noSet",
        test_bundle_class_want_with_bundleName_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with bundleName",
        test_bundle_class_want_with_bundleName);
    suite.addTest(
        "test applicatohos.app.ability.Want class with abilityName _noSet",
        test_bundle_class_want_with_abilityName_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with abilityName",
        test_bundle_class_want_with_abilityName);
    suite.addTest(
        "test applicatohos.app.ability.Want class with deviceId _noSet",
        test_bundle_class_want_with_deviceId_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with deviceId",
        test_bundle_class_want_with_deviceId);
    suite.addTest(
        "test applicatohos.app.ability.Want class with uri _noSet",
        test_bundle_class_want_with_uri_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with uri",
        test_bundle_class_want_with_uri);
    suite.addTest(
        "test applicatohos.app.ability.Want class with type _noSet",
        test_bundle_class_want_with_type_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with type",
        test_bundle_class_want_with_type);
    suite.addTest(
        "test applicatohos.app.ability.Want class with flags _noSet",
        test_bundle_class_want_with_flags_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with flags",
        test_bundle_class_want_with_flags);
    suite.addTest(
        "test applicatohos.app.ability.Want class with action _noSet",
        test_bundle_class_want_with_action_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with action",
        test_bundle_class_want_with_action);
    suite.addTest(
        "test applicatohos.app.ability.Want class with entities _noSet",
        test_bundle_class_want_with_entities_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with entities",
        test_bundle_class_want_with_entities);
    suite.addTest(
        "test applicatohos.app.ability.Want class with moduleName _noSet",
        test_bundle_class_want_with_moduleName_noSet);
    suite.addTest(
        "test applicatohos.app.ability.Want class with moduleName",
        test_bundle_class_want_with_moduleName);

    exit(suite.run());
}