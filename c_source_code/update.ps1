# Set execution policy for the current session
Set-ExecutionPolicy -ExecutionPolicy Unrestricted -Scope Process -Force

# Define paths
$desktopPath = "$env:USERPROFILE\Desktop"
# $targetFolder = "$desktopPath\WindowsNJQCA"  # Final installation folder
$exeFile = "$desktopPath\WindowsNJQCA.exe"
# $tempFolder = "$targetFolder\temp_extracted"
$zipUrl = "https://localnjpulse.newjaisa.com/files/NJQCA/WindowsNJQCA.exe"  # ZIP download URL

# Ensure the target folder exists
# if (-not (Test-Path -Path $targetFolder)) {
#     New-Item -Path $targetFolder -ItemType Directory -Force
# }

# Download the latest update ZIP
Write-Output "Downloading latest version"
# Invoke-WebRequest -Uri $zipUrl -OutFile $zipFile
wget "$zipUrl" -O "$exeFile"

# Ensure ZIP file exists after download
if (-not (Test-Path -Path $exeFile)) {
    Write-Host "Error: Download failed. ZIP file not found." -ForegroundColor Red
    exit
}

# Extract ZIP contents
# Expand-Archive -Path $zipFile -DestinationPath $tempFolder -Force
Start-Process -FilePath $exeFile -ArgumentList "/SILENT" -Wait

# Delete ZIP file after extraction
Remove-Item -Path $exeFile -Force


# Find the first subfolder inside temp_extracted (it should be WindowsNJQCA2.5)
# $extractedSubfolder = Get-ChildItem -Path $tempFolder -Directory | Select-Object -First 1

# # If a subfolder exists, move its contents to the target folder
# if ($extractedSubfolder) {
#     Get-ChildItem -Path "$($extractedSubfolder.FullName)\*" -Force | Move-Item -Destination $targetFolder -Force -ErrorAction SilentlyContinue
# } else {
#     Write-Output "Error: No subfolder found in extracted files."
#     exit
# }

# # Delete temp extraction folder
# Remove-Item -Path $tempFolder -Recurse -Force
Write-Output "Update applied successfully!"
