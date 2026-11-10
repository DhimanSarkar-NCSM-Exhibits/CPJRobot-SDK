#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <string>
#include <typeinfo>
#include <iomanip>
#include <thread>
#include <chrono>

#include "poelidar.h"

using namespace std;
using namespace cpj::poelidar;

int simulate_lidar_data();


// Entry Point
int main(int argc, char **argv) {
    string   dev_ip   = "192.168.1.10";   // Default M1 IP
    uint16_t dev_port = 25168;            // Default M1 Port

    for(int i=1; i<argc; i++) {
        string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            cout << "Usage: " << argv[0] << " [OPTIONS]\n"
                 << "\n"
                 << "Options:\n"
                 << "  -h, --help          Show this help message and exit\n"
                 << "      --ip <IP>       Device IP address (Default: " << dev_ip << ")\n"
                 << "      --port <PORT>   Device port number (Default: " << dev_port << ")\n"
                 << "      --simulate      Simulate LiDAR data output\n";
            return 0;
        }
        else if(arg == "--ip"){
            // assuming value of the key exists
            // refactor with proper handling
            dev_ip = argv[i+1];
        } 
        else if (arg == (string)"--port"){
            dev_port = stoi(argv[i+1]);
        }
        else if (arg == "--simulate"){
            cout << "SIMULATION MODE" << endl;
            simulate_lidar_data();
        }
        else {
            continue;
        }
    }


    /////////////////////////////////////////////////////////////////////
    // LiDAR USE
    ////////////////////////////////////////////////////////////////////

    POELidarDriver *dev = POELidarDriver::CreateDriver(poelidar_model::M1);

    cout << "Attempting to create CPJROBOT M1" << endl;
    cout << "IP:\t" << dev_ip << endl;
    cout << "Port:\t" << dev_port << endl << endl;

    if(!dev->connect(dev_ip, dev_port)){
        cerr << "ERROR:\t Unable to to connect!" << endl;
        delete dev;
        return -1;
    }

    if(!dev->start()) {
        cerr << "ERROR:\t Unable to initialize!" << endl;
        delete dev;
        return -1;
    }

    vector<poelidar_point_t>  laser_scan;

    while (true) { // continous readout loop
        if(!dev->getScanData(laser_scan)) {
            continue;
        }
        
        // vector<vector<float>> frame = {};
        ostringstream polar_r;
        ostringstream polar_theta;
        ostringstream polar_intensity;

        for (int i = 0; i < laser_scan.size(); i++) {
            float angle = laser_scan[i].angle;          //unit: degree
            float range = laser_scan[i].range;          //unit: mm
            float intensity = laser_scan[i].intensity;

            // frame.push_back({angle, range, intensity});

            polar_r         << range     << ",";
            polar_theta     << angle     << ",";
            polar_intensity << intensity << ",";
        }

        // FORMAT: [[polar_r], [polar_theta], [polar_intensity]]
        cout << "[";
        cout << "[" << polar_r.str() << "]" << ",";
        cout << "[" << polar_theta.str() << "]" << ",";
        cout << "[" << polar_intensity.str() << "]";
        cout << "]" << endl;
    }

    dev->stop();
    dev->disconnect();
    delete dev;

    return 0;   
}


int simulate_lidar_data() {
    std::vector<double> polar_r = {
        250.0, 480.0, 730.0, 1020.0, 1350.0, 1600.0, 1850.0, 2100.0,
        2400.0, 2750.0, 3000.0, 3200.0, 3450.0, 3700.0, 4000.0, 4300.0,
        4600.0, 4850.0, 5000.0, 150.0
    };
    std::vector<double> polar_theta = {
        0.0, 18.0, 36.0, 54.0, 72.0, 90.0, 108.0, 126.0,
        144.0, 162.0, 180.0, 198.0, 216.0, 234.0, 252.0, 270.0,
        288.0, 306.0, 324.0, 342.0
    };
    std::vector<int> polar_intensity(polar_r.size(), 255);

    std::cout << std::fixed << std::setprecision(2);

    while (true) {
        std::cout << "[[";
        for (size_t i = 0; i < polar_r.size(); ++i)
            std::cout << polar_r[i] << (i + 1 < polar_r.size() ? ", " : "");
        std::cout << "], [";
        for (size_t i = 0; i < polar_theta.size(); ++i)
            std::cout << polar_theta[i] << (i + 1 < polar_theta.size() ? ", " : "");
        std::cout << "], [";
        for (size_t i = 0; i < polar_intensity.size(); ++i)
            std::cout << polar_intensity[i] << (i + 1 < polar_intensity.size() ? ", " : "");
        std::cout << "]]" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // ~10 Hz frame rate
    }

    return 0;
}