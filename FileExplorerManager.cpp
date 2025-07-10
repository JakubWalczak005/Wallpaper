#include "FileExplorerManager.h"

void FileExplorerManager::updateFileExplorerVec() {
    for (auto& folder : fileExplorerVecBuffer)
        fileExplorerVec.emplace_back(std::move(folder));
    fileExplorerVecBuffer.clear();
}