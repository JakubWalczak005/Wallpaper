#include "FileExplorerManager.h"
#include "FileExplorer.h"
#include "Folder.h"

void FileExplorerManager::updateFileExplorerVec() {
    for (auto& folder : fileExplorerVecBuffer)
        fileExplorerVec.emplace_back(std::move(folder));
    fileExplorerVecBuffer.clear();
}