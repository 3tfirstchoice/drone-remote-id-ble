# capture_log.ps1 - doc log Serial cua rid_tx truc tiep tu cong COM, luu ra CSV
# Vi du: powershell -NoProfile -ExecutionPolicy Bypass -File capture_log.ps1 -Cmd 1 -Count 60 -Out T02.csv
param(
  [string]$Port = "COM8",
  [string]$Cmd = "1",
  [int]$Count = 60,
  [string]$Out = "capture.csv",
  [int]$TimeoutSec = 0
)
if ($TimeoutSec -le 0) { $TimeoutSec = $Count + 30 }

$sp = New-Object System.IO.Ports.SerialPort $Port, 115200, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
$sp.DtrEnable = $false
$sp.RtsEnable = $false
$sp.ReadTimeout = 500
$sp.NewLine = "`n"
try { $sp.Open() } catch {
  Write-Host "Khong mo duoc $Port : $_"
  Write-Host "Hay dong Serial Monitor cua Arduino IDE roi chay lai."
  exit 1
}
Start-Sleep -Milliseconds 300
$sp.DiscardInBuffer()
$sp.Write($Cmd + "h")          # lenh chon kich ban + 'h' de in header

$started = $false
$h = $null
$rows = New-Object System.Collections.ArrayList
$deadline = (Get-Date).AddSeconds($TimeoutSec)
while ($rows.Count -lt $Count -and (Get-Date) -lt $deadline) {
  try { $line = $sp.ReadLine().Trim() }
  catch [System.TimeoutException] { continue }
  catch { Write-Host "Loi doc cong: $_"; break }
  if ($line.StartsWith("# scenario=")) { $started = $true; Write-Host $line; continue }
  if (-not $started) { continue }
  if ($line.StartsWith("H,")) { $h = $line; continue }
  if ($line -match '^D,\d+,') {
    [void]$rows.Add($line)
    if ($rows.Count % 10 -eq 0) { Write-Host ("  ... {0}/{1}" -f $rows.Count, $Count) }
  }
}
$sp.Close()

if ($rows.Count -lt 2) { Write-Host "Khong du du lieu (chi co $($rows.Count) dong D)."; exit 1 }
if ($h) { $all = @($h) + $rows } else { $all = $rows }
$all | Set-Content -Encoding ascii $Out

# cot: 0=D 1=ms 2=utc 3=scn 4=idx 5=lat 6=lon 7=alt_msl 8=alt_agl 9=speed 10=heading 11=sats 12=tx 13=err 14=heap
$f = $rows | ForEach-Object { ,$_.Split(',') }
$ms = $f | ForEach-Object { [int64]$_[1] }
$gap = for ($i = 1; $i -lt $ms.Count; $i++) { $ms[$i] - $ms[$i-1] }
$g = $gap | Measure-Object -Minimum -Maximum -Average
$hp = $f | ForEach-Object { [int64]$_[14] } | Measure-Object -Minimum -Maximum
"So dong D      : " + $rows.Count
"scn            : " + $f[0][3]
"idx dau / cuoi : " + $f[0][4] + " / " + $f[-1][4]
"ms gap min/max/avg : {0} / {1} / {2}" -f $g.Minimum, $g.Maximum, $g.Average
"tx dau / cuoi  : " + $f[0][12] + " / " + $f[-1][12]
"err cuoi       : " + $f[-1][13]
"heap min / max : {0} / {1}" -f $hp.Minimum, $hp.Maximum
"Da luu         : $Out"
