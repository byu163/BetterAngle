# PowerShell script to auto-increment version and update all files
# This script is designed to run in GitHub Actions on push to main

param(
    [string]$CommitRange = ""
)

# Function to parse semantic version
function Parse-Version {
    param([string]$VersionString)
    $parts = $VersionString.Trim() -split '\.'
    return @{
        Major = [int]$parts[0]
        Minor = [int]$parts[1]
        Patch = [int]$parts[2]
        Full = $VersionString.Trim()
    }
}

# Function to increment patch version
function Increment-Version {
    param([hashtable]$Version)
    $newPatch = $Version.Patch + 1
    return @{
        Major = $Version.Major
        Minor = $Version.Minor
        Patch = $newPatch
        Full = "$($Version.Major).$($Version.Minor).$newPatch"
    }
}

# Function to update VERSION file
function Update-VersionFile {
    param([hashtable]$NewVersion)
    $newVersionStr = $NewVersion.Full
    Write-Output "Updating VERSION file to: $newVersionStr"
    Set-Content -Path "VERSION" -Value $newVersionStr -NoNewline
}

# Function to update CMakeLists.txt
function Update-CMakeLists {
    param([hashtable]$NewVersion)
    $content = Get-Content -Path "CMakeLists.txt" -Raw
    $pattern = 'project\(BetterAngle VERSION \d+\.\d+\.\d+'
    $replacement = "project(BetterAngle VERSION $($NewVersion.Full)"
    $newContent = $content -replace $pattern, $replacement
    Set-Content -Path "CMakeLists.txt" -Value $newContent -NoNewline
    Write-Output "Updated CMakeLists.txt to version $($NewVersion.Full)"
}

# Function to update State.h
function Update-StateHeader {
    param([hashtable]$NewVersion)
    $content = Get-Content -Path "include/shared/State.h" -Raw
    # Update V_MAJ, V_MIN, V_PAT defines
    $content = $content -replace '#define V_MAJ \d+', "#define V_MAJ $($NewVersion.Major)"
    $content = $content -replace '#define V_MIN \d+', "#define V_MIN $($NewVersion.Minor)"
    $content = $content -replace '#define V_PAT \d+', "#define V_PAT $($NewVersion.Patch)"
    Set-Content -Path "include/shared/State.h" -Value $content -NoNewline
    Write-Output "Updated include/shared/State.h to version $($NewVersion.Full)"
}

# Function to generate release notes from git commits
function Generate-ReleaseNotes {
    param([string]$CommitRange, [hashtable]$NewVersion)
    
    if ($CommitRange -eq "") {
        Write-Output "No commit range provided, using default release notes"
        return "### BetterAngle Pro v$($NewVersion.Full)`n- Automated build release."
    }
    
    Write-Output "Generating release notes from commit range: $CommitRange"
    $commits = git log --oneline --no-merges $CommitRange
    
    $notes = @()
    $notes += "### BetterAngle Pro v$($NewVersion.Full)"
    
    foreach ($commit in $commits) {
        if ($commit -match '^\w+ (.+)$') {
            $notes += "- $($matches[1])"
        }
    }
    
    if ($notes.Count -eq 1) {
        $notes += "- Automated build release."
    }
    
    return ($notes -join "`n")
}

# Function to update RELEASE_NOTES.md file
function Update-ReleaseNotesFile {
    param([string]$NewReleaseNotes, [hashtable]$NewVersion)
    
    Write-Output "Updating RELEASE_NOTES.md with new release v$($NewVersion.Full)"
    
    # Read existing content
    $existingContent = ""
    if (Test-Path "RELEASE_NOTES.md") {
        $existingContent = Get-Content -Path "RELEASE_NOTES.md" -Raw
    }
    
    # Prepend new release notes
    $newContent = $NewReleaseNotes + "`n`n" + $existingContent
    
    # Write back to file
    Set-Content -Path "RELEASE_NOTES.md" -Value $newContent -Encoding utf8 -NoNewline
    
    Write-Output "Updated RELEASE_NOTES.md successfully"
}

# Main execution
try {
    Write-Output "Starting version bump process..."
    
    # Read current version
    $currentVersionStr = Get-Content -Path "VERSION" -Raw
    Write-Output "Current version: $currentVersionStr"
    
    $currentVersion = Parse-Version -VersionString $currentVersionStr
    $newVersion = Increment-Version -Version $currentVersion
    
    Write-Output "New version will be: $($newVersion.Full)"
    
    # Update files
    Update-VersionFile -NewVersion $newVersion
    Update-CMakeLists -NewVersion $newVersion
    Update-StateHeader -NewVersion $newVersion
    
    # Generate release notes
    $releaseNotes = Generate-ReleaseNotes -CommitRange $CommitRange -NewVersion $newVersion
    
    # Update RELEASE_NOTES.md file
    Update-ReleaseNotesFile -NewReleaseNotes $releaseNotes -NewVersion $newVersion
    
    # Write release notes to temporary file for GitHub Actions
    $releaseNotes | Out-File -FilePath "NEW_RELEASE_NOTES.md" -Encoding utf8
    
    # Output new version for GitHub Actions
    if ($env:GITHUB_OUTPUT) {
        "new_version=$($newVersion.Full)" | Add-Content -Path $env:GITHUB_OUTPUT
        "release_notes_file=NEW_RELEASE_NOTES.md" | Add-Content -Path $env:GITHUB_OUTPUT
    }
    
    Write-Output "Version bump completed successfully!"
    
} catch {
    Write-Error "Error during version bump: $_"
    exit 1
}