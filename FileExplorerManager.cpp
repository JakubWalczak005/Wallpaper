#include "FileExplorerManager.h"
#include "FileExplorer/FileExplorer.h"
#include "FileExplorer/Folder.h"

void FileExplorerManager::updateFileExplorerVec() {
    for (auto* file : fileExplorerVecEraseBuffer)
        std::erase_if(fileExplorerVec, [file](const std::unique_ptr<FileExplorer>& ptr) {return ptr.get() == file;});
    fileExplorerVecEraseBuffer.clear();
    for (auto& folder : fileExplorerVecAddBuffer)
        fileExplorerVec.emplace_back(std::move(folder));
    fileExplorerVecAddBuffer.clear();
}