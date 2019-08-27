# Gbx File Mover #

**Gbx File Mover** is a tool for Microsoft Windows that allows you to move files
from any location to a subfolder of the Documents folder and open them.
It was developed especially for downloaded Challenge.Gbx, Map.Gbx and Replay.Gbx files
of the games [TrackMania Forever](http://trackmaniaforever.com/) and [Maniaplanet](https://www.maniaplanet.com/).

The application requires an installation in which the context menu of .gbx files
is extended by the commands `Move to...` and `Move and open...`.

![Screenshot of GbxMove](http://www.wolfgang-rolke.de/gbxdump/gbxmove.png)

The program uses the shell functions `SHGetSpecialFolderLocation`, `SHGetPathFromIDList`,
`SHGetDesktopFolder`, `SHBrowseForFolder`, `SHFileOperation`, and `ShellExecute`
to display the *Browse For Folder* dialog box, move the file, and open it if desired.

This is a generic C/C++ Win32 desktop project created with Visual C++ 4.0 (Microsoft Developer Studio).
The project files for Visual Studio 2005 and 2017 are simply converted projects to which only a
[Visual Studio Installer project](https://marketplace.visualstudio.com/items?itemName=VisualStudioProductTeam.MicrosoftVisualStudio2017InstallerProjects) for x86 has been added.
