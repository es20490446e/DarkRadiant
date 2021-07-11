#pragma once

#include <string>
#include <memory>
#include "Reference.h"

struct git_repository;

namespace vcs
{

namespace git
{

class Remote;
class Commit;
class Diff;

/**
 * Represents a Git repository at a certain path
 */
class Repository final
{
private:
    git_repository* _repository;
    bool _isOk;

    std::string _path;

public:
    Repository(const std::string& path);
    ~Repository();

    // Status query of this repository object, 
    // returns true if this repository exists and has been successfully opened
    bool isOk() const;

    const std::string& getPath() const;

    // Returns the remote with the given name
    std::shared_ptr<Remote> getRemote(const std::string& name);

    std::string getCurrentBranchName();

    std::string getUpstreamRemoteName(const Reference& reference);

    Reference::Ptr getHead();

    // Performs a fetch from the remote the current branch is tracking
    void fetchFromTrackedRemote();

    bool isUpToDateWithRemote();
    bool fileIsIndexed(const std::string& relativePath);
    bool fileHasUncommittedChanges(const std::string& relativePath);

    // Compares the state of the given ref to the state of its tracked remote,
    // returns the number of commits each of them is ahead of the other one.
    RefSyncStatus getSyncStatusOfBranch(const Reference& reference);

    // Finds a common ancestor of the two refs, to base a merge operation on
    std::shared_ptr<Commit> findMergeBase(const Reference& first, const Reference& second);

    // Get the diff of the reference against the given commit
    std::shared_ptr<Diff> getDiff(const Reference& ref, Commit& commit);

    // Creates a new instance of this repository, not sharing any libgit2 handles with the original
    std::shared_ptr<Repository> clone();

    // Return the raw libgit2 object
    git_repository* _get();

private:
    unsigned int getFileStatus(const std::string& relativePath);
};

}

}