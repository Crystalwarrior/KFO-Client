param(
    [switch]$prerelease,
    [switch]$forceupdate,
    [switch]$autostart
)

Add-Type -AssemblyName PresentationFramework
# Download latest release from github
$programPath = "Attorney_Online.exe"
$repo = "Crystalwarrior/KFO-Client"
$filenamePattern = "*windows-2019-windows.zip"
$pathExtract = "."
$innerDirectory = $true

'Checking for available updates...'
$success = $false
try {
    if ($prerelease) {
        $releasesUri = "https://api.github.com/repos/$repo/releases"
        $downloadAsset = ((Invoke-RestMethod -Method GET -Uri $releasesUri)[0].assets | Where-Object name -like $filenamePattern )
        $downloadUri = $downloadAsset.browser_download_url
        $downloadUpdatedAt = $downloadAsset.updated_at
    }
    else {
        $releasesUri = "https://api.github.com/repos/$repo/releases/latest"
        $downloadAsset = ((Invoke-RestMethod -Method GET -Uri $releasesUri).assets | Where-Object name -like $filenamePattern )
        $downloadUri = $downloadAsset.browser_download_url
        $downloadUpdatedAt = $downloadAsset.updated_at
    }
    $success = $true
}
catch {
    'Error fetching updates!'
    $autostart = $false
}

if ($success) {
    $updateVersionPath = ".\updateversion.txt"
    $localUpdateAt = ""
    if (Test-Path -Path $updateVersionPath) {
        $localUpdateAt = Get-Content -Path $updateVersionPath -Raw
    }

    if (!$forceupdate -and $localUpdateAt -eq $downloadUpdatedAt) {
        'Already up to date!'
    }
    else {
        'Update found!'
        $answer = [System.Windows.MessageBox]::Show("A new update for $programPath has been detected! This process will close all open instances of the client. Continue?","Confirm Update","YesNo")
        if ($answer -eq "Yes") {
            'Closing all open instances of the client...'
            Get-Process -Name $programPath -errorAction 'silentlycontinue' | ForEach-Object {
                $_.CloseMainWindow() | Out-Null
            }
            Start-Sleep -milliseconds 100
            Stop-Process -name $programPath -force -errorAction 'silentlycontinue'

            'Downloading...'
            $pathZip = Join-Path -Path $([System.IO.Path]::GetTempPath()) -ChildPath $(Split-Path -Path $downloadUri -Leaf)

            Invoke-WebRequest -Uri $downloadUri -Out $pathZip

            Remove-Item -Path $pathExtract -Recurse -Force -ErrorAction SilentlyContinue

            'Downloaded! Extracting...'
            if ($innerDirectory) {
                $tempExtract = Join-Path -Path $([System.IO.Path]::GetTempPath()) -ChildPath $((New-Guid).Guid)
                Expand-Archive -Path $pathZip -DestinationPath $tempExtract -Force
                Move-Item -Path "$tempExtract\*" -Destination $pathExtract -Force
                #Move-Item -Path "$tempExtract\*\*" -Destination $location -Force
                Remove-Item -Path $tempExtract -Force -Recurse -ErrorAction SilentlyContinue
            }
            else {
                Expand-Archive -Path $pathZip -DestinationPath $pathExtract -Force
            }

            Remove-Item $pathZip -Force

            # Store the new update date but only after everything goes right
            $downloadUpdatedAt | Out-File -FilePath $updateVersionPath -NoNewline
            'Success!'
        }
        else {
            'Installation cancelled!'
            $autostart = $false
        }
    }
}

if ($autostart) {
    'Automatically starting the program...'
    Start-Process -FilePath $programPath
}
# Start-Sleep -Seconds 3.0
