# # CONFIG
$ohmPath = "OpenHardwareMonitor"
$heavyLoadPath = "HeavyLoad\HeavyLoad.exe"
$interval = 10
$csvPath = "$env:USERPROFILE\Desktop\HeavyLoad_Report.csv"
$duration = 7

$departmentFile = "C:\WindowsNJQCA\Temp_Data\department.txt"
$department = ""
if (Test-Path $departmentFile) {
    $department = (Get-Content $departmentFile -Raw).Trim()
    # Write-Host "Department found: $department"
} else {
    # Write-Host "department.txt NOT FOUND at: $departmentFile"
}
try {
    # $url = "http://10.160.1.18:8000/api/method/nj_lib.utils.common_utils.fetch_heavyload_duration"
    $url="https://erp.newjaisa.com/api/method/nj_lib.utils.common_utils.fetch_heavyload_duration"
    #  $url="https://amzuat.newjaisa.com/api/method/nj_lib.utils.common_utils.fetch_heavyload_duration"

    if ($department -ne "") {
        $encodedDepartment = [uri]::EscapeDataString($department)
        $url += "?department=$encodedDepartment"

    }
    $response = Invoke-RestMethod $url

    if ($response -and $response.message -and $response.message.heavyload_duration) {
        $duration = [int]$response.message.heavyload_duration
        #  Write-Host "Duration from ERP:" $duration "minutes"
    } else {
        $duration = 7
    }
}
catch {
    # Write-Host "ERP failed using default duration: 7 minutes"
}

# Clear existing file if present
if (Test-Path $csvPath) {
    Clear-Content -Path $csvPath
}

# Start OpenHardwareMonitor and capture the process
$ohmProc = Start-Process -FilePath "$ohmPath\OpenHardwareMonitor.exe" -PassThru
Start-Sleep -Seconds 3  # Allow sensors to load

# Start HeavyLoad and capture the process
$heavyLoadProc = Start-Process -FilePath $heavyLoadPath -ArgumentList `
  '/CPU', '4', '/MEMORY', '100', '/FILE', '100', `
  '/TESTFILEPATH', "$env:TEMP", '/GPU', '/TREESIZE', `
  '/DURATION', $duration, '/AUTOEXIT', '/START' -PassThru

# Initialize sensor header once
$sensorKeys = @()
$firstLoop = $true

# Monitoring Loop - continue while HeavyLoad is running
while (-not $heavyLoadProc.HasExited) {
    $time = Get-Date -Format "HH:mm:ss"

    # Get basic counters
    $cpuUsage = (Get-Counter '\Processor(_Total)\% Processor Time').CounterSamples[0].CookedValue
    $ramFree = (Get-Counter '\Memory\Available MBytes').CounterSamples[0].CookedValue
    $diskRead = (Get-Counter '\PhysicalDisk(_Total)\Disk Read Bytes/sec').CounterSamples[0].CookedValue / 1MB
    $diskWrite = (Get-Counter '\PhysicalDisk(_Total)\Disk Write Bytes/sec').CounterSamples[0].CookedValue / 1MB

    # Collect OpenHardwareMonitor sensors
    $sensorValues = @{ }
    Get-WmiObject -Namespace root\OpenHardwareMonitor -Class Sensor | ForEach-Object {
        $key = "$($_.Hardware) | $($_.SensorType) | $($_.Name)"
        $value = [math]::Round($_.Value, 2)
        $sensorValues[$key] = $value
    }

    if ($firstLoop) {
        $sensorKeys = $sensorValues.Keys
        $header = "Time,CPU %,RAM Free MB,Disk Read MB/s,Disk Write MB/s," + ($sensorKeys -join ",") + ",Heavy_Load_Parameter"
        $header | Out-File -FilePath $csvPath -Encoding UTF8
        $firstLoop = $false
    }

    $sensorData = $sensorKeys | ForEach-Object {
        if ($sensorValues.ContainsKey($_)) {
            $sensorValues[$_]
        } else {
            ""
        }
    }

    # Add empty for Heavy_Load_Parameter column in each row
    "$time,$([math]::Round($cpuUsage,2)),$([math]::Round($ramFree,2)),$([math]::Round($diskRead,2)),$([math]::Round($diskWrite,2))," + ($sensorData -join ",") + "," |
        Out-File -Append -FilePath $csvPath -Encoding UTF8

    Start-Sleep -Seconds $interval
}

Stop-Process -Id $ohmProc.Id -Force

# Analyze data after monitoring
$data = Import-Csv -Path $csvPath

# Define the sensor headers to check
$columnsToCheck = @(
    "*Temperature*CPU Core #1*",
    "*Temperature*CPU Core #2*",
    "*Temperature*CPU Core #3*",
    "*Temperature*CPU Core #4*",
    "*Temperature*CPU Package*"
)

$maxTemps = @()
$failFlag = $false
$totalRows = $data.Count

foreach ($colPattern in $columnsToCheck) {
    $matchedCol = $data[0].PSObject.Properties.Name | Where-Object { $_ -like $colPattern }
    if ($matchedCol) {
        $values = $data[3..($totalRows - 4)] | ForEach-Object { [float]($_.$matchedCol) } | Where-Object { $_ -ne $null }
        if ($values.Count -gt 0) {
            $max = ($values | Measure-Object -Maximum).Maximum
            $maxTemps += $max
            if ($max -gt 95) {
                $failFlag = $true
            }
        }
    }
}

# Final result
$finalMax = ($maxTemps | Measure-Object -Maximum).Maximum
$result = if ($failFlag) { "FAIL" } else { "PASS" }

# Final Output - single line
Write-Host "`nHeavy_Load_Parameter Result: $finalMax - $result"

