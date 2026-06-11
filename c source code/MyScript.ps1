
$timeLogFile = "C:\WindowsNJQCA\Temp_Data\restart_times.txt"
$currentTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
Add-Content -Path $timeLogFile -Value "Restarted at: $currentTime"


# Read department file written by NJQCA
$departmentFile = "C:\WindowsNJQCA\Temp_Data\department.txt"
$department = ""
if (Test-Path $departmentFile) {
    $department = (Get-Content $departmentFile -Raw).Trim()
     Write-Host "Department: $department"
}

# ERP API call
# $erpUrl = "http://10.160.1.18:8000/api/method/nj_lib.utils.common_utils.fetch_restart_config"
$erpUrl = "https://erp.newjaisa.com/api/method/nj_lib.utils.common_utils.fetch_restart_config"
# $erpUrl = "https://amzuat.newjaisa.com/api/method/nj_lib.utils.common_utils.fetch_restart_config"

# Default fallback
$maxRestartCount = 3

try {
    # Append department to URL if available
    $url = $erpUrl
    if ($department -ne "") {
        $encodedDepartment = [uri]::EscapeDataString($department)
          $url += "?department=$encodedDepartment"
    }

    $response = Invoke-RestMethod -Uri $url -Method Get -TimeoutSec 10
    $rawValue = $response.message.restart_count
    Write-Host "Restart Count from ERP: $rawValue"

    if ($null -ne $rawValue -and $rawValue.ToString().Trim() -ne "") {
        $maxRestartCount = [int]$rawValue
    }
    Write-Host "Final maxRestartCount: $maxRestartCount"

    if ($maxRestartCount -lt 1) {
        $maxRestartCount = 0
    }
}
catch {
    $maxRestartCount = 3
}



if (Test-Path $timeLogFile) {
    $restartCount = (Get-Content -Path $timeLogFile | Where-Object { $_ -match "^Restarted at:" }).Count
    #Write-Host "Counting restarts"
} else {
    $restartCount = 0
}




if ($restartCount -le $maxRestartCount) {
         
    # Write-Host "restarting:"
    Restart-Computer -Force

} else {
    # Remove registry entry to stop running the script after login
    Remove-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "RunMyScriptOnLogin"
    
    # Log that the registry entry was removed
    $currentTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path $timeLogFile -Value "Removed registry at: $currentTime"
    
    # Calculate the average restart interval
    $times = Get-Content -Path $timeLogFile | ForEach-Object {
        if ($_ -match "Restarted at: (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})") {
            [datetime]$matches[1]
        }
    }

    # Calculate intervals between consecutive restarts
    $intervals = @()
    for ($i = 1; $i -lt $times.Count; $i++) {
        $intervals += ($times[$i] - $times[$i - 1]).TotalSeconds
    }

    # Calculate the average interval in seconds
    if ($intervals.Count -gt 0) {
        $averageIntervalSeconds = ($intervals | Measure-Object -Sum).Sum / $intervals.Count

       
        $averageTime = New-TimeSpan -Seconds $averageIntervalSeconds

        Write-Host "Average Restart Interval: $($averageTime.Hours):$($averageTime.Minutes):$($averageTime.Seconds)"
        Add-Content -Path $timeLogFile -Value "Average Restart Interval: $($averageTime.Hours):$($averageTime.Minutes):$($averageTime.Seconds)"
    } else {
        Write-Host "Not enough data to calculate average restart interval."
    }



#     if (Test-Path $timeLogFile) {
#     # Read the restart times from the file
#     $restartTimes = Get-Content -Path $timeLogFile | Where-Object { $_ -match "Restarted at: (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})" }

#     if ($restartTimes.Count -gt 0) {
#         # Get the first restart time
#         $firstRestartTime = [datetime]($restartTimes[0] -replace 'Restarted at: ', '')
#         Write-Host "Considering events after: $($firstRestartTime.ToString('yyyy-MM-dd HH:mm:ss'))"

#         # Get logs from the System event log for Time-Service after the restart time
#         $logs = Get-WinEvent -LogName System |
#             Where-Object { $_.TimeCreated -gt $firstRestartTime -and $_.ProviderName -eq '*Time-Service*' }

#         # Count warnings and errors
#         $warningCount = ($logs | Where-Object { $_.LevelDisplayName -eq 'Warning' }).Count
#         $errorCount = ($logs | Where-Object { $_.LevelDisplayName -eq 'Error' }).Count

#         # Output the counts
#         Write-Host "Total count of warnings: $warningCount"
#         Write-Host "Total count of errors: $errorCount"
#     } else {
#         Write-Host "No valid restart times found in the file."
#     }
# } else {
#     Write-Host "The specified time log file does not exist."
# }


    
    # Calculate and display the time difference between consecutive restarts
    $previousRestartTime = $times[0]
    for ($i = 1; $i -lt $times.Count; $i++) {
        $timeDifference = $times[$i] - $previousRestartTime
        $previousRestartTime = $times[$i]
        
        Write-Host "Time difference between restart $($i): $($timeDifference.Hours) hours, $($timeDifference.Minutes) minutes, $($timeDifference.Seconds) seconds"
        Add-Content -Path $timeLogFile -Value "restart $($i):$($timeDifference.Hours):$($timeDifference.Minutes):$($timeDifference.Seconds)"
    }

#Start-Process -FilePath "performance.exe"

#  if (Test-Path $timeLogFile) {
#   Remove-Item -Path $timeLogFile
#   # Write-Host "Log file deleted"
#  }

# $exePath = "$env:USERPROFILE\Desktop\WindowsNJQCA\performance.exe"
# Set-Location -Path "$env:USERPROFILE\Desktop\WindowsNJQCA"
# & "$exePath"
#  Start-Process -FilePath "$env:USERPROFILE\Desktop\RunNJQCA.bat"/
try {
    Add-Content -Path $timeLogFile -Value "Starting RunNJQCA_Admin task at: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"

    Start-ScheduledTask `
        -TaskName "RunNJQCA_Admin" `
        -ErrorAction Stop

    Add-Content -Path $timeLogFile -Value "RunNJQCA_Admin task started successfully"
}
catch {
    Add-Content -Path $timeLogFile -Value "RunNJQCA_Admin task start FAILED:"
    Add-Content -Path $timeLogFile -Value $_.Exception.Message
}

}

# Write-Host "Press any key to exit..."
# $x = $host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")