#include <iostream>
#include <algorithm>
#include "dx_iface.h"


bool dx_iface::fileImport(const std::string& fileI, dx_data *fData){
    size_t found = fileI.find_last_of(".");
    std::string fileExt = fileI.substr(found+1);
    std::transform(fileExt.begin(), fileExt.end(),fileExt.begin(), ::toupper);
    cData = fData;
    currentBlock = cData->mBlock;

    if (fileExt == "DXF"){
        //loads dxf
        dxfRW* dxf = new dxfRW(fileI.c_str());
        bool success = dxf->read(this, false);
        delete dxf;
        return success;
    }
    std::cout << "file extension can be dxf" << std::endl;
    return false;
}