# $ohmPath = "OpenHardwareMonitor"
# $interval = 1
# $duration = $true 
# $csvPath = "$env:USERPROFILE\Desktop\HardwareMonitor_Report.csv"

# # Clear existing file if present
# if (Test-Path $csvPath) {
#     Clear-Content -Path $csvPath
# }

# Start-Process -FilePath "$ohmPath\OpenHardwareMonitor.exe" -PassThru
# Start-Sleep -Seconds 5  
# $sensorKeys = @()
# $firstLoop = $true
# while ($duration) {
#     $time = Get-Date -Format "HH:mm:ss"

#     $diskRead = (Get-Counter '\PhysicalDisk(_Total)\Disk Read Bytes/sec').CounterSamples[0].CookedValue / 1KB
#     $diskWrite = (Get-Counter '\PhysicalDisk(_Total)\Disk Write Bytes/sec').CounterSamples[0].CookedValue / 1KB

#     $sensorValues = @{ }
#     Get-WmiObject -Namespace root\OpenHardwareMonitor -Class Sensor | ForEach-Object {
#         $key = "$($_.Hardware) | $($_.SensorType) | $($_.Name)"
#         $value = [math]::Round($_.Value, 2)
#         $sensorValues[$key] = $value
#     }

#     if ($firstLoop) {
#         $sensorKeys = $sensorValues.Keys
#         $header = "Time,Disk Read KB/s,Disk Write KB/s," + ($sensorKeys -join ",")
#         $header | Out-File -FilePath $csvPath -Encoding UTF8
#         $firstLoop = $false
#     }

#     $sensorData = $sensorKeys | ForEach-Object {
#         if ($sensorValues.ContainsKey($_)) {
#             $sensorValues[$_]
#         } else {
#             ""
#         }
#     }

#     "$time,$([math]::Round($diskRead,2)),$([math]::Round($diskWrite,2))," + ($sensorData -join ",") |
#         Out-File -Append -FilePath $csvPath -Encoding UTF8

#     Start-Sleep -Seconds $interval
# }

# Stop OpenHardwareMonitor
# if (-not $ohmProc.HasExited) {
#     Stop-Process -Id $ohmProc.Id -Force
# }

# Analyze temperature data
# $data = Import-Csv -Path $csvPath
# $columnsToCheck = @(
#     "*Temperature*CPU Core #1*",
#     "*Temperature*CPU Core #2*",
#     "*Temperature*CPU Core #3*",
#     "*Temperature*CPU Core #4*",
#     "*Temperature*CPU Package*"
# )

# $maxTemps = @()
# $failFlag = $false
# $totalRows = $data.Count

# foreach ($colPattern in $columnsToCheck) {
#     $matchedCol = $data[0].PSObject.Properties.Name | Where-Object { $_ -like $colPattern }
#     if ($matchedCol) {
#         $values = $data[3..($totalRows - 4)] | ForEach-Object { [float]($_.$matchedCol) } | Where-Object { $_ -ne $null }
#         if ($values.Count -gt 0) {
#             $max = ($values | Measure-Object -Maximum).Maximum
#             $maxTemps += $max
#             if ($max -gt 90) {
#                 $failFlag = $true
#             }
#         }
#     }
# }

# # Final Output
# $finalMax = ($maxTemps | Measure-Object -Maximum).Maximum
# $result = if ($failFlag) { "FAIL" } else { "PASS" }

# Write-Host "`nHardware Monitoring Result: Max Temp = $finalMax°C - $result"
