cd $PSScriptRoot

$drive_detected = @(Get-CimInstance -ClassName Win32_LogicalDisk | ? {$_.VolumeName -match "CRP" -and $_.Size -eq 65536});

if($drive_detected.Length -eq 1){
  ls $drive_detected.DeviceID | Remove-Item;
  Copy-Item .\BUILD\LPC11U35_501\GCC_ARM\*.bin -Destination $drive_detected.DeviceID

  Write-Host "Finished Writing" 
}else{
  Write-Host $drive_detected
  Write-Host "Writing Failure"
}

