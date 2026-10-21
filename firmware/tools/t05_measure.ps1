# t05_measure.ps1 - do thoi gian tu luc cap nguon den goi BLE dau tien (dong "# first_tx ms=N")
# Chay: powershell -NoProfile -ExecutionPolicy Bypass -File t05_measure.ps1
param(
  [string]$Port = "COM8",
  [int]$Trials = 3,
  [int]$OffSeconds = 15
)

function Port-Present { return ([System.IO.Ports.SerialPort]::GetPortNames() -contains $Port) }

function Read-FirstTx($sw) {
  $end = (Get-Date).AddSeconds(10)
  $sp = $null
  $res = $null
  while ((Get-Date) -lt $end -and -not $sp) {
    try {
      $s = New-Object System.IO.Ports.SerialPort $Port, 115200
      $s.DtrEnable = $false
      $s.RtsEnable = $false
      $s.ReadTimeout = 200
      $s.NewLine = "`n"
      $s.Open()
      $sp = $s
    } catch { Start-Sleep -Milliseconds 30 }
  }
  if (-not $sp) { return $null }
  while ((Get-Date) -lt $end -and $null -eq $res) {
    try { $l = $sp.ReadLine() }
    catch [System.TimeoutException] { continue }
    catch { break }
    if ($l -match 'first_tx ms=(\d+)') {
      $res = [pscustomobject]@{ Ms = [int]$Matches[1]; ComToLineSec = [math]::Round($sw.Elapsed.TotalSeconds, 2) }
    }
  }
  $sp.Close()
  return $res
}

$rows = @()
for ($n = 1; $n -le $Trials; $n++) {
  Write-Host ""
  Write-Host "=== Lan $n : hay RUT cap USB-C ra khoi board ==="
  while (Port-Present) { Start-Sleep -Milliseconds 100 }
  Write-Host "Da phat hien rut cap. Cho $OffSeconds giay..."
  Start-Sleep -Seconds $OffSeconds
  Write-Host ">>> BAY GIO moi CAM LAI cap USB-C"
  while (-not (Port-Present)) { Start-Sleep -Milliseconds 5 }
  $sw = [Diagnostics.Stopwatch]::StartNew()
  $r = Read-FirstTx $sw
  if ($null -eq $r) {
    Write-Host "Lan $n : khong doc duoc dong first_tx"
    $rows += [pscustomobject]@{ Lan = $n; FirstTxMs = $null; ComToLineSec = $null }
  } else {
    Write-Host "Lan $n : first_tx ms = $($r.Ms) ; COM xuat hien -> nhan dong first_tx = $($r.ComToLineSec) s"
    $rows += [pscustomobject]@{ Lan = $n; FirstTxMs = $r.Ms; ComToLineSec = $r.ComToLineSec }
  }
}
Write-Host ""
Write-Host "KET QUA:"
$rows | Format-Table -AutoSize
