#include "syncwrapper.h"
#include "folderman.h"
#include "socketapi.h"
#include "vfs_mac.h"

namespace OCC {

Q_LOGGING_CATEGORY(lcSyncWrapper, "nextcloud.gui.wrapper", QtInfoMsg)

SyncWrapper *SyncWrapper::instance()
{
    static SyncWrapper instance;
    return &instance;
}

QString SyncWrapper::removeSlash(QString path){
    if(path.startsWith('/'))
        path.remove(0, 1);

    return path;
}

void SyncWrapper::updateSyncQueue(const QString path, bool syncing) {
    _syncDone.insert(path, syncing);
}

bool SyncWrapper::syncDone(const QString path) {
    return _syncDone.value(removeSlash(path));
}

void SyncWrapper::updateLocalFileTree(const QString &path, csync_instructions_e instruction){
    OCC::FolderMan::instance()->currentSyncFolder()->updateLocalFileTree(path, instruction);
}

void SyncWrapper::openFileAtPath(const QString path){
    initSync(removeSlash(path), CSYNC_INSTRUCTION_SYNC);
}

void SyncWrapper::releaseFileAtPath(const QString path){
    initSync(removeSlash(path), CSYNC_INSTRUCTION_EVAL);
}

void SyncWrapper::writeFileAtPath(const QString path){
    initSync(removeSlash(path), CSYNC_INSTRUCTION_NEW);
}

void SyncWrapper::initSync(const QString path, csync_instructions_e instruction){
    initSyncMode(path);
    updateLocalFileTree(path, instruction);
    _syncDone.insert(path, false);
}

void SyncWrapper::startSync(){
    OCC::FolderMan::instance()->currentSyncFolder()->startSync();
}

QDateTime SyncWrapper::lastAccess(const QString path){
    return SyncJournalDb::instance()->getLastAccess(removeSlash(path));
}

int SyncWrapper::updateLastAccess(const QString path){
    return SyncJournalDb::instance()->updateLastAccess(removeSlash(path));
}

int SyncWrapper::syncMode(const QString path){
    return SyncJournalDb::instance()->getSyncMode(removeSlash(path));
}

void SyncWrapper::initSyncMode(const QString path){
    int result = 1;
    if(SyncJournalDb::instance()->getSyncMode(path) == SyncJournalDb::SyncMode::SYNCMODE_ONLINE){
        result = SyncJournalDb::instance()->setSyncMode(path, SyncJournalDb::SyncMode::SYNCMODE_OFFLINE);
    } else if(SyncJournalDb::instance()->getSyncMode(path) == SyncJournalDb::SyncMode::SYNCMODE_NONE) {
       result = SyncJournalDb::instance()->setSyncMode(path, SyncJournalDb::SyncMode::SYNCMODE_ONLINE);
    }

    if(result == 0)
        qCWarning(lcSyncWrapper) << "Couldn't change file SYNCMODE.";

   result = SyncJournalDb::instance()->setSyncModeDownload(path, SyncJournalDb::SyncModeDownload::SYNCMODE_DOWNLOADED_NO);
   if(result == 0)
        qCWarning(lcSyncWrapper) << "Couldn't set file to SYNCMODE_DOWNLOADED_NO.";
}

int SyncWrapper::syncModeDownload(const QString path){
    return SyncJournalDb::instance()->getSyncModeDownload(removeSlash(path));
}

bool SyncWrapper::shouldSync(const QString path){
    // Check last access
    // check if it is downloaded
    // etc

    if(syncMode(path) == SyncJournalDb::SyncMode::SYNCMODE_NONE)
        return false;

    return true;
}

}