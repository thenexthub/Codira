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

#include "Codira/Basic/Match.h"
#include "common/CommonFunc.h"
#include "StructuralRuleGOTH03.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace std;

const std::string IP_REGEX =
    "((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)(?!\\.|\\d))";
const std::string URL_REGEX =
    "(((http|https|ftp|ftps):\\/\\/)|(www\\.))([\u4e00-\u9fa5\\w\\-]+[.])+(net|com|cn|org|cc|tv|edu|vip|top|hk|gov|biz)"
    "[\u4e00-\u9fa5\\w\\-\\.,@?^=%&:\\/~\\+#]*";
const std::string EMAIL_REGEX =
    "([\u4e00-\u9fa5\\w\\-\\.]+)@(((\\w)+[.])+)(net|com|cn|org|cc|tv|edu|vip|top|hk|gov|biz)";

const int A_TYPE_IP_START = 10;
const int B_TYPE_IP_START = 172;
const int B_TYPE_IP_START_SECTION_B = 16;
const int B_TYPE_IP_END_SECTION_B = 31;
const int C_TYPE_IP_START = 192;
const int C_TYPE_IP_SECTION_B = 168;
const int D_TYPE_IP_START = 224;
const int D_TYPE_IP_END = 239;
const int SYS_LOOPBACK_IP_START = 127;
const int LINK_LOCAL_IP_START = 169;
const int LINK_LOCAL_IP_START_SECTION_B = 254;
const int LINK_LOCAL_IP_END_SECTION_B = 254;
const int RESTRICT_BROADCAST_MAX_SECTION = 255;
const int IP_SECTION_A = 0;
const int IP_SECTION_B = 1;
const int IP_SECTION_C = 2;
const int IP_SECTION_D = 3;
const int DECIMAL = 10;

bool StructuralRuleGOTH03::IsSpecialIpHardcode(std::vector<int> ips) const
{
    if (ips[IP_SECTION_A] == 0 && ips[IP_SECTION_B] == 0 && ips[IP_SECTION_C] == 0 && ips[IP_SECTION_D] == 0) {
        return false;
    }
    if (ips[IP_SECTION_A] == SYS_LOOPBACK_IP_START) {
        return false;
    }
    if (ips[IP_SECTION_A] == LINK_LOCAL_IP_START && ips[IP_SECTION_B] == LINK_LOCAL_IP_START_SECTION_B &&
        ips[IP_SECTION_C] <= LINK_LOCAL_IP_END_SECTION_B) {
        if (ips[IP_SECTION_A] == LINK_LOCAL_IP_START && ips[IP_SECTION_B] == LINK_LOCAL_IP_START_SECTION_B &&
            ips[IP_SECTION_C] == 0 && ips[IP_SECTION_D] == 0) {
            return true;
        }
        return false;
    }
    if (ips[IP_SECTION_A] == RESTRICT_BROADCAST_MAX_SECTION && ips[IP_SECTION_B] == RESTRICT_BROADCAST_MAX_SECTION &&
        ips[IP_SECTION_C] == RESTRICT_BROADCAST_MAX_SECTION && ips[IP_SECTION_D] == RESTRICT_BROADCAST_MAX_SECTION) {
        return false;
    }
    return true;
}

/*
 * Split IP address to four part and judge each part separately
 * Follow the following rules
 * 1.Private network IP range
 * Type A: 10.0.0.0-10.255.255.255;
 * Type B: 172.16.0.0-172.31.255.255;
 * Type C: 192.168.0.0-192.168.255.255;
 * Type D(multicast IP address): 224.0.0.1~239.255.255.255;
 * 2.Special IP address range
 * (1)Private network multicast address: 239.0.0.0-239.255.255.255;
 * (2)Link local address (link local address): 169.254.0.1-169.254.254.255;
 * (the address used when the device communicates in the local network, such as DHCP failure to reserve the private
 * network address) (3)System loopback address: 127.0.0.0-127.255.255.255; (4)Reserved address: 0.0.0.0 (all unclear
 * hosts and destination networks); (5)Restrict broadcast address: 255.255.255.255 (this address cannot be forwarded by
 * the router).
 */
