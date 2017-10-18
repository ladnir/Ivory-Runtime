
# Update this if needed
$MSBuildLocations = 
    'C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MSVC\14.11.25503\bin\HostX64\x64\cl.exe',
    'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.11.25503\bin\HostX64\x64\cl.exe'

$MSBuild = ''
$git = 'git'

$VSVersion = 2017

foreach($path in $MSBuildLocations) {
    if(Test-Path $path) {
        $MSBuild = $path
    }
}

if(!(Test-Path $MSBuild)) {
    $MSBuild = 'C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe'
    $VSVersion = 2015
    
    if(!(Test-Path $MSBuild)){
        Write-Host "Could not find MSBuild as"
        Write-Host "     $MSBuild"
        Write-Host ""
        Write-Host "Please update its location in the script"
        exit
    }
}

$uri = 'http://www.shoup.net/ntl/WinNTL-10_5_0.zip'
$zipFile = "$PWD/WinNTL-10_5_0.zip"
$startDir = $PWD
 
$folder =  "$PWD\NTL"
if(!(Test-Path $folder))
{

     
    if(!(Test-Path $zipFile))
    {
        Write-Host 'downloading ' $uri ' to ' $zipFile
        #Write-Host 'It is 8.5 MB '

        Invoke-WebRequest -Uri $uri -OutFile $zipFile -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::internetexplorer

        Write-Host 'Download Complete'
    }

     
    Write-Host 'Extracting $zipFile to ' $PWD '. This will take a bit... So be patient.'


    Add-Type -assembly “system.io.compression.filesystem”
    [io.compression.zipfile]::ExtractToDirectory($zipFile, $PWD)

    mv "$PWD/WinNTL-10_5_0" $folder

    rm $zipFile
}
else
{
    Write-Host "$folder already exists. Skipping dowload and extract."
}
#echo ""
#echo ""
#echo "      you're on your own to make a solution and build NTL. It can be done with VSC but NTL suggestions linux emulator compiler." 
#echo ""
#echo ""

cp NTL_patch/* $folder -Force -recurse

cd $folder

cp "./visualStudio$VSVersion/*" ./


& $MSBuild ntl.sln  /p:Configuration=Release /p:Platform=x64
& $MSBuild ntl.sln  /p:Configuration=Debug /p:Platform=x64

cd $startDir

