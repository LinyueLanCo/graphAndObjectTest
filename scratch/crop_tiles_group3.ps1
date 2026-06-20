Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile('assets\tex\maps\tileset.png')

# Columns 12 to 16, Rows 2 to 5
$candidates = @()
for ($r = 2; $r -le 5; $r++) {
    for ($c = 12; $c -le 16; $c++) {
        $candidates += @{Col = $c; Row = $r}
    }
}

foreach ($c in $candidates) {
    $x = $c.Col * 16
    $y = $c.Row * 16
    $bmp = new-object System.Drawing.Bitmap 16, 16
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.DrawImage($img, (new-object System.Drawing.Rectangle 0, 0, 16, 16), (new-object System.Drawing.Rectangle $x, $y, 16, 16), [System.Drawing.GraphicsUnit]::Pixel)
    $g.Dispose()
    $savePath = "scratch\tile_col$($c.Col)_row$($c.Row).png"
    $bmp.Save($savePath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "Saved: $savePath (x=$x, y=$y)"
}

$img.Dispose()
