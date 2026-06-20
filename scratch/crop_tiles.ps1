Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile('assets\tex\maps\tileset.png')

# We want to crop a few 16x16 tiles and save them
# Coordinates we want to try (Column, Row)
$candidates = @(
    @{Col = 9; Row = 2},
    @{Col = 9; Row = 3},
    @{Col = 9; Row = 4},
    @{Col = 10; Row = 2},
    @{Col = 10; Row = 3},
    @{Col = 10; Row = 4},
    @{Col = 11; Row = 2},
    @{Col = 11; Row = 3},
    @{Col = 11; Row = 4}
)

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
