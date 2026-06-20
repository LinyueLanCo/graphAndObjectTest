Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile('assets\tex\maps\tileset.png')
Write-Host "Width: $($img.Width)"
Write-Host "Height: $($img.Height)"
$img.Dispose()
