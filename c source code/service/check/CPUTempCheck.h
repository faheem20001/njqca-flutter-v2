
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#pragma once

using namespace std;

class CPUTempCheck : public V2Service {
    IQCService iqcservice;
    Util util;
    PartResultService partresultservice;

public:
    int perform_temp_check(string stockinfoID, string partId, string partconfigid, string item_id, string created_by) {
        cout << "\nStarting Temperature check with CPU load generation..." << endl;

        // Updated PowerShell command for CPU temperature and load
        const string CPU_Temp = R"(powershell -Command "$cpuTemp = (Get-WmiObject -Query 'SELECT * FROM MSAcpi_ThermalZoneTemperature' -Namespace 'root/wmi') | ForEach-Object { ($_.CurrentTemperature - 2732) / 10 }; $cpuLoad = Get-CimInstance -ClassName Win32_Processor | Select-Object -ExpandProperty LoadPercentage; 'CPU Temperature: ' + $cpuTemp + '°C'; 'CPU Load: ' + $cpuLoad + '%'")";

        // Start multiple threads for CPU load generation
        const int num_threads = 4;
        vector<thread> cpu_threads;

        for (int i = 0; i < num_threads; ++i) {
            cpu_threads.emplace_back([]() {
                double a = 1;
                for (int j = 1; j <= 10000; ++j) {
                    a = sqrt(a * j);
                }
            });
        }

        // Initialize loop duration for 1 minute
        auto start_time = chrono::steady_clock::now();
        auto end_time = start_time + chrono::minutes(1);

        while (chrono::steady_clock::now() < end_time) {
            // Execute PowerShell command and fetch CPU stats
            string result = util.executeTerminal(CPU_Temp);

            // Display CPU stats
            if (!result.empty()) {
                cout << "CPU Status:\n" << result << endl;
            } else {
                cerr << "Error: Failed to retrieve CPU temperature and load data." << endl;
            }

            // Wait for 30 seconds before repeating
            this_thread::sleep_for(chrono::seconds(4));
        }

        // Join all threads after the loop
        for (auto& th : cpu_threads) {
            if (th.joinable()) {
                th.join();
            }
        }

        // Prepare data for update
        map<string, string> fieldmap = {
            {"stock_info_id", stockinfoID},
            {"part_id", partId},
            {"part_config_id", partconfigid},
            {"item_id", item_id},
            {"created_by", created_by}
        };
        
        map<string, string> parametrmap;

        // Update part result and sync
        int return_value = partresultservice.updateBulkPartResult(parametrmap, fieldmap);
        iqcservice.ChecksResultSync("CPU_Temp_Check");

        cout << "\nCPU temperature check completed successfully." << endl;
        return return_value;
    }
};
