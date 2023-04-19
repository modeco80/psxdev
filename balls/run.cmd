@echo off
pushd %PSYQ_ROOT%\pcsxr
	:: On my system, OpenGL is kinda broken
	:: if it is not, then:
	:: - Delete these lines (until `pcsx-redux`)
	:: - Delete the following files from the `pcsxr` folder:
	::		dxil.dll
	::		libgallium_wgl.dll
	::		libglapi.dll
	::		opengl32.dll (or you can just delete this one if you're lazy)
	set GALLIUM_DRIVER=d3d12
	pcsx-redux -run -exe %~dp0%1
popd
