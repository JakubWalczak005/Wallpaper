#pragma once
#include "FileExplorer.h"

struct FileExplorerManager {

    std::vector<std::unique_ptr<FileExplorer>> fileExplorerVec;
    std::vector<std::unique_ptr<FileExplorer>> fileExplorerVecBuffer;

    void updateFileExplorerVec();

};