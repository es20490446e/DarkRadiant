#include "Repository.h"

#include <git2.h>
#include "itextstream.h"
#include "Remote.h"
#include "Commit.h"
#include "Tree.h"
#include "Diff.h"
#include "GitException.h"
#include "os/path.h"

namespace vcs
{

namespace git
{

Repository::Repository(const std::string& path) :
    _repository(nullptr),
    _isOk(false),
    _path(os::standardPathWithSlash(path))
{
    if (git_repository_open(&_repository, _path.c_str()) == 0)
    {
        _isOk = true;
    }
    else
    {
        rMessage() << "Failed to open repository at " << _path << std::endl;
    }
}

bool Repository::isOk() const
{
    return _isOk;
}

const std::string& Repository::getPath() const
{
    return _path;
}

Repository::~Repository()
{
    git_repository_free(_repository);
}

std::shared_ptr<Repository> Repository::clone()
{
    return std::make_shared<Repository>(_path);
}

std::shared_ptr<Remote> Repository::getRemote(const std::string& name)
{
    return Remote::CreateFromName(*this, name);
}

Reference::Ptr Repository::getHead()
{
    git_reference* head;
    int error = git_repository_head(&head, _repository);

    if (error == GIT_EUNBORNBRANCH || error == GIT_ENOTFOUND)
    {
        return Reference::Ptr();
    }

    return std::make_shared<Reference>(head);
}

std::string Repository::getCurrentBranchName()
{
    auto head = getHead();
    return head ? head->getShorthandName() : std::string();
}

std::string Repository::getUpstreamRemoteName(const Reference& reference)
{
    git_buf buf;
    memset(&buf, 0, sizeof(git_buf));

    auto error = git_branch_upstream_remote(&buf, _repository, reference.getName().c_str());
    GitException::ThrowOnError(error);

    std::string upstreamRemote = buf.ptr;
    git_buf_dispose(&buf);

    return upstreamRemote;
}

void Repository::fetchFromTrackedRemote()
{
    auto head = getHead();

    if (!head)
    {
        rWarning() << "Could not retrieve HEAD reference from repository" << std::endl;
        return;
    }

    auto trackedBranch = head->getUpstream();

    rMessage() << head->getShorthandName() << " is set up to track " << (trackedBranch ? trackedBranch->getShorthandName() : "-") << std::endl;

    if (!trackedBranch)
    {
        rWarning() << "No tracked remote branch configured, cannot fetch" << std::endl;
        return;
    }

    auto remoteName = getUpstreamRemoteName(*head);
    rMessage() << head->getShorthandName() << " is set up to track remote " << remoteName << std::endl;

    auto remote = getRemote(remoteName);

    if (!remote)
    {
        rWarning() << "Cannot fetch from remote 'origin'" << std::endl;
        return;
    }

    remote->fetch();
}

RefSyncStatus Repository::getSyncStatusOfBranch(const Reference& reference)
{
    RefSyncStatus status;

    auto trackedBranch = reference.getUpstream();

    if (!trackedBranch) throw GitException("No tracked branch, cannot check sync status");

    git_revwalk* walker;
    git_revwalk_new(&walker, _repository);

    // Start from remote
    git_revwalk_push_ref(walker, trackedBranch->getName().c_str());

    // End at local
    git_oid refOid;
    git_reference_name_to_id(&refOid, _repository, reference.getName().c_str());
    git_revwalk_hide(walker, &refOid);

    git_oid id;
    while (!git_revwalk_next(&id, walker))
    {
        rMessage() << Reference::OidToString(&id) << " => ";
        ++status.remoteCommitsAhead;
    }
    rMessage() << std::endl;

    git_revwalk_free(walker);

    // Another walk from local to remote
    git_revwalk_new(&walker, _repository);

    git_revwalk_push(walker, &refOid);
    git_revwalk_hide_ref(walker, trackedBranch->getName().c_str());

    while (!git_revwalk_next(&id, walker))
    {
        rMessage() << Reference::OidToString(&id) << " => ";
        ++status.localCommitsAhead;
    }
    rMessage() << std::endl;

    git_revwalk_free(walker);

    // Initialise the convenience flags
    status.localIsUpToDate = status.localCommitsAhead == 0 && status.remoteCommitsAhead == 0;
    status.localCanBePushed = status.localCommitsAhead > 0 && status.remoteCommitsAhead == 0;

    return status;
}

bool Repository::isUpToDateWithRemote()
{
    auto head = getHead();

    if (!head)
    {
        rWarning() << "Could not retrieve HEAD reference from repository" << std::endl;
        return false;
    }

    return getSyncStatusOfBranch(*head).localIsUpToDate;
}

unsigned int Repository::getFileStatus(const std::string& relativePath)
{
    git_status_options options = GIT_STATUS_OPTIONS_INIT;

    char* paths[] = { const_cast<char*>(relativePath.c_str()) };

    options.pathspec.count = 1;
    options.pathspec.strings = paths;
    options.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
    options.show = GIT_STATUS_SHOW_WORKDIR_ONLY;

    unsigned int statusFlags = 0;

    auto error = git_status_foreach_ext(_repository, &options, [](const char* path, unsigned int flags, void* payload)
    {
        *reinterpret_cast<unsigned int*>(payload) = flags;
        return 0;
    }, &statusFlags);
    GitException::ThrowOnError(error);

    return statusFlags;
}

bool Repository::fileIsIndexed(const std::string& relativePath)
{
    return (getFileStatus(relativePath) & GIT_STATUS_WT_NEW) == 0;
}

bool Repository::fileHasUncommittedChanges(const std::string& relativePath)
{
    return (getFileStatus(relativePath) & GIT_STATUS_WT_MODIFIED) != 0;
}

Commit::Ptr Repository::findMergeBase(const Reference& first, const Reference& second)
{
    git_oid firstOid;
    auto error = git_reference_name_to_id(&firstOid, _repository, first.getName().c_str());
    GitException::ThrowOnError(error);

    git_oid secondOid;
    error = git_reference_name_to_id(&secondOid, _repository, second.getName().c_str());
    GitException::ThrowOnError(error);

    git_oid mergeBase;
    error = git_merge_base(&mergeBase, _repository, &firstOid, &secondOid);
    GitException::ThrowOnError(error);

    git_commit* commit;
    error = git_commit_lookup(&commit, _repository, &mergeBase);
    GitException::ThrowOnError(error);

    return std::make_shared<Commit>(commit);
}

std::shared_ptr<Diff> Repository::getDiff(const Reference& ref, Commit& commit)
{
    git_oid refOid;
    auto error = git_reference_name_to_id(&refOid, _repository, ref.getName().c_str());
    GitException::ThrowOnError(error);

    auto refCommit = Commit::CreateFromOid(_repository, &refOid);
    auto refTree = refCommit->getTree();

    git_diff* diff;
    auto baseTree = commit.getTree();
    error = git_diff_tree_to_tree(&diff, _repository, baseTree->_get(), refTree->_get(), nullptr);
    GitException::ThrowOnError(error);

    return std::make_shared<Diff>(diff);
}

git_repository* Repository::_get()
{
    return _repository;
}

}

}