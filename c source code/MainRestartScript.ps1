
# $scriptPath = "C:\WindowsNJQCAReliability\restart.exe"
# $logDir = "C:\WindowsNJQCAReliability\Temp_Data"
# $timeLogFile = "$logDir\restart_times.txt"
$scriptPath = "C:\WindowsNJQCA\restart.exe"
$logDir = "C:\WindowsNJQCA\Temp_Data"
$timeLogFile = "$logDir\restart_times.txt"

# Check if the Desktop directory exists
if (-not (Test-Path $timeLogFile)) {
    # Create the file if it doesn't exist
    New-Item -ItemType File -Path $timeLogFile -Force | Out-Null
} else {
    Clear-Content -Path $timeLogFile
    Write-Host "Cleared log file: $timeLogFile"
}


# Path to the log file

# Prompt user for restart confirmation
# $userInput = Read-Host "Press '2' to proceed with the restart, or any other key to cancel"

# if ($userInput -eq '2') {
    # Log the current restart time
    $currentTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Add-Content -Path $timeLogFile -Value "Restarted at: $currentTime"
    # ------------------------------------------------------------
    # CREATE NJQCA ADMIN TASK BEFORE RESTART
    # This task is ON-DEMAND only. It will NOT run at every login.
    # ------------------------------------------------------------
    $taskName = "RunNJQCA_Admin"

    try {
        Unregister-ScheduledTask `
            -TaskName $taskName `
            -Confirm:$false `
            -ErrorAction SilentlyContinue

        $currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
        Add-Content -Path $timeLogFile -Value "Creating RunNJQCA_Admin task for user: $currentUser"

        $action = New-ScheduledTaskAction `
        -Execute "C:\WindowsNJQCA\run_njqca.bat" `
        -WorkingDirectory "C:\WindowsNJQCA"

        $principal = New-ScheduledTaskPrincipal `
            -UserId $currentUser `
            -RunLevel Highest `
            -LogonType Interactive

        $settings = New-ScheduledTaskSettingsSet `
            -AllowStartIfOnBatteries `
            -DontStopIfGoingOnBatteries

        Register-ScheduledTask `
            -TaskName $taskName `
            -Action $action `
            -Principal $principal `
            -Settings $settings `
            -Force `
            -ErrorAction Stop

        Add-Content -Path $timeLogFile -Value "RunNJQCA_Admin task created successfully"
    }
    catch {
        Add-Content -Path $timeLogFile -Value "RunNJQCA_Admin task creation FAILED:"
        Add-Content -Path $timeLogFile -Value $_.Exception.Message
        exit 1
    }
    # Set registry key for executable to run on login
    Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "RunMyScriptOnLogin" -Value "$scriptPath"
    # Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "RunMyScriptOnLogin" -Value "powershell.exe -ExecutionPolicy Bypass -File `"$scriptPath`""
    
    # Initiate system restart
    Restart-Computer -Force
