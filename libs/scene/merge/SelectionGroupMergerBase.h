#pragma once

#include <sstream>
#include <map>
#include "iselectiongroup.h"
#include "imap.h"
#include "NodeUtils.h"
#include "math/Hash.h"

namespace scene
{

namespace merge
{

class SelectionGroupMergerBase
{
protected:
    std::stringstream _log;

public:
    std::string getLogMessages() const
    {
        return _log.str();
    }

protected:
    using GroupMembers = std::map<std::string, scene::INodePtr>;

    GroupMembers getGroupMemberFingerprints(selection::ISelectionGroup& group)
    {
        GroupMembers members;

        group.foreachNode([&](const INodePtr& member)
        {
            members.emplace(NodeUtils::GetGroupMemberFingerprint(member), member);
        });

        return members;
    }

    // A group fingerprint only consists of the member fingerprints, its ID and the member ordering is irrelevant
    std::string getGroupFingerprint(selection::ISelectionGroup& group)
    {
        std::set<std::string> memberFingerprints;

        group.foreachNode([&](const INodePtr& member)
        {
            memberFingerprints.emplace(NodeUtils::GetGroupMemberFingerprint(member));
        });

        math::Hash hash;

        for (const auto& fingerprint : memberFingerprints)
        {
            hash.addString(fingerprint);
        }

        return hash;
    }

    using NodeFingerprints = std::map<std::string, scene::INodePtr>;

    NodeFingerprints collectNodeFingerprints(const IMapRootNodePtr& root)
    {
        NodeFingerprints result;

        // Collect all source and base nodes for easier lookup
        root->foreachNode([&](const INodePtr& node)
        {
            if (!std::dynamic_pointer_cast<IGroupSelectable>(node)) return true;

            result.emplace(NodeUtils::GetGroupMemberFingerprint(node), node);
            return true;
        });

        return result;
    }
};

}

}
