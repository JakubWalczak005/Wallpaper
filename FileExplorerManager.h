#pragma once
#include <vector>
#include <memory>

struct FileExplorer;

struct FileExplorerManager {

    std::vector<std::unique_ptr<FileExplorer>> fileExplorerVec;
    std::vector<std::unique_ptr<FileExplorer>> fileExplorerVecBuffer;

    void updateFileExplorerVec();

};