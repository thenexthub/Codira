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
import * as elementName from "elementName";
import * as extensionAbilityInfo from "extensionAbilityInfo";
import * as hapModuleInfo from "hapModuleInfo";
import * as metadata from "metadata";
import * as overlayModuleInfo from "overlayModuleInfo";
import * as skillTest from "skill";

loadLibrary("ani_bundlemanager");

let intDig = 21474;

// Skill.d.ts
let skill: skillTest.Skill = skillTest.getSkill();
function test_bundlemanager_interface_skill_with_actions() {
    let info = skill.actions[0];

    console.log("skill.actions is: " + info);
    arktest.assertEQ(info, "SkillImpl::getActions");
}

function test_bundlemanager_interface_skill_with_entities() {
    let info = skill.entities[0];

    console.log("skill.entities is: " + info);
    arktest.assertEQ(info, "SkillImpl::getEntities");
}

function test_bundlemanager_interface_skill_with_domainVerify() {
    let info = skill.domainVerify;

    console.log("skill.domainVerify is: " + info);
    arktest.assertEQ(info, true);
}

let skillUri: skillTest.SkillUri = skillTest.getSkillUri();
function test_bundlemanager_interface_skilluri_with_scheme() {
    let info = skillUri.scheme;

    console.log("skillUri.scheme is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getScheme");
}

function test_bundlemanager_interface_skilluri_with_host() {
    let info = skillUri.host;

    console.log("skillUri.host is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getHost");
}

