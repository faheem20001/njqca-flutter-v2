function Get-AppPath($appExe) {
    $path = $null
    $regPaths = @(
        "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\$appExe",
        "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\App Paths\$appExe"
    )

    foreach ($reg in $regPaths) {
        $val = (Get-ItemProperty -Path $reg -ErrorAction SilentlyContinue).'(Default)'
        if ($val) {
            $path = $val
            break
        }
    }

    return $path
}
$excelPath   = Get-AppPath "excel.exe"
$wordPath    = Get-AppPath "winword.exe"

if ($excelPath -and $wordPath ) {
    Write-Output "  Excel_found: $excelPath" 
    Write-Output "  Word_found: $wordPath" 
    exit(0)
} elseif($excelPath) {
    Write-Output "  Excel_found: $excelPath" 
    exit(0)
} elseif($wordPath) {
    Write-Output "  Word_found: $wordPath" 
    exit(0)
}
$officeKeys = Get-ChildItem "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" | 
    Where-Object { 
        $_.GetValue("DisplayName") -like "*Microsoft Office*" 
    }

if ($officeKeys) {
    foreach ($key in $officeKeys) {
        $displayName = $key.GetValue("DisplayName")
        $version = $key.GetValue("DisplayVersion")
        Write-Output "Word_Found: $displayName - Version: $version"
        Write-Output "Excel_Found: $displayName - Version: $version"
    }
} else {
    Write-Output "  No MSI-based Office found."
}
