
# $installDate = (Get-CimInstance -ClassName Win32_OperatingSystem).InstallDate
# $currentDate = Get-Date
# $criticalEvents = Get-WinEvent -LogName System -MaxEvents 100 -ErrorAction SilentlyContinue |
#     Where-Object {
#         ($_.ProviderName -like '*Time-Service*') -and
#         ($_.Level -eq 1 -or $_.LevelDisplayName -eq "Error") -and
#         $_.TimeCreated -ge $installDate
#     }

# if ($criticalEvents) {
#     Write-Host "FAILED : (CRITICAL: Found $($criticalEvents.Count) time service errors since OS installation)"
#     exit 0
# }

# $ntpEvents = Get-WinEvent -LogName System -ErrorAction SilentlyContinue |
#     Where-Object { 
#         $_.ProviderName -eq '*Time-Service*' -and
#         $_.TimeCreated -ge $installDate -and
#         $_.TimeCreated -le $currentDate
#     } | Sort-Object TimeCreated
    
#     $consecutiveDnsErrors = 0
#     $previousWas134 = $false
    
#     foreach ($event in $ntpEvents) {
#         if ($event.Id -eq 158) {
#             continue
#         }

#         if ($event.Id -eq 34) {
#             Write-Host "CRITICAL: DNS error detected at $($event.TimeCreated)"
#             exit 0
#         }
#     }
# Write-Host "PASSED: (All time service checks completed successfully)"
# exit 0

 $logFile = "$env:USERPROFILE\Desktop\Battery_Monitor_Log.csv"

 if (!(Test-Path $logFile)) {
     "Timestamp,Laptop Brand,Battery Percentage,Charging Status,Voltage (mV),Charge Rate (mW),Wattage (W),Current (mA),CPU Load (%),Issues" | Out-File -Append -FilePath $logFile
 }

#  # Define min and max charge rate per wattage (Updated as per request)
#  $chargeRateLimits = @{
#      45 = @{ Min = 7000; Max = 50000 }
#      65 = @{ Min = 7000; Max = 60000 }
#      90 = @{ Min = 24000; Max = 80000 }
#      130 = @{ Min = 25000; Max = 100000 }
#      170 = @{ Min = 26000; Max = 120000 }
#  }

#  $adapterLimits = $chargeRateLimits[[int]$adapterWattage]

 # Open the SilverBench website for stress testing
 $min_charging_speed = 6000
 $max_charging_speed = 170000
 Start-Process "https://silver.urih.com/"
 Start-Sleep -Seconds 5

 Write-Host "Monitoring Adapter and Battery Performance... Press Ctrl+C to stop."

 $startTime = Get-Date
 $endTime = $startTime.AddMinutes(1)

 # Detect Laptop Brand
 $computerSystem = Get-WmiObject -Class Win32_ComputerSystem
 $laptopBrand = $computerSystem.Manufacturer
 Write-Host "Detected Laptop Brand: $laptopBrand" -ForegroundColor Cyan

 while ((Get-Date) -lt $endTime) {
     # Get battery details
     $battery = Get-WmiObject -Namespace "root\WMI" -Class "BatteryStatus" | Select-Object -First 1
     $power = Get-WmiObject -Namespace "root\CIMV2" -Class "Win32_Battery" | Select-Object -First 1

     $adapterConnected = $power.BatteryStatus -ne 1
     $batteryPercentage = $power.EstimatedChargeRemaining
     $chargingStatus = $power.BatteryStatus

     $voltage = [double]($battery.Voltage -as [double])
     $chargeRate = [double]($battery.ChargeRate -as [double])

     if ($voltage -gt 0) { $voltage_V = $voltage / 1000.0 } else { $voltage_V = "Unknown" }
     if ($chargeRate -gt 0) { $wattage = $chargeRate / 1000.0 } else { $wattage = 0 }

     if ($voltage_V -is [double] -and $voltage_V -gt 0) {
         $current_mA = [math]::Round(($chargeRate / $voltage_V), 2)
     } else { $current_mA = "Unknown" }

     $cpuLoad = (Get-WmiObject win32_processor | Measure-Object -Property LoadPercentage -Average).Average
     $issues = @()

     if ($batteryPercentage -eq 100 -and $chargeRate -eq 0) {
         $statusText = "Not Charging (Fully Charged)"
     } elseif ($chargingStatus -eq 2) {
         $statusText = "Charging"
     } elseif ($chargingStatus -eq 3) {
         $statusText = "Not Charging"
         $issues += "Adapter connected but not charging"
     } elseif ($chargingStatus -eq 4) {
         $statusText = "Charging Uncontrollable"
         $issues += "Unstable charging detected"
     } elseif ($chargingStatus -eq 1) {
         $statusText = "Not Charging"
         $issues += " Not Charging"
    }

     # Overvoltage Check (Fixed at > 17,000 mV)
     if ($voltage -gt 17000) {
         $issues += "Overvoltage detected"
     }

     if ($statusText -eq "Charging") {
         if ($chargeRate -lt $min_charging_speed) {
             $issues += "Low charging speed for  adapter (Expected min: $min_charging_speed mW)"
         } elseif ($chargeRate -gt $min_charging_speed) {
             $issues += "High charging speed for adapter (Expected max: $max_charging_speed mW)"
         }
     }

     if ($laptopBrand -match "Lenovo") {
        if ($chargeRate -eq 0 -and $statusText -eq "Charging") {
             $issues += "Charging but no power flow (Check battery threshold settings)"
        }
     } else {
        if ($chargeRate -lt 5000 -and $statusText -eq "Charging") {
             $issues += "Low charging speed"
        }
     }

     $logEntry = "$(Get-Date),$laptopBrand,$batteryPercentage,$statusText,$voltage,$chargeRate,$wattage,$current_mA,$cpuLoad," + ($issues -join ", ")
     $logEntry | Out-File -Append -FilePath $logFile

     Write-Host $logEntry -ForegroundColor Green
     Start-Sleep -Seconds 20
 }

 Write-Host "Test completed. Logs saved at $logFile" -ForegroundColor Cyan