function test_bundlemanager_interface_skilluri_with_port() {
    let info = skillUri.port;

    console.log("skillUri.host is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_skilluri_with_path() {
    let info = skillUri.path;

    console.log("skillUri.path is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getPath");
}

function test_bundlemanager_interface_skilluri_with_pathStartWith() {
    let info = skillUri.pathStartWith;

    console.log("skillUri.pathStartWith is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getPathStartWith");
}

function test_bundlemanager_interface_skilluri_with_pathRegex() {
    let info = skillUri.pathRegex;

    console.log("skillUri.pathRegex is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getPathRegex");
}

function test_bundlemanager_interface_skilluri_with_type() {
    let info = skillUri.type;

    console.log("skillUri.type is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getType");
}

function test_bundlemanager_interface_skilluri_with_utd() {
    let info = skillUri.utd;

    console.log("skillUri.utd is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getUtd");
}

function test_bundlemanager_interface_skilluri_with_maxFileSupported() {
    let info = skillUri.maxFileSupported;

    console.log("skillUri.maxFileSupported is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_skilluri_with_linkFeature() {
    let info = skillUri.linkFeature;

    console.log("skillUri.linkFeature is: " + info);
    arktest.assertEQ(info, "SkillUriImpl::getLinkFeature");
}

// Abilityinfo.d.ts
let abiInfo: abilityInfo.AbilityInfo = abilityInfo.getAbilityInfo();
function test_bundlemanager_interface_abilityinfo_with_bundleName() {
    let info = abiInfo.bundleName;

    console.log("abiInfo.bundleName is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getBundleName");
}

function test_bundlemanager_interface_abilityinfo_with_moduleName() {
    let info = abiInfo.moduleName;

    console.log("abiInfo.moduleName is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getModuleName");
}

function test_bundlemanager_interface_abilityinfo_with_name() {
    let info = abiInfo.name;

    console.log("abiInfo.name is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getName");
}

function test_bundlemanager_interface_abilityinfo_with_label() {
    let info = abiInfo.label;

    console.log("abiInfo.label is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getLabel");
}

function test_bundlemanager_interface_abilityinfo_with_labelId() {
    let info = abiInfo.labelId;

    console.log("abiInfo.labelId is: " + info);
    arktest.assertEQ(info, 506 as short);
}

function test_bundlemanager_interface_abilityinfo_with_description() {
    let info = abiInfo.description;

    console.log("abiInfo.description is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getDescription");
}

function test_bundlemanager_interface_abilityinfo_with_descriptionId() {
    let info = abiInfo.descriptionId;

    console.log("abiInfo.descriptionId is: " + info);
    arktest.assertEQ(info, 506 as short);
}

function test_bundlemanager_interface_abilityinfo_with_icon() {
    let info = abiInfo.icon;

    console.log("abiInfo.icon is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getIcon");
}

function test_bundlemanager_interface_abilityinfo_with_iconId() {
    let info = abiInfo.iconId;

    console.log("abiInfo.iconId is: " + info);
    arktest.assertEQ(info, 506 as short);
}

function test_bundlemanager_interface_abilityinfo_with_process() {
    let info = abiInfo.process;

    console.log("abiInfo.process is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getProcess");
}

function test_bundlemanager_interface_abilityinfo_with_exported() {
    let info = abiInfo.exported;

    console.log("abiInfo.exported is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_abilityinfo_with_type() {
    let info = abiInfo.type;

    console.log("abiInfo.type is: " + info);
    arktest.assertEQ(info, abiInfo.type);
}

function test_bundlemanager_interface_abilityinfo_with_orientation() {
    let info = abiInfo.orientation;

    console.log("abiInfo.orientation is: " + info);
    arktest.assertEQ(info, abiInfo.orientation);
}

function test_bundlemanager_interface_abilityinfo_with_launchType() {
    let info = abiInfo.launchType;

    console.log("abiInfo.launchType is: " + info);
    arktest.assertEQ(info, abiInfo.launchType);
}

function test_bundlemanager_interface_abilityinfo_with_permissions() {
    let info = abiInfo.permissions[0];

    console.log("abiInfo.permissions is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getPermissions");
}

function test_bundlemanager_interface_abilityinfo_with_readPermission() {
    let info = abiInfo.readPermission;

    console.log("abiInfo.readPermission is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getReadPermission");
}

function test_bundlemanager_interface_abilityinfo_with_writePermission() {
    let info = abiInfo.writePermission;

    console.log("abiInfo.writePermission is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getWritePermission");
}

function test_bundlemanager_interface_abilityinfo_with_uri() {
    let info = abiInfo.uri;

    console.log("abiInfo.uri is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getUri");
}

function test_bundlemanager_interface_abilityinfo_with_deviceTypes() {
    let info = abiInfo.deviceTypes[0];

    console.log("abiInfo.deviceTypes is: " + info);
    arktest.assertEQ(info, "abilityInfoImpl::getDeviceTypes");
}

function test_bundlemanager_interface_abilityinfo_with_metadata() {
    let info = abiInfo.metadata;

    console.log("abiInfo.metadata is: " + info);
}

function test_bundlemanager_interface_abilityinfo_with_enabled() {
    let info = abiInfo.enabled;

    console.log("abiInfo.enabled is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_abilityinfo_with_supportWindowModes() {
    let info = abiInfo.supportWindowModes[0];

    console.log("abiInfo.type is: " + info);
    arktest.assertEQ(info, abiInfo.supportWindowModes[0]);
}

function test_bundlemanager_interface_abilityinfo_with_excludeFromDock() {
    let info = abiInfo.excludeFromDock;

    console.log("abiInfo.excludeFromDock is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_abilityinfo_with_skills() {
    let info = abiInfo.skills;

    console.log("abiInfo.skills is: " + info);
}

function test_bundlemanager_interface_abilityinfo_with_appIndex() {
    let info = abiInfo.appIndex;

    console.log("abiInfo.appIndex is: " + info);
    arktest.assertEQ(info, 506 as short);
}

function test_bundlemanager_interface_abilityinfo_with_orientationId() {
    let info = abiInfo.orientationId;

    console.log("abiInfo.orientationId is: " + info);
    arktest.assertEQ(info, 506 as short);
}

// OverlayModuleInfo.d.ts
let overInfo: overlayModuleInfo.OverlayModuleInfo =
    overlayModuleInfo.getOverlayModuleInfo();
function test_bundlemanager_interface_overlaymoduleinfo_with_bundleName() {
    let info = overInfo.bundleName;

    console.log("overInfo.bundleName is: " + info);
    arktest.assertEQ(info, "OverlayModuleInfoImpl::getBundleName");
}

function test_bundlemanager_interface_overlaymoduleinfo_with_moduleName() {
    let info = overInfo.moduleName;

    console.log("overInfo.moduleName is: " + info);
    arktest.assertEQ(info, "OverlayModuleInfoImpl::getModuleName");
}

function
test_bundlemanager_interface_overlaymoduleinfo_with_targetModuleName() {
    let info = overInfo.targetModuleName;

    console.log("overInfo.targetModuleName is: " + info);
    arktest.assertEQ(info, "OverlayModuleInfoImpl::getTargetModuleName");
}

function test_bundlemanager_interface_overlaymoduleinfo_with_priority() {
    let info = overInfo.priority;

    console.log("overInfo.priority is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_overlaymoduleinfo_with_state() {
    let info = overInfo.state;

    console.log("overInfo.state is: " + info);
    arktest.assertEQ(info, intDig);
}

// Metadata.d.ts
let metadata_: metadata.Metadata = metadata.getMetadata();
function test_bundlemanager_interface_metadata_with_name() {
    metadata_.name = "bob";
    let info = metadata_.name;

    console.log("metadata.name is: " + info);
    arktest.assertEQ(info, "bob");
}

function test_bundlemanager_interface_metadata_with_value() {
    metadata_.value = "apple";
    let info = metadata_.value;

    console.log("metadata.value is: " + info);
    arktest.assertEQ(info, "apple");
}

function test_bundlemanager_interface_metadata_with_resource() {
    metadata_.resource = "food";
    let info = metadata_.resource;

    console.log("metadata.resource is: " + info);
    arktest.assertEQ(info, "food");
}

function test_bundlemanager_interface_metadata_with_valueId() {
    let info = metadata_.valueId;

    console.log("metadata.valueId is: " + info);
    arktest.assertEQ(info, intDig);
}

// HapModuleInfo.d.ts
let hapInfo: hapModuleInfo.HapModuleInfo = hapModuleInfo.getHapModuleInfo();
let dependency: hapModuleInfo.Dependency = hapModuleInfo.getDependency();
let preloadItem: hapModuleInfo.PreloadItem = hapModuleInfo.getPreloadItem();
let routerItem: hapModuleInfo.RouterItem = hapModuleInfo.getRouterItem();
let dataItem: hapModuleInfo.DataItem = hapModuleInfo.getDataItem();
function test_bundlemanager_interface_hapmoduleinfo_with_name() {
    let info = hapInfo.name;

    console.log("hapInfo.name is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getName");
}

function test_bundlemanager_interface_hapmoduleinfo_with_icon() {
    let info = hapInfo.icon;

    console.log("hapInfo.icon is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getIcon");
}

function test_bundlemanager_interface_hapmoduleinfo_with_iconId() {
    let info = hapInfo.iconId;

    console.log("hapInfo.iconId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_hapmoduleinfo_with_label() {
    let info = hapInfo.label;

    console.log("hapInfo.label is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getLabel");
}

function test_bundlemanager_interface_hapmoduleinfo_with_labelId() {
    let info = hapInfo.labelId;

    console.log("hapInfo.labelId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_hapmoduleinfo_with_description() {
    let info = hapInfo.description;

    console.log("hapInfo.description is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getDescription");
}

function test_bundlemanager_interface_hapmoduleinfo_with_descriptionId() {
    let info = hapInfo.descriptionId;

    console.log("hapInfo.descriptionId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_hapmoduleinfo_with_mainElementName() {
    let info = hapInfo.mainElementName;

    console.log("hapInfo.mainElementName is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getMainElementName");
}

function test_bundlemanager_interface_hapmoduleinfo_with_deviceTypes() {
    let info = hapInfo.deviceTypes[0];

    console.log("hapInfo.deviceTypes is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getDeviceTypes");
}

function test_bundlemanager_interface_hapmoduleinfo_with_installationFree() {
    let info = hapInfo.installationFree;

    console.log("hapInfo.installationFree is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_hapmoduleinfo_with_hashValue() {
    let info = hapInfo.hashValue;

    console.log("hapInfo.hashValue is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getHashValue");
}

function
test_bundlemanager_interface_hapmoduleinfo_with_fileContextMenuConfig() {
    let info = hapInfo.fileContextMenuConfig;

    console.log("hapInfo.fileContextMenuConfig is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getFileContextMenuConfig");
}

function test_bundlemanager_interface_hapmoduleinfo_with_nativeLibraryPath() {
    let info = hapInfo.nativeLibraryPath;

    console.log("hapInfo.nativeLibraryPath is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getNativeLibraryPath");
}

function test_bundlemanager_interface_hapmoduleinfo_with_codePath() {
    let info = hapInfo.codePath;

    console.log("hapInfo.codePath is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getCodePath");
}

function test_bundlemanager_interface_dependency_with_moduleName() {
    let info = dependency.moduleName;

    console.log("dependency.moduleName is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getModuleName");
}

function test_bundlemanager_interface_dependency_with_bundleName() {
    let info = dependency.bundleName;

    console.log("dependency.bundleName is: " + info);
    arktest.assertEQ(info, "HapModuleInfoImpl::getBundleName");
}

function test_bundlemanager_interface_dependency_with_versionCode() {
    let info = dependency.versionCode;

    console.log("dependency.versionCode is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_preloaditem_with_moduleName() {
    let info = preloadItem.moduleName;

    console.log("preloadItem.moduleName is: " + info);
    arktest.assertEQ(info, "PreloadItemImpl::getModuleName");
}

function test_bundlemanager_interface_routerItem_with_name() {
    let info = routerItem.name;

    console.log("routerItem.name is: " + info);
    arktest.assertEQ(info, "RouterItemImpl::getName");
}

function test_bundlemanager_interface_routerItem_with_pageSourceFile() {
    let info = routerItem.pageSourceFile;

    console.log("routerItem.pageSourceFile is: " + info);
    arktest.assertEQ(info, "RouterItemImpl::getPageSourceFile");
}

function test_bundlemanager_interface_routerItem_with_buildFunction() {
    let info = routerItem.buildFunction;

    console.log("routerItem.buildFunction is: " + info);
    arktest.assertEQ(info, "RouterItemImpl::getBuildFunction");
}

function test_bundlemanager_interface_routerItem_with_customData() {
    let info = routerItem.customData;

    console.log("routerItem.customData is: " + info);
    arktest.assertEQ(info, "RouterItemImpl::getCustomData");
}

function test_bundlemanager_interface_dataItem_with_key() {
    let info = dataItem.key;

    console.log("dataItem.key is: " + info);
    arktest.assertEQ(info, "DataItemImpl::getKey");
}

function test_bundlemanager_interface_dataItem_with_value() {
    let info = dataItem.value;

    console.log("dataItem.value is: " + info);
    arktest.assertEQ(info, "DataItemImpl::getValue");
}

// ExtensionAbilityInfo.d.ts
let exAbiInfo: extensionAbilityInfo.ExtensionAbilityInfo =
    extensionAbilityInfo.getExtensionAbilityInfo();
function test_bundlemanager_interface_extensionAbilityInfo_with_bundleName() {
    let info = exAbiInfo.bundleName;

    console.log("exAbiInfo.bundleName is: " + info);
    arktest.assertEQ(info, "ExtensionAbilityInfoImpl::getBundleName");
}

function test_bundlemanager_interface_extensionAbilityInfo_with_moduleName() {
    let info = exAbiInfo.moduleName;

    console.log("exAbiInfo.moduleName is: " + info);
    arktest.assertEQ(info, "ExtensionAbilityInfoImpl::getModuleName");
}

function test_bundlemanager_interface_extensionAbilityInfo_with_name() {
    let info = exAbiInfo.name;

    console.log("exAbiInfo.name is: " + info);
    arktest.assertEQ(info, "ExtensionAbilityInfoImpl::getName");
}

function test_bundlemanager_interface_extensionAbilityInfo_with_labelId() {
    let info = exAbiInfo.labelId;

    console.log("exAbiInfo.labelId is: " + info);
    arktest.assertEQ(info, intDig);
}

function
test_bundlemanager_interface_extensionAbilityInfo_with_descriptionId() {
    let info = exAbiInfo.descriptionId;

    console.log("abiIexAbiInfonfo.descriptionId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_extensionAbilityInfo_with_iconId() {
    let info = exAbiInfo.iconId;

    console.log("exAbiInfo.iconId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_extensionAbilityInfo_with_exported() {
    let info = exAbiInfo.exported;

    console.log("exAbiInfo.exported is: " + info);
    arktest.assertEQ(info, true);
}

function
test_bundlemanager_interface_extensionAbilityInfo_with_extensionAbilityTypeName() {
    let info = exAbiInfo.extensionAbilityTypeName;

    console.log("exAbiInfo.extensionAbilityTypeName is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_extensionAbilityInfo_with_permissions() {
    let info = exAbiInfo.permissions[0];

    console.log("exAbiInfo.permissions is: " + info);
    arktest.assertEQ(info, "ExtensionAbilityInfoImpl::getPermissions");
}

function test_bundlemanager_interface_extensionAbilityInfo_with_enabled() {
    let info = exAbiInfo.enabled;

    console.log("exAbiInfo.enabled is: " + info);
    arktest.assertEQ(info, true);
}

function
test_bundlemanager_interface_extensionAbilityInfo_with_readPermission() {
    let info = exAbiInfo.readPermission;

    console.log("exAbiInfo.readPermission is: " + info);
    arktest.assertEQ(info, "ExtensionAbilityInfoImpl::getReadPermission");
}

function
test_bundlemanager_interface_extensionAbilityInfo_with_writePermission() {
    let info = exAbiInfo.writePermission;

    console.log("exAbiInfo.writePermission is: " + info);
    arktest.assertEQ(info, "ExtensionAbilityInfoImpl::getWritePermission");
}

function test_bundlemanager_interface_extensionAbilityInfo_with_appIndex() {
    let info = exAbiInfo.appIndex;

    console.log("exAbiInfo.appIndex is: " + info);
    arktest.assertEQ(info, intDig);
}

// ApplicationInfo.d.ts
let appLicaInfo: applicationInfo.ApplicationInfo =
    applicationInfo.getApplicationInfo();
let moudleMData: applicationInfo.ModuleMetadata =
    applicationInfo.getModuleMetadata();
let multiMode: applicationInfo.MultiAppMode = applicationInfo.getMultiAppMode();
function test_bundlemanager_interface_applicationInfo_with_name() {
    let info = appLicaInfo.name;

    console.log("appLicaInfo.name is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getName");
}

function test_bundlemanager_interface_applicationInfo_with_description() {
    let info = appLicaInfo.description;

    console.log("appLicaInfo.description is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getDescription");
}

function test_bundlemanager_interface_applicationInfo_with_descriptionId() {
    let info = appLicaInfo.descriptionId;

    console.log("appLicaInfo.descriptionId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_applicationInfo_with_enabled() {
    let info = appLicaInfo.enabled;

    console.log("appLicaInfo.enabled is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_applicationInfo_with_label() {
    let info = appLicaInfo.label;

    console.log("appLicaInfo.label is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getLabel");
}

function test_bundlemanager_interface_applicationInfo_with_labelId() {
    let info = appLicaInfo.labelId;

    console.log("appLicaInfo.labelId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_applicationInfo_with_icon() {
    let info = appLicaInfo.icon;

    console.log("abiInfo.icon is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getIcon");
}

function test_bundlemanager_interface_applicationInfo_with_iconId() {
    let info = appLicaInfo.iconId;

    console.log("appLicaInfo.iconId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_applicationInfo_with_process() {
    let info = appLicaInfo.process;

    console.log("appLicaInfo.process is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getProcess");
}

function test_bundlemanager_interface_applicationInfo_with_permissions() {
    let info = appLicaInfo.permissions[0];

    console.log("appLicaInfo.permissions is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getPermissions");
}

function test_bundlemanager_interface_applicationInfo_with_codePath() {
    let info = appLicaInfo.codePath;

    console.log("appLicaInfo.codePath is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getCodePath");
}

function test_bundlemanager_interface_applicationInfo_with_removable() {
    let info = appLicaInfo.removable;

    console.log("appLicaInfo.removable is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_applicationInfo_with_accessTokenId() {
    let info = appLicaInfo.accessTokenId;

    console.log("appLicaInfo.accessTokenId is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_applicationInfo_with_uid() {
    let info = appLicaInfo.uid;

    console.log("appLicaInfo.uid is: " + info);
    arktest.assertEQ(info, intDig);
}

function
test_bundlemanager_interface_applicationInfo_with_appDistributionType() {
    let info = appLicaInfo.appDistributionType;

    console.log("appLicaInfo.appDistributionType is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getAppDistributionType");
}

function test_bundlemanager_interface_applicationInfo_with_appProvisionType() {
    let info = appLicaInfo.appProvisionType;

    console.log("appLicaInfo.appProvisionType is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getAppProvisionType");
}

function test_bundlemanager_interface_applicationInfo_with_systemApp() {
    let info = appLicaInfo.systemApp;

    console.log("appLicaInfo.systemApp is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_applicationInfo_with_debug() {
    let info = appLicaInfo.debug;

    console.log("appLicaInfo.debug is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_applicationInfo_with_dataUnclearable() {
    let info = appLicaInfo.dataUnclearable;

    console.log("appLicaInfo.dataUnclearable is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_applicationInfo_with_nativeLibraryPath() {
    let info = appLicaInfo.nativeLibraryPath;

    console.log("appLicaInfo.nativeLibraryPath is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getNativeLibraryPath");
}

function test_bundlemanager_interface_applicationInfo_with_appIndex() {
    let info = appLicaInfo.appIndex;

    console.log("appLicaInfo.appIndex is: " + info);
    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_applicationInfo_with_installSource() {
    let info = appLicaInfo.installSource;

    console.log("appLicaInfo.installSource is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getInstallSource");
}

function test_bundlemanager_interface_applicationInfo_with_releaseType() {
    let info = appLicaInfo.releaseType;

    console.log("appLicaInfo.releaseType is: " + info);
    arktest.assertEQ(info, "ApplicationInfoImpl::getReleaseType");
}

function
test_bundlemanager_interface_applicationInfo_with_cloudFileSyncEnabled() {
    let info = appLicaInfo.cloudFileSyncEnabled;

    console.log("appLicaInfo.cloudFileSyncEnabled is: " + info);
    arktest.assertEQ(info, true);
}

function test_bundlemanager_interface_moduleMetadata_with_moduleName() {
    let info = moudleMData.moduleName;

    console.log("moudleMData.moduleName is: " + info);
    arktest.assertEQ(info, "ModuleMetadataImpl::getModuleName");
}

function test_bundlemanager_interface_multiAppMode_with_maxCount() {
    let info = multiMode.maxCount;

    console.log("multiMode.maxCount: " + info);
    arktest.assertEQ(info, intDig);
}

// ElementName.d.ts
let eleName: elementName.ElementName = elementName.getElementName();
function test_bundlemanager_interface_elementName_with_deviceId() {
    eleName.deviceId = "DI0428";
    let info = eleName.deviceId;

    console.log("eleName.deviceId is: " + info);
    arktest.assertEQ(info, "DI0428");
}

function test_bundlemanager_interface_elementName_with_bundleName() {
    eleName.bundleName = "BN0428";
    let info = eleName.bundleName;

    console.log("eleName.bundleName is: " + info);
    arktest.assertEQ(info, "BN0428");
}

function test_bundlemanager_interface_elementName_with_moduleName() {
    eleName.moduleName = "MN0428";
    let info = eleName.moduleName;

    console.log("eleName.moduleName is: " + info);
    arktest.assertEQ(info, "MN0428");
}

function test_bundlemanager_interface_elementName_with_abilityName() {
    eleName.abilityName = "AN0428";
    let info = eleName.abilityName;

    console.log("eleName.abilityName is: " + info);
    arktest.assertEQ(info, "AN0428");
}

function test_bundlemanager_interface_elementName_with_uri() {
    eleName.uri = "Uri0428";
    let info = eleName.uri;

    console.log("eleName.uri is: " + info);
    arktest.assertEQ(info, "Uri0428");
}

function test_bundlemanager_interface_elementName_with_shortName() {
    eleName.shortName = "SN0428";
    let info = eleName.shortName;

    console.log("eleName.shortName is: " + info);
    arktest.assertEQ(info, "SN0428");
}

// BundleInfo.d.ts
let bundInfo: bundleInfo.BundleInfo = bundleInfo.getBundleInfo();
let reqDetail: bundleInfo.ReqPermissionDetail =
    bundleInfo.getReqPermissionDetail();
let used: bundleInfo.UsedScene = bundleInfo.getIUsedScene();
let sigInfo: bundleInfo.SignatureInfo = bundleInfo.getISignatureInfo();
let appClone: bundleInfo.AppCloneIdentity = bundleInfo.getAppCloneIdentity();
function test_bundlemanager_interface_bundleinfo_with_name() {
    let info = bundInfo.name;

    arktest.assertEQ(info, "BundleInfoImpl::getName");
}

function test_bundlemanager_interface_bundleinfo_with_vendor() {
    let info = bundInfo.vendor;

    arktest.assertEQ(info, "BundleInfoImpl::getVendor");
}

function test_bundlemanager_interface_bundleinfo_with_versionCode() {
    let info = bundInfo.versionCode;

    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_bundleinfo_with_versionName() {
    let info = bundInfo.versionName;

    arktest.assertEQ(info, "BundleInfoImpl::getVersionName");
}

function
test_bundlemanager_interface_bundleinfo_with_minCompatibleVersionCode() {
    let info = bundInfo.minCompatibleVersionCode;

    arktest.assertEQ(info, "BundleInfoImpl::getMinCompatibleVersionCode");
}

function test_bundlemanager_interface_bundleinfo_with_targetVersion() {
    let info = bundInfo.targetVersion;

    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_bundleinfo_with_installTime() {
    let info = bundInfo.installTime;

    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_bundleinfo_with_updateTime() {
    let info = bundInfo.updateTime;

    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_bundleinfo_with_appIndex() {
    let info = bundInfo.appIndex;

    arktest.assertEQ(info, intDig);
}

function test_bundlemanager_interface_ReqPermissionDetail_with_name() {
    reqDetail.name = "req0428";
    let info = reqDetail.name;

    arktest.assertEQ(info, "req0428");
}

function test_bundlemanager_interface_ReqPermissionDetail_with_moduleName() {
    reqDetail.moduleName = "MN04280428";
    let info = reqDetail.moduleName;

    arktest.assertEQ(info, "MN04280428");
}

function test_bundlemanager_interface_ReqPermissionDetail_with_reason() {
    reqDetail.reason = "reason0428";
    let info = reqDetail.reason;

    arktest.assertEQ(info, "reason0428");
}

function test_bundlemanager_interface_ReqPermissionDetail_with_reasonId() {
    reqDetail.reasonId = 2025;
    let info = reqDetail.reasonId;

    arktest.assertEQ(info, 2025);
}

function test_bundlemanager_interface_usedScene_with_abilities() {
    used.abilities = ["abilities1", "abilities2", "abilities3"];
    let info = used.abilities[0];

    arktest.assertEQ(info, "abilities1");
}

function test_bundlemanager_interface_usedScene_with_when() {
    used.when = "when0428";
    let info = used.when;

    arktest.assertEQ(info, "when0428");
}

function test_bundlemanager_interface_signatureInfo_with_appId() {
    let info = sigInfo.appId;

    arktest.assertEQ(info, "SignatureInfoImpl::getAppId");
}

function test_bundlemanager_interface_signatureInfo_with_fingerprint() {
    let info = sigInfo.fingerprint;

    arktest.assertEQ(info, "SignatureInfoImpl::getFingerprint");
}

function test_bundlemanager_interface_signatureInfo_with_appIdentifier() {
    let info = sigInfo.appIdentifier;

    arktest.assertEQ(info, "SignatureInfoImpl::getAppIdentifier");
}

function test_bundlemanager_interface_signatureInfo_with_certificate() {
    let info = sigInfo.certificate;

    arktest.assertEQ(info, "SignatureInfoImpl::getCertificate");
}

function test_bundlemanager_interface_appCloneIdentity_with_bundleName() {
    let info = appClone.bundleName;

    arktest.assertEQ(info, "AppCloneIdentityImpl::getBundleName");
}

function test_bundlemanager_interface_appCloneIdentity_with_appIndex() {
    let info = appClone.appIndex;

    arktest.assertEQ(info, intDig);
}

function main() {
    console.log("##############start#############");
    const suite = new arktest.ArkTestsuite("BundleManager Tests");

    // Abilityinfo.d.ts
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_bundleName",
        test_bundlemanager_interface_abilityinfo_with_bundleName);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_moduleName",
        test_bundlemanager_interface_abilityinfo_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_name",
        test_bundlemanager_interface_abilityinfo_with_name);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_label",
        test_bundlemanager_interface_abilityinfo_with_label);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_labelId",
        test_bundlemanager_interface_abilityinfo_with_labelId);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_description",
        test_bundlemanager_interface_abilityinfo_with_description);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_descriptionId",
        test_bundlemanager_interface_abilityinfo_with_descriptionId);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_icon",
        test_bundlemanager_interface_abilityinfo_with_icon);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_iconId",
        test_bundlemanager_interface_abilityinfo_with_iconId);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_process",
        test_bundlemanager_interface_abilityinfo_with_process);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_exported",
        test_bundlemanager_interface_abilityinfo_with_exported);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_type",
        test_bundlemanager_interface_abilityinfo_with_type);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_orientation",
        test_bundlemanager_interface_abilityinfo_with_orientation);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_launchType",
        test_bundlemanager_interface_abilityinfo_with_launchType);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_permissions",
        test_bundlemanager_interface_abilityinfo_with_permissions);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_readPermission",
        test_bundlemanager_interface_abilityinfo_with_readPermission);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_writePermission",
        test_bundlemanager_interface_abilityinfo_with_writePermission);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_uri",
        test_bundlemanager_interface_abilityinfo_with_uri);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_deviceTypes",
        test_bundlemanager_interface_abilityinfo_with_deviceTypes);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_metadata",
        test_bundlemanager_interface_abilityinfo_with_metadata);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_enabled",
        test_bundlemanager_interface_abilityinfo_with_enabled);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_supportWindowModes",
        test_bundlemanager_interface_abilityinfo_with_supportWindowModes);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_excludeFromDock",
        test_bundlemanager_interface_abilityinfo_with_excludeFromDock);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_skills",
        test_bundlemanager_interface_abilityinfo_with_skills);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_appIndex",
        test_bundlemanager_interface_abilityinfo_with_appIndex);
    suite.addTest(
        "test_bundlemanager_interface_abilityinfo_with_orientationId",
        test_bundlemanager_interface_abilityinfo_with_orientationId);

    // ApplicationInfo.d.ts
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_name",
        test_bundlemanager_interface_applicationInfo_with_name);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_description",
        test_bundlemanager_interface_applicationInfo_with_description);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_descriptionId",
        test_bundlemanager_interface_applicationInfo_with_descriptionId);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_enabled",
        test_bundlemanager_interface_applicationInfo_with_enabled);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_label",
        test_bundlemanager_interface_applicationInfo_with_label);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_labelId",
        test_bundlemanager_interface_applicationInfo_with_labelId);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_icon",
        test_bundlemanager_interface_applicationInfo_with_icon);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_iconId",
        test_bundlemanager_interface_applicationInfo_with_iconId);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_process",
        test_bundlemanager_interface_applicationInfo_with_process);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_permissions",
        test_bundlemanager_interface_applicationInfo_with_permissions);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_codePath",
        test_bundlemanager_interface_applicationInfo_with_codePath);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_removable",
        test_bundlemanager_interface_applicationInfo_with_removable);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_accessTokenId",
        test_bundlemanager_interface_applicationInfo_with_accessTokenId);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_uid",
        test_bundlemanager_interface_applicationInfo_with_uid);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_appDistributionType",
        test_bundlemanager_interface_applicationInfo_with_appDistributionType);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_appProvisionType",
        test_bundlemanager_interface_applicationInfo_with_appProvisionType);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_systemApp",
        test_bundlemanager_interface_applicationInfo_with_systemApp);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_debug",
        test_bundlemanager_interface_applicationInfo_with_debug);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_dataUnclearable",
        test_bundlemanager_interface_applicationInfo_with_dataUnclearable);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_nativeLibraryPath",
        test_bundlemanager_interface_applicationInfo_with_nativeLibraryPath);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_appIndex",
        test_bundlemanager_interface_applicationInfo_with_appIndex);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_installSource",
        test_bundlemanager_interface_applicationInfo_with_installSource);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_releaseType",
        test_bundlemanager_interface_applicationInfo_with_releaseType);
    suite.addTest(
        "test_bundlemanager_interface_applicationInfo_with_cloudFileSyncEnabled",
        test_bundlemanager_interface_applicationInfo_with_cloudFileSyncEnabled);
    suite.addTest(
        "test_bundlemanager_interface_moduleMetadata_with_moduleName",
        test_bundlemanager_interface_moduleMetadata_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_multiAppMode_with_maxCount",
        test_bundlemanager_interface_multiAppMode_with_maxCount);

    // BundleInfo.d.ts
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_name",
        test_bundlemanager_interface_bundleinfo_with_name);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_vendor",
        test_bundlemanager_interface_bundleinfo_with_vendor);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_versionCode",
        test_bundlemanager_interface_bundleinfo_with_versionCode);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_versionName",
        test_bundlemanager_interface_bundleinfo_with_versionName);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_minCompatibleVersionCode",
        test_bundlemanager_interface_bundleinfo_with_minCompatibleVersionCode);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_targetVersion",
        test_bundlemanager_interface_bundleinfo_with_targetVersion);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_installTime",
        test_bundlemanager_interface_bundleinfo_with_installTime);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_updateTime",
        test_bundlemanager_interface_bundleinfo_with_updateTime);
    suite.addTest(
        "test_bundlemanager_interface_bundleinfo_with_appIndex",
        test_bundlemanager_interface_bundleinfo_with_appIndex);
    suite.addTest(
        "test_bundlemanager_interface_ReqPermissionDetail_with_name",
        test_bundlemanager_interface_ReqPermissionDetail_with_name);
    suite.addTest(
        "test bundlemanager interface ReqPermissionDetail with moduleName",
        test_bundlemanager_interface_ReqPermissionDetail_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_ReqPermissionDetail_with_reason",
        test_bundlemanager_interface_ReqPermissionDetail_with_reason);
    suite.addTest(
        "test_bundlemanager_interface_ReqPermissionDetail_with_reasonId",
        test_bundlemanager_interface_ReqPermissionDetail_with_reasonId);
    suite.addTest(
        "test_bundlemanager_interface_usedScene_with_abilities",
        test_bundlemanager_interface_usedScene_with_abilities);
    suite.addTest(
        "test_bundlemanager_interface_usedScene_with_when",
        test_bundlemanager_interface_usedScene_with_when);
    suite.addTest(
        "test_bundlemanager_interface_signatureInfo_with_appId",
        test_bundlemanager_interface_signatureInfo_with_appId);
    suite.addTest(
        "test_bundlemanager_interface_signatureInfo_with_fingerprint",
        test_bundlemanager_interface_signatureInfo_with_fingerprint);
    suite.addTest(
        "test_bundlemanager_interface_signatureInfo_with_appIdentifier",
        test_bundlemanager_interface_signatureInfo_with_appIdentifier);
    suite.addTest(
        "test_bundlemanager_interface_signatureInfo_with_certificate",
        test_bundlemanager_interface_signatureInfo_with_certificate);
    suite.addTest(
        "test_bundlemanager_interface_appCloneIdentity_with_bundleName",
        test_bundlemanager_interface_appCloneIdentity_with_bundleName);
    suite.addTest(
        "test_bundlemanager_interface_appCloneIdentity_with_appIndex",
        test_bundlemanager_interface_appCloneIdentity_with_appIndex);

    // ElementName.d.ts
    suite.addTest(
        "test_bundlemanager_interface_elementName_with_deviceId",
        test_bundlemanager_interface_elementName_with_deviceId);
    suite.addTest(
        "test_bundlemanager_interface_elementName_with_bundleName",
        test_bundlemanager_interface_elementName_with_bundleName);
    suite.addTest(
        "test_bundlemanager_interface_elementName_with_moduleName",
        test_bundlemanager_interface_elementName_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_elementName_with_abilityName",
        test_bundlemanager_interface_elementName_with_abilityName);
    suite.addTest(
        "test_bundlemanager_interface_elementName_with_uri",
        test_bundlemanager_interface_elementName_with_uri);
    suite.addTest(
        "test_bundlemanager_interface_elementName_with_shortName",
        test_bundlemanager_interface_elementName_with_shortName);

    // ExtensionAbilityInfo.d.ts
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_bundleName",
        test_bundlemanager_interface_extensionAbilityInfo_with_bundleName);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_moduleName",
        test_bundlemanager_interface_extensionAbilityInfo_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_name",
        test_bundlemanager_interface_extensionAbilityInfo_with_name);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_labelId",
        test_bundlemanager_interface_extensionAbilityInfo_with_labelId);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_descriptionId",
        test_bundlemanager_interface_extensionAbilityInfo_with_descriptionId);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_iconId",
        test_bundlemanager_interface_extensionAbilityInfo_with_iconId);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_exported",
        test_bundlemanager_interface_extensionAbilityInfo_with_exported);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_extensionAbilityTypeName",
        test_bundlemanager_interface_extensionAbilityInfo_with_extensionAbilityTypeName);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_permissions",
        test_bundlemanager_interface_extensionAbilityInfo_with_permissions);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_enabled",
        test_bundlemanager_interface_extensionAbilityInfo_with_enabled);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_readPermission",
        test_bundlemanager_interface_extensionAbilityInfo_with_readPermission);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_writePermission",
        test_bundlemanager_interface_extensionAbilityInfo_with_writePermission);
    suite.addTest(
        "test_bundlemanager_interface_extensionAbilityInfo_with_appIndex",
        test_bundlemanager_interface_extensionAbilityInfo_with_appIndex);

    // HapModuleInfo.d.ts
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_name",
        test_bundlemanager_interface_hapmoduleinfo_with_name);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_icon",
        test_bundlemanager_interface_hapmoduleinfo_with_icon);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_iconId",
        test_bundlemanager_interface_hapmoduleinfo_with_iconId);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_label",
        test_bundlemanager_interface_hapmoduleinfo_with_label);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_labelId",
        test_bundlemanager_interface_hapmoduleinfo_with_labelId);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_description",
        test_bundlemanager_interface_hapmoduleinfo_with_description);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_descriptionId",
        test_bundlemanager_interface_hapmoduleinfo_with_descriptionId);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_mainElementName",
        test_bundlemanager_interface_hapmoduleinfo_with_mainElementName);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_deviceTypes",
        test_bundlemanager_interface_hapmoduleinfo_with_deviceTypes);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_installationFree",
        test_bundlemanager_interface_hapmoduleinfo_with_installationFree);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_hashValue",
        test_bundlemanager_interface_hapmoduleinfo_with_hashValue);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_fileContextMenuConfig",
        test_bundlemanager_interface_hapmoduleinfo_with_fileContextMenuConfig);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_nativeLibraryPath",
        test_bundlemanager_interface_hapmoduleinfo_with_nativeLibraryPath);
    suite.addTest(
        "test_bundlemanager_interface_hapmoduleinfo_with_codePath",
        test_bundlemanager_interface_hapmoduleinfo_with_codePath);
    suite.addTest(
        "test_bundlemanager_interface_dependency_with_moduleName",
        test_bundlemanager_interface_dependency_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_dependency_with_bundleName",
        test_bundlemanager_interface_dependency_with_bundleName);
    suite.addTest(
        "test_bundlemanager_interface_dependency_with_versionCode",
        test_bundlemanager_interface_dependency_with_versionCode);
    suite.addTest(
        "test_bundlemanager_interface_preloaditem_with_moduleName",
        test_bundlemanager_interface_preloaditem_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_routerItem_with_name",
        test_bundlemanager_interface_routerItem_with_name);
    suite.addTest(
        "test_bundlemanager_interface_routerItem_with_pageSourceFile",
        test_bundlemanager_interface_routerItem_with_pageSourceFile);
    suite.addTest(
        "test_bundlemanager_interface_routerItem_with_buildFunction",
        test_bundlemanager_interface_routerItem_with_buildFunction);
    suite.addTest(
        "test_bundlemanager_interface_routerItem_with_customData",
        test_bundlemanager_interface_routerItem_with_customData);
    suite.addTest(
        "test_bundlemanager_interface_dataItem_with_key",
        test_bundlemanager_interface_dataItem_with_key);
    suite.addTest(
        "test_bundlemanager_interface_dataItem_with_value",
        test_bundlemanager_interface_dataItem_with_value);

    // Metadata.d.ts
    suite.addTest(
        "test_bundlemanager_interface_metadata_with_name",
        test_bundlemanager_interface_metadata_with_name);
    suite.addTest(
        "test_bundlemanager_interface_metadata_with_value",
        test_bundlemanager_interface_metadata_with_value);
    suite.addTest(
        "test_bundlemanager_interface_metadata_with_resource",
        test_bundlemanager_interface_metadata_with_resource);
    suite.addTest(
        "test_bundlemanager_interface_metadata_with_valueId",
        test_bundlemanager_interface_metadata_with_valueId);

    // OverlayModuleInfo.d.ts
    suite.addTest(
        "test_bundlemanager_interface_overlaymoduleinfo_with_bundleName",
        test_bundlemanager_interface_overlaymoduleinfo_with_bundleName);
    suite.addTest(
        "test_bundlemanager_interface_overlaymoduleinfo_with_moduleName",
        test_bundlemanager_interface_overlaymoduleinfo_with_moduleName);
    suite.addTest(
        "test_bundlemanager_interface_overlaymoduleinfo_with_targetModuleName",
        test_bundlemanager_interface_overlaymoduleinfo_with_targetModuleName);
    suite.addTest(
        "test_bundlemanager_interface_overlaymoduleinfo_with_priority",
        test_bundlemanager_interface_overlaymoduleinfo_with_priority);
    suite.addTest(
        "test_bundlemanager_interface_overlaymoduleinfo_with_state",
        test_bundlemanager_interface_overlaymoduleinfo_with_state);

    // Skill.d.ts
    suite.addTest(
        "test_bundlemanager_interface_skill_with_actions",
        test_bundlemanager_interface_skill_with_actions);
    suite.addTest(
        "test_bundlemanager_interface_skill_with_entities",
        test_bundlemanager_interface_skill_with_entities);
    suite.addTest(
        "test_bundlemanager_interface_skill_with_domainVerify",
        test_bundlemanager_interface_skill_with_domainVerify);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_scheme",
        test_bundlemanager_interface_skilluri_with_scheme);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_host",
        test_bundlemanager_interface_skilluri_with_host);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_port",
        test_bundlemanager_interface_skilluri_with_port);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_path",
        test_bundlemanager_interface_skilluri_with_path);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_pathStartWith",
        test_bundlemanager_interface_skilluri_with_pathStartWith);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_pathRegex",
        test_bundlemanager_interface_skilluri_with_pathRegex);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_type",
        test_bundlemanager_interface_skilluri_with_type);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_utd",
        test_bundlemanager_interface_skilluri_with_utd);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_maxFileSupported",
        test_bundlemanager_interface_skilluri_with_maxFileSupported);
    suite.addTest(
        "test_bundlemanager_interface_skilluri_with_linkFeature",
        test_bundlemanager_interface_skilluri_with_linkFeature);

    exit(suite.run());
}