#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "dx_iface.h"
#include "dx_data.h"
#include "testVTKRender.h"
int main() {
    bool badState = false;

    std::string inName = "D:/Topo.dxf";
    dx_data fData;

    dx_iface *input = new dx_iface();
    badState = input->fileImport( inName, &fData );
    if (!badState) {
        std::cout << "Error reading file " << inName << std::endl;
        return 0;
    }
    RenderDXFData vis;
    vis.SetDXFData(&fData);
    vis.InitDisplay();
    return 1;
}

