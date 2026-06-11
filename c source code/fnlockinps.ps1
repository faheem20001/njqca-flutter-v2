function Test-FnLockState {
    Write-Host "Step 1: Please make sure Fn Lock is ENABLED (Fn key locked)."
    Read-Host "Then press [Enter] to continue..."

    Write-Host "`nNow press the F1 key ONLY (no Fn key)."
    $key1 = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    Write-Host "`nYou pressed: $($key1.VirtualKeyCode) $($key1.Character)"

    Write-Host "`nStep 2: Please DISABLE Fn Lock (unlock Fn key)."
    Read-Host "Then press [Enter] to continue..."

    Write-Host "`nAgain, press the F1 key ONLY."
    $key2 = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    Write-Host "`nYou pressed: $($key2.VirtualKeyCode) $($key2.Character)"

    if ($key1.VirtualKeyCode -eq 112 -and $key2.VirtualKeyCode -eq 173) {
        Write-Host "`nDetected: Fn Lock was ON in first test, OFF in second. Fn Lock toggle works." -ForegroundColor Green
    } elseif ($key1.VirtualKeyCode -eq 112 -and $key2.VirtualKeyCode -eq 112) {
        Write-Host "`n ℹ️ Detected: Fn Lock is likely still ON for both tests." -ForegroundColor Yellow
    } elseif ($key1.VirtualKeyCode -ne 112 -and $key2.VirtualKeyCode -eq 112) {
        Write-Host "`nUnusual pattern. Possibly reversed behavior or Fn key required manually." -ForegroundColor Red
    } else {
        Write-Host "`nNeither F1 detected — Fn key may be intercepting or media key used." -ForegroundColor Red
    }

}

Test-FnLockState
