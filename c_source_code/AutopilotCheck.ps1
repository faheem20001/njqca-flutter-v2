# ===== AUTOPILOT QC CHECK WITH COLOR STATUS =====

$RegPath = "HKLM:\SOFTWARE\Microsoft\Provisioning\Diagnostics\Autopilot"
$Desktop = [System.IO.Path]::Combine(
    [Environment]::GetFolderPath("Desktop"),
    "Autopilot_Check_Result.txt"
)

Start-Sleep -Seconds 25   # Wait for Microsoft check to complete

$IsAutopilot = $false
$Tenant = ""

if (Test-Path $RegPath) {
    $AP = Get-ItemProperty -Path $RegPath -ErrorAction SilentlyContinue
    if ($AP.CloudAssignedTenantId) {
        $IsAutopilot = $true
        $Tenant = $AP.CloudAssignedDomain
    }
}

# Load Windows Forms
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Create Popup Form
$form = New-Object System.Windows.Forms.Form
$form.Width = 520
$form.Height = 260
$form.StartPosition = "CenterScreen"
$form.TopMost = $true
$form.Font = New-Object System.Drawing.Font("Segoe UI",12,[System.Drawing.FontStyle]::Bold)

$label = New-Object System.Windows.Forms.Label
$label.Width = 480
$label.Height = 160
$label.Left = 20
$label.Top = 30
$label.TextAlign = "MiddleCenter"

if ($IsAutopilot) {
    # 🔴 FAIL DESIGN
    $form.BackColor = "Red"
    $label.ForeColor = "White"

    $label.Text = @"
AUTOPILOT REGISTERED

Device belongs to:
$Tenant

REJECT THIS SYSTEM
DO NOT SEND TO CUSTOMER
"@

    "FAIL - Autopilot registered ($Tenant)" | Out-File $Desktop -Force
    $ResultCode = 1
}
else {
    # 🟢 PASS DESIGN
    $form.BackColor = "Green"
    $label.ForeColor = "White"

    $label.Text = @"
AUTOPILOT CHECK PASSED

System is CLEAN.

OK TO MOVE TO FQC / PACKING
"@

    "PASS - Device is clean" | Out-File $Desktop -Force
    $ResultCode = 0
}

$form.Controls.Add($label)

# Auto close after 10 seconds
$timer = New-Object System.Windows.Forms.Timer
$timer.Interval = 10000
$timer.Add_Tick({ $form.Close() })
$timer.Start()

$form.ShowDialog()

exit $ResultCode