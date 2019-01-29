Projeto AVT - MicroMachines
Compile Instructions:

Open 'PhongLightTest.sln' in Visual Studio and make sure:
1) You have GLEW and GLUT installed on your machine.
2) The Configuration is 'Release' and not 'Debug'.
3) The architecture is 'x86' and not 'x64'.
4) The Debugging Environment (Project > Properties > Configuration Properties > Debugging) has 'PATH=$(ProjectDir)\contrib\dll;$(Path)'. (if not copy that into the field without the quotes)