bool StructuralRuleGOTH03::isIpHardcode(std::vector<int> ips)
{
    if (ips[IP_SECTION_A] == A_TYPE_IP_START) {
        return false;
    }
    if (ips[IP_SECTION_A] == B_TYPE_IP_START &&
        (ips[IP_SECTION_B] >= B_TYPE_IP_START_SECTION_B && ips[IP_SECTION_B] <= B_TYPE_IP_END_SECTION_B)) {
        return false;
    }
    if (ips[IP_SECTION_A] == C_TYPE_IP_START && ips[IP_SECTION_B] == C_TYPE_IP_SECTION_B) {
        return false;
    }
    if (ips[IP_SECTION_A] >= D_TYPE_IP_START && ips[IP_SECTION_A] <= D_TYPE_IP_END) {
        if (ips[IP_SECTION_A] == D_TYPE_IP_START && ips[IP_SECTION_B] == 0 && ips[IP_SECTION_C] == 0 &&
            ips[IP_SECTION_D] == 0) {
            return true;
        }
        return false;
    }
    return IsSpecialIpHardcode(ips);
}

bool StructuralRuleGOTH03::CheckIpAddress(const std::string ip)
{
    if (ip.empty()) {
        return false;
    }
    vector<int> ips;
    std::vector<std::string> ipStr = Utils::SplitString(ip, ".");
    for (const auto& ipPart : ipStr) {
        size_t pos;
        long ipNum = std::stol(ipPart, &pos, DECIMAL);
        if (pos != ipPart.size()) {
            continue;
        }
        ips.push_back(static_cast<int>(ipNum));
    }
    return isIpHardcode(ips);
}

void StructuralRuleGOTH03::RecordErrorLocation(const std::vector<ResultInfo>::iterator iter, CodeCheckDiagKind kind)
{
    Diagnose(iter->filepath, iter->line, iter->column, iter->endLine, iter->endColumn, kind, iter->result);
}

static nlohmann::json CollectWhiteList(const std::string jsonFile, const std::string keyWord)
{
    nlohmann::json jsonData;
    if (CommonFunc::ReadJsonFileToJsonInfo(jsonFile, ConfigContext::GetInstance(), jsonData) == ERR) {
        return nlohmann::json();
    }
    if (!jsonData.contains(keyWord)) {
        Errorln(jsonFile, " read json data failed!");
        return nlohmann::json();
    }
    return jsonData[keyWord];
}

void StructuralRuleGOTH03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;

    std::map<int, std::regex> reg;
    (void)reg.emplace(static_cast<int>(HardcodeType::IP), std::regex(IP_REGEX));
    (void)reg.emplace(static_cast<int>(HardcodeType::URL), regex(URL_REGEX));
    (void)reg.emplace(static_cast<int>(HardcodeType::EMAIL), regex(EMAIL_REGEX));

    std::vector<ResultInfo> result = RegexRule::InitRegexInfo(node, reg);
    std::vector<ResultInfo>::iterator iter = result.begin();

    nlohmann::json whiteList = CollectWhiteList("/config/structural_rule_G_OTH_03.json", "WhiteList");

    for (; iter != result.end(); ++iter) {
        bool found = std::find(whiteList.begin(), whiteList.end(), iter->result) != whiteList.end();
        if (found) {
            continue;
        }
        switch (iter->type) {
            case static_cast<int>(HardcodeType::IP):
                if (CheckIpAddress(iter->result)) {
                    RecordErrorLocation(iter, CodeCheckDiagKind::G_OTH_03_public_ip_hardcode_information);
                }
                break;
            case static_cast<int>(HardcodeType::URL):
                RecordErrorLocation(iter, CodeCheckDiagKind::G_OTH_03_public_url_hardcode_information);
                break;
            case static_cast<int>(HardcodeType::EMAIL):
                RecordErrorLocation(iter, CodeCheckDiagKind::G_OTH_03_public_email_hardcode_information);
                break;
            default:
                break;
        }
    }
    RegexRule::resultVector.clear();
}
} // namespace Codira::CodeCheck
