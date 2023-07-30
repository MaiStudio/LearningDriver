# 1. VS 2008 + WDK

## Development Environment

1. [Visual Studio 2008](http://download.microsoft.com/download/8/1/d/81d3f35e-fa03-485b-953b-ff952e402520/VS2008ProEdition90dayTrialENUX1435622.iso) + Visual Assist X
2. [ddkwizard_setup_v1.3.0-signed.exe](https://sourceforge.net/projects/ddkwizard/files/legacy-releases/ddkwizard_setup_v1.3.0-signed.exe/download)
3. [ddkbuild_v74r60.zip](https://sourceforge.net/projects/ddkbuild/files/legacy-releases/ddkbuild_v74r60.zip/download)
4. [GRMWDK_EN_7600_1.iso](https://www.microsoft.com/download/confirmation.aspx?id=11800)

## Environment Setting

Assuming the installation path is D:\WinDDK\7600.16385.1

1. Install VS2008 and VAX (optional)
2. Install WDK 7.1.0
3. Install ddkwizard, then extract ddkbuild zip and copy the ddkbuild.cmd to D:\WinDDK\7600.16385.1

4. Set Environment Variables add W7BASE with D:\WinDDK\7600.16385.1, and add  D:\WinDDK\7600.16385.1\bin to path
5. In VS2008 Tools—>Options—>Projects and Solutions —> VC++ Directories, add executable D:\WinDDK\7600.16385.1\bin and include path D:\WinDDK\7600.16385.1\inc
6. If need create project, choose DDK Project -> Empty Driver, uncheck "Create PREfast configuration"(PREfast is a C++ analysis tool), finish creation, modify the "SOURCE" file: TARGETTYPE=DRIVER；SOURCES=YourFile.c

## Driver Testing

Open the VS Solution file *.sln

1. Compile W7free for win7/win10 32bit release
2. Compile W7X64free win7/win10 32bit release
3. Compile other projects, such as dll or sample
4. Sign the .sys use 64Signer or others tools (Test Mode Only)



## Win x64 Test Mode

bcdedit.exe /set testsigning on 
bcdedit.exe -set loadoptions DDISABLE_INTEGRITY_CHECKS



# End